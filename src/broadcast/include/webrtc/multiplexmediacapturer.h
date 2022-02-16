

#ifndef WebRTC_MultiplexMediaCapturer_H
#define WebRTC_MultiplexMediaCapturer_H


#include "base/base.h"

//#ifdef HAVE_FFMPEG


#if MP4File
#include "ff/ff.h"
#include "ff/mediacapture.h"
#endif


#include "webrtc/audiopacketmodule.h"
#include "webrtc/videopacketsource.h"

#include "api/peer_connection_interface.h"




namespace base {
namespace wrtc {


//class VideoObserver : public rtc::VideoSinkInterface<webrtc::VideoFrame> {
// public:
//  VideoObserver() {}
//  ~VideoObserver() {}
//  //oid SetVideoCallback(I420FRAMEREADY_CALLBACK callback);
//
// protected:
//  // VideoSinkInterface implementation
//  void OnFrame(const webrtc::VideoFrame& frame) 
//  {
//      int x = 1;
//  }
//
// private:
// 
//  //std::mutex mutex;
//};

    
    
class MultiplexMediaCapturer
{
public:
    MultiplexMediaCapturer();
    ~MultiplexMediaCapturer();

    void openFile(const std::string& dir, const std::string& file,  const std::string& type , bool loop=true);
    
    std::string random_string();

    void addMediaTracks(webrtc::PeerConnectionFactoryInterface* factory,
                        webrtc::PeerConnectionInterface* conn, wrtc::Peer *peer);

    //void start(std::string & cam );
    void stop(std::string & cam ,  std::set< std::string> & tmp);
    
    void remove(wrtc::Peer* conn );
    

    rtc::scoped_refptr<AudioPacketModule> getAudioModule();
    //VideoPacketSource* createVideoSource();

protected:
//    PacketStream _stream;
    #if MP4File
    ff::MediaCapture::Ptr _videoCapture;
    #endif

    
    
    
    //rtc::scoped_refptr<AudioPacketModule> _audioModule;
public:    
    
     std::map< std::string ,  rtc::scoped_refptr<VideoPacketSource> > VideoCapturer;
    
 protected:
     
    rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track;
    
     std::map< std::string,  rtc::scoped_refptr<webrtc::VideoTrackInterface> > video_track;
    
     //std::unique_ptr<VideoObserver> local_video_observer_;
    
    int PlayerID;
    
    


     
 private:
     
       
    std::string fileName;
     
    std::mutex mutexCap;
};


} } // namespace wrtc


//#endif // HAVE_FFMPEG
#endif

