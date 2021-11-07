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
#include "framefilter.h"




namespace base {
namespace wrtc {
    
class FrameFilter;

class FVideoEncoder : public webrtc::VideoEncoder
{
public:

//	struct FEncoderCookie : AVEncoder::FEncoderVideoFrameCookie
//	{
//		virtual ~FEncoderCookie() {}
//		webrtc::EncodedImage EncodedImage;
//		// buffer to hold last encoded frame bitstream, because `webrtc::EncodedImage` doesn't take ownership of
//		// the memory
//		TArray<uint8> EncodedFrameBuffer;
//	};

	FVideoEncoder();
	~FVideoEncoder() override;


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
	webrtc::EncodedImageCallback* Callback = nullptr;
	webrtc::CodecSpecificInfo CodecSpecific;
	webrtc::RTPFragmentationHeader FragHeader;

	//FThreadSafeBool bControlsQuality = false;
	webrtc::VideoBitrateAllocation LastBitrate;
	uint32_t LastFramerate = 0;
        
   };

class FVideoEncoderFactory : public webrtc::VideoEncoderFactory
{
public:
	//explicit FVideoEncoderFactory(FHWEncoderDetails& HWEncoderDetails);
        
       FVideoEncoderFactory();
	/**
	* This is used from the FPlayerSession::OnSucess to let the factory know
	* what session the next created encoder should belong to.
	* It allows us to get the right FPlayerSession <-> FVideoEncoder relationship
	*/
	//void AddSession(FPlayerSession& PlayerSession);

	//
	// webrtc::VideoEncoderFactory implementation
	//
	std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override;
	CodecInfo QueryVideoEncoder(const webrtc::SdpVideoFormat& Format) const override;
	std::unique_ptr<webrtc::VideoEncoder> CreateVideoEncoder(const webrtc::SdpVideoFormat& Format) override;

private:
	//FHWEncoderDetails& HWEncoderDetails;
	//TQueue<FPlayerSession*> PendingPlayerSessions;
};


}//ns webrtc
}//base
