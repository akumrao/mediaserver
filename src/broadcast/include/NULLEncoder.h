
#pragma once

#include "media/engine/internal_encoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "api/video_codecs/video_encoder_factory.h"
#include "api/video_codecs/video_encoder.h"


#include "rtc_base/critical_section.h"
#include "modules/video_coding/utility/quality_scaler.h"
#include "common_video/h264/h264_bitstream_parser.h"
#include "modules/video_coding/codecs/h264/include/h264.h"
#include "media/base/h264_profile_level_id.h"
#include "framefilter.h"

//#include  <list>

#include <queue>

extern "C"
{
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"
#include "libavutil/samplefmt.h"
};

    //#include "base/thread.h"

#include "webrtc/rawVideoFrame.h"

namespace base
{
namespace web_rtc
{

class FrameFilter;


class NULLEncoder : public webrtc::VideoEncoder
{
public:
public:
    typedef struct
    {
        AVCodec *codec = nullptr;
        AVFrame *frame = nullptr;
        AVCodecContext *context = nullptr;
        AVPacket *pkt = nullptr;
    } CodecCtx;


    struct LayerConfig
    {
        int simulcast_idx = 0;
        int width = -1;
        int height = -1;
        bool sending = true;
        bool key_frame_request = false;
        float max_frame_rate = 0;
        uint32_t target_bps = 0;
        uint32_t max_bps = 0;
        bool frame_dropping_on = false;
        int key_frame_interval = 0;

        void SetStreamState(bool send_stream);
    };


    NULLEncoder();
    ~NULLEncoder() override;


    // uint64_t encoderInc{0};

    void SetQualityController(bool bControlsQuality);


    //
    int32_t InitEncode(
        const webrtc::VideoCodec *CodecSetings, int32_t NumberOfCores, size_t MaxPayloadSize) override;


    int32_t RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback *Callback) override;
    int32_t Release() override;
    // int32_t Encode(const webrtc::VideoFrame& Frame, const webrtc::CodecSpecificInfo* CodecSpecificInfo, const
    // std::vector<webrtc::VideoFrameType>* FrameTypes) override;
    //  int32_t Encode1( const webrtc::VideoFrame& inputImage,    const std::vector<webrtc::VideoFrameType>*
    //  frame_types) ;

    int32_t Encode(
        const webrtc::VideoFrame &inputImage, const std::vector<webrtc::VideoFrameType> *frame_types) override;
    // int32_t SetChannelParameters(uint32 PacketLoss, int64 Rtt) override;
    // int32_t SetRates(uint32 Bitrate, uint32 Framerate) override;
    void SetRates(const RateControlParameters &parameters) override;
    // int32_t SetRateAllocation(const webrtc::VideoBitrateAllocation& Allocation, uint32 Framerate) override;
    // ScalingSettings GetScalingSettings() const override;
    // bool SupportsNativeHandle() const override;

    webrtc::VideoEncoder::EncoderInfo GetEncoderInfo() const override
    {
        webrtc::VideoEncoder::EncoderInfo info;
        info.scaling_settings
            = webrtc::VideoEncoder::ScalingSettings(webrtc::VideoEncoder::ScalingSettings::kOff);
        info.supports_native_handle = true;
        info.has_trusted_rate_controller = true;
        info.implementation_name = "Native H264 Encoder";
        info.is_hardware_accelerated = "true";
        info.has_internal_source = false;
        return info;
    }

    static int nativeInstance;

private:
    webrtc::CodecSpecificInfo CodecSpecific;
    webrtc::RTPFragmentationHeader FragHeader;


    bool OpenEncoder(CodecCtx *ctx, LayerConfig &io_param);

    void CloseEncoder(CodecCtx *ctx);

    void SetContext(CodecCtx *ctx, LayerConfig &io_param, bool init);

    void RtpFragmentize(
        webrtc::EncodedImage *encoded_image,
        std::unique_ptr<uint8_t[]> *encoded_image_buffer,
        int w,
        int h,
        std::vector<uint8_t> &packet,
        webrtc::RTPFragmentationHeader *frag_header);


    void ReportError();


    std::vector<LayerConfig> configurations_;
    std::vector<webrtc::EncodedImage> encoded_images_;
    std::vector<std::unique_ptr<uint8_t[]>> encoded_image_buffers_;

    webrtc::VideoCodec codec_;
    webrtc::H264PacketizationMode packetization_mode_;
    size_t max_payload_size_;
    int32_t number_of_cores_;

    // std::mutex en_im_cb;

    webrtc::EncodedImageCallback *encoded_image_callback_ = nullptr;
    ;
    std::string key;
    bool has_reported_init_;
    bool hardware_accelerate;
    bool has_reported_error_;


    webrtc::VideoBitrateAllocation allocation;

    uint64_t vframecount{0};

    std::string metadata;
};


}  // namespace web_rtc
}  // namespace base
