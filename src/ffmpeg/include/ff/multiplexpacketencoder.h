

#ifndef BASE_AV_MultiplexPacketEncoder_H
#define BASE_AV_MultiplexPacketEncoder_H


#include "scy/base.h"
#ifdef HAVE_FFMPEG
#include "scy/av/multiplexencoder.h"
#include "scy/packetstream.h"


namespace scy {
namespace ff {


/// Encodes and multiplexes a realtime video stream form
/// audio / video capture sources.
/// FFmpeg is used for encoding.
class AV_API MultiplexPacketEncoder : public MultiplexEncoder, public PacketProcessor
{
public:
    MultiplexPacketEncoder(const EncoderOptions& options = EncoderOptions());
    virtual ~MultiplexPacketEncoder();

    virtual void encode(VideoPacket& packet);
    virtual void encode(AudioPacket& packet);

    virtual bool accepts(IPacket* packet) override;
    virtual void process(IPacket& packet) override;

protected:
    virtual void onStreamStateChange(const PacketStreamState& state) override;

    friend class PacketStream;

    mutable std::mutex _mutex;
};


#endif


} // namespace ff
} // namespace base


#endif
#endif
