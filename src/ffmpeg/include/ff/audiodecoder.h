
#ifndef BASE_AV_AudioDecoder_H
#define BASE_AV_AudioDecoder_H


#include "base/base.h"

#ifdef HAVE_FFMPEG

#include "ff/packet.h"
#include "ff/audiocontext.h"


namespace base {
namespace ff {

    class MediaCapture;
struct AudioDecoder : public AudioContext
{
    AudioDecoder(AVStream* stream);
    virtual ~AudioDecoder();

    virtual void create() override;
    virtual void close() override;

    /// Decodes a the given input packet.
    /// Returns true an output packet was created, false otherwise.
    //virtual bool decode(uint8_t* data, int size);
    virtual bool decode(AVPacket& ipacket, MediaCapture *mediaCapure);
    
    bool emitPacket( MediaCapture *mediaCapure);

    /// Flushes buffered frames.
    /// This method should be called once after decoding.
    virtual void flush(MediaCapture *mediaCapure);
};


} // namespace ff
} // namespace base


#endif
#endif // BASE_AV_AudioDecoder_H


