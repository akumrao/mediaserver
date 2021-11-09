
#ifndef WebRTC_VideoPacketSource_H
#define WebRTC_VideoPacketSource_H


#include "base/base.h"
//#include "base/packetsignal.h"

#ifdef HAVE_FFMPEG

#if MP4File
#include "ff/packet.h"
#endif
//#include "media/base/videocapturer.h"

#include "base/packet.h"
#include "webrtc/peer.h"
#include "media/base/adapted_video_track_source.h"
#include "rtc_base/timestamp_aligner.h"
#include "framefilter.h"

#include "livethread.h"

namespace base {
namespace wrtc {


/// VideoPacketSource implements a simple `cricket::VideoCapturer` that
/// gets decoded remote video frames from a local media channel.
/// It's used as the remote video source's `VideoCapturer` so that the remote
/// video can be used as a `cricket::VideoCapturer` and in that way a remote
/// video stream can implement the `MediaStreamSourceInterface`.
class VideoPacketSource : public rtc::AdaptedVideoTrackSource, fmp4::FrameFilter
{ 

public:                                                                
      VideoPacketSource(const char *name,  wrtc::Peer *peer, fmp4::FrameFilter *next = NULL);

protected:
    void go(fmp4::Frame *frame)
    {
        
    }

public:
    void run(fmp4::Frame  *frame);
   
public:
    wrtc::Peer *peer;
   
   // VideoPacketSource(const cricket::VideoFormat& captureFormat);
    virtual ~VideoPacketSource();

    /// Set the source `av::VideoPacket` emitter.
    /// The pointer is not managed by this class.
    //arvind
//    void setPacketSource(PacketSignal* source);

    /// Callback that fired when an `av::PlanarVideoPacket`
    /// is ready for processing.
    int onVideoCaptured(IPacket& pac);

    /// cricket::VideoCapturer implementation.

    void myAddRef();
    rtc::RefCountReleaseStatus myRelease();
    SourceState state() const override;
    bool remote() const override;
    bool is_screencast() const override;
    absl::optional<bool> needs_denoising() const override;
    
private:
    mutable volatile int ref_count_;
    std::string playerId;

protected:
    cricket::VideoFormat _captureFormat;
    webrtc::VideoRotation _rotation;
    int64_t _timestampOffset;
    int64_t _nextTimestamp;
   // std::function<void(ff::PlanarVideoPacket& packet)>   _source;
    
    
    AVCodec *codec{nullptr};
    AVCodecContext *cdc_ctx;
    AVPacket *videopkt{nullptr};   
    AVFrame *avframe;
    AVCodecParserContext *parser;
    
    void stopParser();
         
     fmp4::LiveThread  *ffparser{nullptr};
    
     fmp4::DummyFrameFilter *fragmp4_filter{nullptr};
     fmp4::FrameFilter *fragmp4_muxer{nullptr};;
     fmp4::FrameFilter *info{nullptr};;
     fmp4::FrameFilter *txt{nullptr};;
     fmp4::LiveConnectionContext *ctx{nullptr};;
     int slot{1};    
};


// class VideoPacketSourceFactory : public cricket::VideoDeviceCapturerFactory {
// public:
//     VideoPacketSourceFactory() {}
//     virtual ~VideoPacketSourceFactory() {}
//
//     virtual cricket::VideoCapturer* Create(const cricket::Device& device) {
//         return new VideoPacketSource(device.name);
//     }
// };


} } // namespace :wrtc


#endif // HAVE_FFMPEG
#endif


