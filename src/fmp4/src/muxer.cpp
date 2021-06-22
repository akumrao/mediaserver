
#include "muxer.h"
#include "tools.h"
#include "base/logger.h"

 /*

  H.264 comes in a variety of stream formats. One variation is called "Annex B".

(AUD)(SPS)(PPS)(I-Slice)(PPS)(P-Slice)(PPS)(P-Slice) ... (AUD)(SPS)(PPS)(I-Slice).
  
 ALL the NAL Unit start with 001

 
0x67=  11 00111 =  type7   Sequence parameter set ( I-frame)  0x67 = (103)
0x68=  11 01000 =  type8   Piture parameter set ( I-frame) (104)
  same case for p frame( SPS and PPS are same for all Mp4 store SPS and PPS separately for streaming we need sps ad pps  very frequently                     
0x65 = 11 00101 = type 5   Coded slice of an IDR picture (I-frame)
0x41 = 10 00001 = type 1   Coded slice of a non-IDR picture (P-frame)

0x27 = 01 00111 = type 7    Sequence parameter set (B-frame)
0x28 = 01 01000 = type 8    Picture parameter set (B-frame)   
0x25 = 01 00101 = type 5    Coded slice of an IDR picture (B-frame) //The first picture in a coded video sequence is always an IDR picture. An IDR frame is a special type of I-frame in H. 264. An IDR frame specifies that no frame after the IDR frame can reference any frame before it.
0x21 = 01 00001 = type 1    Coded slice of a non-IDR picture (B-frame)
 
* |0|1|2|3|4|5|6|7|
+-+-+-+-+-+-+-+-+
  |F|NRI|  Type   |   F 0 forbidden_zero_bit. This bit must be 0 in the H.264 specification.
  NRI(nal_ref_idc)  for  00((none) I(11) P(10) B01
  * 
  * Access Unit Delimiter (AUD) type 9. An AUD is an optional NALU that can be use to delimit frames in an elementary stream. It is not required (unless otherwise stated by the container/protocol, like TS), and is often not included in order to save space, but it can be useful to finds the start of a frame without having to fully parse each NALU.
  
  I-slice is a portion of a picture composed of macroblocks, all of which are based upon macroblocks within the same picture.
  Thus, H.264 introduces a new concept called slices ��� segments of a picture bigger than macroblocks but smaller than a frame.
  Just as there are I-slices, there are P- and B-slices. P- and B-slices are portions of a picture composed of macroblocks that are not dependent on macroblocks in the same picture. 
  
  H264 encoder sends an IDR (Instantaneous Decoder Refresh) coded picture (a group of I slices ) to clear the contents of the reference picture buffer. When we send an IDR coded picture, the decoder marks all pictures in the reference buffer as ���unused for reference���. All subsequently transmitted slices are decoded without reference to any frame decoded prior to the IDR picture. However, the reference buffer is not cleared with an I frame i.e, any frame after an I frame can use the reference buffer before the I frame. The first picture in a coded video sequence is always an IDR picture.

An IDR frame( Kye frame having sps and pps) is a special type of I-frame in H.264. An IDR frame specifies that no frame after the IDR frame can reference any frame before it. This makes seeking the H.264 file easier and more responsive to the player.

The IDR frames are introduced to avoid any distortions in the video when you want to skip/forward to some place in the video or start watching in the middle of the video.
  */ 

using namespace base;

//#define logger filterlogger //TODO: create a new logger for muxers

// #define MUXPARSE  //enable if you need to see what the byte parser is doing
// #define MUXSTATE //enable if you need to check the state of the muxer

MuxFrameFilter::MuxFrameFilter(const char* name, FrameFilter *next) :
FrameFilter(name, next), active(false), initialized(false), mstimestamp0(0), zerotimeset(false), av_format_ctx(NULL), avio_ctx(NULL),
avio_ctx_buffer(NULL), missing(0), ccf(0), av_dict(NULL), format_name("matroska"),  extradata_count(0) {
    // two substreams per stream
    this->codec_contexes.resize(2, NULL);
    this->streams.resize(2, NULL);

    /* some sekoilu..
    this->internal_basicframe2.payload.reserve(1024*500);
    this->internal_basicframe2.payload.resize(0);

    this->avbuffer = av_buffer_allocz(1024*500);
    this->sps_ok = false;
    this->pps_ok = false;
     */

    this->setupframes.resize(2);
    this->timebase = av_make_q(1, 1000); // we're using milliseconds
    // this->timebase = av_make_q(1,20); // we're using milliseconds
    this->avpkt = new AVPacket();
    av_init_packet(this->avpkt);

    this->prevpts = 0;

    this->avio_ctx_buffer = (uint8_t*) av_malloc(this->avio_ctx_buffer_size);
    this->avio_ctx = NULL;
    // this->avio_ctx = avio_alloc_context(this->avio_ctx_buffer, this->avio_ctx_buffer_size, 1, this, this->read_packet, this->write_packet, this->seek); // no read, nor seek

    /*
    // subclasses define format & muxing parameters
    // For example, fragmented MP4:
    
    format_name = std::string("mp4")

    // -movflags empty_moov+omit_tfhd_offset+frag_keyframe+separate_moof -frag_size
    av_dict_set(&av_dict, "movflags", "empty_moov+omit_tfhd_offset+frag_keyframe+separate_moof", 0);

    // av_dict_set(&av_dict, "frag_size", "500", 500); // nopes
    av_dict_set(&av_dict, "frag_size", "512", 0);
     */
}

