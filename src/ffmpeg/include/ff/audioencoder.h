
#ifndef BASE_AV_AudioEncoder_H
#define BASE_AV_AudioEncoder_H


#include "base/base.h"

#ifdef HAVE_FFMPEG

#include "ff/audiobuffer.h"
#include "ff/audiocontext.h"
#include "ff/ffmpeg.h"
#include "ff/format.h"
#include "ff/packet.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/audio_fifo.h>
}


namespace base {
namespace ff {


struct AudioEncoder : public AudioContext
{
    AudioEncoder(AVFormatContext* format = nullptr);
    virtual ~AudioEncoder();

    virtual void create() override;
    virtual void close() override;

    /// Encode interleaved audio samples.
    ///
    /// @param samples    The input samples to encode.
    /// @param numSamples The number of input samples per channel.
    /// @param pts        The input samples presentation timestamp.
    /// @param opacket    The output packet data will be encoded to.
    virtual bool encode(uint8_t* samples, const int numSamples, const int64_t pts);

    /// Encode planar audio samples.
    ///
    /// @param samples    The input samples to encode.
    /// @param numSamples The number of input samples per channel.
    /// @param pts        The input samples presentation timestamp.
    /// @param opacket    The output packet data will be encoded to.
    virtual bool encode(uint8_t* samples[4], const int numSamples, const int64_t pts);

    /// Encode a single AVFrame from the decoder.
    virtual bool encode(AVFrame* iframe);

    /// Flush remaining packets to be encoded.
    /// This method should be called once before stream closure.
    virtual void flush();

    av::AudioBuffer fifo;
    AVFormatContext* format;
};


} // namespace ff
} // namespace base


#endif
#endif // BASE_AV_AudioEncoder_H

