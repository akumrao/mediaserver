
#include "ff/audiodecoder.h"
#include "ff/audioresampler.h"
#include <assert.h>
#ifdef HAVE_FFMPEG

#include "ff/audioresampler.h"
#include "ff/mediacapture.h"
#include "ff/ffmpeg.h"
#include "base/logger.h"

using std::endl;


namespace base {
namespace ff {


AudioDecoder::AudioDecoder(AVStream* stream)
{
    this->stream = stream;
}


AudioDecoder::~AudioDecoder()
{
    close();
}


void AudioDecoder::create()
{
    assert(stream);
    LTrace("Create: ", stream->index)

    ctx = stream->codec;

    codec = avcodec_find_decoder(ctx->codec_id);
    if (!codec)
        throw std::runtime_error("Cannot find audio decoder.");

    frame = av_frame_alloc();
    if (!frame)
        throw std::runtime_error("Cannot allocate input frame.");

    int ret = avcodec_open2(ctx, codec, nullptr);
    if (ret < 0)
        throw std::runtime_error("Cannot open the audio codec: " + averror(ret));

    // Set the default input and output parameters are set here once the codec
    // context has been opened. The output sample format, channels or sample
    // rate can be modified after this call and a conversion context will be
    // created on the next call to open() to output the desired format.
    initAudioCodecFromContext(ctx, iparams);
    initAudioCodecFromContext(ctx, oparams);

    // Default to s16 interleaved output.
    // oparams.sampleFmt = "s16";
}


void AudioDecoder::close()
{
    AudioContext::close();
}


bool AudioDecoder::emitPacket( MediaCapture *mediaCapure)
{
    //auto sampleFmt = av_get_sample_fmt(dec->oparams.sampleFmt.c_str());
    //assert(av_sample_fmt_is_planar(sampleFmt) == 0 && "planar formats not supported");

    // Set the decoder time in microseconds
    // This value represents the number of microseconds
    // that have elapsed since the brginning of the stream.
    time = frame->pkt_pts > 0 ? static_cast<int64_t>(frame->pkt_pts *
                av_q2d(stream->time_base) * AV_TIME_BASE) : 0;

    // Set the decoder pts in stream time base
    pts = frame->pkt_pts;

    // Set the decoder seconds since stream start
    // http://stackoverflow.com/questions/6731706/syncing-decoded-video-using-ffmpeg
    seconds = (frame->pkt_dts - stream->start_time) * av_q2d(stream->time_base);

    if (resampler) {
        if (!resampler->resample((uint8_t**)frame->extended_data, frame->nb_samples)) {
            LDebug("Samples buffered by resampler")
            return false;
        }

      
        PlanarAudioPacket audio(resampler->outSamples,
            oparams.channels, resampler->outNumSamples,
            oparams.sampleFmt, time);
        outputFrameSize = resampler->outNumSamples;
         mediaCapure->emit(audio);
        assert(audio.size() == resampler->outBufferSize);
    }
    else {
        

        PlanarAudioPacket audio(frame->data,
            oparams.channels, frame->nb_samples,
            oparams.sampleFmt, time);
        outputFrameSize = frame->nb_samples;
          mediaCapure->emit(audio);
    }

    // opacket.pts = dec->frame->pkt_pts; // Decoder PTS values may be out of sequence
    // opacket.dts = dec->frame->pkt_dts;

    // assert(opacket.data);
    // assert(opacket.size);
    // assert(opacket.pts >= 0);
    // assert(opacket.dts >= 0);

    return true;
}


bool AudioDecoder::decode(AVPacket& ipacket, MediaCapture *mediaCapure)
{
    assert(ctx);
    assert(codec);
    assert(frame);
    assert(!stream || ipacket.stream_index == stream->index);



    int ret, frameDecoded = 0;
    while (ipacket.size > 0) {
        ret = avcodec_decode_audio4(ctx, frame, &frameDecoded, &ipacket);
        if (ret < 0) {
            error = "Audio decoder error: " + averror(ret);
            LError(error)
            throw std::runtime_error(error);
        }

        if (frameDecoded) {

             STrace << "Decoded frame:"
   
                 << "\n\tInput Frame PTS: " << ipacket.pts
                 << "\n\tInput Bytes: " << ipacket.size
                 << "\n\tFrame PTS: " << frame->pts
                 << "\n\tDecoder PTS: " << pts
                 << endl;

            // fps.tick();
            emitPacket(mediaCapure);
        }

        ipacket.size -= ret;
        ipacket.data += ret;
    }
    assert(ipacket.size == 0);
    return !!frameDecoded;
}


void AudioDecoder::flush(MediaCapture *mediaCapure)
{
    AVPacket ipacket;
    av_init_packet(&ipacket);
    ipacket.data = nullptr;
    ipacket.size = 0;

    // av_init_packet(&opacket);
    // opacket.data = nullptr;
    // opacket.size = 0;

    int frameDecoded = 0;
    av_frame_unref(frame);
    avcodec_decode_audio4(ctx, frame, &frameDecoded, &ipacket);

    if (frameDecoded) {
        LTrace("Flushed audio frame: ", frame->pts)
        assert(0);
        emitPacket(mediaCapure);
    }
}


// void AudioContext::process(IPacket& packet)
// {
//     LTrace("Process")
//
//     auto apacket = dynamic_cast<AudioPacket*>(&packet);
//     if (!apacket)
//         throw std::runtime_error("Unknown audio packet type.");
//
//     decode(apacket->data(), apacket->size());
// }


} // namespace ff
} // namespace base


#endif