MuxFrameFilter::~MuxFrameFilter() {
    deActivate();
    av_free(avio_ctx_buffer);
    av_free_packet(avpkt);
    delete avpkt;
    av_dict_free(&av_dict);
}

void MuxFrameFilter::initMux() {
    int i;
    AVCodecID codec_id;
    initialized = false;
    missing = 0;

    this->defineMux(); // fills av_dict & format_name

    // create output context, open files
    i = avformat_alloc_output_context2(&av_format_context, NULL, format_name.c_str(), NULL);

    if (!av_format_context) {
        SError << "MuxFrameFilter : initMux : FATAL : could not create output context!  Have you registered codecs and muxers? ";
        avformat_free_context(av_format_context);
        exit(2);
    }

    // *** custom IO *** // https://www.ffmpeg.org/doxygen/3.2/avformat_8h_source.html
    av_format_context->pb = avio_ctx;
    // av_format_context->flags |= AVFMT_FLAG_NOFILLIN;
    // none of this is actually needed..
    // av_format_context->flags |= AVFMT_TS_NONSTRICT;
    // av_format_context->flags = AVFMT_FLAG_NOBUFFER;
    // av_format_context->flags = AVFMT_FLAG_FLUSH_PACKETS;
    // av_format_context->flags = AVFMT_FLAG_CUSTOM_IO; 
    /*
    av_format_context->flags |= AVFMT_FLAG_CUSTOM_IO;
    av_format_context->flags |= AVFMT_FLAG_FLUSH_PACKETS;
    av_format_context->flags |= AVFMT_FLAG_NOBUFFER;
    av_format_context->flags |= AVFMT_FLAG_NONBLOCK;
    av_format_context->flags |= AVFMT_FLAG_NOFILLIN;
    av_format_context->flags |= AVFMT_FLAG_NOPARSE;
     */

    // *** normal file IO *** (for comparison / debugging)
    // i = avio_open(&av_format_context->pb, "paska", AVIO_FLAG_WRITE);
    // std::cout << "avformat_init_output" << std::endl;
    avformat_init_output(av_format_context, &av_dict);

    // use the saved setup frames (if any) to set up the streams
    // Frame *frame; // alias
    for (auto it = setupframes.begin(); it != setupframes.end(); it++) 
    {
        SetupFrame &setupframe = *it;
        if (setupframe.stream_index >=0) { // not been initialized
            AVCodecID codec_id = setupframe.codec_id;

            if (codec_id == AV_CODEC_ID_H264)
            {
                AVCodecContext *av_codec_context;
                AVStream *av_stream;

                av_codec_context = avcodec_alloc_context3(avcodec_find_decoder(codec_id));
                av_codec_context->width = 720; // dummy values .. otherwise mkv muxer refuses to co-operate
                av_codec_context->height = 576;
                av_codec_context->bit_rate = 1024 * 1024;
                av_codec_context->time_base = timebase; // 1/1000
                av_codec_context->flags |= CODEC_FLAG_GLOBAL_HEADER;
                ///*
                av_codec_context->extradata = extradata_videoframe.payload.data();
                av_codec_context->extradata_size = extradata_videoframe.payload.size();
                //*/

                /*
                std::cout << "initMux: extradata_size: " << av_codec_context->extradata_size 
                    << std::endl;
                 */

                // std::cout << "avformat_new_stream" << std::endl;
                av_stream = avformat_new_stream(av_format_context, av_codec_context->codec); // av_codec_context->codec == AVCodec (i.e. we create a stream having a certain codec)

                // av_stream->time_base = av_codec_context->time_base;
                // av_stream->codec->codec_tag = 0;
                av_stream->time_base = timebase; // 1/1000
                av_stream->id = setupframe.stream_index;
                /*
                // write some reasonable values here.  I'm unable to re-write this .. should be put into av_codec_context ?
                AVRational fps = AVRational();
                //fps.num = 20;
                //fps.den = 1;
                fps.num = 1000;
                fps.den = 1;
                av_stream->avg_frame_rate = fps;
                 */
                // av_stream->codec->time_base = av_stream->time_base;
                // NOTE: member "codec" is deprecated, should use "codecpar"
                i = avcodec_parameters_from_context(av_stream->codecpar, av_codec_context);

                /*
                std::cout << "initMux: extradata_size 2: " << 
                    av_stream->codec->extradata_size 
                    << std::endl;

                std::cout << "initMux: extradata_size 3: " << 
                    av_stream->codecpar->extradata_size 
                    << std::endl;
                // yes, that's correct
                 */


                //av_stream->codec->extradata = extradata_frame.payload.data();
                //av_stream->codec->extradata_size = extradata_frame.payload.size();

                // std::cout << "MuxFrameFilter : initMux : context and stream " << std::endl;
                codec_contexes[setupframe.stream_index] = av_codec_context;
                streams[setupframe.stream_index] = av_stream;
                
                // std::cout << "initMux: codec_ctx timebase: " << av_codec_context->time_base.num << "/" << av_codec_context->time_base.den << std::endl;
                // std::cout << "initMux: stream timebase   : " << av_stream->time_base.num << "/" << av_stream->time_base.den << std::endl;
                // std::cout << "initMux: stream->codecpar timebase   : " << av_stream->codecpar->time_base.num << "/" << av_stream->codecpar->time_base.den << std::endl;
            }
            else if (codec_id== AV_CODEC_ID_AAC)
            {
                AVCodecContext *av_codec_context;
                AVStream *av_stream;

                av_codec_context = avcodec_alloc_context3(avcodec_find_decoder(codec_id));
                
               // av_codec_context->sample_fmt  = codec->sample_fmts ?  codec->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
                
                av_codec_context->bit_rate = 64000;
                av_codec_context->time_base = timebase; // 1/1000
                av_codec_context->flags |= CODEC_FLAG_GLOBAL_HEADER;
                av_codec_context->sample_rate = 44100;
                av_codec_context->profile = FF_PROFILE_AAC_LOW;
              
                av_codec_context->extradata = extradata_audioframe.payload.data();
                av_codec_context->extradata_size = extradata_audioframe.payload.size();
                
                //unsigned short int aacspec = 5136;  //0001010000010000 hex 1410 https://kxcblog.com/post/avcodec/aac-audio-specific-config/#:~:text=AAC%20Audio%20AudioSpecificConfig
             
                //AAC AudioSpecificConfig
                //memcpy( av_codec_context->extradata, &aacspec , 2);
                        
                 av_codec_context->channel_layout = AV_CH_LAYOUT_STEREO;
                 av_codec_context->channels        = av_get_channel_layout_nb_channels(av_codec_context->channel_layout);

                // ost->st->time_base = (AVRational){ 1, c->sample_rate };
                // std::cout << "avformat_new_stream" << std::endl;
                av_stream = avformat_new_stream(av_format_context, av_codec_context->codec); // av_codec_context->codec == AVCodec (i.e. we create a stream having a certain codec)

                // av_stream->time_base = av_codec_context->time_base;
                // av_stream->codec->codec_tag = 0;
                av_stream->time_base = timebase; // 1/1000
                av_stream->id = setupframe.stream_index;
                /*
                // write some reasonable values here.  I'm unable to re-write this .. should be put into av_codec_context ?
                AVRational fps = AVRational();
                //fps.num = 20;
                //fps.den = 1;
                fps.num = 1000;
                fps.den = 1;
                av_stream->avg_frame_rate = fps;
                 */
                // av_stream->codec->time_base = av_stream->time_base;
                // NOTE: member "codec" is deprecated, should use "codecpar"
                i = avcodec_parameters_from_context(av_stream->codecpar, av_codec_context);
                

                av_stream->codecpar->extradata = ( uint8_t *)av_malloc(av_codec_context->extradata_size + FF_INPUT_BUFFER_PADDING_SIZE);
                av_stream->codecpar->extradata_size =  av_codec_context->extradata_size ;
                memcpy(av_stream->codecpar->extradata, av_codec_context->extradata, av_codec_context->extradata_size);
                

                // std::cout << "MuxFrameFilter : initMux : context and stream " << std::endl;
                codec_contexes[setupframe.stream_index] = av_codec_context;
                streams[setupframe.stream_index] = av_stream;
                
 
            }
            
            initialized = true; // so, at least one substream init'd
        
        } // got setupframe
    }

    if (!initialized) {
        SError << "Stream is not setup correctly";
        return;
    }

    ///*
    // codec_contexes, streams, av_format_context reserved !
    // std::cout << "MuxFrameFilter: writing header" << std::endl;

    i = avformat_write_header(av_format_context, &av_dict);
    if (i < 0) {
        SError << "MuxFrameFilter : initMux : Error occurred while muxing";
        perror("libValkka: MuxFrameFilter: initMux");
        exit(2);
        // av_err2str(i)
        // closeMux();
        // return;
    }

    //*/

    /*
    // test re-write // works OK at this point (before writing any actual frames)
    AVRational fps = AVRational();
    fps.num = 10;
    fps.den = 1;
    streams[0]->avg_frame_rate = fps;
    i=avformat_write_header(av_format_context, NULL); // re-write
     */

    // so far so good ..
    if (zerotime > 0) { // user wants to set time reference explicitly and not from first arrived packet ..
        mstimestamp0 = zerotime;
        zerotimeset = true;
    } else {
        zerotimeset = false;
    }

    //
    //std::cout << "timebase " 
    //    << streams[0]->time_base.num << " " << streams[0]->time_base.den << std::endl;
    // this mf persists in 16000 ticks per second.  mf
}

