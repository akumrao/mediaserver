// Copyright Epic Games, Inc. All Rights Reserved.


#include "webrtc/VideoDecoder.h"
#include "base/logger.h"
#include <list>

#include "third_party/openh264/src/codec/api/svc/codec_app_def.h"
#include "common_video/libyuv/include/webrtc_libyuv.h"
#include "modules/video_coding/utility/simulcast_rate_allocator.h"

#include "common_video/h264/sps_parser.h"
#include "common_video/h264/h264_common.h"


#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "modules/video_coding/codecs/h264/include/h264.h"

#include <api/video_codecs/sdp_video_format.h>
#include <media/base/media_constants.h>
#include "absl/strings/match.h"

const size_t kYPlaneIndex = 0;
const size_t kUPlaneIndex = 1;
const size_t kVPlaneIndex = 2;


const uint8_t start_code[4] = {0, 0, 0, 1};

//#include "rtc_base/ref_counted_object.h"
//#include "rtc_base/atomic_ops.h"


inline webrtc::SdpVideoFormat CreateH264Format(webrtc::H264::Profile profile, webrtc::H264::Level level, const std::string &packetization_mode) {
    const absl::optional<std::string> profile_string =
            webrtc::H264::ProfileLevelIdToString(webrtc::H264::ProfileLevelId(profile, level));
    //	check(profile_string);
    return webrtc::SdpVideoFormat
            (
            cricket::kH264CodecName,{
        {cricket::kH264FmtpProfileLevelId, *profile_string},
        {cricket::kH264FmtpLevelAsymmetryAllowed, "1"},
        {cricket::kH264FmtpPacketizationMode, packetization_mode}
    }
    );
}

//////////////////////////////////////////////////////////////////////////

namespace base {
    namespace wrtc {
                
        
        
        
        bool IsFormatSupported(
                const std::vector<webrtc::SdpVideoFormat>& supported_formats,
                const webrtc::SdpVideoFormat& format) {
            for (const webrtc::SdpVideoFormat& supported_format : supported_formats) {
                if (cricket::IsSameCodec(format.name, format.parameters,
                        supported_format.name,
                        supported_format.parameters)) {
                    return true;
                }
            }
            return false;
        }
        
        ////////////////////////////////////////////////////////////////////////////////////////////// 

        FVideoDecoderFactory::FVideoDecoderFactory() : supported_codecs(webrtc::SupportedH264Codecs()) {
        }

        //void FVideoDecoderFactory::AddSession(FPlayerSession& PlayerSession)
        //{
        //	PendingPlayerSessions.Enqueue(&PlayerSession);
        //}

      

