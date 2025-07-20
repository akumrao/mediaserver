
#pragma once

#include "modules/video_coding/codecs/vp9/include/vp9.h"
#include "modules/video_coding/codecs/vp9/vp9_impl.h"
#include "media/base/codec.h"


namespace base
{
namespace web_rtc
{


class Vp9Encoder : public webrtc::VP9EncoderImpl
{
public:
    Vp9Encoder();

    ~Vp9Encoder() { --vp9instance; }


    int Encode(
        const webrtc::VideoFrame &inputImage, const std::vector<webrtc::VideoFrameType> *frame_types) override;
    
 


    static int vp9instance;
};


}  // namespace web_rtc
}  // namespace base
