
#include "webrtc/multiplexmediacapturer.h"

//#ifdef HAVE_FFMPEG

#if MP4File
#include "ff/audioresampler.h"
#include "ff/ffmpeg.h"
#endif



#include "base/filesystem.h"
#include "base/logger.h"
#include "webrtc/webrtc.h"
//#include "media/engine/webrtcvideocapturerfactory.h"
#include "modules/video_capture/video_capture_factory.h"
#include <random>


#include "api/media_stream_interface.h"
#include "api/video/video_sink_interface.h"



const char kStreamId[] = "stream_id";

namespace base {
namespace wrtc {

   
    
MultiplexMediaCapturer::MultiplexMediaCapturer(): 
#if MP4File
_videoCapture(std::make_shared<ff::MediaCapture>()),
#endif
//  _audioModule(AudioPacketModule::Create()), 
PlayerID(0)
{
     using std::placeholders::_1;
   // _stream.attachSource(_videoCapture, true);
    // _stream.attach(std::make_shared<av::RealtimePacketQueue<av::MediaPacket>>(0), 5);
    // _stream.attach(std::make_shared<av::RealtimePacketQueue<av::PlanarVideoPacket>>(0), 5);
  //  _stream.emitter += packetSlot(_audioModule.get(), &AudioPacketModule::onAudioCaptured);
    #if MP4File
    ff::MediaCapture::function_type var = std::bind(&AudioPacketModule::onAudioCaptured , _audioModule.get(), _1);
    
    _videoCapture->cbProcessAudio.push_back(var);
    #endif

//      local_video_observer_.reset(new VideoObserver());
}


MultiplexMediaCapturer::~MultiplexMediaCapturer()
{
}


void MultiplexMediaCapturer::openFile(const std::string& dir, const std::string& file,  const std::string& type , bool loop)
{
    
    #if MP4File

    // Open the capture file
    _videoCapture->setLoopInput(loop);
    _videoCapture->setLimitFramerate(true);

    if(!dir.empty())         
        _videoCapture->openDir(dir,type);
    else if( !file.empty() )
       _videoCapture->openFile(dir + "/"+ file , type);



    // Set the output settings
    if (_videoCapture->audio()) {
        _videoCapture->audio()->oparams.sampleFmt = "s16";
        _videoCapture->audio()->oparams.sampleRate = 48000;
        _videoCapture->audio()->oparams.channels = 2;
        _videoCapture->audio()->recreateResampler();
        // _videoCapture->audio()->resampler->maxNumSamples = 480;
        // _videoCapture->audio()->resampler->variableOutput = false;
    }

    // Convert to yuv420p for WebRTC compatability
    if (_videoCapture->video()) {
        _videoCapture->video()->oparams.pixelFmt = "yuv420p"; // nv12
        // _videoCapture->video()->oparams.width = capture_format.width;
        // _videoCapture->video()->oparams.height = capture_format.height;
    }
    
     #endif
}


//VideoPacketSource* MultiplexMediaCapturer::createVideoSource()
//{
//    using std::placeholders::_1;
//    assert(_videoCapture->video());
//    auto oparams = _videoCapture->video()->oparams;
//    //auto source = new VideoPacketSource();
//    
//    _videoCapture->cbProcessVideo = std::bind(&VideoPacketSource::onVideoCaptured ,source , _1);
//    
////    source->setPacketSource(&_stream.emitter); // nullified on VideoPacketSource::Stop
//    
//    
//    return source;
//}


std::string MultiplexMediaCapturer::random_string()
{
//    std::string str("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
//
//    std::random_device rd;
//    std::mt19937 generator(rd());
//
//    std::shuffle(str.begin(), str.end(), generator);
//
//    return str.substr(0, 8);    // assumes 32 < number of characters in str
    
   std::string str = std::to_string(++PlayerID);
 
    return str;
   
}

//rtc::scoped_refptr<AudioPacketModule> MultiplexMediaCapturer::getAudioModule()
//{
//   return _audioModule;
//}


void MultiplexMediaCapturer::addMediaTracks(
    webrtc::PeerConnectionFactoryInterface* factory,
     webrtc::PeerConnectionInterface* conn , wrtc::Peer *peer)
{

    SInfo  << "MultiplexMediaCapturer::addMediaTracks" ; 
    

 //  std::string rnd=   random_string();

//  std::string audioLable = kAudioLabel + rnd;
//  std::string videoLable = kVideoLabel + rnd;
//  std::string streamId =  kStreamId + rnd;

  std::string audioLable = kAudioLabel;
  std::string videoLable = kVideoLabel;
  std::string streamId =  kStreamId;
  

  
  
  
  //if (ffparser)
  {
   //   using std::placeholders::_1;
//      assert(_videoCapture->video());
 //     auto oparams = _videoCapture->video()->oparams;
      //auto source = new VideoPacketSource();
      mutexCap.lock(); 
      std::string &cam = peer->getCam();
      if( VideoCapturer.find(cam) == VideoCapturer.end())
      {
         VideoCapturer[cam] = new rtc::RefCountedObject<VideoPacketSource>("VideoCapturer" , cam);
         video_track[cam] =     factory->CreateVideoTrack(videoLable, VideoCapturer[cam]);
       
        #if BYPASSGAME
        VideoCapturer[cam]->start();
        #endif
      }
      mutexCap.unlock();   
      
     
      
        video_track[cam]->set_enabled(true);
        conn->AddTrack(video_track[cam], {streamId});
         
        VideoCapturer[cam]->myAddRef(peer->peerid());
         
       
         
        
      
    }        
          
          

}



void MultiplexMediaCapturer::remove(wrtc::Peer* conn )
{
    
    std::string cam = conn->getCam();
    
    
    
    std::vector<rtc::scoped_refptr<webrtc::RtpSenderInterface>> senders =     conn->_peerConnection->GetSenders();


    for (const auto& sender : senders) {
        
        conn->_peerConnection->RemoveTrack(sender);
     
    }
  
     

    std::map< std::string, rtc::scoped_refptr<VideoPacketSource> > ::iterator vsItr;
    vsItr = VideoCapturer.find(cam);
    if (vsItr != VideoCapturer.end() && (rtc::RefCountReleaseStatus::kDroppedLastRef == VideoCapturer[cam]->myRelease(conn->peerid()))) {

        #if BYPASSGAME
        VideoCapturer[cam]->stop();
        VideoCapturer[cam]->join();
        #endif
        
        mutexCap.lock();
//        std::map< std::string, rtc::scoped_refptr<webrtc::VideoTrackInterface> >::iterator it;
//        it = video_track.find(cam);
//        if (it != video_track.end()) {
//            SInfo << "MultiplexMediaCapturer::stop()1 cam " << cam;
//         //   video_track[cam]->Release();
//            video_track.erase(it);
//        }
        
        
        SInfo << "VideoCapturer::stop() cam " << cam;
        video_track.erase(cam);
        //video_track[cam]->Release();
       // VideoCapturer[cam]->Release();
       VideoCapturer.erase(vsItr);
        mutexCap.unlock();


    }
 
     
    
}
void MultiplexMediaCapturer::stop(std::string & cam , std::set< std::string> & sPeerIds )
{
                 
   std::map< std::string ,  rtc::scoped_refptr<VideoPacketSource> > ::iterator vsItr;
   vsItr=VideoCapturer.find(cam);
    
   if( vsItr != VideoCapturer.end()  )
   {
       VideoCapturer[cam]->reset( sPeerIds);
   }

  
}


} } // namespace wrtc


//#endif // HAVE_FFMPEG