int MuxFrameFilter::write_packet(void *opaque, uint8_t *buf, int buf_size) {
    // std::cout << "dummy" << std::endl;
    return 0; // re-define in child classes
}

void MuxFrameFilter::closeMux() {
    int i;

    if (initialized) {
        // std::cout << "MuxFrameFilter: closeMux: freeing ctx" << std::endl;
        // avio_closep(&avio_ctx);        
        avformat_free_context(av_format_context);
        // avformat_close_input(&av_format_context); // nopes
        for (auto it = codec_contexes.begin(); it != codec_contexes.end(); ++it) {
            if (*it != NULL) {
                // avcodec_free_context should not try to free this!
                (*it)->extradata = NULL;
                (*it)->extradata_size = 0;
                // std::cout << "MuxFrameFilter: closeMux: context " << (long unsigned)(*it) << std::endl;
                avcodec_close(*it);
                avcodec_free_context(&(*it));
                *it = NULL;
            }
        }
        for (auto it = streams.begin(); it != streams.end(); ++it) {
            if (*it != NULL) {
                // std::cout << "MuxFrameFilter: closeMux: stream" << std::endl;
                // eh.. nothing needed here.. enough to call close on the context
                *it = NULL;
            }
        }
        av_free(avio_ctx);
    }
    initialized = false;
    has_extraVideodata = false;
    has_extraAudiodata = false;
    extradata_count = 0;
    prevpts = 0;
    missing = 0;
    ccf = 0;
}

