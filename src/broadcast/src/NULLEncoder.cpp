/*
 * https://www.beikejiedeliulangmao.top/webrtc/use-in-java/port-limit-and-encode/
 * Restrict connection ports
Looking back at the previous completion process of port restriction, when creating PeerConnectionFactory , we instantiated a SocketFactory and a default NetworkManager, and then when creating PeerConnection , we created a PortAllocator through these two instances, and set the PortAllocator Inject into PeerConnection. In the whole process, the code that actually performs port restriction is in SocketFactory, of course, the API of PortAllocator is also used. Here you may have doubts, isn't there an interface in PortAllocator that can limit the port range, why do you need SocketFactory?

std::unique_ptr<cricket::PortAllocator> port_allocator(
new cricket::BasicPortAllocator(network_manager.get(), socket_factory.get()));
port_allocator->SetPortRange(this->min_port, this->max_port); // Port allocator
I only set the port through this API at the time, but I found that it would still apply for ports beyond the limit to do other things, so in the end I directly rewritten SocketFactory and banned all illegal port applications. In addition, because there are still some subnet IPs that cannot be used on our server, I also processed them in SocketFactory. My implementation is as follows:

rtc::AsyncPacketSocket *
rtc::SocketFactoryWrapper::CreateUdpSocket(const rtc::SocketAddress &local_address, uint16_t min_port,
                                            uint16_t max_port) {
   
    if (min_port < this->min_port || max_port > this->max_port) {
        WEBRTC_LOG("Create udp socket cancelled, port out of range, expect port range is:" +
                    std::to_string(this->min_port) + "->" + std::to_string(this->max_port)
                    + "parameter port range is: " + std::to_string(min_port) + "->" + std::to_string(max_port),
                    LogLevel::INFO);
        return nullptr;
    }
    // 
    if (!local_address.IsPrivateIP() || local_address.HostAsURIString().find(this->white_private_ip_prefix) == 0) {
        rtc::AsyncPacketSocket *result = BasicPacketSocketFactory::CreateUdpSocket(local_address, min_port, max_port);
        const auto *address = static_cast<const void *>(result);
        std::stringstream ss;
        ss << address;
        WEBRTC_LOG("Create udp socket, min port is:" + std::to_string(min_port) + ", max port is: " +
                    std::to_string(max_port) + ", result is: " + result->GetLocalAddress().ToString() + "->" +
                    result->GetRemoteAddress().ToString() + ", new socket address is: " + ss.str(), LogLevel::INFO);

        return result;
    } else {
        WEBRTC_LOG("Create udp socket cancelled, this ip is not in while list:" + local_address.HostAsURIString(),
                    LogLevel::INFO);
        return nullptr;
    }
}
*/



#include "NULLEncoder.h"
#include "base/logger.h"
#include "absl/strings/match.h"
#include "webrtc/rawVideoFrame.h"

//#include "muxer.h"

#include "common_video/libyuv/include/webrtc_libyuv.h"
#include "modules/video_coding/utility/simulcast_rate_allocator.h"
#include "tools.h"

#include "base/base64.h"

#include "common_video/h264/sps_parser.h"
#include "common_video/h264/h264_common.h"

#ifdef WEBRTC_LINUX
#include <ffnvcodec/nvEncodeAPI.h>
#endif

#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "modules/video_coding/codecs/h264/include/h264.h"

const size_t kYPlaneIndex = 0;
const size_t kUPlaneIndex = 1;
const size_t kVPlaneIndex = 2;


const uint8_t start_code[4] = {0, 0, 0, 1};


//#include "rtc_base/ref_counted_object.h"
//#include "rtc_base/atomic_ops.h"

//////////////////////////////////////////////////////////////////////////

namespace base {
    namespace web_rtc {


       int NULLEncoder::nativeInstance = 0;
            
