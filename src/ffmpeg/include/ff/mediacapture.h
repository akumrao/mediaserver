#ifndef BASE_AV_MediaCapture_H
#define BASE_AV_MediaCapture_H


#include "base/base.h"

#ifdef HAVE_FFMPEG

#include "ff/audiodecoder.h"
#include "ff/ffmpeg.h"
#include "ff/icapture.h"
#include "ff/packet.h"
#include "ff/videodecoder.h"
#include "base/interface.h"
//#include "base/packetsignal.h"


namespace base {
namespace ff {


/// This class implements a cross platform audio, video, screen and
/// video file capturer.
class AV_API MediaCapture : public ICapture, public Thread
{
public:
    typedef std::shared_ptr<MediaCapture> Ptr;

    MediaCapture();
    virtual ~MediaCapture();

    virtual void openFile(const std::string& file);
    virtual void openDir(const std::string& dir);
    
    std::vector<std::string> files;
    std::string dir;
    
     int fileNo{0};
    
    // #ifdef HAVE_FFMPEG_AVDEVICE
    // virtual void openCamera(const std::string& device, int width = -1, int height = -1, double framerate = -1);
    // virtual void openMicrophone(const std::string& device, int channels = -1, int sampleRate = -1);
    // #endif
    virtual void close();

    virtual void start() override;
    virtual void stop() override;

    virtual void run() override;

    virtual void getEncoderFormat(Format& format);
    virtual void getEncoderAudioCodec(AudioCodec& params);
    virtual void getEncoderVideoCodec(VideoCodec& params);

    /// Continuously loop the input file when set.
    void setLoopInput(bool flag);

    /// Limit playback to video FPS.
    void setLimitFramerate(bool flag);

    /// Set to use realtime PTS calculation.
    /// This is preferred when sing live captures as FFmpeg provided values are
    /// not always reliable.
    void setRealtimePTS(bool flag);

    AVFormatContext* formatCtx() const;
    VideoDecoder* video() const;
    AudioDecoder* audio() const;
    bool stopping() const;
    std::string error() const;

    /// Signals that the capture thread is closing.
    /// Careful, this signal is emitted from inside the tread contect.
//    NullSignal Closing;

        void emit(IPacket& packet);
        
       // std::function<void(IPacket&) > cbProcessAudio;
       // std::function<void(IPacket&) > cbProcessVideo;  
        
        using function_type = std::function<int(IPacket&) > ;

  
        // here we will store all binded functions 
        std::vector<function_type> cbProcessVideo;
        std::vector<function_type> cbProcessAudio;
        
        int audioBuffSize;
  
        
protected:
    virtual void openStream(const std::string& filename, AVInputFormat* inputFormat, AVDictionary** formatParams);

     


protected:
    mutable std::mutex _mutex;
    //Thread _thread;
    AVFormatContext* _formatCtx;
    VideoDecoder* _video;
    AudioDecoder* _audio;
    std::string _error;
    bool _stopping;
    bool _looping;
    bool _realtime;
    bool _ratelimit;
};


} // namespace ff
} // namespace base


#endif
#endif // BASE_AV_MediaCapture_H


/// @\}