void MuxFrameFilter::deActivate_() {
    if (initialized) {
        av_write_trailer(av_format_context);
        closeMux();
    }
    active = false;
}

void MuxFrameFilter::run(Frame* frame) {
    this->go(frame);
    // chaining of run is done at write_packet
}

void MuxFrameFilter::go(Frame* frame) {
    std::unique_lock<std::mutex> lk(this->mutex);

    static int ncount = 0;

    std::cout << "Got frame " << ncount++ << std::endl;



    //std::cout << "MuxFrameFilter: go: frame " << *frame << std::endl;

    internal_frame.n_slot = frame->n_slot;

    // make a copy of the setup frames ..
    if (frame->getFrameClass() == FrameClass::setup) { // SETUPFRAME
        SetupFrame *setupframe = static_cast<SetupFrame*> (frame);
        if (setupframe->sub_type == SetupFrameType::stream_init) { // INIT
            if (setupframe->stream_index > 1) {
                SError << "MuxFrameFilter : too many subsessions! ";
            } else {
#ifdef MUXSTATE
                std::cout << "MuxFrameFilter:  go: state: got setup frame " << *setupframe << std::endl;
#endif
                SInfo << "MuxFrameFilter :  go : got setup frame " << *setupframe << std::endl;
                setupframes[setupframe->stream_index].copyFrom(setupframe);
                
                 mstimestamp0 = setupframe->mstimestamp;
            }
            return;
        } // INIT
    }// SETUPFRAME
    
    else if (frame->getFrameClass() == FrameClass::basic) { // BASICFRAME
        BasicFrame *basicframe = static_cast<BasicFrame*> (frame);
        
        if (basicframe->codec_id == AV_CODEC_ID_AAC) {
            
            if (!has_extraAudiodata) 
            {
                 SInfo << "MuxFrameFilter : appending extraAudiodata";
                 extradata_audioframe.payload.insert(
                            extradata_audioframe.payload.end(),
                            basicframe->payload.begin(),
                            basicframe->payload.end()
                            );
                 has_extraAudiodata = true;
            }
            else
            {
                
               if (!initialized) 
               { // can't init this file.. de-activate
                 
                    initMux(); // modifies member initialized
                    writeFrame(basicframe);
                }
                else
                {
                    writeFrame(basicframe);
                   // write_audio_frame(av_format_context, &audio_st);
                }
                
            }
           
             
            
        }//end Audio read
        
        else if (basicframe->codec_id == AV_CODEC_ID_H264 )
        {
            // https://stackoverflow.com/questions/54119705/fragmented-mp4-problem-playing-in-browser
            // http://aviadr1.blogspot.com/2010/05/h264-extradata-partially-explained-for.html
            if (!has_extraVideodata) 
            {
                // this kind of stuff should be in the frame class itself..
                // should arrive in sps, pps order
                if ((basicframe->h264_pars.slice_type == H264SliceType::sps) or
                        (basicframe->h264_pars.slice_type == H264SliceType::pps)) {
#ifdef MUXSTATE
                    std::cout << "MuxFrameFilter: go: state: appending extradata" << std::endl;
#endif
                  
                    extradata_videoframe.payload.insert(
                            extradata_videoframe.payload.end(),
                            basicframe->payload.begin(),
                            basicframe->payload.end()
                            );
               

                    if (basicframe->h264_pars.slice_type == H264SliceType::sps and
                            extradata_count == 0) {
                        extradata_count = 1;
                          SInfo << "MuxFrameFilter : appending extraVideodata SPS";
                    }

                    if (basicframe->h264_pars.slice_type == H264SliceType::pps and
                            extradata_count == 1) {
                        extradata_count = 2;
                         SInfo << "MuxFrameFilter : appending extraVideodata PPS";
                    }

                    if (extradata_count >= 2) {
                        has_extraVideodata = true;
    #ifdef MUXSTATE
                        std::cout << "MuxFrameFilter: go: state: extradata ok" << std::endl;
    #endif
                        extradata_videoframe.copyMetaFrom(basicframe); // timestamps etc.
                    }
                }
            }
            else //end header read
            {
                  
                if (!initialized)
                { // got setup frames, writing has been requested, but file has not been opened yet
        #ifdef MUXSTATE
                    std::cout << "MuxFrameFilter: go: state: calling initMux & setting initialized=true" << std::endl;
        #endif
                    initMux(); // modifies member initialized
                    if (!initialized) { // can't init this file.. de-activate
                        deActivate_();
                    } else {
                        // set zero time
                        // mstimestamp0 = extradata_videoframe.mstimestamp;
                       
                        extradata_videoframe.mstimestamp = mstimestamp0;
                        extradata_videoframe.stream_index = 0;
                        // std::cout << "writing extradata" << std::endl;
                        writeFrame(&extradata_videoframe); // send sps & pps data to muxer only once
                        // std::cout << "wrote extradata" << std::endl;
                        writeFrame(basicframe);
                    }
                }
                else
                {
                     writeFrame(basicframe);
                }
                
            }// end  content read
        }// end h264

    


    }// BASICFRAME
    else {
        SError << "MuxFrameFilter : go: needs BasicFrame";
    }
}

