
#include "webrtc/multiplexmediacapturer.h"

//#ifdef HAVE_FFMPEG


#include "base/filesystem.h"
#include "base/logger.h"
#include "webrtc/webrtc.h"
//#include "media/engine/webrtcvideocapturerfactory.h"
#include "modules/video_capture/video_capture_factory.h"
#include <random>
#include "Settings.h"

#include "api/media_stream_interface.h"
#include "api/video/video_sink_interface.h"

#include <chrono>
#include <thread>

#include "muxer.h"

const char kStreamId[] = "stream_id";

namespace base
{
namespace web_rtc
{
    
 extern RestApi *self;
    

MultiplexMediaCapturer::MultiplexMediaCapturer()
    :
#if MP4File
      _videoCapture(std::make_shared<ff::MediaCapture>()),
#endif
      _audioModule(AudioPacketModule::Create()),
      PlayerID(0)
{
    using std::placeholders::_1;
    // _stream.attachSource(_videoCapture, true);
    // _stream.attach(std::make_shared<av::RealtimePacketQueue<av::MediaPacket>>(0), 5);
    // _stream.attach(std::make_shared<av::RealtimePacketQueue<av::PlanarVideoPacket>>(0), 5);
    //  _stream.emitter += packetSlot(_audioModule.get(), &AudioPacketModule::onAudioCaptured);
#if MP4File
    ff::MediaCapture::function_type var
        = std::bind(&AudioPacketModule::onAudioCaptured, _audioModule.get(), _1);

    _videoCapture->cbProcessAudio.push_back(var);
#endif

    //      local_video_observer_.reset(new VideoObserver());
}


MultiplexMediaCapturer::~MultiplexMediaCapturer() {}


void MultiplexMediaCapturer::openFile(
    const std::string &dir, const std::string &file, const std::string &type, bool loop)
{
#if MP4File

    // Open the capture file
    _videoCapture->setLoopInput(loop);
    _videoCapture->setLimitFramerate(true);

    if (!dir.empty())
        _videoCapture->openDir(dir, type);
    else if (!file.empty())
        _videoCapture->openFile(dir + "/" + file, type);


    // Set the output settings
    if (_videoCapture->audio())
    {
        _videoCapture->audio()->oparams.sampleFmt = "s16";
        _videoCapture->audio()->oparams.sampleRate = 8000;
        _videoCapture->audio()->oparams.channels = 1;
        _videoCapture->audio()->recreateResampler();
        // _videoCapture->audio()->resampler->maxNumSamples = 480;
        // _videoCapture->audio()->resampler->variableOutput = false;
    }

    // Convert to yuv420p for WebRTC compatability
    if (_videoCapture->video())
    {
        _videoCapture->video()->oparams.pixelFmt = "yuv420p";  // nv12
        // _videoCapture->video()->oparams.width = capture_format.width;
        // _videoCapture->video()->oparams.height = capture_format.height;
    }

#endif
    
    
    json node =  Settings::getJsonNode();
                  
                 
    for (json::iterator it = node.begin(); it != node.end(); ++it)
    {
        std::string key;

        json value;

        if(node.is_object())
        {   
            key = it.key();
            value = it.value();


            if( value.find("recording") != value.end() )
            {
                
                if( value["recording"] == "on")
                {
                  mapLiveThread[key ]  = new stLiveThread(nullptr,  key, key, true );

                  SInfo << "Added camera for recording: " << key;
      
                 /////////////////////
                    
                }
                
            }
        }
    }
    
    
}


// VideoPacketSource* MultiplexMediaCapturer::createVideoSource()
//{
//     using std::placeholders::_1;
//     assert(_videoCapture->video());
//     auto oparams = _videoCapture->video()->oparams;
//     //auto source = new VideoPacketSource();
//
//     _videoCapture->cbProcessVideo = std::bind(&VideoPacketSource::onVideoCaptured ,source , _1);
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
    webrtc::PeerConnectionFactoryInterface *factory,
    webrtc::PeerConnectionInterface *conn,
    web_rtc::Peer *peer,
    st_track &trackInfo,
    std::string &room)
{
    
    std::string camID = trackInfo.camid;     
   // st_track tmptkInfo;
    
   
    mutexCap.lock();
    
    std::map< std::string,  stLiveThread*>::iterator it = mapLiveThread.find(camID);
             
    if( it !=  mapLiveThread.end())
    {

        mapLiveThread[camID ]->ctx->muLiveFrame.lock();
        
        std::map< std::string, FrameFilter* >::iterator itr =  mapLiveThread[camID ]->ctx->setLiveFrame.begin();
        
        while(  itr != mapLiveThread[camID ]->ctx->setLiveFrame.end() )
        {
            std::string trackid =  itr->first;
                    
            VideoPacketSource *src = (VideoPacketSource *)itr->second;
             
            if( src->trackInfo.encoder == "NATIVE" &&  trackInfo.encoder == "NATIVE")
            {
                trackInfo   = src->trackInfo;
                break;
            }
            else // check if width  mitmatch by more than half
            {
                //TBD
            }
             
           ++itr;
        }
        mapLiveThread[camID ]->ctx->muLiveFrame.unlock();
    }
    mutexCap.unlock();
    
    std::string trackid = trackInfo.getTrackId();
    SInfo << "MultiplexMediaCapturer::addMediaTracks " << trackid;


    std::string audioLable = trackid + "_aud";
    std::string videoLable = trackid;
    std::string streamId = kStreamId;


//    if (trackInfo.audio)
//    {
//        cricket::AudioOptions AudioSourceOptions;
//        AudioSourceOptions.echo_cancellation = false;
//        AudioSourceOptions.auto_gain_control = false;
//        AudioSourceOptions.noise_suppression = false;
//        // AudioSourceOptions.audio_jitter_buffer_enable_rtx_handling = true;
//        // AudioSourceOptions.audio_jitter_buffer_max_packets =true;
//        // AudioSourceOptions.audio_network_adaptor =true;
//
//
//        mutexCap.lock();
//
//        peer->mapcam[audioLable] = trackInfo;
//
//        if (mapAudioCapturer.find(audioLable) == mapAudioCapturer.end())
//        {
//            // mapAudioCapturer[audioLable] =   AudioPacketModule::Create();
//            // mapaudio_track[audioLable] =     factory->CreateAudioTrack(    audioLable,
//            // factory->CreateAudioSource( AudioSourceOptions));
//
//            mapAudioCapturer[audioLable] = new rtc::RefCountedObject<LocalAudioSource>(
//                "audioCapturer", AudioSourceOptions, peer->mapcam[audioLable]);
//            //  mapAudioCapturer[audioLable]->start();
//
//            mapaudio_track[audioLable] = factory->CreateAudioTrack(audioLable, mapAudioCapturer[audioLable]);
//
//            SInfo << "Added Audio track and started Audio thread " << audioLable;
//        }
//
//
//        mutexCap.unlock();
//        // mapaudio_track[audioLable]->set_enabled(true);
//        conn->AddTrack(mapaudio_track[audioLable], {trackid});
//        mapAudioCapturer[audioLable]->myAddRef(peer->peerid());
//    }


    // if (liveThread)
    {
        //   using std::placeholders::_1;
        //      assert(_videoCapture->video());
        //     auto oparams = _videoCapture->video()->oparams;
        // auto source = new VideoPacketSource();
        
       if( !peer->addtrackInfo( trackid,  trackInfo))
       {
           SError << peer->peerid() << " has " <<  trackid;
           return ;// already this track exist
       }
        
        mutexCap.lock();
        
       // std::string camID = trackInfo.camid;

        if (mapVideoSource.find(trackid) == mapVideoSource.end())
        {
          //  Settings::setNodeState(trackInfo.camid , "init" );
            mapVideoSource[trackid]
                = new rtc::RefCountedObject<VideoPacketSource>("mapVideoSource", trackInfo);
            
            if (mapvideo_track.find(trackid) == mapvideo_track.end())
            {
                mapvideo_track[trackid] = factory->CreateVideoTrack(videoLable, mapVideoSource[trackid]);
            }
            
            
            if( mapLiveThread.find(trackInfo.camid) ==  mapLiveThread.end())
            mapLiveThread[trackInfo.camid ]  = new stLiveThread(mapVideoSource[trackid], trackInfo.camid, trackid, false );
            else
            mapLiveThread[trackInfo.camid ]->addVideoPacketSource(trackid, mapVideoSource[trackid]) ;    

        }
      

        mutexCap.unlock();
        
        SInfo << "Added Cam: "<< trackInfo.camid  <<  " trackid: " << trackid;
  /*      
        
        std::vector<rtc::scoped_refptr<webrtc::RtpSenderInterface>> senders = conn->GetSenders();
        for (const auto &sender : senders)
        {
            rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track = sender->track();

            if (track && track->id() == trackid)
            {

               // conn->_peerConnection->RemoveTrack(sender);
                 //track->set_enabled(false);
                 //sender.get()->ReplaceTrack(null);
                // conn->_peerConnection->RemoveTrack(sender);
                // conn->_peerConnection->RemoveStream( conn->_peerConnection->remote_streams()->at(0) );
                sender->SetTrack(mapvideo_track[trackid]);
                 
                mapvideo_track[trackid]->set_enabled(true);
                //conn->AddTrack(mapvideo_track[trackid], {trackid});

                mapVideoSource[camID]->myAddPeerRef(peer->peerid());
                mapVideoSource[camID]->myAddTrackRef( trackid , trackInfo);
        
                return;
            }
        }
*/
        mapvideo_track[trackid]->set_enabled(true);
        conn->AddTrack(mapvideo_track[trackid], {trackid});

        mapLiveThread[trackInfo.camid ]->myAddPeerRef(peer->peerid());

    }
}

void MultiplexMediaCapturer::remove(web_rtc::Peer *conn)
{
    
    SInfo << "MultiplexMediaCapturer::remove peerid " << conn->peerid() ;
    

    std::vector<rtc::scoped_refptr<webrtc::RtpSenderInterface>> senders = conn->_peerConnection->GetSenders();

    for (const auto &sender : senders)
    {
        rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track = sender->track();
     
       // if (track && track->id() == trackId)
        {
            conn->_peerConnection->RemoveTrack(sender);
            // track->set_enabled(false);
            // sender.get()->ReplaceTrack(null);
            // conn->_peerConnection->RemoveTrack(sender);
            // conn->_peerConnection->RemoveStream( conn->_peerConnection->remote_streams()->at(0) );
            // sender->SetTrack(nullptr);
        }
   

     
   // st_track trackInfo;
   // mutexCap.lock();         
   // conn->gettrackInfo(trackId, trackInfo);
  //  mutexCap.unlock();
     
    std::string trackId = track->id();
    
    st_track trackInfo;
    
    
    mutexCap.lock();
    
    conn->gettrackInfo(trackId, trackInfo);
    
    std::string camID = trackInfo.camid;
    
    std::map< std::string,  stLiveThread*>::iterator it = mapLiveThread.find(camID);
             
    if( it !=   mapLiveThread.end())
    {
       
        // SInfo << "Delete Camera : " << cam  << " no " << parser[cam ]->refCount;

        if( (rtc::RefCountReleaseStatus::kDroppedLastRef == it->second->myReleasePeer(conn->peerid() )  ) && !it->second->removeVideoPacketSource(trackId) )
        {
            
                
            //mapLiveThread[camID ]->ctx->muRecFrame.lock();
            if(!mapLiveThread[camID ]->ctx->fragmp4_muxer)
            {
                delete mapLiveThread[camID ];
                mapLiveThread.erase(it);

            }
            //mapLiveThread[camID ]->ctx->muRecFrame.unlock();
        

            
            mapvideo_track.erase(trackId);
            mapVideoSource.erase(trackId);
        }

    }
 

   
    //mapvideo_track.erase(trackId);
    // mapvideo_track[cam]->Release();
    // mapVideoSource[cam]->Release();
    //mapVideoSource.erase(trackId);
    conn->mapcam.erase(trackId);
    mutexCap.unlock();
        

//    std::map<std::string, rtc::scoped_refptr<LocalAudioSource>>::iterator asItr;
//    asItr = mapAudioCapturer.find(trackId);
//    if (asItr != mapAudioCapturer.end()
//        && (rtc::RefCountReleaseStatus::kDroppedLastRef == mapAudioCapturer[trackId]->myRelease(conn->peerid())))
//    {
//        //        asItr->second->stop();
//
//        //  asItr->second->join();
//
//        mutexCap.lock();
//
//        SInfo << "mapAudioCapturer::stop() cam " << trackId;// << ", start time " << trackInfo.start << ", end time " << trackInfo.end;
//        mapaudio_track.erase(trackId);
//        mapAudioCapturer.erase(asItr);
//        conn->mapcam.erase(trackId);
//        mutexCap.unlock();
//    }
    
    }

}

void MultiplexMediaCapturer::mute(web_rtc::Peer *conn, std::string &trackId, std::string &cmd, bool act)
{
    // std::string cam = conn->getCam().camid;


    st_track trackInfo;
    
    conn->gettrackInfo(trackId, trackInfo);

    // std::string clubID = cam.getclubId();

    std::vector<rtc::scoped_refptr<webrtc::RtpSenderInterface>> senders = conn->_peerConnection->GetSenders();


    for (const auto &sender : senders)
    {
        rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track = sender->track();
        if (track && track->id() == trackId)
        {
            // SInfo <<  track->kind() << " " <<  trackid ;
            track->set_enabled(!act);

            oncommand(conn, trackId, cmd, act, 0);
        }
    }
}


//void MultiplexMediaCapturer::remove(web_rtc::Peer *conn, std::string &trackId)
//{
//
//    
////     for (const auto& transceiver : conn->_peerConnection->GetTransceivers()) 
////     {
////       
////        transceiver->SetDirection( webrtc::RtpTransceiverDirection::kInactive);
////     }
////     
//
//    st_track trackInfo;
//    mutexCap.lock();
//    
//    conn->gettrackInfo(trackId, trackInfo);
//    
//    std::string camID = trackInfo.camid;
//    
//    std::map< std::string,  stLiveThread*>::iterator it = mapLiveThread.find(camID);
//             
//    if( it !=   mapLiveThread.end())
//    {
//       
//        // SInfo << "Delete Camera : " << cam  << " no " << parser[cam ]->refCount;
//
//
//
//         {
//             delete mapLiveThread[camID ];
//
//             mapLiveThread.erase(it);
//         }
//
//    }
//    mutexCap.unlock();
//   
//
//    std::vector<rtc::scoped_refptr<webrtc::RtpSenderInterface>> senders = conn->_peerConnection->GetSenders();
//
//
//    for (const auto &sender : senders)
//    {
//        rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track = sender->track();
//     
//        if (track && track->id() == trackId)
//        {
//            conn->_peerConnection->RemoveTrack(sender);
//            // track->set_enabled(false);
//            // sender.get()->ReplaceTrack(null);
//            // conn->_peerConnection->RemoveTrack(sender);
//            // conn->_peerConnection->RemoveStream( conn->_peerConnection->remote_streams()->at(0) );
//            // sender->SetTrack(nullptr);
//        }
//    }
//
//     
//   // st_track trackInfo;
//   // mutexCap.lock();         
//   // conn->gettrackInfo(trackId, trackInfo);
//  //  mutexCap.unlock();
//     
//   // std::string camID = trackInfo.camid;
//    std::map<std::string, rtc::scoped_refptr<VideoPacketSource>>::iterator vsItr;
//    vsItr = mapVideoSource.find(trackId);
//    if (vsItr != mapVideoSource.end()
//        && (rtc::RefCountReleaseStatus::kDroppedLastRef == mapVideoSource[trackId]->myReleasePeer(conn->peerid())))
//    {
//        
// 
//        
//#if BYPASSGAME
//        mapVideoSource[cam]->stop();
//        mapVideoSource[cam]->join();
//#endif
//
//        mutexCap.lock();
//        //        std::map< std::string, rtc::scoped_refptr<webrtc::VideoTrackInterface> >::iterator it;
//        //        it = mapvideo_track.find(cam);
//        //        if (it != mapvideo_track.end()) {
//        //            SInfo << "MultiplexMediaCapturer::stop()1 cam " << cam;
//        //         //   mapvideo_track[cam]->Release();
//        //            mapvideo_track.erase(it);
//        //        }
//
//
//        SInfo << "mapVideoSource::stop() cam " << trackId;// << ", start time " << trackInfo.start << ", end time "    << trackInfo.end;
//        mapvideo_track.erase(trackId);
//        // mapvideo_track[cam]->Release();
//        // mapVideoSource[cam]->Release();
//        mapVideoSource.erase(vsItr);
//        conn->mapcam.erase(trackId);
//        mutexCap.unlock();
//    }
//
//
//    std::map<std::string, rtc::scoped_refptr<LocalAudioSource>>::iterator asItr;
//    asItr = mapAudioCapturer.find(trackId);
//    if (asItr != mapAudioCapturer.end()
//        && (rtc::RefCountReleaseStatus::kDroppedLastRef == mapAudioCapturer[trackId]->myRelease(conn->peerid())))
//    {
//        //        asItr->second->stop();
//
//        //  asItr->second->join();
//
//        mutexCap.lock();
//
//        SInfo << "mapAudioCapturer::stop() cam " << trackId;// << ", start time " << trackInfo.start << ", end time " << trackInfo.end;
//        mapaudio_track.erase(trackId);
//        mapAudioCapturer.erase(asItr);
//        conn->mapcam.erase(trackId);
//        mutexCap.unlock();
//    }
//}


void MultiplexMediaCapturer::stop(std::string & camID , std::set< std::string> & sPeerIds )
{

    SInfo << "MultiplexMediaCapturer::stop " << camID ;

    
    mutexCap.lock();
    
    std::map< std::string,  stLiveThread*>::iterator it = mapLiveThread.find(camID);
             
    if( it !=  mapLiveThread.end())
    {
           
        it->second->resetPeer(sPeerIds);
         
        // 
       
        mapLiveThread[camID ]->ctx->muLiveFrame.lock();
        
        
        std::map< std::string, FrameFilter* >::iterator itr =  mapLiveThread[camID ]->ctx->setLiveFrame.begin();
        
        while(  itr != mapLiveThread[camID ]->ctx->setLiveFrame.end() )
        {
            std::string trackid =  itr->first;
                    
            itr = mapLiveThread[camID ]->ctx->setLiveFrame.erase(itr);
            //tmp->run(&basicframe);
            mapvideo_track.erase(trackid);
            mapVideoSource.erase(trackid);
           
        }
        mapLiveThread[camID ]->ctx->muLiveFrame.unlock();

        delete mapLiveThread[camID ];
        mapLiveThread.erase(it);

    }
    mutexCap.unlock();

}


// void MultiplexMediaCapturer::getPeerids(st_track & cam , std::set< std::string> & sPeerIds )
//{
//    std::string trackid = cam.getTrackId();
//
//    std::map< std::string ,  rtc::scoped_refptr<VideoPacketSource> > ::iterator vsItr;
//    vsItr=mapVideoSource.find(trackid);
//
//    if( vsItr != mapVideoSource.end()  )
//    {
//        sPeerIds = mapVideoSource[trackid]->setPeerid;
//    }
//
// }


void MultiplexMediaCapturer::oncommand(
    web_rtc::Peer *conn, std::string &trackId, std::string &cmd, int arg1, int arg2)
{
    // std::string cam = conn->getCam().camid;

    st_track trackInfo;
    
    conn->gettrackInfo(trackId, trackInfo);

    std::string camID = trackInfo.camid;
       
    if (cmd == "muteaudio")
    {
        std::map<std::string, rtc::scoped_refptr<LocalAudioSource>>::iterator asItr;
        asItr = mapAudioCapturer.find(camID);
        if (asItr != mapAudioCapturer.end())
        {
            mutexCap.lock();
            SInfo << "mapAudioCapturer::oncommand() cam " << trackId << ", cmd " << cmd << ", " << arg1;
            asItr->second->oncommand(cmd, arg1, arg2);


            mutexCap.unlock();
        }
    }
    else
    {
        std::map<std::string, rtc::scoped_refptr<VideoPacketSource>>::iterator vsItr;
        vsItr = mapVideoSource.find(camID);
        if (vsItr != mapVideoSource.end())
        {
            mutexCap.lock();
            SInfo << "mapVideoSource::oncommand() cam " << trackId << ", cmd " << cmd << ", " << arg1;
            vsItr->second->oncommand(cmd, arg1, arg2);


            mutexCap.unlock();
        }
    }
}

void MultiplexMediaCapturer::onAnswer(
    web_rtc::Peer *conn, en_EncType &encType, std::string &vtrackId, std::string &atrackId)
{
    // std::string cam = conn->getCam().camid;
    st_track trackInfo;
    
    conn->gettrackInfo(vtrackId, trackInfo);

    std::string camid = trackInfo.camid;


    std::map<std::string, rtc::scoped_refptr<VideoPacketSource>>::iterator vsItr;
    vsItr = mapVideoSource.find(camid);
    if (vsItr != mapVideoSource.end())
    {
        mutexCap.lock();
        SInfo << "mapVideoSource::onAnswer() cam " << trackInfo.camid << ", encType " << encType;
        vsItr->second->onAnswer();
        mutexCap.unlock();
    }


    if (atrackId.size())
    {
        st_track trackInfo;
        conn->gettrackInfo(atrackId, trackInfo);
        std::string camid = trackInfo.camid;

        // std::string trackid = cam.getTrackId();
        std::map<std::string, rtc::scoped_refptr<LocalAudioSource>>::iterator asItr;
        asItr = mapAudioCapturer.find(camid);
        if (asItr != mapAudioCapturer.end())
        {
            mutexCap.lock();
            SInfo << "mapAudioCapturer::onAnswer() cam " << trackInfo.camid << ", encType "
                  << "PCMU";
            asItr->second->onAnswer();
            mutexCap.unlock();
        }
    }
}


MultiplexMediaCapturer::stLiveThread::stLiveThread( VideoPacketSource *src,  std::string& camID, std::string& trackid, bool recording )
{
    
    SInfo << "MultiplexMediaCapturer::stLiveThread::stLiveThread for cam:" << camID << " track:"  << trackid;
     
    
    //std::string recording;
   // if( Settings::getNodeState(camID, "recording" , recording ))
    
    json add;
    if( Settings::getJsonNodeState(camID,  add ) && add.find("rtsp") != add.end())
    {
        if(recording)
        {


            ctx = new web_rtc::LiveConnectionContext(LiveConnectionType::rtsp, add["rtsp"], slot, camID, trackid, Settings::configuration.tcpRtsp, nullptr  ); // Request livethread to write into filter info
            ctx->fragmp4_filter = new DummyFrameFilter("fragmp4", camID, nullptr);
            ctx->fragmp4_muxer = new FragMP4MuxFrameFilter("fragmp4muxer", ctx->fragmp4_filter);
            SInfo  <<   camID  << " " <<    "recording";
            liveThread = new LiveThread("live");
        }
        else
        {

            ctx = new web_rtc::LiveConnectionContext(LiveConnectionType::rtsp, add["rtsp"], slot, camID, trackid, Settings::configuration.tcpRtsp, src ) ; // Request livethread to write into filter info

                //liveThread->registerStreamCall(*ctx);
               // liveThread->playStreamCall(*ctx);
            Settings::setNodeState(camID , "streaming" );
            SInfo  <<   camID  << " " <<    "streaming";
            liveThread = new LiveThread("live");

        }
    }
    else
    {
        SError << "Could not find camera at Json Repository "  << camID; 
    }

    ctx->txt = new web_rtc::TextFrameFilter("txt", camID, self);
   // info = new web_rtc::InfoFrameFilter("info", nullptr);

    
    liveThread->start();

    liveThread->registerStreamCall(*ctx);
    liveThread->playStreamCall(*ctx);
    
}



void MultiplexMediaCapturer::stLiveThread::addVideoPacketSource(std::string& trackid, VideoPacketSource *src )
{
    SInfo << "MultiplexMediaCapturer::stLiveThread::addVideoPacketSource "  << trackid;
    ctx->addLiveFrameSource( trackid, src);
}

int MultiplexMediaCapturer::stLiveThread::removeVideoPacketSource(std::string & trackid )
{
    SInfo << "MultiplexMediaCapturer::stLiveThread::removeVideoPacketSource"  << trackid;
    return ctx->removeLiveFrameSource(trackid);
}
        
   
MultiplexMediaCapturer::stLiveThread::~stLiveThread()
{
    if(liveThread)
    {
        if(liveThread)
        {
            
            SInfo << "~stLiveThread( )"  << ctx->cam;
            
            std::string state;
              
            if( Settings::getNodeState(ctx->cam, "state" , state ))
            {
                if( state == "streaming"  &&  !Settings::setNodeState(ctx->cam , "stopped" ) )
                {
                     SError << "Could not find camera at Json Repository "  << ctx->cam; 
                }
                 
            }
            
           
           // liveThread->stopStream(ctx);

            //liveThread->deregisterStream(ctx);
            liveThread->stop();
            liveThread->join();


            delete liveThread;
            liveThread =nullptr;
            
            if(ctx->txt)
              delete ctx->txt;
            ctx->txt = nullptr;
            
            ctx->muRecFrame.lock();
            if(ctx->fragmp4_filter)
            {
                
                delete ctx->fragmp4_filter;
                ctx->fragmp4_filter = nullptr;
            }
//            
            if(ctx->fragmp4_muxer)
            delete ctx->fragmp4_muxer;
            ctx->fragmp4_muxer = nullptr;
            ctx->muRecFrame.unlock();
           
            
            if(ctx)
            delete ctx;
            ctx = nullptr;
            

            
//            if(info)
//            delete info;
//            info = nullptr;
//            

        }
    }
    
}

void MultiplexMediaCapturer::startRecording( std::string &cam )
{

    SInfo << "MultiplexMediaCapturer::startRecording " << cam ;

            
    mutexCap.lock();
    
    std::map< std::string,  stLiveThread*>::iterator it = mapLiveThread.find(cam);
             
    if( it !=  mapLiveThread.end())
    {
       
        mapLiveThread[cam ]->ctx->muRecFrame.lock();
        
        if(!mapLiveThread[cam ]->ctx->fragmp4_muxer)
        {
            
            mapLiveThread[cam ]->ctx->fragmp4_filter = new DummyFrameFilter("fragmp4", cam, nullptr);
            mapLiveThread[cam ]->ctx->fragmp4_muxer = new FragMP4MuxFrameFilter("fragmp4muxer", mapLiveThread[cam ]->ctx->fragmp4_filter);
                    
        }
            
        
        mapLiveThread[cam ]->ctx->muRecFrame.unlock();

        
        
    }
    else
    {
        mapLiveThread[cam ]  = new stLiveThread(nullptr,  cam, cam, true );
    }
    mutexCap.unlock();
    
    
}
    
void MultiplexMediaCapturer::stopRecording( std::string &cam )
{
    
    SInfo << "MultiplexMediaCapturer::stopRecording " << cam ;
     
    mutexCap.lock();
    
    std::map< std::string,  stLiveThread*>::iterator it = mapLiveThread.find(cam);
             
    if( it !=  mapLiveThread.end())
    {               
        mapLiveThread[cam ]->ctx->muRecFrame.lock();
        if(mapLiveThread[cam ]->ctx->fragmp4_muxer)
        {
            delete mapLiveThread[cam ]->ctx->fragmp4_filter; 
            delete mapLiveThread[cam ]->ctx->fragmp4_muxer; 
            
            mapLiveThread[cam ]->ctx->fragmp4_filter = nullptr;
            mapLiveThread[cam ]->ctx->fragmp4_muxer = nullptr;
                    
        }
        mapLiveThread[cam ]->ctx->muRecFrame.unlock();
        
        
        mapLiveThread[cam ]->ctx->muLiveFrame.lock();
        int size =  mapLiveThread[cam ]->ctx->setLiveFrame.size();
        mapLiveThread[cam ]->ctx->muLiveFrame.unlock();
        
        if(! size)
        {
            delete mapLiveThread[cam ];
            mapLiveThread.erase(it);
        }
        
    }
    mutexCap.unlock();
    
}
    


}  // namespace web_rtc
}  // namespace base
