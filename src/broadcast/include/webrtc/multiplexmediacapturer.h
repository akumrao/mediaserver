

#ifndef WebRTC_MultiplexMediaCapturer_H
#define WebRTC_MultiplexMediaCapturer_H


#include "base/base.h"

#ifdef HAVE_FFMPEG

#include "ff/ff.h"
#include "ff/mediacapture.h"
#include "webrtc/audiopacketmodule.h"
#include "webrtc/videopacketsource.h"

#include "api/peer_connection_interface.h"





////////////////////////////////////////////////////////////////////////////////////////////////////

#include "pc/local_audio_source.h"




class MyLocalAudioSource : public webrtc::LocalAudioSource, public base::Thread
{
public:
  

    static rtc::scoped_refptr<MyLocalAudioSource> Create(const std::string& sTrackName, const cricket::AudioOptions& audio_options)
    {
        rtc::scoped_refptr<MyLocalAudioSource> source(
            new rtc::RefCountedObject<MyLocalAudioSource>(sTrackName, audio_options));
        return source;
    }

    const cricket::AudioOptions options() const override { return m_Options; }

    void AddSink(webrtc::AudioTrackSinkInterface* sink) override 
    {
        m_pAudioTrackSinkInterface = sink;
    }
    void RemoveSink(webrtc::AudioTrackSinkInterface* sink) override
    {
        m_pAudioTrackSinkInterface = 0;
    }

    void OnData(const void* pAudioData, int nBitPerSample, int nSampleRate, size_t nNumChannels, size_t nNumFrames)
    {
        if (m_pAudioTrackSinkInterface)
        {
            m_pAudioTrackSinkInterface->OnData(pAudioData, nBitPerSample, nSampleRate, nNumChannels, nNumFrames);
        }
    }
protected:

    MyLocalAudioSource(const std::string& sTrackName, const cricket::AudioOptions& audio_options) : m_sTrackName(sTrackName), m_Options(audio_options), m_pAudioTrackSinkInterface(0)
    {
    }
    ~MyLocalAudioSource() override {}

private:
    std::string m_sTrackName;
    cricket::AudioOptions m_Options;
    webrtc::AudioTrackSinkInterface* m_pAudioTrackSinkInterface;
    
    public:
    void start();
    void stop();
    void run();
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////






#include "base/thread.h"




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
    ff::MediaCapture::Ptr _videoCapture;
    rtc::scoped_refptr<AudioPacketModule> _audioModule;
    
    rtc::scoped_refptr<VideoPacketSource> VideoCapturer;
    
    
    rtc::scoped_refptr<MyLocalAudioSource> ausrc;
    
    
      rtc::scoped_refptr<webrtc::AudioTrackInterface> audio_track;
      rtc::scoped_refptr<webrtc::VideoTrackInterface> video_track;
      
    
    int PlayerID;
};


} } // namespace wrtc


#endif // HAVE_FFMPEG
#endif

