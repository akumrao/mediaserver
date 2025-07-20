

#ifndef WebRTC_LocalAudioSource_H
#define WebRTC_LocalAudioSource_H

#include "framefilter.h"
//#include "pipelinethread.h"
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "api/peer_connection_interface.h"
#include "pc/local_audio_source.h"

#include "livethread.h"
#include "muxframe.h"


namespace base {
namespace web_rtc {

class LocalAudioSource : public webrtc::LocalAudioSource, public base::web_rtc::FrameFilter 
{
public:
  

//    static rtc::scoped_refptr<LocalAudioSource> Create(const std::string& sTrackName, const cricket::AudioOptions& audio_options)
//    {
//        rtc::scoped_refptr<LocalAudioSource> source(
//            new rtc::RefCountedObject<LocalAudioSource>(sTrackName, audio_options));
//        return source;
//    }

    const cricket::AudioOptions options() const override { return m_Options; }

    void AddSink(webrtc::AudioTrackSinkInterface* sink) override 
    {
        sink_lock_.lock();
        sinks_.push_back(sink);
        sink_lock_.unlock();
    }
    void RemoveSink(webrtc::AudioTrackSinkInterface* sink) override
    {
         sink_lock_.lock();
         sinks_.remove(sink);
         sink_lock_.unlock();
    }

    void OnData(const void* pAudioData, int nBitPerSample, int nSampleRate, size_t nNumChannels, size_t nNumFrames)
    {
        
            
      sink_lock_.lock();
        for (auto* sink : sinks_) {
          // When peerconnection acts as an audio source, it should not provide
          // absolute capture timestamp.
          sink->OnData(pAudioData, nBitPerSample, nSampleRate, nNumChannels, nNumFrames);
        }
      sink_lock_.unlock();
    }
protected:

    LocalAudioSource(const char *name, const cricket::AudioOptions& audio_options, std::string &camID,   base::web_rtc::FrameFilter *next = NULL) ;
    ~LocalAudioSource() override;
    
    
private:
    //std::string m_sTrackName;
    cricket::AudioOptions m_Options;
    
    std::list<webrtc::AudioTrackSinkInterface*> sinks_;
public:
    
    void onAnswer();
    
    //void start();
   // void stop();
  //  void run();
    
   std::string &camID;
    
protected:
    void go( base::web_rtc::Frame *frame)
    {
     }
    
    void run( base::web_rtc::Frame *frame);
    
public:    
    
    void myAddRef(std::string peerid);
    rtc::RefCountReleaseStatus myRelease( std::string peerid);
    void reset(  std::set< std::string> & peeerids );
    std::set< std::string> setPeerid;
    
    std::mutex mutexAudioSource;
    std::mutex  sink_lock_;
    
    void StartLive();
    void StopLive();
    
    LiveThread  *ffparser{nullptr};
    
    //web_rtc::DummyFrameFilter *fragmp4_filter{nullptr};
   // web_rtc::FrameFilter *fragmp4_muxer{nullptr};;
    web_rtc::FrameFilter *info{nullptr};;
    web_rtc::FrameFilter *txt{nullptr};;
    web_rtc::LiveConnectionContext *ctx{nullptr};
    
    void oncommand( std::string & cmd , int first,  int second);
     
    int slot{1}; 
    
    //std::mutex mutexbuf;
    std::vector<uint8_t> buffer;
    
};

}}

//#endif // HAVE_FFMPEG
#endif

