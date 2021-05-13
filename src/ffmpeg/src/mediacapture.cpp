
#include "ff/mediacapture.h"
#include <assert.h>
#ifdef HAVE_FFMPEG

#include "base/filesystem.h"
#include "ff/devicemanager.h"
#include "base/logger.h"
#include "base/platform.h"
#include <chrono>
#include <thread>


extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}


using std::endl;


namespace base {
    namespace ff {

        MediaCapture::MediaCapture()
        : _formatCtx(nullptr)
        , _video(nullptr)
        , _audio(nullptr)
        , _stopping(false)
        , _looping(false)
        , _realtime(false)
        , _ratelimit(false) {
            initializeFFmpeg();
        }

        MediaCapture::~MediaCapture() {
            stop();
            close();
            uninitializeFFmpeg();
        }

        void MediaCapture::close() {
            LTrace("Closing")


            if (_video) {
                delete _video;
                _video = nullptr;
            }

            if (_audio) {
                delete _audio;
                _audio = nullptr;
            }

            if (_formatCtx) {
                avformat_close_input(&_formatCtx);
                _formatCtx = nullptr;
            }

            LTrace("Closing: OK")
        }

        void MediaCapture::openFile(const std::string& file) {
            LTrace("Opening file: ", file)
            openStream(file, nullptr, nullptr);
        }
        
        void MediaCapture::openDir(const std::string& dr) {
            LTrace("Opening openDir: ", dr)
            base::fs::readdir_filter(dr, files, "mp3");
            dir = dr;
            openStream( dir + "/" +files[fileNo++], nullptr, nullptr);
           
        }
        

        void MediaCapture::openStream(const std::string& filename, AVInputFormat* inputFormat, AVDictionary** formatParams) {
            LTrace("Opening stream: ", filename)

            if (_formatCtx)
                throw std::runtime_error("Capture has already been initialized");

            if (avformat_open_input(&_formatCtx, filename.c_str(), inputFormat, formatParams) < 0)
                throw std::runtime_error("Cannot open the media source: " + filename);

            // _formatCtx->max_analyze_duration = 0;
            if (avformat_find_stream_info(_formatCtx, nullptr) < 0)
                throw std::runtime_error("Cannot find stream information: " + filename);

            av_dump_format(_formatCtx, 0, filename.c_str(), 0);

            for (unsigned i = 0; i < _formatCtx->nb_streams; i++) {
                auto stream = _formatCtx->streams[i];
                auto codec = stream->codec;
                if (!_video && codec->codec_type == AVMEDIA_TYPE_VIDEO) {
                    _video = new VideoDecoder(stream);
                    // _video->emitter.attach(packetSlot(this, &MediaCapture::emit));
                    _video->create();
                    _video->open();
                } else if (!_audio && codec->codec_type == AVMEDIA_TYPE_AUDIO) {
                    _audio = new AudioDecoder(stream);
                    //            _audio->emitter.attach(packetSlot(this, &MediaCapture::emit));
                    _audio->create();
                    _audio->open();
                }
            }

            if (!_video && !_audio)
                throw std::runtime_error("Cannot find a valid media stream: " + filename);
        }

        void MediaCapture::start() {
            LInfo("MediaCapture Starting")

            std::lock_guard<std::mutex> guard(_mutex);
           // assert(_video || _audio);

            if (!running()) {
               LInfo("Initializing thread")
                _stopping = false;
                Thread::start();
            }
        }

        void MediaCapture::stop() {
            LTrace("Stopping")

            std::lock_guard<std::mutex> guard(_mutex);

            _stopping = true;
            if (running()) {
                LTrace("Terminating thread")
                Thread::stop();
                Thread::join();
            }
        }

        void MediaCapture::emit(IPacket& packet) {

            //if(!isStable)
            // return;
            
            for (auto f : cbProcessAudio)
            { 
                if (f && packet.className() == std::string("PlanarAudioPacket")) {
                    LTrace("Emit: Audio Size  ", packet.size(), ", ", packet.className())
                  audioBuffSize = f(packet);
                }
            }
            
            for (auto f : cbProcessVideo)
            { 
                if (f && packet.className() == std::string("PlanarVideoPacket")) {
                    LTrace("Emit: Video Size  ", packet.size(), ", ", packet.className())
                    int y = f(packet);
                }
            }

            /// This method ensures compatibility with the given
            /// packet type. Return false to reject the packet.



        }

