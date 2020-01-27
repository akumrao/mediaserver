

#include "webrtc/peermanager.h"
//#include "base/memory.h"


using std::endl;


namespace base {
namespace wrtc {


PeerManager::PeerManager() // (PeerFactoryContext* context)
    // : _context(context)
{
}


PeerManager::~PeerManager()
{
}


void PeerManager::sendSDP(Peer* conn, const std::string& type, const std::string& sdp)
{
    assert(0 && "virtual");
}


void PeerManager::sendCandidate(Peer* conn, const std::string& mid, int mlineindex, const std::string& sdp)
{
    assert(0 && "virtual");
}


void PeerManager::recvSDP(const std::string& token, const json& message)
{
    auto conn = PeerManager::get(token, false);
    if (!conn) {
        assert(0 && "peer mismath");
        return;
    }

    std::string type = message.value(kSessionDescriptionTypeName, "");
    std::string sdp = message.value(kSessionDescriptionSdpName, "");
    if (sdp.empty() || (type != "offer" && type != "answer")) {
        LError("Received bad sdp: ", type, ": ", sdp)
        assert(0 && "bad sdp");
        return;
    }

    conn->recvSDP(type, sdp);

    LDebug("Received ", type, ": ", sdp)
}


void PeerManager::recvCandidate(const std::string& token, const json& message)
{
    auto conn = PeerManager::get(token, false);
    if (!conn) {
        assert(0 && "peer mismath");
        return;
    }

    std::string mid = message.value(kCandidateSdpMidName, "");
    int mlineindex = message.value(kCandidateSdpMlineIndexName, -1);
    std::string sdp = message.value(kCandidateSdpName, "");
    if (mlineindex == -1 || mid.empty() || sdp.empty()) {
        LError("Invalid candidate format")
        assert(0 && "bad candiate");
        return;
    }

    LDebug("Received candidate: ", sdp)

    conn->recvCandidate(mid, mlineindex, sdp);
}


void PeerManager::onAddRemoteStream(Peer* conn, webrtc::MediaStreamInterface* stream)
{
    assert(0 && "virtual");
}


void PeerManager::onRemoveRemoteStream(Peer* conn, webrtc::MediaStreamInterface* stream)
{
    assert(0 && "virtual");
}


void PeerManager::onStable(Peer* conn)
{
}


void PeerManager::onClosed(Peer* conn)
{
    LDebug("Deleting peer connection: ", conn->peerid())
//arvind
  //  if (remove(conn))
//        deleteLater<Peer>(conn); // async delete
}


void PeerManager::onFailure(Peer* conn, const std::string& error)
{
    LDebug("Deleting peer connection: ", conn->peerid())
//arvind
   // if (remove(conn))
     //   deleteLater<Peer>(conn); // async delete
}


