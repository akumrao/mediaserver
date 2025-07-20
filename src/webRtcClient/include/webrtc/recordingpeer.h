
#ifndef WebRTC_RecordingPeer_H
#define WebRTC_RecordingPeer_H


#include "base/base.h"

#ifdef HAVE_FFMPEG

#include "av/av.h"
#include "av/mediacapture.h"
#include "webrtc/multiplexmediacapturer.h"
#include "webrtc/peer.h"
#include "webrtc/streamrecorder.h"


namespace base {
namespace wrtc {


/// Peer connection class for recording the remote peer video.
class RecordingPeer : public Peer
{
public:
    /// Create the recording peer connection.
    RecordingPeer(PeerManager* manager,
                  PeerFactoryContext* context,
                  const std::string& peerid,
                  const std::string& token,
                  const av::EncoderOptions& options = av::EncoderOptions());
    virtual ~RecordingPeer();

    /// inherited from PeerObserver
    virtual void OnAddStream(webrtc::MediaStreamInterface* stream) override;
    virtual void OnRemoveStream(webrtc::MediaStreamInterface* stream) override;
    virtual void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override;

protected:
    std::unique_ptr<StreamRecorder> _recorder;
};


} } // namespace wrtc


#endif // HAVE_FFMPEG
#endif
