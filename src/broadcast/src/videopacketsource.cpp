

#include "webrtc/videopacketsource.h"

#ifdef HAVE_FFMPEG

#include "ff/ffmpeg.h"
#include "ff/videocontext.h"
#include "ff/videodecoder.h"
#include "ff/fpscounter.h"

#include "api/video/i420_buffer.h"
#include "rtc_base/atomic_ops.h"
#include <chrono>

using std::endl;

#define WebRTC_USE_DECODER_PTS 1


namespace base {
namespace wrtc {


VideoPacketSource::VideoPacketSource(std::string &playerId):playerId(playerId)
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


VideoPacketSource::VideoPacketSource(const cricket::VideoFormat& captureFormat)
    : _captureFormat(captureFormat)
    , _rotation(webrtc::kVideoRotation_0)
    , _timestampOffset(0)
    , _nextTimestamp(0)
//    , _source(nullptr)
{
    // Default supported formats. Use SetSupportedFormats to over write.
    std::vector<cricket::VideoFormat> formats;
    formats.push_back(_captureFormat);
    //SetSupportedFormats(formats);

    // formats.push_back(cricket::VideoFormat(1280, 720, _fpsInterval, _codec));
    // formats.push_back(cricket::VideoFormat(640, 480, _fpsInterval, _codec));
    // formats.push_back(cricket::VideoFormat(320, 240, _fpsInterval, _codec));
    // formats.push_back(cricket::VideoFormat(160, 120, _fpsInterval, _codec));
}


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

void VideoPacketSource::onVideoCaptured(IPacket& pac)
{
    //if(!IsRunning())
  //  return;
    
    ff::PlanarVideoPacket& packet = (ff::PlanarVideoPacket&)pac;
    
   

    assert(packet.width > 0);
    assert(packet.height > 0);

    int adapted_width;
    int adapted_height;
    int crop_width;
    int crop_height;
    int crop_x;
    int crop_y;
    int64_t timestamp;
   // int64_t translated_camera_time_us;

#if WebRTC_USE_DECODER_PTS
    // Set the packet timestamp.
    // Since the stream may not be playing from the beginning we
    // store the first packet timestamp and subtract it from
    // subsequent packets.
    if (!_timestampOffset)
        _timestampOffset = -packet.time;
    timestamp = packet.time + _timestampOffset;

    // NOTE: Initial packet time cannot be 0 for some reason.
    // WebRTC sets the initial packet time to 1000 so we will do the same.
    timestamp += 1000;
#else
     _nextTimestamp += _captureFormat.interval;
     timestamp = _nextTimestamp / rtc::kNumNanosecsPerMicrosec;
#endif

     if (!AdaptFrame(packet.width, packet.height,
         timestamp, //rtc::TimeNanos() / rtc::kNumNanosecsPerMicrosec,
         &adapted_width, &adapted_height,
         &crop_width, &crop_height,
         &crop_x, &crop_y)) {
         //LWarn("Adapt frame failed", packet.time)
         return;
     }

   // int64_t TimestampUs = rtc::TimeMicros();
    rtc::scoped_refptr<webrtc::I420Buffer> buffer = webrtc::I420Buffer::Copy(
            packet.width, packet.height,
            packet.buffer[0], packet.linesize[0],
            packet.buffer[1], packet.linesize[1],
            packet.buffer[2], packet.linesize[2]);
    
    
    webrtc::VideoFrame Frame = webrtc::VideoFrame::Builder().
		set_video_frame_buffer(buffer).
		set_rotation(webrtc::kVideoRotation_0).
		set_timestamp_us(timestamp).
		build();

	//UE_LOG(PixelStreamer, VeryVerbose, TEXT("(%d) captured video %lld"), RtcTimeMs(), TimestampUs);
     SInfo << "On video frame for Player : " << playerId << " " <<  packet.width, 'x', packet.height;
     OnFrame(Frame);  //arvind
        

    // OnFrame(webrtc::VideoFrame(
    //     buffer, _rotation,
    //     translated_camera_time_us), // timestamp
    //     packet.width, packet.height);

#if 0 // Old code pre f5297a0
    cricket::CapturedFrame frame;
    frame.width = packet.width;
    frame.height = packet.height;
    frame.pixel_width = 1;
    frame.pixel_height = 1;
    frame.fourcc = cricket::FOURCC_NV12;
    frame.data = packet.data();
    frame.data_size = packet.size();
    // frame.time_stamp = packet.time; // time in microseconds is ignored

    SignalFrameCaptured(this, &frame);
#endif
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
