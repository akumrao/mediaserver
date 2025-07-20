


#include "VideoEncoder.h"
#include "base/logger.h"
#include "absl/strings/match.h"
#include "webrtc/rawVideoFrame.h"

//#include "muxer.h"

#include "common_video/libyuv/include/webrtc_libyuv.h"
#include "modules/video_coding/utility/simulcast_rate_allocator.h"



#include "common_video/h264/sps_parser.h"
#include "common_video/h264/h264_common.h"



#include "base/base64.h"



#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "modules/video_coding/codecs/h264/include/h264.h"



const uint8_t start_code[4] = {0, 0, 0, 1};




namespace base {
    namespace web_rtc {


VideoEncoder::VideoEncoder(en_EncType encType)
//	HWEncoderDetails(InHWEncoderDetails),
//	PlayerSession(&InPlayerSession)
{

	CodecSpecific.codecType = webrtc::kVideoCodecH264;
	// #TODO: Probably smarter setting of `packetization_mode` is required, look at `H264EncoderImpl` ctor
	// CodecSpecific.codecSpecific.H264.packetization_mode = webrtc::H264PacketizationMode::SingleNalUnit;
	CodecSpecific.codecSpecific.H264.packetization_mode = webrtc::H264PacketizationMode::NonInterleaved;

        //info = new fmp4::InfoFrameFilter("arvind", nullptr);
        
//	UE_LOG(PixelStreamer, Log, TEXT("WebRTC VideoEncoder created%s"), bControlsQuality? TEXT(", quality controller"): TEXT(""));
}

VideoEncoder::~VideoEncoder()
{

}



//int32_t VideoEncoder::InitEncode(const webrtc::VideoCodec* codec_settings, const webrtc::VideoEncoder::Settings& settings)
//{
//	return 0;
//}


void VideoEncoder::ReportError() {

    SError << "VideoEncoder::codec error" ;
}


 int32_t VideoEncoder::InitEncode(const webrtc::VideoCodec* CodecSetings, int32_t NumberOfCores, size_t MaxPayloadSize)
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



    configurations_[0].sending = true;
     
    codec_.maxFramerate = 30;
    webrtc::SimulcastRateAllocator init_allocator(codec_);


    webrtc::VideoBitrateAllocation allocation = init_allocator.GetAllocation(
      codec_.startBitrate * 1000, codec_.maxFramerate);

    SetRates(RateControlParameters(allocation, codec_.maxFramerate));
    return WEBRTC_VIDEO_CODEC_OK;
 }
  
int32_t VideoEncoder::RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback* InCallback)
{
     encoded_image_callback_ = InCallback;
     //HWEncoderDetails.Encoder->RegisterListener(*this);
    return 0;
}


void VideoEncoder::SetRates(const RateControlParameters& parameters)
{

    if (encoders_.empty()) {
    SWarn << "Encoder is not initialized";
    return;
  }

  if (parameters.framerate_fps < 1.0) {
    RTC_LOG(LS_WARNING) << "Invalid frame rate: " << parameters.framerate_fps;
    return;
  }

      
   uint32_t bps =    parameters.bitrate.get_sum_bps();
              
  if (bps == 0) {
    // Encoder paused, turn off all encoding.
    for (size_t i = 0; i < configurations_.size(); ++i)
      configurations_[i].SetStreamState(false);
     SInfo << "bitrate " << parameters.bitrate.get_sum_bps();
    return;
  }

  // At this point, bitrate allocation should already match codec settings.
  if (codec_.maxBitrate > 0)
  {
      if(bps >  codec_.maxBitrate*1000)
      {
          SWarn << "get_sum_kbps() "  << bps  <<  " >  codec_.maxBitrate= " <<   codec_.maxBitrate;
      }
  }
      
  if( bps <  codec_.minBitrate*1000)
  {
       SWarn << "get_sum_kbps() <  codec_.maxBitrate";
  }
  
    
    
}