        std::vector<webrtc::SdpVideoFormat> FVideoDecoderFactory::GetSupportedFormats() const {
            // return { CreateH264Format(webrtc::H264::kProfileBaseline, webrtc::H264::kLevel3_1),
            //	CreateH264Format(webrtc::H264::kProfileConstrainedBaseline, webrtc::H264::kLevel3_1) };
            // return { CreateH264Format(webrtc::H264::kProfileMain, webrtc::H264::kLevel3_1) };
            //return {CreateH264Format(webrtc::H264::kProfileConstrainedBaseline, webrtc::H264::kLevel5_2)};
            // return { CreateH264Format(webrtc::H264::kProfileHigh, webrtc::H264::kLevel5_1) };

            //ffprobe -show_streams  for profile and level

//            std::vector<webrtc::SdpVideoFormat> supported_codecs;
//
//            // supported_codecs.push_back(webrtc::SdpVideoFormat(cricket::kH264CodecName));
//
//            supported_codecs.emplace_back(CreateH264Format(webrtc::H264::kProfileBaseline, webrtc::H264::kLevel3_1, "1"));
//            // Need a constrained baseline h264 support, because safari only support this type of profile.
//           // supported_codecs.emplace_back(CreateH264Format(webrtc::H264::kProfileMain, webrtc::H264::kLevel3_1, "1"));
//
//            // supported_codecs.push_back(SdpVideoFormat(cricket::kVp8CodecName));
//
//            // 
//
//            return supported_codecs;
            
            
              std::vector<webrtc::SdpVideoFormat> formats;
            //formats.push_back(webrtc::SdpVideoFormat(cricket::kVp8CodecName));
//            for (const webrtc::SdpVideoFormat& format : SupportedVP9Codecs())
//              formats.push_back(format);
            for (const webrtc::SdpVideoFormat& h264_format : webrtc::SupportedH264Codecs())
              formats.push_back(h264_format);
            return formats;
        }

//        webrtc::VideoEncoderFactory::CodecInfo
//        FVideoDecoderFactory::QueryVideoEncoder(const webrtc::SdpVideoFormat& Format) const {
//            CodecInfo Info;
//            Info.is_hardware_accelerated = true;
//            Info.has_internal_source = true;
//            return Info;
//        }

  
        std::unique_ptr<webrtc::VideoDecoder> FVideoDecoderFactory::CreateVideoDecoder(const webrtc::SdpVideoFormat& format) {
            //FPlayerSession* Session;
            //bool res = PendingPlayerSessions.Dequeue(Session);
            //checkf(res, TEXT("no player session associated with encoder instance"));

            
            
            // auto VideoEncoder = std::make_unique<FVideoDecoder>();   // for cam encoders
           // return VideoEncoder;


            if (!IsFormatSupported(GetSupportedFormats(), format)) {
               SInfo << "Trying to create decoder for unsupported format";
                return nullptr;
            }

//            if (absl::EqualsIgnoreCase(format.name, cricket::kVp8CodecName))
//                return VP8Decoder::Create();
//            if (absl::EqualsIgnoreCase(format.name, cricket::kVp9CodecName))
//                return VP9Decoder::Create();
            if (absl::EqualsIgnoreCase(format.name, cricket::kH264CodecName))
                return webrtc::H264Decoder::Create();

            RTC_NOTREACHED();
            return nullptr;
            
            
        }

//////////////////////////////////////////////////////////////////////////////////////////


void FVideoDecoder::copyFrame(AVFrame *frame, const webrtc::I420BufferInterface *buffer) {
    frame->width = buffer->width();
    frame->height = buffer->height();
    frame->format = AV_PIX_FMT_YUV420P;
    frame->data[kYPlaneIndex] = const_cast<uint8_t *>(buffer->DataY());
    frame->data[kUPlaneIndex] = const_cast<uint8_t *>(buffer->DataU());
    frame->data[kVPlaneIndex] = const_cast<uint8_t *>(buffer->DataV());
}




FVideoDecoder::FVideoDecoder()//:
//	HWEncoderDetails(InHWEncoderDetails),
//	PlayerSession(&InPlayerSession)
{

  std::string filename="/tmp/test.264";
    
   fp = fopen(filename.c_str(), "wb");
    if (!fp) {
        fprintf(stderr, "Could not open %s\n", filename.c_str());
        exit(1);
    }


//	check(PlayerSession);
//
//	bControlsQuality = PlayerSession->IsOriginalQualityController();
//
	CodecSpecific.codecType = webrtc::kVideoCodecH264;
	// #TODO: Probably smarter setting of `packetization_mode` is required, look at `H264EncoderImpl` ctor
	// CodecSpecific.codecSpecific.H264.packetization_mode = webrtc::H264PacketizationMode::SingleNalUnit;
	CodecSpecific.codecSpecific.H264.packetization_mode = webrtc::H264PacketizationMode::NonInterleaved;

        //info = new fmp4::InfoFrameFilter("arvind", nullptr);
        
//	UE_LOG(PixelStreamer, Log, TEXT("WebRTC VideoEncoder created%s"), bControlsQuality? TEXT(", quality controller"): TEXT(""));
}

FVideoDecoder::~FVideoDecoder()
{
	//UE_LOG(PixelStreamer, Log, TEXT("WebRTC VideoEncoder destroyed"));
}

void FVideoDecoder::SetQualityController(bool bControlsQualityNow)
{
//	if (bControlsQuality != bControlsQualityNow)
//	{
//		UE_LOG(PixelStreamer, Log, TEXT("%s : PlayerId=%d, controls quality %d"), TEXT(__FUNCTION__), PlayerSession->GetPlayerId(), bControlsQualityNow);
//		bControlsQuality = bControlsQualityNow;
//	}
}

//int32_t FVideoDecoder::InitEncode(const webrtc::VideoCodec* codec_settings, const webrtc::VideoEncoder::Settings& settings)
//{
//	return 0;
//}


void FVideoDecoder::ReportError() {

    SError << "FVideoDecoder::codec error" ;
}


