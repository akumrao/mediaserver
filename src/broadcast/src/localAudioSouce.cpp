
#include "webrtc/localAudioSouce.h"
#include <chrono>
#include <thread>

#include "base/logger.h"
#define tcprequest true

#include "system_wrappers/include/sleep.h"

#include "Settings.h"



namespace base {
namespace web_rtc {

  static const int kSamplesPerSecond = 8000;
    
//void LocalAudioSource::stop()
//{
//     SInfo << "LocalAudioSource::stop()";
//
//     base::Thread::stop();
//     join();
//}

// void LocalAudioSource::start()
// {
//     base::Thread::start();
// }


void LocalAudioSource::onAnswer()
{
   
               
    ffparser->start();

     ffparser->registerStreamCall(*ctx); // arvind
    ffparser->playStreamCall(*ctx);
    
   // ffparser->registerStream(ctx);
   // ffparser->playStream(ctx);  
}


//void LocalAudioSource::run()
//{
//       
//    static const uint8_t kNumberOfChannels = 1;
//    
//    static const size_t kNumberSamples = 80;
//   // static const size_t kBytesPerSample = sizeof (AudioPacketModule::Sample) * kNumberOfChannels;
//    static const size_t kBufferBytes = 160;//kNumberSamples * kBytesPerSample;
//
//
// //   uint8_t test[kBufferBytes];
//
//  
//
//   // FILE* in_file = fopen("/var/tmp/audio/out.ul", "rb");
//    
//    //uint8_t encoded[kBufferBytes];
//    
////    
////    memset( encoded, 'a',  kBufferBytes);
////    
////    encoded[0]  = '6';
////    encoded[81]  = '7';
////    encoded[82]  = '8';
////    encoded[159] = '9';
//    
//   
//    while(!stopped())
//    {
//       int64_t currentTime = rtc::TimeMillis();
//          
//        mutexbuf.lock();
//    
//        if( buffer.size() > 160)
//        {
//            this->OnData(&buffer[0], 16, kSamplesPerSecond,1, 80);
//
//            buffer.erase(buffer.begin(), buffer.begin() + 160);
//         }
//
//        mutexbuf.unlock();
//         
//         
//        int64_t deltaTimeMillis = rtc::TimeMillis() - currentTime;
//        
//        
//        if (deltaTimeMillis < 20)
//        {
//                webrtc::SleepMs(20 - deltaTimeMillis);
//        }
//        
//        
//    }
//   
/*    
    int sz= 0;
    while(!stopped())
    {
        if (( sz = fread(encoded, 1, kBufferBytes, in_file)) <= 0)
        {
             printf("Failed to read raw data! \n");


         }else if(feof(in_file))
         {
             fseek(in_file, 0, SEEK_SET);
//              std::cout << " end end" << std::endl << std::flush;
         }

    
        if(sz != kBufferBytes)
        {
            // std::cout << " second bad end " << std::endl << std::flush;
        }
    
        
        this->OnData(encoded, 16, kSamplesPerSecond,1, 80);
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        
    }

    fclose(in_file);

  */

//}

LocalAudioSource::LocalAudioSource(const char *name, const cricket::AudioOptions& audio_options, std::string &camID,  base::web_rtc::FrameFilter *next) :  m_Options(audio_options), camID(camID),  base::web_rtc::FrameFilter(name, next)
{
    
    StartLive();
}


LocalAudioSource::~LocalAudioSource() 
{
    sink_lock_.lock();
     sinks_.clear();
    sink_lock_.unlock();
    
    StopLive();
}    
   

void LocalAudioSource::myAddRef(  std::string peerid)  {
   
    mutexAudioSource.lock();
    
    setPeerid.insert(peerid);
    
    mutexAudioSource.unlock();
 // const int count =   rtc::AtomicOps::Increment(&ref_count_);  //arvind
 // SInfo << "VideoPacketSource::AddRef()" << count;
  
}

rtc::RefCountReleaseStatus LocalAudioSource::myRelease(  std::string peerid )  {
    
    std::set< std::string> ::iterator itr;
    int count =1;
    
    mutexAudioSource.lock();
    itr = setPeerid.find(peerid);
    
    if( itr != setPeerid.end())
    {
        setPeerid.erase(itr);
    }
    
    count = setPeerid.size();
    mutexAudioSource.unlock();
  
  
    SInfo << "AudioPacketSource::Release()" << count;

    if (count == 0) {

      return rtc::RefCountReleaseStatus::kDroppedLastRef;
    }
    return rtc::RefCountReleaseStatus::kOtherRefsRemained;
}


