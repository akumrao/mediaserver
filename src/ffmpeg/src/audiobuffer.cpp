

#include "ff/audiobuffer.h"
#include <assert.h>
#ifdef HAVE_FFMPEG

#include "ff/ffmpeg.h"
#include "base/logger.h"
#include "base/platform.h"


using std::endl;


namespace base {
namespace ff {


AudioBuffer::AudioBuffer()
    : fifo(nullptr)
{
    LTrace("Create")
}


AudioBuffer::~AudioBuffer()
{
    LTrace("Destroy")
    close();
}

void AudioBuffer::alloc(const std::string& sampleFmt, int channels, int numSamples)
{
    STrace << "Create audio buffer:\n"
           << "\n\tNb Channels: " << channels
           << "\n\tSample Fmt: " << sampleFmt
           << "\n\tNb Samples: " << numSamples
           << "\n\tfifo: " << fifo
           << endl;

    enum AVSampleFormat format = av_get_sample_fmt(sampleFmt.c_str());
    assert(!fifo);
    assert(channels);
    assert(format != AV_SAMPLE_FMT_NONE);

    // Create the FIFO buffer based on the specified sample format.
    fifo = av_audio_fifo_alloc(format, channels, numSamples);
    if (!fifo) {
        throw std::runtime_error("Cannot allocate FIFO: Out of memory");
    }
}


void AudioBuffer::reset()
{
    LTrace("Reset")

    if (fifo) {
        av_audio_fifo_reset(fifo);
    }
}


void AudioBuffer::close()
{
    LTrace("Close")

    if (fifo) {
        av_audio_fifo_free(fifo);
        fifo = nullptr;
    }
}


void AudioBuffer::write(void** samples, int numSamples)
{
    int error;
    assert(fifo);
    assert(samples);
    assert(samples[0]);

    // Make the FIFO as large as it needs to be to hold both
    // the old and the new samples.
    if ((error = av_audio_fifo_realloc(fifo, av_audio_fifo_size(fifo) + numSamples)) < 0) {
        throw std::runtime_error("Cannot reallocate FIFO: " + averror(error));
    }

    // Store the new samples in the FIFO buffer.
    if (av_audio_fifo_write(fifo, (void**)samples, numSamples) < numSamples) {
        throw std::runtime_error("Cannot write data to FIFO");
    }
}


bool AudioBuffer::read(void** samples, int numSamples)
{
    LTrace("Read samples: ", numSamples)

    // Make sure that there is one frame worth of samples in the FIFO
    // buffer so that the encoder can do its work.
    // Since the decoder's and the encoder's frame size may differ, we
    // need to FIFO buffer to store as many frames worth of input samples
    // that they make up at least one frame worth of output samples.
    if (available() < numSamples) {
        LTrace("No packets in buffer")
        return false;
    }

    // Read as many samples from the FIFO buffer as required.
    // The samples are stored in the output buffer temporarily.
    if (av_audio_fifo_read(fifo, (void**)samples, numSamples) < numSamples) {
        throw std::runtime_error("Cannot read data from FIFO");
    }

    return true;
}


int AudioBuffer::available() const
{
    if (fifo)
        return av_audio_fifo_size(fifo);
    return 0;
}


} // namespace ff
} // namespace base


#endif


/// @\}