int32_t VideoEncoder::Release() {
        //HWEncoderDetails.Encoder->UnregisterListener(*this);

    SInfo << " VideoEncoder Close Encoder H264";

    encoded_image_callback_ = nullptr;



    return WEBRTC_VIDEO_CODEC_OK;

}

webrtc::VideoFrameType VideoEncoder::ConvertToVideoFrameType(AVFrame *frame) {
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
void VideoEncoder::RtpFragmentize(webrtc::EncodedImage *encoded_image,
                                           std::unique_ptr<uint8_t[]> *encoded_image_buffer, AVPacket *packet,
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
        
        
         size_t new_capacity = CalcBufferSize(webrtc::VideoType::kI420, encoded_image->_encodedWidth, encoded_image->_encodedHeight);
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
    
    
#if 0
    std::vector<webrtc::H264::NaluIndex> NALUIndices = webrtc::H264::FindNaluIndices(packet->data, packet->size);
                //webrtc::H264::FindNaluIndices(  Cookie->EncodedImage.buffer(), Cookie->EncodedImage.capacity() );
    
    SInfo <<  " nalu count  :" << NALUIndices.size();
            
//    for (webrtc::H264::NaluIndex index : NALUIndices) 
//    {
//          webrtc::H264::NaluType nalu_type = webrtc::H264::ParseNaluType(packet->data[index.payload_start_offset]);
//           SInfo <<  " nalu:" << nalu_type << " payload_start_offset:" << index.payload_start_offset << " start_offset:" << index.start_offset << " size:" << index.payload_size;
//
//    }              
#endif                
    
    
    int idata_start =  data_start_index.size();
    if(!metadata.empty())
    {
        idata_start = idata_start + 1;
    }
    
        
    frag_header->VerifyAndAllocateFragmentationHeader(idata_start);
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
    

    //metadata= "<arvind> test  </arvind>";
   
   if(!metadata.empty())
   {
       
        metadata=   base64::encode(metadata);
    
        //  uint32_t some_long = 10;

          uint32_t network_byte_order;

      // convert and send
          network_byte_order = htonl(metadata.size());


          memcpy(encoded_image->data()+ encoded_image->size(), start_code, sizeof(start_code));
          encoded_image->set_size(encoded_image->size() + sizeof(start_code));
          //SInfo <<  " RtpFragmentize index0 " <<  index <<  " size" <<  encoded_image->size() << " metadata.size() " << metadata  ;

          frag_header->fragmentationOffset[index] = encoded_image->size();


          memcpy(encoded_image->data() + encoded_image->size(), metadata.c_str()  , static_cast<size_t>(metadata.size()) );
           encoded_image->set_size(encoded_image->size() + metadata.size());
          //encoded_image->_length += *it_length;

          memcpy(encoded_image->data() + encoded_image->size(), &network_byte_order  , static_cast<size_t>(sizeof(network_byte_order))    );
          //encoded_image->_length += *it_length;
          encoded_image->set_size(encoded_image->size() + sizeof(network_byte_order));


          frag_header->fragmentationLength[index] = static_cast<size_t>( metadata.size() + sizeof(network_byte_order));

	//  SInfo <<  " RtpFragmentize index1 " <<  index <<  " size" <<  encoded_image->size() << " *it_length " << metadata.size() + sizeof(network_byte_order) ;
	    
	    
//	    for( int x = 0 ; x < 6 ; ++x )
//	    {
//		
//		
//		SInfo << "  frag_header->fragmentationOffset "  <<   frag_header->fragmentationOffset[x]; 
//
//		SInfo <<  "  frag_header->fragmentationLength "  <<  frag_header->fragmentationLength[x];
//		    
//		
//	    }
   }
    
    //     SInfo <<  " RtpFragmentize index " <<  index <<  " size" <<  encoded_image->size() << " *it_length " << sizeof(myTest) ;
 
}



int32_t VideoEncoder::Encode(const webrtc::VideoFrame& input_frame, const std::vector<webrtc::VideoFrameType>* frame_types)
{

    if (!encoded_image_callback_) {
        SWarn  << "InitEncode() has been called, but a callback function "
            << "has not been set with RegisterEncodeCompleteCallback()";
        ReportError();
        return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
    }

    //webrtc::I420BufferInterface *frame_buffer = (webrtc::I420BufferInterface *) input_frame.video_frame_buffer().get();
    
    H264FrameBuffer* frame_buffer = static_cast<H264FrameBuffer*> (input_frame.video_frame_buffer().get());
      
            
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
                //SInfo << " send_key_frame " << send_key_frame;
                break;
            }
        }
    }
    
    
    if ( frame_types == nullptr || !configurations_[0].sending ||  (*frame_types)[0] == webrtc::VideoFrameType::kEmptyFrame) {
        
        SInfo << "Pause return ";
        return WEBRTC_VIDEO_CODEC_OK;
    }
    
    
    if( webrtc::VideoFrameBuffer::Type::kI420  ==  frame_buffer->type()) // pause or muted condtion
    {
          return WEBRTC_VIDEO_CODEC_OK;
    }
     