void MuxFrameFilter::writeFrame(BasicFrame* basicframe) {
    /*
    A word of warning about achieving browser-compatible frag-mp4 (by Sampsa & Petri).

    Trying to make libavformat to work browser-MSE's was completely mental

    - sps & pps packets must be at the "extradata" structure only
    - sps & pps packets should not be resend to the muxer
    - ..when you play a stream using ffplay / vlc, you can see in the output text saying "no frame!"
    - ..this typically happens when there is sps, pps or some "auxiliary information" data in the stream
    - ..a stream that gives "no frame!" is a stream that the browser players consider corrupt.  Why?  That's just the way it is
    - ..so, the stream must be pruned of all extra "crap".  Through only i, p & b frames into the muxer

    You're welcome.
     */

#ifdef MUXSTATE
    std::cout << "MuxFrameFilter: writeFrame: state: ready, active, initd: " << int(ready) << " " << int(active)
            << " " << int(initialized) << std::endl;
#endif

    if (!initialized) {
        return;
    }

    long int dt = (basicframe->mstimestamp - mstimestamp0);
    if (dt < 0) {
        dt = 0;
    }
    // std::cout << "MuxFrameFilter : writing frame with mstimestamp " << dt << std::endl;
    //SInfo << "MuxFrameFilter : writing frame with mstimestamp " << dt;
    SInfo << "MuxFrameFilter : writing frame " << *basicframe;
    // internal_basicframe2.fillAVPacket(avpkt); // copies metadata to avpkt, points to basicframe's payload
    // internal_basicframe.fillAVPacket(avpkt);
    basicframe->fillAVPacket(avpkt); // copies metadata to avpkt, points to basicframe's payload

    /*
    std::cout << "DEBUG: frame: " << *basicframe << std::endl;
    std::cout << "DEBUG: subses index: " << basicframe->stream_index << std::endl;
    std::cout << "DEBUG: streams: " << streams.size() << std::endl;
     */
    AVStream *av_stream = streams[basicframe->stream_index];
    AVCodecContext *av_codec_context = codec_contexes[basicframe->stream_index];

    // std::cout << "writeFrame: codec_ctx timebase: " << av_codec_context->time_base.num << "/" << av_codec_context->time_base.den << std::endl;
    // std::cout << "writeFrame: stream timebase   : " << av_stream->time_base.num << "/" << av_stream->time_base.den << std::endl;
    // stream timebase goes automagically to value 1/16000
    // std::cout << "writeFrame: stream->codecpar timebase   : " << av_stream->codecpar->time_base.num << "/" << av_stream->codecpar->time_base.den << std::endl;

    // avpkt->buf = NULL;
    if (dt >= 0) {
        avpkt->pts = (int64_t) (dt);
        /*
        avpkt->pts = 
            (streams[basicframe->stream_index]->time_base.den * dt)/
            (streams[basicframe->stream_index]->time_base.num * 1000);
            // complicated & stupid
         */
        
        // NOTICE: this is critical.  the mp4 muxer goes sour if you dont feed
        // it increasing timestamps.  went almost nuts for this.
        if (avpkt->pts <= prevpts) {
            avpkt->pts = prevpts + 1;
        }
        prevpts = avpkt->pts;
        // std::cout << "avpkt->pts: " << avpkt->pts << std::endl;
    } else {
        std::cout << "fragmp4mux: NEGATIVE TIMESTAMP" << std::endl;
        avpkt->pts = AV_NOPTS_VALUE;
    }

    if (basicframe->h264_pars.slice_type == H264SliceType::idr) {  //arvind
        avpkt->flags = AV_PKT_FLAG_KEY;
    }

    av_packet_rescale_ts(avpkt, av_codec_context->time_base, av_stream->time_base);

    /*
    if (basicframe->h264_pars.slice_type == H264SliceType::sps) {
        avpkt->flags = AV_PKT_FLAG_KEY;
    }
     */
    /*
    if (basicframe->isSeekable()) {
        avpkt->flags = AV_PKT_FLAG_KEY;
    }
     */

    // std::cout << "avpkt->pts: " << avpkt->pts << std::endl;
    // std::cout << "MuxFrameFilter : avpkt size " << avpkt->size << std::endl;
    
    SInfo << "Stream " << basicframe->stream_index << " DTS " << dt << " PTS "  <<  avpkt->pts  << " size " << avpkt->size << " timesbae "  << av_stream->time_base.den;
    
    int res = av_interleaved_write_frame(av_format_context, avpkt); // => this calls write_packet
    // std::cout << "MuxFrameFilter : av_write_frame returned " << res << std::endl;
    if (res < 0) {
        SError << "MuxFrameFilter: av_write_frame returned < 0 : muxer reset";
        av_write_trailer(av_format_context); // if we don't call this we'll get massive memleaks!  
        closeMux();
        // std::cout << "MuxFrameFilter: initialized now " << int(initialized) << std::endl;
    } else {
        // used to crasssshh here, but not anymore, after we have defined dummy read & seek functions!
        // int res = av_write_frame(av_format_context, avpkt);
        //std::cout << "res =" << res << std::endl;
        av_write_frame(av_format_context, NULL); // create a custom fragment
        /*
        av_buffer_unref(&(avpkt->buf));
        av_packet_unref(avpkt);
         */
    }
}

