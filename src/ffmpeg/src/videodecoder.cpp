#include "ff/videodecoder.h"
#include "ff/mediacapture.h"
#ifdef HAVE_FFMPEG

#include "ff/ffmpeg.h"
#include "base/logger.h"
#include <assert.h>

using std::endl;


namespace base {
namespace ff {


VideoDecoder::VideoDecoder(AVStream* stream)
{
    this->stream = stream;
}


VideoDecoder::~VideoDecoder()
{
    close();
}


void VideoDecoder::create()
{
    assert(stream);
    LTrace("Create: ", stream->index)

    ctx = stream->codec;

    codec = avcodec_find_decoder(ctx->codec_id);
    if (!codec)
        throw std::runtime_error("Video codec missing or unsupported.");

    frame = av_frame_alloc();
    if (frame == nullptr)
        throw std::runtime_error("Cannot allocate video input frame.");

    int ret = avcodec_open2(ctx, codec, nullptr);
    if (ret < 0)
        throw std::runtime_error("Cannot open the audio codec: " + averror(ret));

    // Set the default input and output parameters are set here once the codec
    // context has been opened. The output pixel format, width or height can be
    // modified after this call and a conversion context will be created on the
    // next call to open() to output the desired format.
    initVideoCodecFromContext(stream, ctx, iparams);
    initVideoCodecFromContext(stream, ctx, oparams);

    // Default to bgr24 interleaved output.
    // oparams.pixelFmt = "bgr24";
}


void VideoDecoder::open()
{
    VideoContext::open();
}


void VideoDecoder::close()
{
    VideoContext::close();
}


void VideoDecoder::emitPacket( AVFrame* frame, MediaCapture *mediaCapure)
{
    frame->pts = av_frame_get_best_effort_timestamp(frame);

    // Set the decoder time in microseconds
    // This value represents the number of microseconds
    // that have elapsed since the beginning of the stream.
    int64_t pts = frame->pts;
    this->time = pts > 0 ? static_cast<int64_t>(pts *
                av_q2d(stream->time_base) * AV_TIME_BASE) : 0;

    // Set the decoder pts in stream time base
    this->pts = pts;

    // Set the decoder seconds since stream start
    this->seconds = (pts - stream->start_time) * av_q2d(stream->time_base);

    STrace << "Decoded video frame:"
        << "\n\tFrame Pkt DTS: " << frame->pkt_dts
        << "\n\tFrame Pkt PTS: " << frame->pkt_pts
        << "\n\tFrame PTS: " << frame->pts
        << "\n\tTimestamp: " << this->time
        << "\n\tPTS: " << this->pts
        << "\n\tSeconds: " << this->seconds
        << endl;

    PlanarVideoPacket video(frame->data, frame->linesize, oparams.pixelFmt,
                            frame->width, frame->height, time);
    //arvind
//     video.source = frame;
    // video.opaque = dec;

     mediaCapure->emit(video);
}


// bool VideoDecoder::decode(uint8_t* data, int size)
// {
//    AVPacket ipacket;
//    av_init_packet(&ipacket);
//    ipacket.stream_index = stream->index;
//    ipacket.data = data;
//    ipacket.size = size;
//    return decode(ipacket);
// }


bool VideoDecoder::decode(AVPacket& ipacket,MediaCapture *mediaCapure)
{
    assert(ctx);
    assert(codec);
    assert(frame);
    assert(!stream || ipacket.stream_index == stream->index);

    int ret, frameDecoded = 0;
    while (ipacket.size > 0) {
        ret = avcodec_decode_video2(ctx, frame, &frameDecoded, &ipacket);
        if (ret < 0) {
            error = "Audio decoder error: " + averror(ret);
            LError(error)
            throw std::runtime_error(error);
        }

        if (frameDecoded) {
            // assert(bytesDecoded == ipacket.size);

            // STrace << "Decoded frame:"
            //     << "\n\tFrame Size: " << opacket.size
            //     << "\n\tFrame PTS: " << opacket.pts
            //     << "\n\tInput Frame PTS: " << ipacket.pts
            //     << "\n\tInput Bytes: " << ipacket.size
            //     << "\n\tBytes Decoded: " << bytesDecoded
            //     << "\n\tFrame PTS: " << frame->pts
            //     << "\n\tDecoder PTS: " << pts
            //     << endl;

            // fps.tick();
            emitPacket(convert(frame),mediaCapure); //, frame, opacket, &ptsSeconds
        }

        ipacket.size -= ret;
        ipacket.data += ret;
    }
    assert(ipacket.size == 0);
    return !!frameDecoded;
}


void VideoDecoder::flush(MediaCapture *mediaCapure) // AVPacket& opacket
{
    AVPacket ipacket;
    av_init_packet(&ipacket);
    ipacket.data = nullptr;
    ipacket.size = 0;

    // av_init_packet(&opacket);
    // opacket.data = nullptr;
    // opacket.size = 0;

    int frameDecoded = 0;
    do {
        avcodec_decode_video2(ctx, frame, &frameDecoded, &ipacket);
        if (frameDecoded) {
            LTrace("Flushed video frame")
            emitPacket(convert(frame), mediaCapure); //, opacket stream, ctx, &pts, oparams
            // return true;
        }
    } while (frameDecoded);
}


} // namespace ff
} // namespace base


#endif



