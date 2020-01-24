
#ifndef BASE_AV_AudioContext_H
#define BASE_AV_AudioContext_H


#include "base/base.h"
//#include "base/packetsignal.h"

#ifdef HAVE_FFMPEG

#include "ff/ffmpeg.h"
#include "ff/format.h"
#include "ff/fpscounter.h"
#include "ff/packet.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}


namespace base {
namespace ff {


struct AudioResampler;


struct AudioContext
{
    AudioContext();
    virtual ~AudioContext();

    /// Initialize the `AVCodecContext` with default values
    virtual void create() = 0;

    /// Open the AVCodecContext
    virtual void open();

    /// Close the AVCodecContext
    virtual void close();

    /// Create the audio resampling context.
    virtual bool recreateResampler();

//    PacketSignal emitter;

    AudioCodec iparams;  ///< input parameters
    AudioCodec oparams;  ///< output parameters
    AVStream* stream;    ///< encoder or decoder stream
    AVCodecContext* ctx; ///< encoder or decoder context
    AVCodec* codec;      ///< encoder or decoder codec
    AVFrame* frame;      ///< last encoded or decoded frame
    AudioResampler* resampler; ///< audio resampler
    int outputFrameSize; ///< encoder or decoder output frame size
    int64_t time;        ///< stream time in codec time base
    int64_t pts;         ///< last packet pts value
    double seconds;      ///< video time in seconds
    std::string error;   ///< error message
};


void initAudioCodecFromContext(const AVCodecContext* ctx, AudioCodec& params);
AVSampleFormat selectSampleFormat(AVCodec* codec, AudioCodec& params);
bool isSampleFormatSupported(AVCodec* codec, enum AVSampleFormat sampleFormat);
void initDecodedAudioPacket(const AVStream* stream, const AVCodecContext* ctx,
                            const AVFrame* frame, AVPacket* opacket);
bool frameIsPlanar(const std::string& pixfmt);
bool frameIsPlanar(AVSampleFormat format);


} // namespace ff
} // namespace base


#endif
#endif // BASE_AV_AudioContext_H


/// @\}
