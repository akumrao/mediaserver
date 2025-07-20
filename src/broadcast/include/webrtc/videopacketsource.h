
#ifndef WebRTC_VideoPacketSource_H
#define WebRTC_VideoPacketSource_H


#include "base/base.h"
#include "NULLDecoder.h"



#if MP4File
#include "ff/packet.h"
#endif
//#include "media/base/videocapturer.h"

#include "base/packet.h"
#include "webrtc/peer.h"
#include "media/base/adapted_video_track_source.h"
#include "rtc_base/timestamp_aligner.h"

#include "api/video/i420_buffer.h"

#include "framefilter.h"
#include "NV_Decoder.h"
#include "livethread.h"


extern "C"
{

  #include <libswscale/swscale.h>
}

#if BYPASSGAME
#include "base/thread.h"
#endif
//#include "pipelinethread.h"
#include "webrtc/rawVideoFrame.h"


namespace base {
namespace web_rtc {


/// VideoPacketSource implements a simple `cricket::VideoCapturer` that
/// gets decoded remote video frames from a local media channel.
/// It's used as the remote video source's `VideoCapturer` so that the remote
/// video can be used as a `cricket::VideoCapturer` and in that way a remote
/// video stream can implement the `MediaStreamSourceInterface`.
class VideoPacketSource : public rtc::AdaptedVideoTrackSource, public web_rtc::FrameFilter
#if BYPASSGAME
, public base::Thread
#endif

{ 

public:                                                                
      VideoPacketSource(const char *name, st_track &trackInfo, web_rtc::FrameFilter *next = NULL);

protected:
    void go(web_rtc::Frame *frame)
    {
        
    }

public:
  
  //  void run();
    void run(web_rtc::Frame  *frame);
    
    void runNULLEnc(web_rtc::Frame  *frame);
   
public:
    st_track trackInfo;
   
   // VideoPacketSource(const cricket::VideoFormat& captureFormat);
    virtual ~VideoPacketSource();

    /// Set the source `av::VideoPacket` emitter.
    /// The pointer is not managed by this class.
    //arvind
//    void setPacketSource(PacketSignal* source);


    /// cricket::VideoCapturer implementation.

//    void myAddPeerRef(std::string peerid);
//    rtc::RefCountReleaseStatus myReleasePeer( std::string peerid);
//    void resetPeer(  std::set< std::string> & peerids ); 
    SourceState state() const override;
    bool remote() const override;
    bool is_screencast() const override;
    absl::optional<bool> needs_denoising() const override;
    
    
    
    
    void oncommand( std::string & cmd, int arg1,  int arg2);
    
    void onAnswer();
    
    
    NULLDecoder *nullDecoder{nullptr};

//    std::set< std::string> setPeerId;
//    std::mutex mtPeerId;  
private:
    
   // std::mutex mutextxt;
  //  std::string metaData;
      
    
    
    std::mutex mtTrackId;
    
    bool reset_sws_cts{false};


    //void decodeFrame(uint8_t* data, int size);
    
   // std::string res{"HD"};
    
    struct SwsContext *sws_ctx{nullptr};
    
    uint8_t *dst_data[4];
    int dst_linesize[4];
    
   // mutable volatile int ref_count_;
   // std::string playerId;

protected:
    webrtc::VideoRotation _rotation;
    int64_t _timestampOffset;

    H264_Decoder *decoder{nullptr};
    
         

private:
    
    AVCodec *codec{nullptr};
    AVCodecContext *cdc_ctx{nullptr};
   // AVPacket *videopkt{nullptr};   
    AVFrame *avframe{nullptr};
    AVCodecParserContext *parser{nullptr};
    void StartParser(AVCodecID codeID);
    void StopParser(); 
    
    std::vector<uint8_t> buffer;

};




} } // namespace :web_rtc


#endif