        void MediaCapture::run() {
           
            if(_stopping)
                return;
                      
            LInfo("Running")
              
            do {
              
                
                try {
                     
                    int res;
                    AVPacket ipacket;
                    av_init_packet(&ipacket);

                    // Looping variables
                    int64_t videoPtsOffset = 0;
                    int64_t audioPtsOffset = 0;

                    // Realtime variables
                    int64_t startTime = time::hrtime();

                    // Rate limiting variables
                    int64_t lastTimestamp = time::hrtime();
                    int64_t frameInterval = _video ? fpsToInterval(int(_video->iparams.fps)) : 0;

                    // Reset the stream back to the beginning when looping is enabled
                    if (_looping) {
                        LTrace("Looping")
                        for (unsigned i = 0; i < _formatCtx->nb_streams; i++) {
                            if (avformat_seek_file(_formatCtx, i, 0, 0, 0, AVSEEK_FLAG_FRAME) < 0) {
                                throw std::runtime_error("Cannot reset media stream");
                            }
                        }
                    }

                    // Read input packets until complete
                    while ((res = av_read_frame(_formatCtx, &ipacket)) >= 0) {
                        //            STrace << "Read frame: "
                        //                   << "pts=" << ipacket.pts << ", "
                        //                   << "dts=" << ipacket.dts << endl;

                        if (_stopping)
                            break;

                        if (_video && ipacket.stream_index == _video->stream->index) {

                            // Realtime PTS calculation in microseconds
                            if (_realtime) {
                                ipacket.pts = time::hrtime() - startTime;
                            } else if (_looping) {
                                // Set the PTS offset when looping
                                if (ipacket.pts == 0 && _video->pts > 0)
                                    videoPtsOffset = _video->pts;
                                ipacket.pts += videoPtsOffset;
                            }

                            // Decode and emit
                            if (_video->decode(ipacket, this)) {

                                STrace << "Decoded video: "
                                        << "time=" << _video->time << ", "
                                        << "pts=" << _video->pts << endl;
                            }

                            // Pause the input stream in rate limited mode if the
                            // decoder is working too fast
                            if (_ratelimit) {
                                auto nsdelay = frameInterval - (time::hrtime() - lastTimestamp);
                                LDebug("Sleep delay: ", nsdelay, ", ", (time::hrtime() - lastTimestamp), ", ", frameInterval)
                                std::this_thread::sleep_for(std::chrono::nanoseconds(nsdelay));
                                // base::sleep( nsdelay/1000);
                                lastTimestamp = time::hrtime();
                            }
                        } else if (_audio && ipacket.stream_index == _audio->stream->index) {

                            // Set the PTS offset when looping
                            if (_looping) {
                                if (ipacket.pts == 0 && _audio->pts > 0)
                                    videoPtsOffset = _audio->pts;
                                ipacket.pts += audioPtsOffset;
                            }

                            // Decode and emit
                            if (_audio->decode(ipacket, this)) {
                                STrace << "Decoded Audio: "
                                        << "time=" << _audio->time << ", "
                                        << "pts=" << _audio->pts << endl;
                            }
                            
                            if(audioBuffSize > 50*1920)
                                std::this_thread::sleep_for(std::chrono::milliseconds(25));
                        }

                        av_packet_unref(&ipacket);
                    }

                    // Flush remaining packets
                    if (!_stopping && res < 0) {
                        if (_video)
                            _video->flush(this);
                        if (_audio)
                            _audio->flush(this);
                    }

                    // End of file or error
                    LTrace("Decoder EOF: ", res)
                } catch (std::exception& exc) {
                    _error = exc.what();
                    LError("Decoder Error: ", _error)
                } catch (...) {
                    _error = "Unknown Error";
                    LError("Unknown Error")
                }

                LInfo( "looping back");

              close();


              openStream( dir + "/" +files[fileNo++], nullptr, nullptr);
               if(fileNo == files.size() )
                   fileNo = 0;

              if(!_looping && !files.size())
                  break;
              
              if (this->audio()) {
                    this->audio()->oparams.sampleFmt = "s16";
                    this->audio()->oparams.sampleRate = 48000;
                    this->audio()->oparams.channels = 2;
                    this->audio()->recreateResampler();
                    // _videoCapture->audio()->resampler->maxNumSamples = 480;
                    // _videoCapture->audio()->resampler->variableOutput = false;
                }

                // Convert to yuv420p for WebRTC compatability
                if (this->video()) {
                    this->video()->oparams.pixelFmt = "yuv420p"; // nv12
                    // _videoCapture->video()->oparams.width = capture_format.width;
                    // _videoCapture->video()->oparams.height = capture_format.height;
                }

            }while(!_stopping) ;

        }

        void MediaCapture::getEncoderFormat(Format& format) {
            format.name = "Capture";
            getEncoderVideoCodec(format.video);
            getEncoderAudioCodec(format.audio);
        }

        void MediaCapture::getEncoderAudioCodec(AudioCodec& params) {
            std::lock_guard<std::mutex> guard(_mutex);
            if (_audio) {
                assert(_audio->oparams.channels);
                assert(_audio->oparams.sampleRate);
                assert(!_audio->oparams.sampleFmt.empty());
                params = _audio->oparams;
            }
        }

        void MediaCapture::getEncoderVideoCodec(VideoCodec& params) {
            std::lock_guard<std::mutex> guard(_mutex);
            if (_video) {
                assert(_video->oparams.width);
                assert(_video->oparams.height);
                assert(!_video->oparams.pixelFmt.empty());
                params = _video->oparams;
            }
        }

        AVFormatContext* MediaCapture::formatCtx() const {
            std::lock_guard<std::mutex> guard(_mutex);
            return _formatCtx;
        }

        VideoDecoder* MediaCapture::video() const {
            std::lock_guard<std::mutex> guard(_mutex);
            return _video;
        }

        AudioDecoder* MediaCapture::audio() const {
            std::lock_guard<std::mutex> guard(_mutex);
            return _audio;
        }

        void MediaCapture::setLoopInput(bool flag) {
            // _thread.setRepeating(flag);
            _looping = flag;
        }

        void MediaCapture::setLimitFramerate(bool flag) {
            _ratelimit = flag;
        }

        void MediaCapture::setRealtimePTS(bool flag) {
            _realtime = flag;
        }

        bool MediaCapture::stopping() const {
            std::lock_guard<std::mutex> guard(_mutex);
            return _stopping;
        }

        std::string MediaCapture::error() const {
            std::lock_guard<std::mutex> guard(_mutex);
            return _error;
        }


    } // namespace ff
} // namespace base


#endif


/// @\}
