

#ifndef WebRTC_MultiplexMediaCapturer_H
#define WebRTC_MultiplexMediaCapturer_H


#include "base/base.h"

#ifdef HAVE_FFMPEG


#if MP4File
#include "ff/ff.h"
#include "ff/mediacapture.h"
#endif


#include "webrtc/audiopacketmodule.h"
#include "webrtc/videopacketsource.h"

#include "api/peer_connection_interface.h"



namespace base {
namespace wrtc {


class MultiplexMediaCapturer
{
public:
    MultiplexMediaCapturer();
    ~MultiplexMediaCapturer();

    void openFile(const std::string& dir, const std::string& file,  const std::string& type , bool loop=true);
    
    std::string random_string();

    void addMediaTracks(webrtc::PeerConnectionFactoryInterface* factory,
                        webrtc::PeerConnectionInterface* conn);

    void start();
    void stop();

    rtc::scoped_refptr<AudioPacketModule> getAudioModule();
    //VideoPacketSource* createVideoSource();

protected:
//    PacketStream _stream;
    #if MP4File
    ff::MediaCapture::Ptr _videoCapture;
    #endif
    
    rtc::scoped_refptr<AudioPacketModule> _audioModule;
    
    rtc::scoped_refptr<VideoPacketSource> VideoCapturer;
    
    
      rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track;
      rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track;
      
    
    int PlayerID;
};


} } // namespace wrtc


#endif // HAVE_FFMPEG
#endif