void MuxFrameFilter::activate(long int zerotime) {
    std::unique_lock<std::mutex> lk(this->mutex);
    if (active) {
        deActivate_();
    }

    this->zerotime = zerotime;
    this->active = true;
}

void MuxFrameFilter::deActivate() {
    std::unique_lock<std::mutex> lk(this->mutex);

    // std::cout << "FileFrameFilter: deActivate:" << std::endl;
    deActivate_();
    // std::cout << "FileFrameFilter: deActivate: bye" << std::endl;
}

FragMP4MuxFrameFilter::FragMP4MuxFrameFilter(const char* name, FrameFilter *next) :
MuxFrameFilter(name, next), got_ftyp(false), got_moov(false) {
    internal_frame.meta_type = MuxMetaType::fragmp4;
    internal_frame.meta_blob.resize(sizeof (FragMP4Meta));

    ftyp_frame.meta_type = MuxMetaType::fragmp4;
    ftyp_frame.meta_blob.resize(sizeof (FragMP4Meta));

    moov_frame.meta_type = MuxMetaType::fragmp4;
    moov_frame.meta_blob.resize(sizeof (FragMP4Meta));
}

FragMP4MuxFrameFilter::~FragMP4MuxFrameFilter() {
}

void FragMP4MuxFrameFilter::sendMeta() {
    std::unique_lock<std::mutex> lk(this->mutex);
    if (!next) {
        return;
    }
    if (got_ftyp and got_moov) {
        std::cout << "FragMP4MuxFrameFilter: sending metadata!" << std::endl;
        next->run(&ftyp_frame);
        next->run(&moov_frame);
    } else {
        std::cout << "FragMP4MuxFrameFilter: No metadata!" << std::endl;
    }
}