 void LocalAudioSource::reset(  std::set< std::string> & peeerids )  {
    
    std::set< std::string> tmp;
    mutexAudioSource.lock();
   
    peeerids =    setPeerid;
    
    setPeerid.clear();
    
    mutexAudioSource.unlock();
    
    //mutexAudioSource.lock();
    
    //setPeerid.insert(peerid);
    
    //mutexAudioSource.unlock();
    
    
}
 
 
 
 
 
 
 
 
 
void LocalAudioSource::StopLive()
{
    
//    stop();
            
    if(ffparser)
    {
        if(ffparser)
        {
            
            SInfo << "Stopping LocalAudioSource "  << ctx->cam;
            
            std::string state;
              

            
          //  Settings::configuration.rtsp[ctx->cam]["state"]= "stopped";
              
           // ffparser->stopStream(ctx);

            //ffparser->deregisterStream(ctx);
            ffparser->stop();
            ffparser->join();


            delete ffparser;
            ffparser =nullptr;
            
            if(ctx)
            delete ctx;
            ctx = nullptr;
//            
//            if(fragmp4_filter)        
//             delete fragmp4_filter;
//            fragmp4_filter = nullptr;
//            
            //if(fragmp4_muxer)
            //delete fragmp4_muxer;
            //fragmp4_muxer = nullptr;
            
//            if(info)
//            delete info;
//            info = nullptr;
            
            if(txt)
            delete txt;
            txt = nullptr;
        }
    }
    
}


void LocalAudioSource::StartLive()
{
   //fragmp4_filter = new web_rtc::DummyFrameFilter("fragmp4", cam.camid);
       // fragmp4_muxer = new web_rtc::FragMP4MuxFrameFilter("fragmp4muxer", fragmp4_filter);

  //  info = new web_rtc::InfoFrameFilter("info", nullptr);

     txt = new web_rtc::TextFrameFilter("txt", camID, nullptr);
  

   // web_rtc::FrameFilter *tmpVc =(web_rtc::FrameFilter *) VideoCapturer.get();

   // std::string  cam = peer->getCam();

    std::string add;


     if( Settings::getNodeState(camID, "rtsp" , add ))
    {
      //  ctx = new LiveConnectionContext(LiveConnectionType::rtsp, add, slot, camID, tcprequest, this , info, txt); // Request livethread to write into filter info
        
        //ffparser =  new LiveThread("live");
        
      //  Settings::setNodeAudioState(cam , "streaming" ); // arvind
         
       // ffparser->audio = true;  // arvind


    }
}

 
void  LocalAudioSource::run( base::web_rtc::Frame *frame){

     
    web_rtc::BasicFrame *basic_frame = static_cast<web_rtc::BasicFrame *> (frame);
     
    // mutexbuf.lock();
     
    std::copy(basic_frame->data, basic_frame->data + basic_frame->sz, std::back_inserter(buffer));
     
     
    while( buffer.size() >= 160)
    {
          this->OnData(&buffer[0], 16, kSamplesPerSecond,1, 80);

          buffer.erase(buffer.begin(), buffer.begin() + 160);
    }
     
   //  mutexbuf.unlock();
     
        
  //   SInfo <<  "LocalAudioSource::run  " <<   basic_frame->sz;
             
             
    
//    if (!codec) {
//        StartParser(basic_frame->codec_id);
//    }
//
//    uint8_t* data = NULL;
//    int size = 0;
//
//    std::copy(basic_frame->data, basic_frame->data + basic_frame->sz, std::back_inserter(buffer));

     
     //this->OnData(basic_frame->data, 16, 8000,1, 80);
 }
 
 

 void LocalAudioSource::oncommand( std::string & cmd , int first,  int second)
 {
      //  ffparser->pausedAudio(first);  //arvind
    
 }

 


}}
