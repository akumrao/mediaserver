

#include "webrtc/videopacketsource.h"
#include "webrtc/rawVideoFrame.h"

#ifdef HAVE_FFMPEG
#if MP4File
#include "ff/ffmpeg.h"
#include "ff/videocontext.h"
#include "ff/videodecoder.h"
#include "ff/fpscounter.h"
 #endif
#include "api/video/i420_buffer.h"
#include "rtc_base/ref_counted_object.h"
#include "rtc_base/atomic_ops.h"
#include <chrono>

using std::endl;

#define WebRTC_USE_DECODER_PTS 1


namespace base {
namespace wrtc {
    


    
VideoPacketSource::VideoPacketSource(std::string &playerId, const char *name, fmp4::FrameFilter *next):playerId(playerId), fmp4::FrameFilter(name, next)
    , _rotation(webrtc::kVideoRotation_0)
    , _timestampOffset(0)
    , _nextTimestamp(0)
//    , _source(nullptr)
{
    // Default supported formats. Use SetSupportedFormats to over write.
    std::vector<cricket::VideoFormat> formats;
    formats.push_back(_captureFormat);
   // SetSupportedFormats(formats);
}


//VideoPacketSource::VideoPacketSource(const cricket::VideoFormat& captureFormat)
//    : _captureFormat(captureFormat)
//    , _rotation(webrtc::kVideoRotation_0)
//    , _timestampOffset(0)
//    , _nextTimestamp(0)
////    , _source(nullptr)
//{
//    // Default supported formats. Use SetSupportedFormats to over write.
//    std::vector<cricket::VideoFormat> formats;
//    formats.push_back(_captureFormat);
//    //SetSupportedFormats(formats);
//
//    // formats.push_back(cricket::VideoFormat(1280, 720, _fpsInterval, _codec));
//    // formats.push_back(cricket::VideoFormat(640, 480, _fpsInterval, _codec));
//    // formats.push_back(cricket::VideoFormat(320, 240, _fpsInterval, _codec));
//    // formats.push_back(cricket::VideoFormat(160, 120, _fpsInterval, _codec));
//}


VideoPacketSource::~VideoPacketSource()
{
    LDebug(": Destroying")
}


//void VideoPacketSource::setPacketSource(PacketSignal* source)
//{
//    assert(!_source);
    // if (_source)
    //     _source->detach(packetSlot(this, &VideoPacketSource::onVideoCaptured));
    //_source = source;
//}


/*cricket::CaptureState VideoPacketSource::Start(const cricket::VideoFormat& format)
{
    LDebug("VideoPacketSource::Start")

    // NOTE: The requested format must match the input format until
    // we implememnt pixel format conversion and resizing inside
    // this class.
    RTC_CHECK(_captureFormat == format);
    if (capture_state() == cricket::CS_RUNNING) {
        LWarn("Start called when it's already started.")
        return capture_state();
    }

//    if (_source)
    //    _source->attach(packetSlot(this, &VideoPacketSource::onVideoCaptured));
    
    
    // using std::placeholders::_1;
//    _source= std::bind( &VideoPacketSource::onVideoCaptured, this, _1 );
    
    SetCaptureFormat(&format);
    return cricket::CS_RUNNING;
}


void VideoPacketSource::Stop()
{
    LDebug("Stop")
    if (capture_state() == cricket::CS_STOPPED) {
        LWarn("Stop called when it's already stopped.")
        return;
    }

//    if (_source)
  //      _source->detach(packetSlot(this, &VideoPacketSource::onVideoCaptured));

    SetCaptureFormat(nullptr);
    SetCaptureState(cricket::CS_STOPPED);
}
*/

void VideoPacketSource::run(Frame *frame)
{
    
    SInfo << "ideoPacketSource::run";
    
    int64_t TimestampUs = rtc::TimeMicros();

     rtc::scoped_refptr<FRawFrameBuffer> Buffer = new rtc::RefCountedObject<FRawFrameBuffer>(frame);


	webrtc::VideoFrame Frame = webrtc::VideoFrame::Builder().
		set_video_frame_buffer(Buffer).
		set_rotation(webrtc::kVideoRotation_0).
		set_timestamp_us(TimestampUs).
		build();


	OnFrame(Frame);  //arvind
    
    return ;
}

/*
bool VideoPacketSource::IsRunning()
{
    return capture_state() == cricket::CS_RUNNING;
}


bool VideoPacketSource::GetPreferredFourccs(std::vector<uint32_t>* fourccs)
{
    if (!fourccs)
        return false;

    // This class does not yet support multiple pixel formats.
    fourccs->push_back(_captureFormat.fourcc);
    return true;
}


bool VideoPacketSource::GetBestCaptureFormat(const cricket::VideoFormat& desired, cricket::VideoFormat* best_format)
{
    if (!best_format)
        return false;

    // Use the supported format as the best format.
    // best_format->width = desired.width;
    // best_format->height = desired.height;
    // best_format->fourcc = _codec;
    // best_format->interval = desired.interval;

    *best_format = _captureFormat;
    return true;
}


bool VideoPacketSource::IsScreencast() const
{
    return false;
}
*/


void VideoPacketSource::AddRef() const {
  rtc::AtomicOps::Increment(&ref_count_);  //arvind
}

rtc::RefCountReleaseStatus VideoPacketSource::Release() const {
   const int count = rtc::AtomicOps::Decrement(&ref_count_);  //arvind
   if (count == 0) {
     return rtc::RefCountReleaseStatus::kDroppedLastRef;
   }
  return rtc::RefCountReleaseStatus::kOtherRefsRemained;
}

webrtc::MediaSourceInterface::SourceState VideoPacketSource::state() const {
  return kLive;
}

bool VideoPacketSource::remote() const {
  return false;
}

bool VideoPacketSource::is_screencast() const {
  return false;
}

absl::optional<bool> VideoPacketSource::needs_denoising() const {
  return false;
}

} } // namespace wrtc


#endif // HAVE_FFMPEG
