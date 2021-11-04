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

/*


#include "api/media_stream_interface.h"
#include "api/data_channel_interface.h"
#include "api/media_stream_interface.h"
#include "api/peer_connection_interface.h"

#include "api/audio_codecs/audio_format.h"
#include "api/audio_codecs/audio_decoder_factory_template.h"
#include "api/audio_codecs/audio_encoder_factory_template.h"
#include "api/audio_codecs/opus/audio_decoder_opus.h"
#include "api/audio_codecs/opus/audio_encoder_opus.h"
//#include "api/test/fakeconstraints.h"
#include "api/video_codecs/video_decoder.h"
#include "api/video_codecs/sdp_video_format.h"
#include "api/video/video_frame.h"
#include "api/video/video_frame_buffer.h"
#include "api/video/i420_buffer.h"
#include "api/video/video_sink_interface.h"

#include "rtc_base/thread.h"
#include "rtc_base/logging.h"
//#include "rtc_base/flags.h"
#include "rtc_base/ssl_adapter.h"
#include "rtc_base/arraysize.h"
#include "rtc_base/net_helper.h"
#include "rtc_base/string_utils.h"
#include "rtc_base/signal_thread.h"

#include "pc/session_description.h"

// #include "media/base/videocapturer.h"
// #include "media/engine/webrtcvideocapturerfactory.h"
// #include "media/engine/internaldecoderfactory.h"
// #include "media/engine/internalencoderfactory.h"
// 
// #include "media/engine/webrtcvideoencoderfactory.h"
// #include "media/base/adaptedvideotracksource.h"
///

#include "media/engine/internal_decoder_factory.h"
#include "media/engine/internal_encoder_factory.h"


#include "media/base/media_channel.h"
#include "media/base/video_common.h"

#include "api/audio/audio_mixer.h"
#include "api/audio_codecs/audio_decoder_factory.h"
#include "api/audio_codecs/audio_encoder_factory.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/audio_options.h"
#include "api/create_peerconnection_factory.h"
#include "api/rtp_sender_interface.h"
#include "api/video_codecs/builtin_video_decoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "api/video_codecs/video_decoder_factory.h"
#include "api/video_codecs/video_encoder_factory.h"

#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"
#include "modules/video_capture/video_capture.h"
#include "modules/video_capture/video_capture_factory.h"
//




#include "common_video/h264/h264_bitstream_parser.h"
#include "common_video/h264/h264_common.h"

#include "media/base/video_broadcaster.h"



//#import "base/RTCVideoCapturer.h"

//#include "base/RTCMacros.h"
#include "media/base/adapted_video_track_source.h"
#include "rtc_base/timestamp_aligner.h"
*/
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


