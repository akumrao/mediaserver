
#pragma once

#include "VideoEncoder.h"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"
#include "libavutil/opt.h"
#include "libavutil/samplefmt.h"
};

#include "NV_Encoder.h"

namespace base {
namespace web_rtc {

    // class FrameFilter;

    class X264Encoder : public VideoEncoder, public H264_Encoder {
    public:
        X264Encoder(en_EncType encType);
        ~X264Encoder() override;

        void SetRates(const RateControlParameters& parameters);

        int32_t
        Encode(const webrtc::VideoFrame& input_frame, const std::vector<webrtc::VideoFrameType>* frame_types);

        void write_frame(AVPacket* enc_pkt);

        static int x264hwinstance;
    };

} // namespace web_rtc
} // namespace base
