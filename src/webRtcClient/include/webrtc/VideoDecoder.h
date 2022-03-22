// Copyright Epic Games, Inc. All Rights Reserved.

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
//#include "framefilter.h"

#include "api/video_codecs/sdp_video_format.h"
#include "api/video_codecs/video_decoder.h"
#include "api/video_codecs/video_decoder_factory.h"
#include "rtc_base/system/rtc_export.h"

extern "C" {
#include "libavutil/opt.h"
#include "libavcodec/avcodec.h"
#include "libavutil/channel_layout.h"
#include "libavutil/common.h"
#include "libavutil/imgutils.h"
#include "libavutil/mathematics.h"
#include "libavutil/samplefmt.h"
};


namespace base {
namespace wrtc {
    




class FVideoDecoder : public webrtc::VideoEncoder
{
public:


    public:
    typedef struct {
        AVCodec *codec = nullptr;
        AVFrame *frame = nullptr;
        AVCodecContext *context = nullptr;
        AVPacket *pkt = nullptr;
    } CodecCtx;


    struct LayerConfig {
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

//	struct FEncoderCookie : AVEncoder::FEncoderVideoFrameCookie
//	{
//		virtual ~FEncoderCookie() {}
//		webrtc::EncodedImage EncodedImage;
//		// buffer to hold last encoded frame bitstream, because `webrtc::EncodedImage` doesn't take ownership of
//		// the memory
//		TArray<uint8> EncodedFrameBuffer;
//	};

	FVideoDecoder();
	~FVideoDecoder() override;
        
        
        uint64_t encoderInc{0};

	void SetQualityController(bool bControlsQuality);

	//
	// AVEncoder::IVideoEncoderListener
	//
	//void OnEncodedVideoFrame(const AVEncoder::FAVPacket& Packet, AVEncoder::FEncoderVideoFrameCookie* Cookie) override;

	//
	// webrtc::VideoEncoder interface
	//
	int32_t InitEncode(const webrtc::VideoCodec* CodecSetings, int32_t NumberOfCores, size_t MaxPayloadSize) override;
        
    
//	int32_t InitEncode(const webrtc::VideoCodec* codec_settings,
//                     const webrtc::VideoEncoder::Settings& settings) override;

	int32_t RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback* Callback) override;
	int32_t Release() override;
	//int32_t Encode(const webrtc::VideoFrame& Frame, const webrtc::CodecSpecificInfo* CodecSpecificInfo, const std::vector<webrtc::VideoFrameType>* FrameTypes) override;
	 int32_t Encode( const webrtc::VideoFrame& inputImage,    const std::vector<webrtc::VideoFrameType>* frame_types) override;
	//int32_t SetChannelParameters(uint32 PacketLoss, int64 Rtt) override;
	//int32_t SetRates(uint32 Bitrate, uint32 Framerate) override;
	void SetRates(const RateControlParameters& parameters) override;
	//int32_t SetRateAllocation(const webrtc::VideoBitrateAllocation& Allocation, uint32 Framerate) override;
	//ScalingSettings GetScalingSettings() const override;
	//bool SupportsNativeHandle() const override;

    webrtc::VideoEncoder::EncoderInfo GetEncoderInfo() const override
    {
        webrtc::VideoEncoder::EncoderInfo info;
        info.scaling_settings = webrtc::VideoEncoder::ScalingSettings(24, 34);
        info.supports_native_handle = true;
        info.implementation_name = "Hardware H264 Encoder";
        return info;
    }

private:
	// Player session that this encoder instance belongs to
	//FHWEncoderDetails& HWEncoderDetails;
	//FPlayerSession* PlayerSession = nullptr;
	//webrtc::EncodedImageCallback* Callback = nullptr;
	webrtc::CodecSpecificInfo CodecSpecific;
	webrtc::RTPFragmentationHeader FragHeader;

	//FThreadSafeBool bControlsQuality = false;
	//webrtc::VideoBitrateAllocation LastBitrate;
	//uint32_t LastFramerate = 0;



   
     FILE* fp;


    bool OpenEncoder(CodecCtx *ctx, LayerConfig &io_param);

    void CloseEncoder(CodecCtx *ctx);

    void SetContext(CodecCtx *ctx, LayerConfig &io_param, bool init);

    void RtpFragmentize(webrtc::EncodedImage *encoded_image,
                        std::unique_ptr<uint8_t[]> *encoded_image_buffer,
                        const webrtc::VideoFrameBuffer &frame_buffer, AVPacket *packet,
                        webrtc::RTPFragmentationHeader *frag_header);

    void copyFrame(AVFrame *frame, const webrtc::I420BufferInterface *buffer);

//    webrtc::FrameType ConvertToVideoFrameType(AVFrame *frame);

    webrtc::VideoFrameType ConvertToVideoFrameType(AVFrame *frame);

    webrtc::H264BitstreamParser h264_bitstream_parser_;

    // Reports statistics with histograms.

    void ReportError();

    std::vector<CodecCtx *> encoders_;

    std::vector<LayerConfig> configurations_;
    std::vector<webrtc::EncodedImage> encoded_images_;
    std::vector<std::unique_ptr<uint8_t[]>> encoded_image_buffers_;

    webrtc::VideoCodec codec_;
    webrtc::H264PacketizationMode packetization_mode_;
    size_t max_payload_size_;
    int32_t number_of_cores_;
    webrtc::EncodedImageCallback *encoded_image_callback_= nullptr;;
    std::string key;
    bool has_reported_init_;
    bool hardware_accelerate;
    bool has_reported_error_;

        
};



class FVideoDecoderFactory : public webrtc::VideoDecoderFactory
{
public:
	//explicit FVideoDecoderFactory(FHWEncoderDetails& HWEncoderDetails);
        
       FVideoDecoderFactory();
	/**
	* This is used from the FPlayerSession::OnSucess to let the factory know
	* what session the next created encoder should belong to.
	* It allows us to get the right FPlayerSession <-> FVideoDecoder relationship
	*/
	//void AddSession(FPlayerSession& PlayerSession);

	//
	// webrtc::VideoEncoderFactory implementation
	//
	//std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override;
	//CodecInfo QueryVideoEncoder(const webrtc::SdpVideoFormat& Format) const override;
	//std::unique_ptr<webrtc::VideoEncoder> CreateVideoEncoder(const webrtc::SdpVideoFormat& Format) override;
        
        std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override;
        std::unique_ptr<webrtc::VideoDecoder> CreateVideoDecoder(const webrtc::SdpVideoFormat& format) override;
  

private:
	//FHWEncoderDetails& HWEncoderDetails;
	//TQueue<FPlayerSession*> PendingPlayerSessions;
    
    
     std::vector<webrtc::SdpVideoFormat> supported_codecs;
};


}//ns webrtc
}//base
