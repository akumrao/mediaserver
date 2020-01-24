

#ifndef BASE_AV_VideoDecoder_H
#define BASE_AV_VideoDecoder_H


#include "base/base.h"

#ifdef HAVE_FFMPEG

#include "ff/packet.h"
#include "ff/videocontext.h"


namespace base {
namespace ff {

class MediaCapture;
struct VideoDecoder : public VideoContext
{
    VideoDecoder(AVStream* stream);
    virtual ~VideoDecoder();

    virtual void create() override;
    virtual void open() override;
    virtual void close() override;

    void emitPacket( AVFrame* frame, MediaCapture *mediaCapure);
    
    /// Decodes a the given input packet.
    /// Input packets should use the raw `AVStream` time base. Time base
    /// conversion will happen internally.
    /// Returns true an output packet was was decoded, false otherwise.
    virtual bool decode(AVPacket& ipacket, MediaCapture *mediaCapure);

    /// Flushes buffered frames.
    /// This method should be called after decoding
    /// until false is returned.
    virtual void flush(MediaCapture *mediaCapure);
};


} // namespace ff
} // namespace base


#endif
#endif // BASE_AV_VideoDecoder_H


