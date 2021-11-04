// Copyright Epic Games, Inc. All Rights Reserved.

#include "VideoEncoder.h"


inline webrtc::SdpVideoFormat CreateH264Format(webrtc::H264::Profile profile, webrtc::H264::Level level)
{
	const absl::optional<std::string> profile_string =
		webrtc::H264::ProfileLevelIdToString(webrtc::H264::ProfileLevelId(profile, level));
//	check(profile_string);
	return webrtc::SdpVideoFormat
	(
		cricket::kH264CodecName,
		{
			{cricket::kH264FmtpProfileLevelId, *profile_string},
			{cricket::kH264FmtpLevelAsymmetryAllowed, "1"},
			{cricket::kH264FmtpPacketizationMode, "1"}
		}
	);
}

//////////////////////////////////////////////////////////////////////////


FVideoEncoderFactory::FVideoEncoderFactory()
{}

//void FVideoEncoderFactory::AddSession(FPlayerSession& PlayerSession)
//{
//	PendingPlayerSessions.Enqueue(&PlayerSession);
//}

std::vector<webrtc::SdpVideoFormat> FVideoEncoderFactory::GetSupportedFormats() const
{
	// return { CreateH264Format(webrtc::H264::kProfileBaseline, webrtc::H264::kLevel3_1),
	//	CreateH264Format(webrtc::H264::kProfileConstrainedBaseline, webrtc::H264::kLevel3_1) };
	// return { CreateH264Format(webrtc::H264::kProfileMain, webrtc::H264::kLevel3_1) };
	return {CreateH264Format(webrtc::H264::kProfileConstrainedBaseline, webrtc::H264::kLevel5_2)};
	// return { CreateH264Format(webrtc::H264::kProfileHigh, webrtc::H264::kLevel5_1) };
}

webrtc::VideoEncoderFactory::CodecInfo
FVideoEncoderFactory::QueryVideoEncoder(const webrtc::SdpVideoFormat& Format) const
{
	CodecInfo Info;
	Info.is_hardware_accelerated = true;
	Info.has_internal_source = false;
	return Info;
}

std::unique_ptr<webrtc::VideoEncoder> FVideoEncoderFactory::CreateVideoEncoder(const webrtc::SdpVideoFormat& Format)
{
	//FPlayerSession* Session;
	//bool res = PendingPlayerSessions.Dequeue(Session);
	//checkf(res, TEXT("no player session associated with encoder instance"));

	auto VideoEncoder = std::make_unique<FVideoEncoder>();
	//Session->SetVideoEncoder(VideoEncoder.get());
	return VideoEncoder;
}

//
// FVideoEncoder
//

FVideoEncoder::FVideoEncoder()//:
//	HWEncoderDetails(InHWEncoderDetails),
//	PlayerSession(&InPlayerSession)
{



//	check(PlayerSession);
//
//	bControlsQuality = PlayerSession->IsOriginalQualityController();
//
//	CodecSpecific.codecType = webrtc::kVideoCodecH264;
//	// #TODO: Probably smarter setting of `packetization_mode` is required, look at `H264EncoderImpl` ctor
//	// CodecSpecific.codecSpecific.H264.packetization_mode = webrtc::H264PacketizationMode::SingleNalUnit;
//	CodecSpecific.codecSpecific.H264.packetization_mode = webrtc::H264PacketizationMode::NonInterleaved;
//
//	UE_LOG(PixelStreamer, Log, TEXT("WebRTC VideoEncoder created%s"), bControlsQuality? TEXT(", quality controller"): TEXT(""));
}

FVideoEncoder::~FVideoEncoder()
{
	//UE_LOG(PixelStreamer, Log, TEXT("WebRTC VideoEncoder destroyed"));
}

void FVideoEncoder::SetQualityController(bool bControlsQualityNow)
{
//	if (bControlsQuality != bControlsQualityNow)
//	{
//		UE_LOG(PixelStreamer, Log, TEXT("%s : PlayerId=%d, controls quality %d"), TEXT(__FUNCTION__), PlayerSession->GetPlayerId(), bControlsQualityNow);
//		bControlsQuality = bControlsQualityNow;
//	}
}

//int32_t FVideoEncoder::InitEncode(const webrtc::VideoCodec* codec_settings, const webrtc::VideoEncoder::Settings& settings)
//{
//	return 0;
//}

 int32_t FVideoEncoder::InitEncode(const webrtc::VideoCodec* CodecSetings, int32_t NumberOfCores, size_t MaxPayloadSize)
 {
     return 0;
 }
  
int32_t FVideoEncoder::RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback* InCallback)
{
	Callback = InCallback;
	//HWEncoderDetails.Encoder->RegisterListener(*this);
	return 0;
}

int32_t FVideoEncoder::Release()
{
	//HWEncoderDetails.Encoder->UnregisterListener(*this);
	Callback = nullptr;
	return 0;
}

int32_t FVideoEncoder::Encode(const webrtc::VideoFrame& Frame, const std::vector<webrtc::VideoFrameType>* FrameTypes)
{
	
	return WEBRTC_VIDEO_CODEC_OK;
}


// int32_t FVideoEncoder::SetChannelParameters(uint32_t PacketLoss, int64 Rtt)
// {
// 	return 0;
// }

void FVideoEncoder::SetRates(const RateControlParameters& parameters)
{
	LastBitrate  = parameters.bitrate;
	LastFramerate = parameters.framerate_fps;
	//checkNoEntry(); // unexpected call, if even happens, check if passed Bitrate/Framerate should be taken into account
}

// int32_t FVideoEncoder::SetRateAllocation(const webrtc::VideoBitrateAllocation& Allocation, uint32_t Framerate)
// {
// 	LastBitrate = Allocation;
// 	LastFramerate = Framerate;

// 	if (bControlsQuality)
// 	{
// 		UE_LOG(PixelStreamer, Log, TEXT("%s : PlayerId=%d, Bitrate=%u kbps, framerate=%u"), TEXT(__FUNCTION__), PlayerSession->GetPlayerId(), Allocation.get_sum_kbps(), Framerate);
// 	}

// 	return 0;
// }

// webrtc::VideoEncoder::ScalingSettings FVideoEncoder::GetScalingSettings() const
// {
// 	return ScalingSettings{24, 34};
// }

// bool FVideoEncoder::SupportsNativeHandle() const
// {
// 	return true;
// }
