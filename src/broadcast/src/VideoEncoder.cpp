// Copyright Epic Games, Inc. All Rights Reserved.


#include "VideoEncoder.h"
#include "base/logger.h"


#include "webrtc/rawVideoFrame.h"
//#include "rtc_base/ref_counted_object.h"
//#include "rtc_base/atomic_ops.h"
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

namespace base {
namespace wrtc {
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
	//return {CreateH264Format(webrtc::H264::kProfileConstrainedBaseline, webrtc::H264::kLevel5_2)};
	// return { CreateH264Format(webrtc::H264::kProfileHigh, webrtc::H264::kLevel5_1) };
    
    //ffprobe -show_streams  for profile and level
    
    std::vector<webrtc::SdpVideoFormat> supported_codecs;
    supported_codecs.push_back(webrtc::SdpVideoFormat(cricket::kH264CodecName));
   // supported_codecs.push_back(SdpVideoFormat(cricket::kVp8CodecName));
       
    return supported_codecs;
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
	CodecSpecific.codecType = webrtc::kVideoCodecH264;
	// #TODO: Probably smarter setting of `packetization_mode` is required, look at `H264EncoderImpl` ctor
	// CodecSpecific.codecSpecific.H264.packetization_mode = webrtc::H264PacketizationMode::SingleNalUnit;
	CodecSpecific.codecSpecific.H264.packetization_mode = webrtc::H264PacketizationMode::NonInterleaved;

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

int32_t FVideoEncoder::Encode(const webrtc::VideoFrame& frame, const std::vector<webrtc::VideoFrameType>* FrameTypes)
{
    
           
    
    FRawFrameBuffer* RawFrame = static_cast<FRawFrameBuffer*>(frame.video_frame_buffer().get());
	// the frame managed to pass encoder queue so disable frame drop notification
    BasicFrame &buf = RawFrame->GetBuffer();
    
    SDebug << "Encode frame: " << RawFrame->frameNo  << "  size: " <<  buf.payload.size(); 
    
    // sendBuffer is not copied here.
     webrtc::EncodedImage encodedImage(buf.payload.data(), buf.payload.size(), buf.payload.size());

   
    encodedImage._completeFrame = true;
    encodedImage._frameType = webrtc::VideoFrameType::kVideoFrameKey;
    
    // Scan for and create mark all fragments.
    webrtc::RTPFragmentationHeader fragmentationHeader;
    uint32_t fragIdx = 0;
    for (uint32_t i = 0; i < buf.payload.size() - 5; ++i) {
      uint8_t* ptr = buf.payload.data() + i;
      int prefixLengthFound = 0;
      if (ptr[0] == 0x00 && ptr[1] == 0x00 && ptr[2] == 0x00 && ptr[3] == 0x01
        && ((ptr[4] & 0x1f) != 0x09 /* ignore access unit delimiters */)) {
        prefixLengthFound = 4;
      } else if (ptr[0] == 0x00 && ptr[1] == 0x00 && ptr[2] == 0x01
        && ((ptr[3] & 0x1f) != 0x09 /* ignore access unit delimiters */)) {
        prefixLengthFound = 3;
      }

      // Found a key frame, mark is as such in case
      // MFSampleExtension_CleanPoint wasn't set on the sample.
      if (prefixLengthFound > 0 && (ptr[prefixLengthFound] & 0x1f) == 0x05) {
        encodedImage._completeFrame = true;
        encodedImage._frameType = webrtc::VideoFrameType::kVideoFrameKey;
      }

      if (prefixLengthFound > 0) {
        fragmentationHeader.VerifyAndAllocateFragmentationHeader(fragIdx + 1);
        fragmentationHeader.fragmentationOffset[fragIdx] = i + prefixLengthFound;
        fragmentationHeader.fragmentationLength[fragIdx] = 0;  // We'll set that later
        // Set the length of the previous fragment.
        if (fragIdx > 0) {
          fragmentationHeader.fragmentationLength[fragIdx - 1] =
            i - fragmentationHeader.fragmentationOffset[fragIdx - 1];
        }
        //fragmentationHeader.fragmentationPlType[fragIdx] = 0;
        //fragmentationHeader.fragmentationTimeDiff[fragIdx] = 0;
        ++fragIdx;
        i += 5;
      }
    }
    // Set the length of the last fragment.
    if (fragIdx > 0) {
      fragmentationHeader.fragmentationLength[fragIdx - 1] =
        buf.payload.size() -
        fragmentationHeader.fragmentationOffset[fragIdx - 1];
    }



    encodedImage.SetTimestamp(frame.timestamp());

//        encodedImage._encodedWidth = frame.frameWidth;
   // encodedImage._encodedHeight = frame.frameHeight;


    encodedImage.ntp_time_ms_ = frame.ntp_time_ms();
    encodedImage.capture_time_ms_ = frame.render_time_ms();
    encodedImage.rotation_ = frame.rotation();
    //encodedImage.timing_.encode_start_ms = rtc::TimeMicros() / 1000;

    if (Callback != nullptr) {
          webrtc::CodecSpecificInfo codecSpecificInfo;
          codecSpecificInfo.codecType = webrtc::kVideoCodecH264;
          codecSpecificInfo.codecSpecific.H264.packetization_mode = webrtc::H264PacketizationMode::NonInterleaved;
          Callback->OnEncodedImage(
            encodedImage, &codecSpecificInfo, &fragmentationHeader);
    }
	
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
}// ns webrtc
}//ns base