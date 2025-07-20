


#include "X264Encoder.h"
#include "base/logger.h"
#include "webrtc/rawVideoFrame.h"
#ifdef WEBRTC_LINUX
#    include <ffnvcodec/nvEncodeAPI.h>
#endif


//////////////////////////////////////////////////////////////////////////

namespace base
{
namespace web_rtc
{


X264Encoder::X264Encoder(en_EncType encType) : VideoEncoder(encType), H264_Encoder(encType)
{
    ++x264hwinstance;
}

X264Encoder::~X264Encoder()
{
    --x264hwinstance;
}


// int32_t X264Encoder::InitEncode(const webrtc::VideoCodec* CodecSetings, int32_t NumberOfCores, size_t
// MaxPayloadSize)
// {

//       return ::InitEncode(const webrtc::VideoCodec* CodecSetings, int32_t NumberOfCores, size_t
//       MaxPayloadSize)
// }


void X264Encoder::SetRates(const RateControlParameters &parameters)
{
    if (parameters.framerate_fps < 1.0)
    {
        SWarn << "Invalid frame rate: " << parameters.framerate_fps;
        return;
    }


    uint32_t bps = parameters.bitrate.get_sum_bps();

    if (bps == 0)
    {
        // Encoder paused, turn off all encoding.
        for (size_t i = 0; i < configurations_.size(); ++i) configurations_[i].SetStreamState(false);
        SInfo << "bitrate " << parameters.bitrate.get_sum_bps();
        return;
    }

    // At this point, bitrate allocation should already match codec settings.
    if (codec_.maxBitrate > 0)
    {
        if (bps > codec_.maxBitrate * 1000)
        {
            SWarn << "get_sum_kbps() " << bps << " >  codec_.maxBitrate= " << codec_.maxBitrate;
        }
    }

    if (bps < codec_.minBitrate * 1000) { SWarn << "get_sum_kbps() <  codec_.maxBitrate"; }


    c->bit_rate = bps * 0.7;
    c->rc_max_rate = bps * 0.85;
    c->rc_min_rate = bps * 0.1;
    c->rc_buffer_size = bps * 2;  // buffer_s
}


int32_t X264Encoder::Encode(
    const webrtc::VideoFrame &input_frame, const std::vector<webrtc::VideoFrameType> *frame_types)
{
    // SInfo << "Encode";

    //    if (encoders_.empty()) {
    //        ReportError();
    //        return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
    //    }
    if (!encoded_image_callback_)
    {
        SWarn << "InitEncode() has been called, but a callback function "
              << "has not been set with RegisterEncodeCompleteCallback()";
        ReportError();
        return WEBRTC_VIDEO_CODEC_UNINITIALIZED;
    }

    // webrtc::I420BufferInterface *frame_buffer = (webrtc::I420BufferInterface *)
    // input_frame.video_frame_buffer().get();

    H264FrameBuffer *frame_buffer = static_cast<H264FrameBuffer *>(input_frame.video_frame_buffer().get());


    bool send_key_frame = false;
    for (auto &configuration : configurations_)
    {
        if (configuration.key_frame_request && configuration.sending)
        {
            send_key_frame = true;
            break;
        }
    }
    if (!send_key_frame && frame_types)
    {
        for (size_t i = 0; i < frame_types->size() && i < configurations_.size(); ++i)
        {
            if ((*frame_types)[i] == webrtc::VideoFrameType::kVideoFrameKey && configurations_[i].sending)
            {
                send_key_frame = true;
                // SInfo << " send_key_frame " << send_key_frame;
                break;
            }
        }
    }


    if (frame_types == nullptr || !configurations_[0].sending
        || (*frame_types)[0] == webrtc::VideoFrameType::kEmptyFrame)
    {
        SInfo << "Pause return ";
        return WEBRTC_VIDEO_CODEC_OK;
    }


    if (webrtc::VideoFrameBuffer::Type::kI420 == frame_buffer->type())  // pause or muted condtion
    {
        return WEBRTC_VIDEO_CODEC_OK;
    }


    //    RTC_DCHECK_EQ(configurations_[0].width, frame_buffer->width());
    //    RTC_DCHECK_EQ(configurations_[0].height, frame_buffer->height());


    // Encode image for each layer.


    if (send_key_frame)
    {
        // API doc says ForceIntraFrame(false) does nothing, but calling this
        // function forces a key frame regardless of the |bIDR| argument's value.
        // (If every frame is a key frame we get lag/delays.)
        frame_buffer->qframe->key_frame = 1;
        frame_buffer->qframe->pict_type = AV_PICTURE_TYPE_I;
        configurations_[0].key_frame_request = false;

        SInfo << " Send Key frame";
    }


    int i = 0;

    //  fwrite(encoders_[i]->pkt->data, 1, encoders_[i]->pkt->size, fp);


    encoded_images_[i]._encodedWidth = static_cast<uint32_t>(frame_buffer->width());
    encoded_images_[i]._encodedHeight = static_cast<uint32_t>(frame_buffer->height());
    encoded_images_[i].SetTimestamp(input_frame.timestamp());
    encoded_images_[i].ntp_time_ms_ = input_frame.ntp_time_ms();
    encoded_images_[i].capture_time_ms_ = input_frame.render_time_ms();
    encoded_images_[i].rotation_ = input_frame.rotation();


    encoded_images_[i].content_type_ = (codec_.mode == webrtc::VideoCodecMode::kScreensharing)
                                         ? webrtc::VideoContentType::SCREENSHARE
                                         : webrtc::VideoContentType::UNSPECIFIED;
    // encoded_images_[i].timing_.flags = webrtc::VideoSendTiming::kInvalid;
    // encoded_images_[i]._frameType = ConvertToVideoFrameType(frame_buffer->qframe);


    encoded_images_[i].timing_.encode_start_ms = rtc::TimeMicros() / 1000;

    encoded_images_[i].timing_.encode_finish_ms = (rtc::TimeMicros() / 1000) + 10;
    ;
    encoded_images_[i].timing_.flags = webrtc::VideoSendTiming::kTriggeredByTimer;


    encoded_images_[i]._completeFrame = true;

    metadata = frame_buffer->txt;

    // SInfo <<  this <<   " size " <<   frame_buffer->qframe << " frm " << frame_buffer->frmNo;

    encodeFrame(frame_buffer->dec_ctx, frame_buffer->qframe, frame_buffer->width(), frame_buffer->height());


    //        else {
    //            encoders_[i]->frame->key_frame = 0;
    //            encoders_[i]->frame->pict_type = AV_PICTURE_TYPE_P;
    //        }
    //


    return WEBRTC_VIDEO_CODEC_OK;
}


void X264Encoder::write_frame(AVPacket *enc_pkt)
{
    int i = 0;
    // SInfo <<  "encode frame no " << encoderInc <<  " type "  << (int) encoded_images_[i]._frameType << " w "
    // << configurations_[i].width << " h " << configurations_[i].height;

    // Split encoded image up into fragments. This also updates
    // |encoded_image_|.
    webrtc::RTPFragmentationHeader frag_header;
    RtpFragmentize(&encoded_images_[i], &encoded_image_buffers_[i], enc_pkt, &frag_header);
    // av_packet_unref(encoders_[i]->pkt);
    // Encoder can skip frames to save bandwidth in which case
    // |encoded_images_[i]._length| == 0.
    if (encoded_images_[i].size() > 0)
    {
        // Parse QP.

        //            h264_bitstream_parser_.ParseBitstream(encoded_images_[i].data(),
        //                                        encoded_images_[i].size());
        //            h264_bitstream_parser_.GetLastSliceQp(&encoded_images_[i].qp_);


        // Deliver encoded image.
        webrtc::CodecSpecificInfo codec_specific;
        // codec_specific.codecType = webrtc::kVideoCodecH264;
        // codec_specific.codecSpecific.H264.packetization_mode =
        //    packetization_mode_;
        // codec_specific.codecSpecific.H264.simulcast_idx =
        // static_cast<uint8_t>(configurations_[i].simulcast_idx);

        codec_specific.codecType = webrtc::kVideoCodecH264;
        ;
        codec_specific.codecSpecific.H264.packetization_mode = webrtc::H264PacketizationMode::NonInterleaved;
        codec_specific.codecSpecific.H264.temporal_idx
            = -1;  // kNoTemporalIdx = -1.
                   //  codec_specific.codecSpecific.H264.idr_frame =    info.eFrameType == videoFrameTypeIDR;
        codec_specific.codecSpecific.H264.base_layer_sync = false;


        encoded_image_callback_->OnEncodedImage(encoded_images_[i], &codec_specific, &frag_header);
    }
}


int X264Encoder::x264hwinstance = 0;


}  // namespace web_rtc
}  // namespace base