        NULLEncoder::NULLEncoder()//:
        {

            ++nativeInstance;

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

        NULLEncoder::~NULLEncoder() {
              --nativeInstance;
                         
            SInfo << "~NULLEncoder()";
              
        }

        void NULLEncoder::SetQualityController(bool bControlsQualityNow) {
            SInfo << "NULLEncoder::SetQualityController";
        }

        //int32_t NULLEncoder::InitEncode(const webrtc::VideoCodec* codec_settings, const webrtc::VideoEncoder::Settings& settings)
        //{
        //	return 0;
        //}

        void NULLEncoder::ReportError() {

            SError << "NULLEncoder::codec error";
            
          //  stop();
        }

        int32_t NULLEncoder::InitEncode(const webrtc::VideoCodec* CodecSetings, int32_t NumberOfCores, size_t MaxPayloadSize) {
            
            SInfo << "NULLEncoder::InitEncoder";
            
                    
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

            int number_of_streams = 1; //webrtc::SimulcastUtility::NumberOfSimulcastStreams(*CodecSetings);
            bool doing_simulcast = (number_of_streams > 1);

            //    if (doing_simulcast && (!webrtc::SimulcastUtility::ValidSimulcastResolutions(
            //            *CodecSetings, number_of_streams) ||
            //                            !webrtc::SimulcastUtility::ValidSimulcastTemporalLayers(
            //                                    *CodecSetings, number_of_streams))) {
            //        return WEBRTC_VIDEO_CODEC_ERR_SIMULCAST_PARAMETERS_NOT_SUPPORTED;
            //    }
            encoded_images_.resize(static_cast<unsigned long> (number_of_streams));
            encoded_image_buffers_.resize(static_cast<unsigned long> (number_of_streams));
            configurations_.resize(static_cast<unsigned long> (number_of_streams));
          
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

               // codec_.H264()->keyFrameInterval = 33;
                 codec_.maxFramerate = 25;
                
                
                // Set internal settings from codec_settings
                configurations_[i].simulcast_idx = idx;
                configurations_[i].sending = false;
                configurations_[i].width = codec_.simulcastStream[idx].width;
                configurations_[i].height = codec_.simulcastStream[idx].height;
                configurations_[i].max_frame_rate = static_cast<float> (codec_.maxFramerate);
                configurations_[i].frame_dropping_on = false ;// codec_.H264()->frameDroppingOn;
                configurations_[i].key_frame_interval = codec_.H264()->keyFrameInterval;

                // Codec_settings uses kbits/second; encoder uses bits/second.
                configurations_[i].max_bps = codec_.maxBitrate * 1000;
                configurations_[i].target_bps = codec_.startBitrate * 1000;
                
                // Initialize encoded image. Default buffer size: size of unencoded data.
//                const size_t new_capacity = CalcBufferSize(webrtc::VideoType::kI420, codec_.simulcastStream[idx].width,
//                        codec_.simulcastStream[idx].height);
//                encoded_images_[i].Allocate(new_capacity);
//                encoded_images_[i]._completeFrame = true;
//                encoded_images_[i]._encodedWidth = codec_.simulcastStream[idx].width;
//                encoded_images_[i]._encodedHeight = codec_.simulcastStream[idx].height;
                encoded_images_[i].set_size(0);
                
                configurations_[i].sending = true;
                
            }

            webrtc::SimulcastRateAllocator init_allocator(codec_);


            allocation = init_allocator.GetAllocation(
                    codec_.startBitrate * 1000, codec_.maxFramerate);

            SetRates(RateControlParameters(allocation, codec_.maxFramerate));
            return WEBRTC_VIDEO_CODEC_OK;
        }

        int32_t NULLEncoder::RegisterEncodeCompleteCallback(webrtc::EncodedImageCallback* InCallback) {
            
           // en_im_cb.lock();
            encoded_image_callback_ = InCallback;
           // en_im_cb.unlock();
            //HWEncoderDetails.Encoder->RegisterListener(*this);
            return 0;
        }