void FragMP4MuxFrameFilter::defineMux() {
    this->avio_ctx = avio_alloc_context(this->avio_ctx_buffer, this->avio_ctx_buffer_size, 1,
            this, this->read_packet, this->write_packet, this->seek); // no read, nor seek
    // .. must be done here, so that read/write_packet points to the correct static function
    format_name = std::string("mp4");

    // -movflags empty_moov+omit_tfhd_offset+frag_keyframe+separate_moof -frag_size
    // av_dict_set(&av_dict, "movflags", "empty_moov+omit_tfhd_offset+frag_keyframe+separate_moof", 0);
    // av_dict_set(&av_dict, "movflags", "empty_moov+omit_tfhd_offset+frag_keyframe+separate_moof+frag_custom", 0);
    // av_dict_set(&av_dict, "movflags", "empty_moov+omit_tfhd_offset+separate_moof", 0);
    av_dict_set(&av_dict, "movflags", "empty_moov+omit_tfhd_offset+separate_moof+frag_custom", 0);

    // no need for any of this, really.. the latency is small anyway
    // av_dict_set(&av_dict, "frag_size", "500", 500); // nopes
    // av_dict_set(&av_dict, "frag_size", "512", 0);
    // av_dict_set(&av_dict, "frag_size", "200000", 0);
    // av_dict_set(&av_dict, "frag_size", "10240", 0);

}

int FragMP4MuxFrameFilter::write_packet(void *opaque, uint8_t *buf, int buf_size_) {
    // what's coming here?  A complete muxed "frame" or a bytebuffer with several frames.  
    // The frames may continue in the next bytebuffer.
    // It seems that once "frag_size" has been set to a small value, this starts getting complete frames, 
    // instead of several frames in the same bytebuffer
    //
    
    //SInfo  << "write_packet " << buf_size_;
    
    // buf_size_ : amount of data libavformat gives us
    FragMP4MuxFrameFilter* me = static_cast<FragMP4MuxFrameFilter*> (opaque);
    MuxFrame& internal_frame = me->internal_frame;
    uint32_t &missing = me->missing;
    uint32_t &ccf = me->ccf;

    /*
     
    ffmpeg buffers:
    
    0         cc       cc+len   buf_size-1
    |------|---|---------|--|-----|      |----|---|--------|--|----|
    
    frame payload:
    
    new round with len & missing
                 
                  cff
    0             | len
    .......+++++++......
    |------------------------|
                  ............
                    missing
     */

    uint32_t cc = 0; // index of box start byte at the current byte buffer // ccf is current index in the non-complete mp4 box
    uint32_t len = 0; // number of bytes: either a complete box or what is missing from the box
    // ..consume this many bytes from the current byte buffer and add them to the frame buffer
    uint32_t buf_size = uint32_t(buf_size_); // size of the current byte buffer to be consumed
    int i;
    uint32_t boxlen;
    char boxname[4];

#ifdef MUXPARSE
    std::cout << "\n====>buf_size: " << buf_size << std::endl;
    std::cout << "dump: ";
    for (i = 0; i <= 8; i++) {
        std::cout << int(buf[i]) << " ";
    }
    std::cout << std::endl;
#endif

    while (cc < buf_size) 
    { // consume bytes given by libavformat while they last
        if (missing > 0) 
        {
#ifdef MUXPARSE
            std::cout << "taking up missing bytes " << missing << std::endl;
#endif
            len = missing;
        } 
        else 
        { // start of a new mp4 box
#ifdef MUXPARSE
            std::cout << std::endl << "start: [";
            for (i = 0; i <= 9; i++) {
                std::cout << int(buf[cc + i]) << " ";
            }
            std::cout << "]" << " " << ccf << std::endl;
#endif
            len = deserialize_uint32_big_endian(buf + cc); // resolve the packet length from the mp4 headers

            // if (len > 14000) { // enable to test muxer reinit
            if (len > 99999999) { // absurd value .. this bytestream parser has gone sour.
                std::cout << "MuxFrameFilter: overflow: len: " << len << std::endl;
                // exit(2);
                return -1;
            } else if (len < 1)
            {
                SError << "MuxFrameFilter: packet of length zero!";
                cc += 4;
                continue;
            }
#ifdef MUXPARSE
            std::cout << " ==> len: " << len << std::endl;
#endif

            internal_frame.reserve(len); // does nothing if already has this capacity
            internal_frame.resize(len);
        }

#ifdef MUXPARSE
        std::cout << "cc + len: " << cc + len << std::endl;
#endif

        if ((cc + len) > buf_size) { // required number of bytes is larger than the buffer
            memcpy(internal_frame.payload.data() + ccf, buf + cc, buf_size - cc); // copy the rest of the buffer
            ccf += buf_size - cc;
            missing = len - (buf_size - cc); // next time this is called, ingest more bytes
#ifdef MUXPARSE
            std::cout << "missing bytes: " << missing << std::endl;
#endif
            cc += buf_size;
        } else
        { // all required bytes are in the buffer
            memcpy(internal_frame.payload.data() + ccf, buf + cc, len);
            missing = 0;
#ifdef MUXPARSE
            std::cout << "FragMP4MuxFrameFilter: OUT: len: " << internal_frame.payload.size() << " dump:" << internal_frame.dumpPayload() << std::endl;
#endif
            ccf = 0;
            cc += len;

            getLenName(internal_frame.payload.data(), boxlen, boxname);
#ifdef MUXPARSE
            std::cout << "FragMP4MuxFrameFilter: got box " << std::string(boxname) << std::endl;
#endif
            // std::cout << "FragMP4MuxFrameFilter: got box " << std::string(boxname) << std::endl;
            // set the frame type that also defines the metadata
            // internal_frame.meta_type = MuxMetaType::fragmp4; // at ctor
            FragMP4Meta* metap;
            // internal_frame.meta_blob.resize(sizeof(FragMP4Meta)); // at ctor
            metap = (FragMP4Meta*) (internal_frame.meta_blob.data());
            // set values in-place:
            ///*
            if (strcmp(boxname, "moof") == 0) {
                metap->is_first = moofHasFirstSampleFlag(internal_frame.payload.data());
                //#ifdef MUXPARSE
                STrace << "FragMP4MuxFrameFilter: moof first sample flag: " << int(metap->is_first) ;
                // #endif
            }
            //*/
            memcpy(&metap->name[0], boxname, 4);

            std::cout << "boxname " << boxname << std::endl;

            // TODO: get timestamp from the MP4 structure
            // at the moment, internal_frame does not have any timestamp
            // metap->mstimestamp = internal_frame.mstimestamp; 
            metap->mstimestamp = 0; // n/a for the moment
            metap->size = boxlen; // internal_frame.payload.size();
            metap->slot = internal_frame.n_slot;

            if (strcmp(boxname, "ftyp") == 0) {
                me->ftyp_frame.copyFrom(&internal_frame);
                me->got_ftyp = true;
                std::cout << "FragMP4MuxFrameFilter: got ftyp" << std::endl;
            } else if (strcmp(boxname, "moov") == 0) {
                me->moov_frame.copyFrom(&internal_frame);
                me->got_moov = true;
                std::cout << "FragMP4MuxFrameFilter: got moov" << std::endl;
                // std::cout << "FragMP4MuxFrameFilter: metadata cached" << std::endl;
            }

#ifdef MUXPARSE
            std::cout << "FragMP4MuxFrameFilter: sending frame downstream " << std::endl;
#endif
            if (me->next) {
                // std::cout << ">size " << internal_frame.payload.size() << std::endl;
                me->next->run(&internal_frame);
            }
#ifdef MUXPARSE
            std::cout << "FragMP4MuxFrameFilter: frame sent " << std::endl;
#endif
        }
        // cc += len; // move on to next box
#ifdef MUXPARSE
        std::cout << "FragMP4MuxFrameFilter: cc = " << cc << " / " << buf_size << std::endl;
#endif
    }
    return 0;
}

