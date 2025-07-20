

#include "VPEncoder.h"

#include "base/logger.h"


//////////////////////////////////////////////////////////////////////////

namespace base
{
namespace web_rtc
{

/////////////////////////////////////////////////////////////////////////////////////////////////////////

int Vp9Encoder::vp9instance = 0;



Vp9Encoder::Vp9Encoder(): webrtc::VP9EncoderImpl(cricket::VideoCodec())
{
     ++vp9instance;
}

int Vp9Encoder::Encode(
    const webrtc::VideoFrame &input_frame, const std::vector<webrtc::VideoFrameType> *frame_types)
{
    // H264FrameBuffer* frame_buffer = static_cast<H264FrameBuffer*> (input_frame.video_frame_buffer().get());


    return webrtc::VP9EncoderImpl::Encode(input_frame, frame_types);


    // WEBRTC_VIDEO_CODEC_OK;
}


}  // namespace web_rtc
}  // namespace base
