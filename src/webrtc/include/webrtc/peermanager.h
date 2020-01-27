
#ifndef WebRTC_PeerManager_H
#define WebRTC_PeerManager_H


#include "base/collection.h"
#include "json/json.h"
#include "webrtc/webrtc.h"
#include "webrtc/peer.h"

#include "api/peerconnectioninterface.h"

#include <iostream>
#include <string>


namespace base {
namespace wrtc {


class Peer;


class PeerManager : public PointerCollection<std::string, Peer>
{
public:
    PeerManager();
    virtual ~PeerManager();

    virtual void sendSDP(Peer* conn, const std::string& type, const std::string& sdp);
    virtual void sendCandidate(Peer* conn, const std::string& mid, int mlineindex, const std::string& sdp);

    virtual void recvSDP(const std::string& token, const json& data);
    virtual void recvCandidate(const std::string& token, const json& data);

    virtual void onAddRemoteStream(Peer* conn, webrtc::MediaStreamInterface* stream);
    virtual void onRemoveRemoteStream(Peer* conn, webrtc::MediaStreamInterface* stream);

    virtual void onStable(Peer* conn);
    virtual void onClosed(Peer* conn);
    virtual void onFailure(Peer* conn, const std::string& error);
};


} } // namespace wrtc


#endif