void getLenName(uint8_t* data, uint32_t& len, char* name) {
    uint32_t cc = 0;
    len = deserialize_uint32_big_endian(data + cc);
    cc += 4;
    memcpy(name, data + cc, 4); // name consists of 4 bytes
}

uint32_t getSubBoxIndex(uint8_t* data, const char name[4]) {
    // returns start index of the subbox
    uint32_t cc = 0;
    uint32_t thislen;
    char thisname[4];
    char name_[4];
    uint32_t len_;

    getLenName(data, thislen, &thisname[0]);
    cc += 8; // len & name, both 4 bytes
    while (cc <= thislen) {
        getLenName(data + cc, len_, &name_[0]); // take the next sub-box
        // std::cout << "NAME:" << name_ << std::endl;
        if (strcmp(name, name_) == 0) {
            return cc;
        }
        cc += len_;
    }
    return 0;
}

bool moofHasFirstSampleFlag(uint8_t* data) {
    /*
    [moof [traf [trun]]]
     */
    uint32_t cc = 0;
    uint8_t* current_box;
    current_box = data;
    // std::cout << "looking for traf" << std::endl;
    cc = getSubBoxIndex(current_box, "traf");
    current_box = current_box + cc;
    // std::cout << "looking for trun" << std::endl;
    cc = getSubBoxIndex(current_box, "trun");
    current_box = current_box + cc;
    // we're at trun now
    //ISO/IEC 14496-12:2012(E) .. pages 5 and 57
    //bytes: (size 4), (name 4), (version 1 + tr_flags 3)
    return (current_box[10 + 1] & 4) == 4;
}


/* correct:
<Box: ftyp of 36 bytes> False
<Box: moov of 773 bytes> False
<Box: moof of 104 bytes> True
<Box: mdat of 110622 bytes> False
<Box: moof of 108 bytes> False
<Box: mdat of 4788 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 6681 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 11342 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 10721 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 17788 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 14732 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 13937 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 28250 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 10608 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 12796 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 27097 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 5577 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 13950 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 39919 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 8324 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 11605 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 26379 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 5257 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 27924 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 5349 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 28010 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 10311 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 13025 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 28500 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 51782 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 14090 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 18932 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 20703 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 21285 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 9702 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 38228 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 4038 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 13065 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 27714 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 5358 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 14695 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 29267 bytes> False
<Box: moof of 100 bytes> False
<Box: mdat of 10533 bytes> False
 */






