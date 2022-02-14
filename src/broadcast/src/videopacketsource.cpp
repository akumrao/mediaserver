

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
#include "base/platform.h"

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
    
    SInfo << " VideoPacketSource " << this;
    
    #if BYPASSGAME
    StartParser();
    #else
     StartParser(); 
     StartLive(); 
    #endif
 
}

void VideoPacketSource::StopLive()
{
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
    }
    
}

void VideoPacketSource::StartParser()
{
 
     
             
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

    if(codec->capabilities & CODEC_CAP_TRUNCATED) {
        cdc_ctx->flags |= CODEC_FLAG_TRUNCATED;
    }

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

  
}


void VideoPacketSource::StartLive()
{
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


VideoPacketSource::~VideoPacketSource()
{
    SInfo << "~VideoPacketSource " << this;
    

    #if BYPASSGAME
    StopParser();
    #else
    StopLive();
    StopParser();
    #endif

}

void VideoPacketSource::StopParser()
{
    SInfo << "stopParser";
 
        
    if(parser) {
        av_parser_close(parser);
        parser = NULL;
    }

    if(cdc_ctx) {
      avcodec_close(cdc_ctx);
       avcodec_free_context(&cdc_ctx);
      av_free(cdc_ctx);
      cdc_ctx = NULL;
    }

    if(avframe) {
      av_frame_free(&avframe);
      av_free(avframe);
      avframe = NULL;
    }

    #if BYPASSGAME
      if(fp) {
        fclose(fp);
        fp = NULL;
      }
    #endif
    
    

    SInfo << "stoppedParser";
}




#if BYPASSGAME
void VideoPacketSource::run()
{
    
    load( "/var/tmp/test.264", 30.0f);
        
   
    while(!this->stopped())
        
    {
#if(0)
      int64_t TimestampUs = rtc::TimeMicros();
      
      rtc::scoped_refptr<webrtc::I420Buffer> Buffer =
	webrtc::I420Buffer::Create(720,576);
        
         webrtc::VideoFrame Frame = webrtc::VideoFrame::Builder().
                       set_video_frame_buffer(Buffer).
                       set_rotation(webrtc::kVideoRotation_0).
                       set_timestamp_us(TimestampUs).
                       build(); 

                      // SDebug << "ideoPacketSource::OnFrame";

                       OnFrame(Frame);  //arvind
      base::sleep(40); 
#else
      readFrame();
       base::sleep(40); 
#endif
    }
    
      SInfo << "run end";
}

bool VideoPacketSource::load(std::string filepath, float fps) {
    
        
  

  fp = fopen(filepath.c_str(), "rb");

  if(!fp) {
    printf("Error: cannot open: %s\n", filepath.c_str());
    return false;
  }


  if(fps > 0.0001f) {
    frame_delay = (1.0f/fps) * 1000ull * 1000ull * 1000ull;
    //frame_timeout = rx_hrtime() + frame_delay;
  }

  // kickoff reading...
  readBuffer();

  return true;
}

bool VideoPacketSource::readFrame() {

  // uint64_t now = rx_hrtime();
  // if(now < frame_timeout) {
  //   return false;
  // }

  bool needs_more = false;

  while(!update(needs_more)) { 
    if(needs_more) {
      readBuffer();
    }
  }

  // it may take some 'reads' before we can set the fps
  if(frame_timeout == 0 && frame_delay == 0) {
    double fps = av_q2d(cdc_ctx->time_base);
    if(fps > 0.0) {
      frame_delay = fps * 1000ull * 1000ull * 1000ull;
    }
  }

  // if(frame_delay > 0) {
  //   frame_timeout = rx_hrtime() + frame_delay;
  // }

  return true;
}

int VideoPacketSource::readBuffer() {

  int bytes_read = (int)fread(inbuf, 1, H264_INBUF_SIZE, fp);

  if(bytes_read) {
    std::copy(inbuf, inbuf + bytes_read, std::back_inserter(buffer));
  }
  else
  {
      
 
        if(feof(fp))
        {

             if (fseek(fp, 0, SEEK_SET))
            return 0;
            return readBuffer() ;

        }
      
  }

  return bytes_read;
}


bool VideoPacketSource::update(bool& needsMoreBytes) {

  needsMoreBytes = false;

  if(!fp) {
    printf("Cannot update .. file not opened...\n");
    return false;
  }

  if(buffer.size() == 0) {
    needsMoreBytes = true;
    return false;
  }

  uint8_t* data = NULL;
  int size = 0;
  int len = av_parser_parse2(parser, cdc_ctx, &data, &size, 
                             &buffer[0], buffer.size(), 0, 0, AV_NOPTS_VALUE);

  if(size == 0 && len >= 0) {
    needsMoreBytes = true;
    return false;
  }

  if(len) {
    decodeFrame(&buffer[0], size);
    buffer.erase(buffer.begin(), buffer.begin() + len);
    return true;
  }

  return false;
}

#endif




void VideoPacketSource::decodeFrame(uint8_t* data, int size) {

        int adapted_width;
        int adapted_height;
        int crop_width;
        int crop_height;
        int crop_x;
        int crop_y;


        AVPacket pkt;
        int got_picture = 0;
        int len = 0;

        av_init_packet(&pkt);

        pkt.data = data;
        pkt.size = size;

        len = avcodec_decode_video2(cdc_ctx, avframe, &got_picture, &pkt);
        if (len < 0) {
            SWarn <<" Error while decoding a frame for cam " <<  cam;
        }

        if (got_picture == 0) {
            return;
        }
        
        
         int64_t TimestampUs = rtc::TimeMicros();


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
                avframe->data[0], avframe->linesize[0],
                avframe->data[1], avframe->linesize[1],
                avframe->data[2], avframe->linesize[2]);


        webrtc::VideoFrame Frame = webrtc::VideoFrame::Builder().
                set_video_frame_buffer(Buffer).
                set_rotation(webrtc::kVideoRotation_0).
                set_timestamp_us(TimestampUs).
                build();

        // SDebug << "ideoPacketSource::OnFrame";

        OnFrame(Frame); 


            //  ++frame;

            //  if(cb_frame) {
            //    cb_frame(picture, &pkt, cb_user);
            //  }
}

void VideoPacketSource::run(fmp4::Frame *frame)
{
    
  
    static uint frameNo = 0;
    

       
    fmp4::BasicFrame *basic_frame = static_cast<fmp4::BasicFrame *>(frame);
     
   // fragmp4_filter->run(basic_frame);   // arvind create /tmp/test.h264 files 
   

 
 


	//rtc::scoped_refptr<webrtc::I420Buffer> Buffer =
	//	webrtc::I420Buffer::Create(720,576);
        

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
            // basic_frame->fillAVPacket(videopkt);

        uint8_t* data = NULL;
        int size = 0;
            
        std::copy(basic_frame->payload.data(), basic_frame->payload.data() +  basic_frame->payload.size(), std::back_inserter(buffer));
    
           // int len = av_parser_parse2(parser, cdc_ctx, &data, &size, 
            //                           basic_frame->payload.data(),  basic_frame->payload.size(), 0, 0, AV_NOPTS_VALUE);
            
        int len = av_parser_parse2(parser, cdc_ctx, &data, &size,  &buffer[0], buffer.size(), 0, 0, AV_NOPTS_VALUE);

        if(size == 0 && len >= 0) {
          return ;
        }




        if(len) 
        {
            decodeFrame(&buffer[0], size);
            buffer.erase(buffer.begin(), buffer.begin() + len);
            return ;
        }
            
       
    
    return ;
}



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