//    RTC_DCHECK_EQ(configurations_[0].width, frame_buffer->width());
//    RTC_DCHECK_EQ(configurations_[0].height, frame_buffer->height());


    // Encode image for each layer.
        
        
    if (send_key_frame )
    {
        // API doc says ForceIntraFrame(false) does nothing, but calling this
        // function forces a key frame regardless of the |bIDR| argument's value.
        // (If every frame is a key frame we get lag/delays.)
        frame_buffer->qframe->key_frame = 1;
        frame_buffer->qframe->pict_type = AV_PICTURE_TYPE_I;
        configurations_[0].key_frame_request = false;
        
        SInfo << " Send Key frame" ;
        
    } 
    
    
    int i = 0;  

  //  fwrite(encoders_[i]->pkt->data, 1, encoders_[i]->pkt->size, fp);


    encoded_images_[i]._encodedWidth = static_cast<uint32_t>(frame_buffer->width());
    encoded_images_[i]._encodedHeight = static_cast<uint32_t>(frame_buffer->height());
    encoded_images_[i].SetTimestamp(input_frame.timestamp());
    encoded_images_[i].ntp_time_ms_ = input_frame.ntp_time_ms();
    encoded_images_[i].capture_time_ms_ = input_frame.render_time_ms();
    encoded_images_[i].rotation_ = input_frame.rotation();

    
    encoded_images_[i].content_type_ =
          (codec_.mode == webrtc::VideoCodecMode::kScreensharing)
          ? webrtc::VideoContentType::SCREENSHARE
          : webrtc::VideoContentType::UNSPECIFIED;
   // encoded_images_[i].timing_.flags = webrtc::VideoSendTiming::kInvalid;
    //encoded_images_[i]._frameType = ConvertToVideoFrameType(frame_buffer->qframe);
   
   
   
   
    encoded_images_[i].timing_.encode_start_ms = rtc::TimeMicros() / 1000;

    encoded_images_[i].timing_.encode_finish_ms = (rtc::TimeMicros() / 1000) + 10;;
    encoded_images_[i].timing_.flags = webrtc::VideoSendTiming::kTriggeredByTimer;


    encoded_images_[i]._completeFrame = true;

    metadata =  frame_buffer->txt;
    
    //SInfo <<  this <<   " size " <<   frame_buffer->qframe << " frm " << frame_buffer->frmNo;
    
//    encodeFrame( frame_buffer->dec_ctx, frame_buffer->qframe ,frame_buffer->width(), frame_buffer->height() );
    
  

    return WEBRTC_VIDEO_CODEC_OK;
}




void VideoEncoder::LayerConfig::SetStreamState(bool send_stream) {
    if (send_stream && !sending) {
        // Need a key frame if we have not sent this stream before.
        key_frame_request = true;
    }
    sending = send_stream;
}




    }// ns webrtc
}//ns base


