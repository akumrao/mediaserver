

#ifndef BASE_AV_AudioBuffer_H
#define BASE_AV_AudioBuffer_H


#include <string>

#include "ff/ff.h"

#ifdef HAVE_FFMPEG

extern "C" {
#include <libavutil/audio_fifo.h>
}


namespace base {
namespace ff {


struct AudioBuffer
{
    AudioBuffer();
    ~AudioBuffer();

    void alloc(const std::string& sampleFmt, int channels, int numSamples = 1024);
    void reset();
    void close();

    void write(void** samples, int numSamples);
    bool read(void** samples, int numSamples);

    int available() const;

    AVAudioFifo* fifo;
};


} // namespace ff
} // namespace base


#endif
#endif // BASE_AV_AudioBuffer_H

