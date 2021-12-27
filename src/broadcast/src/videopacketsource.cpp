

#include "webrtc/videopacketsource.h"
#include "webrtc/rawVideoFrame.h"
#include "webrtc/peermanager.h"

#ifdef HAVE_FFMPEG
#if MP4File
#include "ff/ffmpeg.h"
#include "ff/videocontext.h"
#include "ff/videodecoder.h"
#include "ff/fpscounter.h"
 #endif
#include "api/video/i420_buffer.h"
#include "rtc_base/ref_counted_object.h"
#include "rtc_base/atomic_ops.h"
#include <chrono>

extern "C"
{
//#include <libavutil/timestamp.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/parseutils.h>
}


#include "muxer.h"
#include "Settings.h"

using std::endl;

#define WebRTC_USE_DECODER_PTS 1

#define BYPASSGAME 1

namespace base {
    
 extern fmp4::ReadMp4 *self;
    
namespace wrtc {
    
#define tcprequest true
    

VideoPacketSource::VideoPacketSource( const char *name,  std::string cam, fmp4::FrameFilter *next):cam(cam),fmp4::FrameFilter(name, next)
    , _rotation(webrtc::kVideoRotation_0)
    , _timestampOffset(0)
    , _nextTimestamp(0)
//    , _source(nullptr)
{
    // Default supported formats. Use SetSupportedFormats to over write.
    std::vector<cricket::VideoFormat> formats;
    formats.push_back(_captureFormat);
   // SetSupportedFormats(formats);
    
    
          
    
    if ((videopkt = av_packet_alloc()) == NULL) {
                fprintf(stderr, "av_packet_alloc failed.\n");
                //goto ret3;
                return;
       }
            
        av_init_packet(videopkt );
     
     
             
    //ffmpeg -decoders
    
    /*
        VFS..D h264                 H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10
        V....D libopenh264          OpenH264 H.264 / AVC / MPEG-4 AVC / MPEG-4 part 10 (codec h264)
        V..... h264_cuvid           Nvidia CUVID H264 decoder (codec h264)
    */            
     //   codec = avcodec_find_decoder_by_name("h264_cuvid");

       // if(!codec)
      //     codec = avcodec_find_decoder_by_name("libopenh264");

        if(!codec)
          codec = avcodec_find_decoder_by_name("h264");

        if(!codec)
        codec = avcodec_find_decoder(AV_CODEC_ID_H264);
     
     	if (codec == NULL)
    	{
    		SError<<  "avcodec_find_decoder failed";
    		
    	}
            
    	if ((cdc_ctx = avcodec_alloc_context3(codec)) == NULL)
    	{
                SError<<  "avcodec_alloc_context3 failed";
    	
    	}
        
//        if(codec->capabilities & CODEC_CAP_TRUNCATED) {
//            cdc_ctx->flags |= CODEC_FLAG_TRUNCATED;
//        }
        
        int ret ;
    	if ((ret = avcodec_open2(cdc_ctx, codec, NULL)) < 0)
    	{
    		SError<<  "avcodec_open2 failed";
      
    	}
            
        if ((avframe = av_frame_alloc()) == NULL)
    	{
                SError<<  "av_frame_alloc failed";
    	}
            
            
        if ((parser = av_parser_init(codec->id)) == NULL)
	{
		
	    SError<<  "av_parser_init failed";
	}
        
        
     
        fragmp4_filter = new fmp4::DummyFrameFilter("fragmp4", cam, nullptr);
       // fragmp4_muxer = new fmp4::FragMP4MuxFrameFilter("fragmp4muxer", fragmp4_filter);

        info = new fmp4::InfoFrameFilter("info", nullptr);

        txt = new fmp4::TextFrameFilter("txt", cam, self);


        ffparser = new fmp4::LiveThread("live");

        ffparser->start();

       // fmp4::FrameFilter *tmpVc =(fmp4::FrameFilter *) VideoCapturer.get();

       // std::string  cam = peer->getCam();
        
        std::string add;
        
        
        if( Settings::getNodeState(cam, "rtsp" , add ))
        {
       // std::string &add =  Settings::configuration.rtsp[cam]["rtsp"].get<std::string>();

            ctx = new fmp4::LiveConnectionContext(fmp4::LiveConnectionType::rtsp, add, slot, cam, tcprequest, this , info, txt); // Request livethread to write into filter info
            ffparser->registerStreamCall(*ctx);
            ffparser->playStreamCall(*ctx);

         //   Settings::configuration.rtsp[cam]["state"]="streaming";
            Settings::setNodeState(cam , "streaming" );
            
            SInfo  <<   cam  << " " <<    "streaming";
        }
        else
        {
            SError << "Could not find camera at Json Repository "  << cam; 
        }
      
}


//VideoPacketSource::VideoPacketSource(const cricket::VideoFormat& captureFormat)
//    : _captureFormat(captureFormat)
//    , _rotation(webrtc::kVideoRotation_0)
//    , _timestampOffset(0)
//    , _nextTimestamp(0)
////    , _source(nullptr)
//{
//    // Default supported formats. Use SetSupportedFormats to over write.
//    std::vector<cricket::VideoFormat> formats;
//    formats.push_back(_captureFormat);
//    //SetSupportedFormats(formats);
//
//    // formats.push_back(cricket::VideoFormat(1280, 720, _fpsInterval, _codec));
//    // formats.push_back(cricket::VideoFormat(640, 480, _fpsInterval, _codec));
//    // formats.push_back(cricket::VideoFormat(320, 240, _fpsInterval, _codec));
//    // formats.push_back(cricket::VideoFormat(160, 120, _fpsInterval, _codec));
//}


VideoPacketSource::~VideoPacketSource()
{
    LDebug(": Destroying")
    
    	//avformat_close_input(&fmt_ctx);

	//avformat_free_context(fmt_ctx);

    stopParser();


}

void VideoPacketSource::stopParser()
{
    SInfo << "stopParser";
    if(ffparser)
    {
        if(ffparser)
        {
            
            SInfo << "Stopping cam "  << ctx->cam; 
            
            if( !Settings::setNodeState(ctx->cam , "stopped" ) )
            {
                 SError << "Could not find camera at Json Repository "  << ctx->cam; 
            }
            
          //  Settings::configuration.rtsp[ctx->cam]["state"]= "stopped";
              
            ffparser->stopStreamCall(*ctx);

            ffparser->deregisterStreamCall(*ctx);
            ffparser->stop();
            ffparser->join();


            delete ffparser;
            ffparser =nullptr;
            
            if(ctx)
            delete ctx;
            ctx = nullptr;
            
            if(fragmp4_filter)        
             delete fragmp4_filter;
            fragmp4_filter = nullptr;
            
            //if(fragmp4_muxer)
            //delete fragmp4_muxer;
            //fragmp4_muxer = nullptr;
            
            if(info)
            delete info;
            info = nullptr;
            
            if(txt)
            delete txt;
            txt = nullptr;
        }



        av_frame_free(&avframe);
        av_packet_free(&videopkt);

        avcodec_close(cdc_ctx);
        avcodec_free_context(&cdc_ctx);
    
    }

    
}

//void VideoPacketSource::setPacketSource(PacketSignal* source)
//{
//    assert(!_source);
    // if (_source)
    //     _source->detach(packetSlot(this, &VideoPacketSource::onVideoCaptured));
    //_source = source;
//}


/*cricket::CaptureState VideoPacketSource::Start(const cricket::VideoFormat& format)
{
    LDebug("VideoPacketSource::Start")

    // NOTE: The requested format must match the input format until
    // we implememnt pixel format conversion and resizing inside
    // this class.
    RTC_CHECK(_captureFormat == format);
    if (capture_state() == cricket::CS_RUNNING) {
        LWarn("Start called when it's already started.")
        return capture_state();
    }

//    if (_source)
    //    _source->attach(packetSlot(this, &VideoPacketSource::onVideoCaptured));
    
    
    // using std::placeholders::_1;
//    _source= std::bind( &VideoPacketSource::onVideoCaptured, this, _1 );
    
    SetCaptureFormat(&format);
    return cricket::CS_RUNNING;
}


void VideoPacketSource::Stop()
{
    LDebug("Stop")
    if (capture_state() == cricket::CS_STOPPED) {
        LWarn("Stop called when it's already stopped.")
        return;
    }

//    if (_source)
  //      _source->detach(packetSlot(this, &VideoPacketSource::onVideoCaptured));

    SetCaptureFormat(nullptr);
    SetCaptureState(cricket::CS_STOPPED);
}
*/


void VideoPacketSource::run(fmp4::Frame *frame)
{
    
  
    static uint frameNo = 0;
    
    int64_t TimestampUs = rtc::TimeMicros();
       
    fmp4::BasicFrame *basic_frame = static_cast<fmp4::BasicFrame *>(frame);
     
   // fragmp4_filter->run(basic_frame);   // arvind create /tmp/test.h264 files 
   

    int adapted_width;
    int adapted_height;
    int crop_width;
    int crop_height;
    int crop_x;
    int crop_y;
 
 
    
    #if BYPASSGAME

	//rtc::scoped_refptr<webrtc::I420Buffer> Buffer =
	//	webrtc::I420Buffer::Create(720,576);
        
             int ret = 0;
             int got_picture = 0;
             
            // basic_frame->fillAVPacket(videopkt);
             
//             if ((ret = av_parser_parse2(parser, cdc_ctx, &videopkt->data, &videopkt->size,
//                  basic_frame->payload.data(), basic_frame->payload.size(), AV_NOPTS_VALUE, AV_NOPTS_VALUE, 0)) < 0) {
//                        SError << "av_parser_parse2 failed" ;;
//                        //goto ret8;
//               }
             
//                printf("[Packet]Size:%6d\t", videopkt->size);
//                    switch (parser->pict_type) {
//                        case AV_PICTURE_TYPE_I: printf("Type:I\t");
//                            break;
//                        case AV_PICTURE_TYPE_P: printf("Type:P\t");
//                            break;
//                        case AV_PICTURE_TYPE_B: printf("Type:B\t");
//                            break;
//                        default: printf("Type:Other\t");
//                            break;
//                    };
//                    printf("Number:%4d\n", parser->output_picture_number);
//                    
//		
             basic_frame->fillAVPacket(videopkt);

             while (videopkt->size > 0)
             {
                    ret = avcodec_decode_video2(cdc_ctx, avframe, &got_picture, videopkt);
                    if (ret < 0) {
                       basic_frame->fillPars();
                       fmp4::InfoFrameFilter tmp("VideoPacketSource", nullptr);
                       tmp.run( basic_frame);
                       SError << "Decode Error" ;

                       return ;
                   }
                   if (got_picture) 
                   {


                       //const char *pixelFmt = av_get_pix_fmt_name(cdc_ctx->pix_fmt);

                       //const char *pixelFm2t = av_get_pix_fmt_name((AVPixelFormat)avframe->format);


                     //  SInfo << "Found Frame" ;

                      if (!AdaptFrame(avframe->width, avframe->height,
                           TimestampUs, //rtc::TimeNanos() / rtc::kNumNanosecsPerMicrosec,
                           &adapted_width, &adapted_height,
                           &crop_width, &crop_height,
                           &crop_x, &crop_y)) {
                           //LWarn("Adapt frame failed", packet.time)
                           return;
                       }



                       rtc::scoped_refptr<webrtc::I420Buffer> Buffer = webrtc::I420Buffer::Copy(
                       avframe->width, avframe->height,
                       avframe->data[0],avframe->linesize[0],
                       avframe->data[1], avframe->linesize[1],
                       avframe->data[2], avframe->linesize[2]);


                       webrtc::VideoFrame Frame = webrtc::VideoFrame::Builder().
                       set_video_frame_buffer(Buffer).
                       set_rotation(webrtc::kVideoRotation_0).
                       set_timestamp_us(TimestampUs).
                       build(); 

                      // SDebug << "ideoPacketSource::OnFrame";

                       OnFrame(Frame);  //arvind


                   } // if found
                    
                    
                videopkt->size -= ret;
                videopkt->data += ret;
             
           }//while
                    
 
     #else

        
//        if (wrtc::PeerManager::exists(playerId)) {
//                LDebug("Peer already has session: ")
//                return;
//            }
//
//        auto conn = wrtc::PeerManager::get(playerId);
//        if (conn)
//        {
//          //  conn->push(playerId)  ;
//        }
        
         fmp4::BasicFrame *bframe = new  fmp4::BasicFrame();
         bframe->payload = basic_frame->payload;
         bframe->codec_id = basic_frame->codec_id; 
         bframe->fillPars();
         
         
             
       if ( bframe->h264_pars.frameType == H264SframeType::i && bframe->h264_pars.slice_type == H264SliceType::idr) //AUD Delimiter
       {
           frameNo = 1;
           peer->pushFrame(bframe);
       }
       else if (bframe->h264_pars.slice_type == H264SliceType::sps ||  bframe->h264_pars.slice_type == H264SliceType::pps) //AUD Delimiter
       {
          peer->pushFrame(bframe);
       }
       else if(frameNo )
       {
           peer->pushFrame(bframe);
       }else
       {
           return;      
       }
        
   
        rtc::scoped_refptr<FRawFrameBuffer> Buffer = new rtc::RefCountedObject<FRawFrameBuffer>(peer,frameNo);
	
     #endif

    
                
     
        
    
    return ;
}

/*
bool VideoPacketSource::IsRunning()
{
    return capture_state() == cricket::CS_RUNNING;
}


bool VideoPacketSource::GetPreferredFourccs(std::vector<uint32_t>* fourccs)
{
    if (!fourccs)
        return false;

    // This class does not yet support multiple pixel formats.
    fourccs->push_back(_captureFormat.fourcc);
    return true;
}


bool VideoPacketSource::GetBestCaptureFormat(const cricket::VideoFormat& desired, cricket::VideoFormat* best_format)
{
    if (!best_format)
        return false;

    // Use the supported format as the best format.
    // best_format->width = desired.width;
    // best_format->height = desired.height;
    // best_format->fourcc = _codec;
    // best_format->interval = desired.interval;

    *best_format = _captureFormat;
    return true;
}


bool VideoPacketSource::IsScreencast() const
{
    return false;
}
*/


void VideoPacketSource::myAddRef(  std::string peerid)  {
   
    mutexVideoSoure.lock();
    
    setPeerid.insert(peerid);
    
    mutexVideoSoure.unlock();
 // const int count =   rtc::AtomicOps::Increment(&ref_count_);  //arvind
 // SInfo << "VideoPacketSource::AddRef()" << count;
  
}

rtc::RefCountReleaseStatus VideoPacketSource::myRelease(  std::string peerid )  {
    
    std::set< std::string> ::iterator itr;
    int count =1;
    
    mutexVideoSoure.lock();
    itr = setPeerid.find(peerid);
    
    if( itr != setPeerid.end())
    {
        setPeerid.erase(itr);
    }
    
    count = setPeerid.size();
    mutexVideoSoure.unlock();
    
    
  
  
    SInfo << "VideoPacketSource::Release()" << count;
    
   if (count == 0) {
     
     return rtc::RefCountReleaseStatus::kDroppedLastRef;
   }
  return rtc::RefCountReleaseStatus::kOtherRefsRemained;
}




 void VideoPacketSource::reset(  std::set< std::string> & peeerids )  {
    
    std::set< std::string> tmp;
    mutexVideoSoure.lock();
   
    peeerids =    setPeerid;
    
    setPeerid.clear();
    
    mutexVideoSoure.unlock();
    
}


webrtc::MediaSourceInterface::SourceState VideoPacketSource::state() const {
  return kLive;
}

bool VideoPacketSource::remote() const {
  return false;
}

bool VideoPacketSource::is_screencast() const {
  return false;
}

absl::optional<bool> VideoPacketSource::needs_denoising() const {
  return false;
}

} } // namespace wrtc


#endif // HAVE_FFMPEG