        int32_t NULLEncoder::Release() {
            //HWEncoderDetails.Encoder->UnregisterListener(*this);
            SInfo << "NULLEncoder::Release";
           
           // en_im_cb.lock();
            encoded_image_callback_ = nullptr;
           // en_im_cb.unlock();


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


        void NULLEncoder::SetRates(const RateControlParameters& parameters) {
            
            // SInfo << "NULLEncoder::SetRates";
             
            //LastBitrate  = parameters.bitrate;
            //LastFramerate = parameters.framerate_fps;
            //checkNoEntry(); // unexpected call, if even happens, check if passed Bitrate/Framerate should be taken into account


            if (parameters.framerate_fps < 1.0) {
                RTC_LOG(LS_WARNING) << "Invalid frame rate: " << parameters.framerate_fps;
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

            codec_.maxFramerate = static_cast<uint32_t> (parameters.framerate_fps);
 
           
                
            

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

        void NULLEncoder::RtpFragmentize(webrtc::EncodedImage *encoded_image,
                std::unique_ptr<uint8_t[]> *encoded_image_buffer,
                 int w, int h, std::vector<uint8_t> &packet,
                webrtc::RTPFragmentationHeader *frag_header) {
            std::list<int> data_start_index;
            std::list<int> data_length;
            int payload_length = 0;
            for (int i = 2; i < packet.size(); i++) {
                if (i > 2
                        && packet.data()[i - 3] == start_code[0]
                        && packet.data()[i - 2] == start_code[1]
                        && packet.data()[i - 1] == start_code[2]
                        && packet.data()[i] == start_code[3]) {
                    if (!data_start_index.empty()) {
                        data_length.push_back((i - 3 - data_start_index.back()));
                    }
                    data_start_index.push_back(i + 1);
                } else if (packet.data()[i - 2] == start_code[1] &&
                        packet.data()[i - 1] == start_code[2] &&
                        packet.data()[i] == start_code[3]) {
                    if (!data_start_index.empty()) {
                        data_length.push_back((i - 2 - data_start_index.back()));
                    }
                    data_start_index.push_back(i + 1);
                }
            }
            if (!data_start_index.empty()) {
                data_length.push_back((packet.size() - data_start_index.back()));
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


                size_t new_capacity = CalcBufferSize(webrtc::VideoType::kI420, w,
                       h);
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
            std::vector<webrtc::H264::NaluIndex> NALUIndices = webrtc::H264::FindNaluIndices(packet.data(), packet.size());
            //webrtc::H264::FindNaluIndices(  Cookie->EncodedImage.buffer(), Cookie->EncodedImage.capacity() );
             SInfo <<  " nalu count  :" << NALUIndices.size();
//            for (webrtc::H264::NaluIndex index : NALUIndices) {
//                webrtc::H264::NaluType nalu_type = webrtc::H264::ParseNaluType(packet.data()[index.payload_start_offset]);
//                SInfo << " nalu:" << nalu_type << " payload_start_offset:" << index.payload_start_offset << " start_offset:" << index.start_offset << " size:" << index.payload_size;
//
//            }
#endif


            int idata_start =  data_start_index.size();
            if(!metadata.empty())
            {
                idata_start = idata_start + 1;
            }
            frag_header->VerifyAndAllocateFragmentationHeader(idata_start);
    
            for (auto it_start = data_start_index.begin(), it_length = data_length.begin();
                    it_start != data_start_index.end(); ++it_start, ++it_length, ++index) {
                memcpy(encoded_image->data() + encoded_image->size(), start_code, sizeof (start_code));
                encoded_image->set_size(encoded_image->size() + sizeof (start_code));
                frag_header->fragmentationOffset[index] = encoded_image->size();
                memcpy(encoded_image->data() + encoded_image->size(), packet.data() + *it_start,
                        static_cast<size_t> (*it_length));
                //encoded_image->_length += *it_length;
                encoded_image->set_size(encoded_image->size() + *it_length);
                frag_header->fragmentationLength[index] = static_cast<size_t> (*it_length);

              //  SInfo << " RtpFragmentize index " << index << " size" << encoded_image->size();
            }
               
               
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

            }
            
        
        }

        
        
        int32_t NULLEncoder::Encode(const webrtc::VideoFrame& input_frame, const std::vector<webrtc::VideoFrameType>* frame_types) 
        {
          //  SInfo << "Encode";

          
            if (!encoded_image_callback_) {
                SWarn << "InitEncode() has been called, but a callback function "
                        << "has not been set with RegisterEncodeCompleteCallback()";
                ReportError();
                return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
            }
            

           //  SInfo << "NULLEncoder::Encode";

            NULLEncBuffer* RawFrame = static_cast<NULLEncBuffer*> (input_frame.video_frame_buffer().get());
            
            
            if( webrtc::VideoFrameBuffer::Type::kI420  ==  RawFrame->type()) // pause or muted condtion
            {
                  return WEBRTC_VIDEO_CODEC_OK;
            }
            

          ///  SInfo <<  "frame no "  <<   RawFrame->frameNo   ;
            
//            payload.timestamp = input_frame.timestamp();
//            payload.ntp_time_ms_ = input_frame.ntp_time_ms();
//            payload.capture_time_ms_ = input_frame.render_time_ms();
            

//            mt.lock();
//            queue.push(payload );
//            mt.unlock();
            
//            if( codec_.maxFramerate != RawFrame->fps)
//            {
//                codec_.maxFramerate = RawFrame->fps;
//                SetRates(RateControlParameters(allocation, codec_.maxFramerate));
//            }
            
            
                // compute frametype
            //uint8_t* data =  payload.data();
           
            
           // RTC_DCHECK_EQ(configurations_[0].width, RawFrame->width());
           // RTC_DCHECK_EQ(configurations_[0].height, RawFrame->height());

            // Encode image for each layer.
            
            

            for (size_t i = 0; i < encoded_images_.size(); ++i) 
            {
             // EncodeFrame input.

                if (!configurations_[i].sending) {
                    SWarn << this << " Client Inactive "     ;

                    continue;
                }

                if(!encoded_image_callback_)
                {
                    break;
                }
            
            
           

                Store payload;

                  

                if (frame_types != nullptr) {
                    // Skip frame?
                    
                    if ((*frame_types)[i] == webrtc::VideoFrameType::kEmptyFrame) {
                        SWarn << " webrtc::VideoFrameType::kEmptyFrame";
                    }
                    
                    
                    
                    if ((*frame_types)[i] == webrtc::VideoFrameType::kVideoFrameKey) {
                        SWarn << " webrtc::VideoFrameType::kVideoFrameKey";
                        
                        if(!RawFrame->qframe->key((long)this, &payload))
                        {
                             return WEBRTC_VIDEO_CODEC_OK;
                        }
                      
                    }
                    else
                    {
                       if(!RawFrame->qframe->next((long)this, &payload))
                        {
                             return WEBRTC_VIDEO_CODEC_OK;
                        } 
                    }
                    
                    
                }
                        
                        
                        
                
                 int dataSize = payload.payload.size();
                
                
                  if (!dataSize)
                  {
                      SWarn << "no frame size";
		      return WEBRTC_VIDEO_CODEC_OK;
                  }


//                if (!vframecount)
//                {
//
//                   SWarn << this << " frame no "  <<  payload.vframecount  <<  " frame size "  << dataSize <<  " idr "   <<  payload.idr <<  " fametype " << (int) payload._frameType << " w " <<  RawFrame->width() << " h " << RawFrame->height() ;
//
//                }
//
//                if( payload.vframecount !=  vframecount++)
//                {
//                   SWarn << this << " frame no "  <<    payload.vframecount  <<  " frame size "  << dataSize <<  " idr "   <<  payload.idr <<  " fametype " << (int) payload._frameType << " w " <<  RawFrame->width() << " h " << RawFrame->height() ;
//
//                }


                        //++encoderInc;


                        //        SInfo << "frame type " << (int) ConvertToVideoFrameType(encoders_[i]->frame);

                {

                    encoded_images_[i]._encodedWidth = static_cast<uint32_t> (payload.width_);
                    encoded_images_[i]._encodedHeight = static_cast<uint32_t> (payload.height_);
                    encoded_images_[i].SetTimestamp( input_frame.timestamp());
                    encoded_images_[i].ntp_time_ms_ = input_frame.ntp_time_ms();
                    encoded_images_[i].capture_time_ms_ = input_frame.render_time_ms();
                    encoded_images_[i].rotation_ = webrtc::kVideoRotation_0;
                    encoded_images_[i].content_type_ =
                            (codec_.mode == webrtc::VideoCodecMode::kScreensharing)
                            ? webrtc::VideoContentType::SCREENSHARE
                            : webrtc::VideoContentType::UNSPECIFIED;
                    encoded_images_[i].timing_.flags = webrtc::VideoSendTiming::kInvalid;
                    encoded_images_[i]._frameType = payload._frameType;

                    encoded_images_[i].timing_.encode_start_ms = rtc::TimeMicros() / 1000;

                    encoded_images_[i].timing_.encode_finish_ms = (rtc::TimeMicros() / 1000) + 10;;
                    encoded_images_[i].timing_.flags = webrtc::VideoSendTiming::kTriggeredByTimer;


                    encoded_images_[i]._completeFrame = true;

                    metadata =  RawFrame->txt;

                  // SInfo << this << " encode frame no " << encoderInc <<  " type "  << (int) encoded_images_[i]._frameType;

                    // Split encoded image up into fragments. This also updates
                    // |encoded_image_|.
                    webrtc::RTPFragmentationHeader frag_header;
                    RtpFragmentize(&encoded_images_[i], &encoded_image_buffers_[i], payload.width_, payload.height_, payload.payload ,   &frag_header);
                    // av_packet_unref(encoders_[i]->pkt);
                    // Encoder can skip frames to save bandwidth in which case
                    // |encoded_images_[i]._length| == 0.
                    if (encoded_images_[i].size() > 0) {

                        // Parse QP.
                     //   h264_bitstream_parser_.ParseBitstream(encoded_images_[i].data(),
                     //           encoded_images_[i].size());
                      //  h264_bitstream_parser_.GetLastSliceQp(&encoded_images_[i].qp_);

                        // Deliver encoded image.
                        webrtc::CodecSpecificInfo codec_specific;
                        //codec_specific.codecType = webrtc::kVideoCodecH264;
                        // codec_specific.codecSpecific.H264.packetization_mode =
                        //    packetization_mode_;
                        //codec_specific.codecSpecific.H264.simulcast_idx = static_cast<uint8_t>(configurations_[i].simulcast_idx);

                        codec_specific.codecType = webrtc::kVideoCodecH264;

                        codec_specific.codecSpecific.H264.packetization_mode = webrtc::H264PacketizationMode::NonInterleaved;;
                        codec_specific.codecSpecific.H264.temporal_idx = -1; // kNoTemporalIdx = -1.
                        //  codec_specific.codecSpecific.H264.idr_frame =    info.eFrameType == videoFrameTypeIDR;
                        codec_specific.codecSpecific.H264.base_layer_sync = false;
                        //en_im_cb.lock();
                        encoded_image_callback_->OnEncodedImage(encoded_images_[i],
                                &codec_specific, &frag_header);
                        //en_im_cb.unlock();
                    }
                    
                }


            }//end for
           

            return WEBRTC_VIDEO_CODEC_OK;
        }
        


       


        void NULLEncoder::LayerConfig::SetStreamState(bool send_stream) {
            if (send_stream && !sending) {
                // Need a key frame if we have not sent this stream before.
                key_frame_request = true;
            }
            sending = send_stream;
        }

        
        




    }// ns webrtc
}//ns base