 int32_t FVideoDecoder::InitEncode(const webrtc::VideoCodec* CodecSetings, int32_t NumberOfCores, size_t MaxPayloadSize)
 {

     if (!CodecSetings || CodecSetings->codecType != webrtc::kVideoCodecH264) {
        ReportError();
        return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
    }
    if (CodecSetings->maxFramerate == 0) {
        ReportError();
        return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
    }
    if (CodecSetings->width < 1 || CodecSetings->height < 1) {
        ReportError();
        return WEBRTC_VIDEO_CODEC_ERR_PARAMETER;
    }

    int32_t release_ret = Release();
    if (release_ret != WEBRTC_VIDEO_CODEC_OK) {
        ReportError();
        return release_ret;
    }

    int number_of_streams = 1;//webrtc::SimulcastUtility::NumberOfSimulcastStreams(*CodecSetings);
    bool doing_simulcast = (number_of_streams > 1);

//    if (doing_simulcast && (!webrtc::SimulcastUtility::ValidSimulcastResolutions(
//            *CodecSetings, number_of_streams) ||
//                            !webrtc::SimulcastUtility::ValidSimulcastTemporalLayers(
//                                    *CodecSetings, number_of_streams))) {
//        return WEBRTC_VIDEO_CODEC_ERR_SIMULCAST_PARAMETERS_NOT_SUPPORTED;
//    }
    encoded_images_.resize(static_cast<unsigned long>(number_of_streams));
    encoded_image_buffers_.resize(static_cast<unsigned long>(number_of_streams));
    encoders_.resize(static_cast<unsigned long>(number_of_streams));
    configurations_.resize(static_cast<unsigned long>(number_of_streams));
    for (int i = 0; i < number_of_streams; i++) {
        encoders_[i] = new CodecCtx();
    }
   // number_of_cores_ = number_of_cores;
   // max_payload_size_ = max_payload_size;
    codec_ = *CodecSetings;

    // Code expects simulcastStream resolutions to be correct, make sure they are
    // filled even when there are no simulcast layers.
    if (codec_.numberOfSimulcastStreams == 0) {
        codec_.simulcastStream[0].width = codec_.width;
        codec_.simulcastStream[0].height = codec_.height;
    }

    for (int i = 0, idx = number_of_streams - 1; i < number_of_streams;
         ++i, --idx) {
        // Temporal layers still not supported.
        if (CodecSetings->simulcastStream[i].numberOfTemporalLayers > 1) {
            Release();
            return WEBRTC_VIDEO_CODEC_ERR_SIMULCAST_PARAMETERS_NOT_SUPPORTED;
        }

   
        codec_.H264()->keyFrameInterval = 25;
        codec_.maxFramerate = 25;

        // Set internal settings from codec_settings
        configurations_[i].simulcast_idx = idx;
        configurations_[i].sending = false;
        configurations_[i].width = codec_.simulcastStream[idx].width;
        configurations_[i].height = codec_.simulcastStream[idx].height;
        configurations_[i].max_frame_rate = static_cast<float>(codec_.maxFramerate);
        configurations_[i].frame_dropping_on = codec_.H264()->frameDroppingOn;
        configurations_[i].key_frame_interval = codec_.H264()->keyFrameInterval;

        // Codec_settings uses kbits/second; encoder uses bits/second.
        configurations_[i].max_bps = codec_.maxBitrate * 1000;
        configurations_[i].target_bps = codec_.startBitrate * 1000;
        if (!OpenEncoder(encoders_[i], configurations_[i])) {
            Release();
            ReportError();
            return WEBRTC_VIDEO_CODEC_ERROR;
        }
        // Initialize encoded image. Default buffer size: size of unencoded data.
           const size_t new_capacity =   CalcBufferSize(webrtc::VideoType::kI420, codec_.simulcastStream[idx].width,
                       codec_.simulcastStream[idx].height);
        encoded_images_[i].Allocate(new_capacity);
        encoded_images_[i]._completeFrame = true;
        encoded_images_[i]._encodedWidth = codec_.simulcastStream[idx].width;
        encoded_images_[i]._encodedHeight = codec_.simulcastStream[idx].height;
        encoded_images_[i].set_size(0);
    }

    webrtc::SimulcastRateAllocator init_allocator(codec_);


    webrtc::VideoBitrateAllocation allocation = init_allocator.GetAllocation(
      codec_.startBitrate * 1000, codec_.maxFramerate);

    SetRates(RateControlParameters(allocation, codec_.maxFramerate));
    return WEBRTC_VIDEO_CODEC_OK;
 }
  
int32_t FVideoDecoder::RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback* InCallback)
{
     encoded_image_callback_ = InCallback;
     //HWEncoderDetails.Encoder->RegisterListener(*this);
    return 0;
}

int32_t FVideoDecoder::Release() {
        //HWEncoderDetails.Encoder->UnregisterListener(*this);

    SInfo << "Close Encoder H264";

    encoded_image_callback_ = nullptr;


//    while (!encoders_.empty()) {
//        CodecCtx *encoder = encoders_.back();
//        CloseEncoder(encoder);
//        encoders_.pop_back();
//    }
//    configurations_.clear();
//    encoded_images_.clear();
//    encoded_image_buffers_.clear();
    return WEBRTC_VIDEO_CODEC_OK;

}

//int32_t FVideoDecoder::Encode(const webrtc::VideoFrame& frame, const std::vector<webrtc::VideoFrameType>* FrameTypes)
//{
//
//
//
//    FRawFrameBuffer* RawFrame = static_cast<FRawFrameBuffer*>(frame.video_frame_buffer().get());
//
//    wrtc::Peer *peer =  RawFrame->GetPlayer();
//
//    fmp4::BasicFrame *buf = peer->popFrame();
//
//    if(!buf)
//      WEBRTC_VIDEO_CODEC_OK ;
//
//
//	// the frame managed to pass encoder queue so disable frame drop notification
//   // = RawFrame->GetBuffer();
//
//   if ( buf->h264_pars.frameType == H264SframeType::i && buf->h264_pars.slice_type == H264SliceType::idr) //AUD Delimiter
//   {
//
//
////       info->run(buf);
//
//       SDebug << " Key frame " ;
//
//   }
//   if (buf->h264_pars.slice_type == H264SliceType::sps ||  buf->h264_pars.slice_type == H264SliceType::pps) //AUD Delimiter
//   {
//      //  info->run(buf);
//       SDebug << " SPS or PPS " ;
//
//   }
//
//
//
//
//    // sendBuffer is not copied here.
//    webrtc::EncodedImage encodedImage(buf->payload.data(), buf->payload.size(), buf->payload.size());
//
//
//    encodedImage._completeFrame = true;
//    encodedImage._frameType = webrtc::VideoFrameType::kVideoFrameKey;
//
//    // Scan for and create mark all fragments.
//    webrtc::RTPFragmentationHeader fragmentationHeader;
//    uint32_t fragIdx = 0;
//    for (uint32_t i = 0; i < buf->payload.size() - 5; ++i) {
//      uint8_t* ptr = buf->payload.data() + i;
//      int prefixLengthFound = 0;
//      if (ptr[0] == 0x00 && ptr[1] == 0x00 && ptr[2] == 0x00 && ptr[3] == 0x01
//        && ((ptr[4] & 0x1f) != 0x09 /* ignore access unit delimiters */)) {
//        prefixLengthFound = 4;
//      } else if (ptr[0] == 0x00 && ptr[1] == 0x00 && ptr[2] == 0x01
//        && ((ptr[3] & 0x1f) != 0x09 /* ignore access unit delimiters */)) {
//        prefixLengthFound = 3;
//      }
//
//      // Found a key frame, mark is as such in case
//      // MFSampleExtension_CleanPoint wasn't set on the sample.
//      if (prefixLengthFound > 0 && (ptr[prefixLengthFound] & 0x1f) == 0x05) {
//        encodedImage._completeFrame = true;
//        encodedImage._frameType = webrtc::VideoFrameType::kVideoFrameKey;
//
//        SDebug << " Key frame " ;
//
//      }
//
//      if (prefixLengthFound > 0) {
//        fragmentationHeader.VerifyAndAllocateFragmentationHeader(fragIdx + 1);
//        fragmentationHeader.fragmentationOffset[fragIdx] = i + prefixLengthFound;
//        fragmentationHeader.fragmentationLength[fragIdx] = 0;  // We'll set that later
//        // Set the length of the previous fragment.
//        if (fragIdx > 0) {
//          fragmentationHeader.fragmentationLength[fragIdx - 1] =
//            i - fragmentationHeader.fragmentationOffset[fragIdx - 1];
//        }
//        //fragmentationHeader.fragmentationPlType[fragIdx] = 0;
//        //fragmentationHeader.fragmentationTimeDiff[fragIdx] = 0;
//        ++fragIdx;
//        i += 5;
//      }
//    }
//    // Set the length of the last fragment.
//    if (fragIdx > 0) {
//      fragmentationHeader.fragmentationLength[fragIdx - 1] =
//        buf->payload.size() -
//        fragmentationHeader.fragmentationOffset[fragIdx - 1];
//    }
//
//
//
//    encodedImage.SetTimestamp(frame.timestamp());
//
////        encodedImage._encodedWidth = frame.frameWidth;
//   // encodedImage._encodedHeight = frame.frameHeight;
//
//
//    encodedImage.ntp_time_ms_ = frame.ntp_time_ms();
//    encodedImage.capture_time_ms_ = frame.render_time_ms();
//    encodedImage.rotation_ = frame.rotation();
//    //encodedImage.timing_.encode_start_ms = rtc::TimeMicros() / 1000;
//
//    if (Callback != nullptr) {
//
//           SDebug << "Encode frame: " << RawFrame->frameNo  << "  size: " <<  buf->payload.size();
//
//          webrtc::CodecSpecificInfo codecSpecificInfo;
//          codecSpecificInfo.codecType = webrtc::kVideoCodecH264;
//          codecSpecificInfo.codecSpecific.H264.packetization_mode = webrtc::H264PacketizationMode::NonInterleaved;
//          Callback->OnEncodedImage(
//            encodedImage, &codecSpecificInfo, &fragmentationHeader);
//    }
//
//    return WEBRTC_VIDEO_CODEC_OK;
//}


// int32_t FVideoDecoder::SetChannelParameters(uint32_t PacketLoss, int64 Rtt)
// {
// 	return 0;
// }

void FVideoDecoder::SetRates(const RateControlParameters& parameters)
{
	//LastBitrate  = parameters.bitrate;
	//LastFramerate = parameters.framerate_fps;
	//checkNoEntry(); // unexpected call, if even happens, check if passed Bitrate/Framerate should be taken into account
    
    if (encoders_.empty()) {
//    RTC_LOG(LS_WARNING) << "SetRates() while uninitialized.";
    return;
  }

  if (parameters.framerate_fps < 1.0) {
  //  RTC_LOG(LS_WARNING) << "Invalid frame rate: " << parameters.framerate_fps;
    return;
  }

  if (parameters.bitrate.get_sum_bps() == 0) {
    // Encoder paused, turn off all encoding.
    for (size_t i = 0; i < configurations_.size(); ++i)
      configurations_[i].SetStreamState(false);
    return;
  }

  // At this point, bitrate allocation should already match codec settings.
  if (codec_.maxBitrate > 0)
    RTC_DCHECK_LE(parameters.bitrate.get_sum_kbps(), codec_.maxBitrate);
  RTC_DCHECK_GE(parameters.bitrate.get_sum_kbps(), codec_.minBitrate);
  if (codec_.numberOfSimulcastStreams > 0)
    RTC_DCHECK_GE(parameters.bitrate.get_sum_kbps(),
                  codec_.simulcastStream[0].minBitrate);

  codec_.maxFramerate = static_cast<uint32_t>(parameters.framerate_fps);

  size_t stream_idx = encoders_.size() - 1;
  for (size_t i = 0; i < encoders_.size(); ++i, --stream_idx) {
    // Update layer config.
    configurations_[i].target_bps =
        parameters.bitrate.GetSpatialLayerSum(stream_idx);
    configurations_[i].max_frame_rate = parameters.framerate_fps;

    if (configurations_[i].target_bps) {
      configurations_[i].SetStreamState(true);

      // Update h264 encoder.
//      SBitrateInfo target_bitrate;
//      memset(&target_bitrate, 0, sizeof(SBitrateInfo));
//      target_bitrate.iLayer = SPATIAL_LAYER_ALL,
//      target_bitrate.iBitrate = configurations_[i].target_bps;
//      encoders_[i]->SetOption(ENCODER_OPTION_BITRATE, &target_bitrate);
//      encoders_[i]->SetOption(ENCODER_OPTION_FRAME_RATE, &configurations_[i].max_frame_rate);
    } else {
      configurations_[i].SetStreamState(false);
    }
  }
    
}



webrtc::VideoFrameType FVideoDecoder::ConvertToVideoFrameType(AVFrame *frame) {
    switch (frame->pict_type) {
        case AV_PICTURE_TYPE_I:
            if (frame->key_frame) {
                return webrtc::VideoFrameType::kVideoFrameKey;
            }
        case AV_PICTURE_TYPE_P:
        case AV_PICTURE_TYPE_B:
        case AV_PICTURE_TYPE_S:
        case AV_PICTURE_TYPE_SI:
        case AV_PICTURE_TYPE_SP:
        case AV_PICTURE_TYPE_BI:
            return webrtc::VideoFrameType::kVideoFrameDelta;
        case AV_PICTURE_TYPE_NONE:
            break;
    }
    SWarn << ": Unexpected/invalid frame type: " <<  std::to_string(frame->pict_type);
    return webrtc::VideoFrameType::kEmptyFrame;
}



// Helper method used by FFmpegH264EncoderImpl::Encode.
// Copies the encoded bytes from |info| to |encoded_image| and updates the
// fragmentation information of |frag_header|. The |encoded_image->_buffer| may
// be deleted and reallocated if a bigger buffer is required.
//
// After OpenH264 encoding, the encoded bytes are stored in |info| spread out
// over a number of layers and "NAL units". Each NAL unit is a fragment starting
// with the four-byte start code {0,0,0,1}. All of this data (including the
// start codes) is copied to the |encoded_image->_buffer| and the |frag_header|
// is updated to point to each fragment, with offsets and lengths set as to
// exclude the start codes.
void FVideoDecoder::RtpFragmentize(webrtc::EncodedImage *encoded_image,
                                           std::unique_ptr<uint8_t[]> *encoded_image_buffer,
                                           const webrtc::VideoFrameBuffer &frame_buffer, AVPacket *packet,
                                           webrtc::RTPFragmentationHeader *frag_header) {
    std::list<int> data_start_index;
    std::list<int> data_length;
    int payload_length = 0;
    for (int i = 2; i < packet->size; i++) {
        if (i > 2
            && packet->data[i - 3] == start_code[0]
            && packet->data[i - 2] == start_code[1]
            && packet->data[i - 1] == start_code[2]
            && packet->data[i] == start_code[3]) {
            if (!data_start_index.empty()) {
                data_length.push_back((i - 3 - data_start_index.back()));
            }
            data_start_index.push_back(i + 1);
        } else if (packet->data[i - 2] == start_code[1] &&
                   packet->data[i - 1] == start_code[2] &&
                   packet->data[i] == start_code[3]) {
            if (!data_start_index.empty()) {
                data_length.push_back((i - 2 - data_start_index.back()));
            }
            data_start_index.push_back(i + 1);
        }
    }
    if (!data_start_index.empty()) {
        data_length.push_back((packet->size - data_start_index.back()));
    }

    for (auto &it : data_length) {
        payload_length += +it;
    }
    // Calculate minimum buffer size required to hold encoded data.
    auto required_size = payload_length + data_start_index.size() * 4;
    
    
       
    if (encoded_image->size() < required_size) {
        // Increase buffer size. Allocate enough to hold an unencoded image, this
        // should be more than enough to hold any encoded data of future frames of
        // the same size (avoiding possible future reallocation due to variations in
        // required size).
        
        
         size_t new_capacity = CalcBufferSize(webrtc::VideoType::kI420, frame_buffer.width(),
                                         frame_buffer.height());
        if (new_capacity < required_size) {
          // Encoded data > unencoded data. Allocate required bytes.
           SWarn
              << "Encoding produced more bytes than the original image "
              << "data! Original bytes: " << new_capacity
              << ", encoded bytes: " << required_size << ".";
          new_capacity = required_size;
        }
        encoded_image->Allocate(new_capacity);

    }
    // Iterate layers and NAL units, note each NAL unit as a fragment and copy
    // the data to |encoded_image->_buffer|.
    int index = 0;
    encoded_image->set_size(0);
    
    
    
    std::vector<webrtc::H264::NaluIndex> NALUIndices = webrtc::H264::FindNaluIndices(packet->data, packet->size);
                //webrtc::H264::FindNaluIndices(  Cookie->EncodedImage.buffer(), Cookie->EncodedImage.capacity() );
    for (webrtc::H264::NaluIndex index : NALUIndices) 
    {
          webrtc::H264::NaluType nalu_type = webrtc::H264::ParseNaluType(packet->data[index.payload_start_offset]);
//          SInfo <<  " nalu:" << nalu_type << " payload_start_offset:" << index.payload_start_offset << " start_offset:" << index.start_offset << " size:" << index.payload_size;

    }              
                
    
    
    frag_header->VerifyAndAllocateFragmentationHeader(data_start_index.size());
    for (auto it_start = data_start_index.begin(), it_length = data_length.begin();
         it_start != data_start_index.end(); ++it_start, ++it_length, ++index) {
        memcpy(encoded_image->data()+ encoded_image->size(), start_code, sizeof(start_code));
        encoded_image->set_size(encoded_image->size() + sizeof(start_code));
        frag_header->fragmentationOffset[index] = encoded_image->size();
        memcpy(encoded_image->data() + encoded_image->size(), packet->data + *it_start,
               static_cast<size_t>(*it_length));
        //encoded_image->_length += *it_length;
        encoded_image->set_size(encoded_image->size() + *it_length);
        frag_header->fragmentationLength[index] = static_cast<size_t>(*it_length);
        
//        SInfo <<  " RtpFragmentize index " <<  index <<  " size" <<  encoded_image->size() ;
    }
}







int32_t FVideoDecoder::Encode(const webrtc::VideoFrame& input_frame, const std::vector<webrtc::VideoFrameType>* frame_types)
{
    //SInfo << "Encode";
    
    if (encoders_.empty()) {
        ReportError();
        return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
    }
    if (!encoded_image_callback_) {
        SWarn  << "InitEncode() has been called, but a callback function "
            << "has not been set with RegisterEncodeCompleteCallback()";
        ReportError();
        return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
    }

    webrtc::I420BufferInterface *frame_buffer = (webrtc::I420BufferInterface *) input_frame.video_frame_buffer().get();

    bool send_key_frame = false;
    for (auto &configuration : configurations_) {
        if (configuration.key_frame_request && configuration.sending) {
            send_key_frame = true;
            break;
        }
    }
    if (!send_key_frame && frame_types) {
        for (size_t i = 0; i < frame_types->size() && i < configurations_.size();
             ++i) {
            if ((*frame_types)[i] == webrtc::VideoFrameType::kVideoFrameKey && configurations_[i].sending) {
                send_key_frame = true;
                break;
            }
        }
    }

    RTC_DCHECK_EQ(configurations_[0].width, frame_buffer->width());
    RTC_DCHECK_EQ(configurations_[0].height, frame_buffer->height());

    // Encode image for each layer.
    for (size_t i = 0; i < encoders_.size(); ++i) {
        // EncodeFrame input.
      
        if (!configurations_[i].sending) {
            continue;
        }
        if (frame_types != nullptr) {
            // Skip frame?
            if ((*frame_types)[i] == webrtc::VideoFrameType::kEmptyFrame) {
                continue;
            }
        }
        if (send_key_frame || encoders_[i]->frame->pts % configurations_[i].key_frame_interval == 0) {
            // API doc says ForceIntraFrame(false) does nothing, but calling this
            // function forces a key frame regardless of the |bIDR| argument's value.
            // (If every frame is a key frame we get lag/delays.)
            encoders_[i]->frame->key_frame = 1;
            encoders_[i]->frame->pict_type = AV_PICTURE_TYPE_I;
            configurations_[i].key_frame_request = false;
        } else {
            encoders_[i]->frame->key_frame = 0;
            encoders_[i]->frame->pict_type = AV_PICTURE_TYPE_P;
        }
        
        
         AVFrame *frame =  encoders_[i]->frame;
         AVCodecContext *c = encoders_[i]->context;
         int y,x;
         
         av_init_packet(encoders_[i]->pkt);
         encoders_[i]->pkt->data = NULL;    // packet data will be allocated by the encoder
         encoders_[i]->pkt->size = 0;


          /* make sure the frame data is writable */
        int ret = av_frame_make_writable(encoders_[i]->frame);
        if (ret < 0) {
              SError <<  "FFMPEG send frame failed, returned "  << std::to_string(ret);
            ReportError();
            return WEBRTC_VIDEO_CODEC_ERROR;
        }



       

        static int ii = 0;
        
          copyFrame(encoders_[i]->frame, frame_buffer);
        
//        for (y = 0; y < c->height; y++) {
//            for (x = 0; x < c->width; x++) {
//                frame->data[0][y * frame->linesize[0] + x] = x + y + ii * 3;
//            }
//        }
//
//        /* Cb and Cr */
//        for (y = 0; y < c->height/2; y++) {
//            for (x = 0; x < c->width/2; x++) {
//                frame->data[1][y * frame->linesize[1] + x] = 128 + y + ii * 2;
//                frame->data[2][y * frame->linesize[2] + x] = 64 + x + ii * 5;
//            }
//        }
//        ii = ++ii %25 ;
        
        frame->pts = ++encoderInc;;

        int got_output;
        ret = avcodec_encode_video2(encoders_[i]->context, encoders_[i]->pkt, encoders_[i]->frame, &got_output);
        if (ret < 0) {
              SError <<  "FFMPEG send frame failed, returned "  << std::to_string(ret);
            ReportError();
            return WEBRTC_VIDEO_CODEC_ERROR;
        }



//        SInfo << "frame type " << (int) ConvertToVideoFrameType(encoders_[i]->frame);

           if (got_output)
           {
               
               
            fwrite(encoders_[i]->pkt->data, 1, encoders_[i]->pkt->size, fp);
                

            encoded_images_[i]._encodedWidth = static_cast<uint32_t>(configurations_[i].width);
            encoded_images_[i]._encodedHeight = static_cast<uint32_t>(configurations_[i].height);
            encoded_images_[i].SetTimestamp(input_frame.timestamp());
            encoded_images_[i].ntp_time_ms_ = input_frame.ntp_time_ms();
            encoded_images_[i].capture_time_ms_ = input_frame.render_time_ms();
            encoded_images_[i].rotation_ = input_frame.rotation();
            encoded_images_[i].content_type_ =
                    (codec_.mode == webrtc::VideoCodecMode::kScreensharing)
                    ? webrtc::VideoContentType::SCREENSHARE
                    : webrtc::VideoContentType::UNSPECIFIED;
            encoded_images_[i].timing_.flags = webrtc::VideoSendTiming::kInvalid;
            encoded_images_[i]._frameType = ConvertToVideoFrameType(encoders_[i]->frame);
            
            
           // SInfo <<  "encode frame no " << encoderInc <<  " type "  << (int) encoded_images_[i]._frameType << " w " << configurations_[i].width << " h " << configurations_[i].height;

            // Split encoded image up into fragments. This also updates
            // |encoded_image_|.
            webrtc::RTPFragmentationHeader frag_header;
            RtpFragmentize(&encoded_images_[i], &encoded_image_buffers_[i], *frame_buffer, encoders_[i]->pkt, &frag_header);
           // av_packet_unref(encoders_[i]->pkt);
            // Encoder can skip frames to save bandwidth in which case
            // |encoded_images_[i]._length| == 0.
            if (encoded_images_[i].size() > 0) {
                // Parse QP.

                h264_bitstream_parser_.ParseBitstream(encoded_images_[i].data(),
                                            encoded_images_[i].size());
                h264_bitstream_parser_.GetLastSliceQp(&encoded_images_[i].qp_);


                // Deliver encoded image.
                webrtc::CodecSpecificInfo codec_specific;
                //codec_specific.codecType = webrtc::kVideoCodecH264;
               // codec_specific.codecSpecific.H264.packetization_mode =
                    //    packetization_mode_;
                //codec_specific.codecSpecific.H264.simulcast_idx = static_cast<uint8_t>(configurations_[i].simulcast_idx);

                codec_specific.codecType =  webrtc::kVideoCodecH264;;
                codec_specific.codecSpecific.H264.packetization_mode =    webrtc::H264PacketizationMode::NonInterleaved;
                codec_specific.codecSpecific.H264.temporal_idx = -1; // kNoTemporalIdx = -1.
                  //  codec_specific.codecSpecific.H264.idr_frame =    info.eFrameType == videoFrameTypeIDR;
                codec_specific.codecSpecific.H264.base_layer_sync = false;


                encoded_image_callback_->OnEncodedImage(encoded_images_[i],
                                                        &codec_specific, &frag_header);
            }
        }
    }

    return WEBRTC_VIDEO_CODEC_OK;
}


bool FVideoDecoder::OpenEncoder(FVideoDecoder::CodecCtx *ctx, FVideoDecoder::LayerConfig &config) {
    int ret;
    /* find the mpeg1 video encoder */
#ifdef WEBRTC_LINUX
    if (hardware_accelerate) {
        ctx->codec = avcodec_find_encoder_by_name("h264_nvenc");
    }
#endif
    if (!ctx->codec) {
        ctx->codec = avcodec_find_encoder_by_name("libx264");
    }
    if (!ctx->codec) {
        SError <<  "Codec not found" ;
        return false;
    }

    
   
    

    ctx->context = avcodec_alloc_context3(ctx->codec);
    if (!ctx->context) {
        SError <<  "Could not allocate video codec context";
        return false;
    }
  //  config.target_bps = config.max_bps;
    
    
//        /* put sample parameters */
//    ctx->context->bit_rate = 400000;
//    /* resolution must be a multiple of two */
//    ctx->context->width = 720;
//    ctx->context->height = 576;
//    /* frames per second */
//    ctx->context->time_base = (AVRational){1, 25};
//    ctx->context->framerate = (AVRational){25, 1};
//
//    /* emit one intra frame every ten frames
//     * check frame pict_type before passing frame
//     * to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
//     * then gop_size is ignored and the output of encoder
//     * will always be I frame irrespective to gop_size
//     */
//    ctx->context->gop_size = 25;
//  //  c->max_b_frames = 1;
//    ctx->context->max_b_frames = 0;
//    ctx->context->pix_fmt = AV_PIX_FMT_YUV420P;
//   
//    ctx->context->color_range = AVCOL_RANGE_JPEG;
    
    
    SetContext(ctx, config, true);
    /* open it */
    ret = avcodec_open2(ctx->context, ctx->codec, nullptr);
    if (ret < 0) {
         SError << "Could not open codec, error code:" <<  std::to_string(ret);
        avcodec_free_context(&(ctx->context));
        return false;
    }

    ctx->frame = av_frame_alloc();
    if (!ctx->frame) {
         SError << "Could not allocate video frame";
        return false;
    }
    ctx->frame->format = ctx->context->pix_fmt;
    ctx->frame->width = ctx->context->width;
    ctx->frame->height = ctx->context->height;
    ctx->frame->color_range = ctx->context->color_range;




     SInfo <<  "Open encoder: "  <<  std::string(ctx->codec->name)  <<  " width " <<     ctx->frame->width  << " height " <<   ctx->frame->height;

    /* the image can be allocated by any means and av_image_alloc() is
     * just the most convenient way if av_malloc() is to be used */
    //ret = av_image_alloc(ctx->frame->data, ctx->frame->linesize, ctx->context->width, ctx->context->height,
    //                     ctx->context->pix_fmt, 32);

    ret = av_frame_get_buffer(ctx->frame, 32);

    if (ret < 0) {
         SError << ": Could not allocate raw picture buffer";
        return false;
    }
    //ctx->frame->pts = 1;
    ctx->pkt = av_packet_alloc();
    
    AVPacket *pkt =  ctx->pkt;
    //////////////////////////////////////////////////////////
    
//    AVFrame *frame =  ctx->frame;
//    AVCodecContext *c =ctx->context;
//    int y,x,i;
//    int got_output;
//    
//    for (i = 0; i < 625; i++) {
//         av_init_packet(pkt);
//         pkt->data = NULL;    // packet data will be allocated by the encoder
//         pkt->size = 0;
//
//       // fflush(stdout);
//
//        /* make sure the frame data is writable */
//        ret = av_frame_make_writable(frame);
//        if (ret < 0)
//            exit(1);
//
//        /* prepare a dummy image */
//        /* Y */
//        for (y = 0; y < c->height; y++) {
//            for (x = 0; x < c->width; x++) {
//                frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
//            }
//        }
//
//        /* Cb and Cr */
//        for (y = 0; y < c->height/2; y++) {
//            for (x = 0; x < c->width/2; x++) {
//                frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
//                frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
//            }
//        }
//
//        frame->pts = i;
//
//        /* encode the image */
//        int ret = avcodec_encode_video2(c, pkt, frame, &got_output);
//        if (ret < 0) {
//            fprintf(stderr, "Error encoding frame\n");
//            exit(1);
//        }
//
//        if (got_output) {
//            printf("Write frame %3d (size=%5d)\n", i, pkt->size);
//           // fwrite(pkt->data, 1, pkt->size, f);
//           // av_packet_unref(&pkt);
//        }
//    }

    
    
    
    
    /////////////////////////////////////////////////////////
    return true;
}

void FVideoDecoder::CloseEncoder(FVideoDecoder::CodecCtx *ctx) {
    if (ctx) {
        if (ctx->context) {
            avcodec_close(ctx->context);
            avcodec_free_context(&(ctx->context));
        }
        if (ctx->frame) {
            av_frame_free(&(ctx->frame));
        }
        if (ctx->pkt) {
            av_packet_free(&(ctx->pkt));
        }
        SInfo << "Close encoder context and release context, frame, packet";
        delete ctx;
    }
}

#ifdef WEBRTC_LINUX
typedef struct NvencDynLoadFunctions
{
    void *cuda_dl;
    void *nvenc_dl;

    NV_ENCODE_API_FUNCTION_LIST nvenc_funcs;
    int nvenc_device_count;
} NvencDynLoadFunctions;

typedef struct NvencContext
{
    AVClass *avclass;

    NvencDynLoadFunctions nvenc_dload_funcs;

    NV_ENC_INITIALIZE_PARAMS init_encode_params;
    NV_ENC_CONFIG encode_config;
} NvencContext;
#endif

void FVideoDecoder::SetContext(CodecCtx *ctx, FVideoDecoder::LayerConfig &config, bool init) {
    if (init) {
        AVRational rational = {1, 25};
        ctx->context->time_base = rational;
        ctx->context->framerate = (AVRational){25, 1};
        ctx->context->max_b_frames = 0;
        ctx->context->pix_fmt = AV_PIX_FMT_YUV420P;
        ctx->context->codec_type = AVMEDIA_TYPE_VIDEO;
        ctx->context->codec_id = AV_CODEC_ID_H264;
        ctx->context->gop_size =  config.key_frame_interval;
        ctx->context->color_range = AVCOL_RANGE_JPEG;
        if (std::string(ctx->codec->name) == "libx264") {
            av_opt_set(ctx->context->priv_data, "preset", "ultrafast", 0);
            av_opt_set(ctx->context->priv_data, "tune", "zerolatency", 0);
           
           // av_opt_set(ctx->context->priv_data, "preset","medium",0);
         //  ctx->context->level = 31;
           //av_opt_set(ctx->context->priv_data, "profile", "main", 0);
            
           // av_opt_set(ctx->context->priv_data, "refs", "4", 0);
           // av_dict_set(&opts, "qpmin", "4", 0);


           // av_opt_set(ctx->context->priv_data, "profile", "high", 0);
            //av_opt_set(ctx->context->priv_data, "level", "4.1", 0); //bluray compatibility level

            //av_opt_set(ctx->context->priv_data, "crf", "17", 0);


            
            

        }
        av_log_set_level(AV_LOG_ERROR);
        SInfo <<  "Init bitrate: " + std::to_string(config.target_bps);
    } else {
        if (config.target_bps == 300000) {
            return;
        }
        SInfo <<  " Change bitrate: " <<  std::to_string(config.target_bps);
    }
    config.key_frame_request = true;
    ctx->context->width = config.width;
    ctx->context->height = config.height;

//    ctx->context->bit_rate = config.target_bps * 0.7;
//    ctx->context->rc_max_rate = config.target_bps * 0.85;
//    ctx->context->rc_min_rate = config.target_bps * 0.1;
//    ctx->context->rc_buffer_size = config.target_bps * 2;
#ifdef WEBRTC_LINUX
    if (std::string(ctx->codec->name) == "h264_nvenc") {
        NvencContext* nvenc_ctx = (NvencContext*)ctx->context->priv_data;
        nvenc_ctx->encode_config.rcParams.averageBitRate = ctx->context->bit_rate;
        nvenc_ctx->encode_config.rcParams.maxBitRate = ctx->context->rc_max_rate;
        return;
    }
#endif


}









void FVideoDecoder::LayerConfig::SetStreamState(bool send_stream) {
    if (send_stream && !sending) {
        // Need a key frame if we have not sent this stream before.
        key_frame_request = true;
    }
    sending = send_stream;
}



    }// ns webrtc
}//ns base
