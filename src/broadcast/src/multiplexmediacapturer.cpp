
#include "webrtc/multiplexmediacapturer.h"

//#ifdef HAVE_FFMPEG

#if MP4File
#include "ff/audioresampler.h"
#include "ff/ffmpeg.h"
#endif

#include "muxer.h"
 #include "Settings.h"

#include "base/filesystem.h"
#include "base/logger.h"
#include "webrtc/webrtc.h"
//#include "media/engine/webrtcvideocapturerfactory.h"
#include "modules/video_capture/video_capture_factory.h"
#include <random>



const char kStreamId[] = "stream_id";

namespace base {
namespace wrtc {

MultiplexMediaCapturer::MultiplexMediaCapturer(): 
#if MP4File
_videoCapture(std::make_shared<ff::MediaCapture>()),
#endif
  _audioModule(AudioPacketModule::Create()), PlayerID(0)
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

rtc::scoped_refptr<AudioPacketModule> MultiplexMediaCapturer::getAudioModule()
{
   return _audioModule;
}


void MultiplexMediaCapturer::addMediaTracks(
    webrtc::PeerConnectionFactoryInterface* factory,
     webrtc::PeerConnectionInterface* conn)
{

    SInfo  << "MultiplexMediaCapturer::addMediaTracks" ; 
    
    // This capturer is multicast, meaning it can be used as the source
    // for multiple Peer objects.
    //
    // KLUDGE: Pixel format conversion should happen on the
    // VideoPacketSource rather than on the decoder becasue different
    // peers may request different optimal output video sizes.

    // Create and add the audio stream
    // if (_videoCapture->audio()) { //arvind
    //     stream->AddTrack(factory->CreateAudioTrack(
    //         kAudioLabel, factory->CreateAudioSource(nullptr)));
    // }

    // Create and add the video stream
    // if (_videoCapture->video()) {  //arvind
    //     stream->AddTrack(factory->CreateVideoTrack(
    //         kVideoLabel, factory->CreateVideoSource(createVideoSource(), nullptr)));
    // }

    // Default WebRTC video stream for testing
    // stream->AddTrack(factory->CreateVideoTrack(
    //     kVideoLabel, factory->CreateVideoSource(openVideoDefaultWebRtcCaptureDevice(), nullptr)));
    
    // stream->AddTrack(factory->CreateVideoTrack(
    //     kVideoLabel, factory->CreateVideoSource(openVideoDefaultWebRtcCaptureDevice(), nullptr)));
    
   std::string rnd=   random_string();

//  std::string audioLable = kAudioLabel + rnd;
//  std::string videoLable = kVideoLabel + rnd;
//  std::string streamId =  kStreamId + rnd;

  std::string audioLable = kAudioLabel;
  std::string videoLable = kVideoLabel;
  std::string streamId =  kStreamId;
  
#if MP4File
  if (_videoCapture->audio())
  {

    
      cricket::AudioOptions AudioSourceOptions;
      AudioSourceOptions.echo_cancellation = false;
      AudioSourceOptions.auto_gain_control = false;
      AudioSourceOptions.noise_suppression = false;
     // AudioSourceOptions.audio_jitter_buffer_enable_rtx_handling = true;
     // AudioSourceOptions.audio_jitter_buffer_max_packets =true;
      //AudioSourceOptions.audio_network_adaptor =true;
      
      if(!audio_track)
      audio_track =   factory->CreateAudioTrack(    audioLable, factory->CreateAudioSource( AudioSourceOptions));
   
    //stream->AddTrack(audio_track);
    // peer_connection_->AddTransceiver(audio_track);
      conn->AddTrack(audio_track, {streamId});
      
  } 


  if (_videoCapture->video())
  {
      using std::placeholders::_1;
      assert(_videoCapture->video());
      auto oparams = _videoCapture->video()->oparams;
      //auto source = new VideoPacketSource();
       VideoCapturer = new rtc::RefCountedObject<VideoPacketSource>(rnd);
       
      ff::MediaCapture::function_type var = std::bind(&VideoPacketSource::onVideoCaptured ,VideoCapturer , _1);

      _videoCapture->cbProcessVideo.push_back(var);
      

      
      if(!video_track)
        video_track =     factory->CreateVideoTrack(videoLable, VideoCapturer);
        
         video_track->set_enabled(true);
         conn->AddTrack(video_track, {streamId});
    }
#endif
  
  
  
  //if (ffparser)
  {
   //   using std::placeholders::_1;
//      assert(_videoCapture->video());
 //     auto oparams = _videoCapture->video()->oparams;
      //auto source = new VideoPacketSource();
       VideoCapturer = new rtc::RefCountedObject<VideoPacketSource>(rnd,"VideoCapturer" );
       
    //  ff::MediaCapture::function_type var = std::bind(&VideoPacketSource::onVideoCaptured ,VideoCapturer , _1);

    //  _videoCapture->cbProcessVideo.push_back(var);
      

      
      if(!video_track)
        video_track =     factory->CreateVideoTrack(videoLable, VideoCapturer);
        
         video_track->set_enabled(true);
         conn->AddTrack(video_track, {streamId});
    }        
          
          
      //stream->AddTrack(video_track);

//      video_track->set_enabled(true);
//      
//
//      if (local_video_observer_) {
//            video_track->AddOrUpdateSink(local_video_observer_.get(),
//                                                   rtc::VideoSinkWants());
//      }
//    }
    
}


void MultiplexMediaCapturer::start()
{
    #if MP4File
    //_stream.start
    _videoCapture->start();
    #endif
    
    SInfo << "MultiplexMediaCapturer::start()" ;
    
    
    
    fragmp4_filter = new fmp4::DummyFrameFilter("fragmp4", nullptr);
    fragmp4_muxer = new fmp4::FragMP4MuxFrameFilter("fragmp4muxer", fragmp4_filter);

    info = new fmp4::InfoFrameFilter("info", nullptr);

    txt = new fmp4::TextFrameFilter("txt", nullptr);


    ffparser = new fmp4::LiveThread("live");

    ffparser->start();

    fmp4::FrameFilter *tmpVc =(fmp4::FrameFilter *) VideoCapturer.get();
    

    ctx = new fmp4::LiveConnectionContext(fmp4::LiveConnectionType::rtsp, Settings::configuration.rtsp2, slot, false, tmpVc , info, txt); // Request livethread to write into filter info
    ffparser->registerStreamCall(*ctx);
    ffparser->playStreamCall(*ctx);
            
}

void MultiplexMediaCapturer::stop()
{
  // _stream.stop();
    //_videoCapture->stop();
    SInfo << "MultiplexMediaCapturer::stop()" ;
    
     ffparser->stopStreamCall(*ctx);

    ffparser->deregisterStreamCall(*ctx);
    ffparser->stop();
    ffparser->join();



    delete ffparser;
    delete ctx;
    delete fragmp4_filter;
    delete fragmp4_muxer;
    delete info;
    delete txt;
}


} } // namespace wrtc


//#endif // HAVE_FFMPEG
