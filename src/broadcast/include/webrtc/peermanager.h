
#ifndef WebRTC_PeerManager_H
#define WebRTC_PeerManager_H


#include "base/collection.h"
#include "json/json.h"
#include "webrtc/webrtc.h"
#include "webrtc/peer.h"

#include "webrtc/multiplexmediacapturer.h"

#include <iostream>
#include <string>


namespace base
{
namespace web_rtc
{


class Peer;

class PeerManager : public PointerCollection<std::string, Peer>
{
public:
    PeerManager();
    virtual ~PeerManager();

    virtual void sendSDP(Peer *conn, const std::string &type, const std::string &sdp);
    virtual void sendCandidate(Peer *conn, const std::string &mid, int mlineindex, const std::string &sdp);

    virtual void recvSDP(const std::string &token, const json &data);
    virtual void recvCandidate(const std::string &token, const json &data);

    virtual void onAddRemoteStream(Peer *conn, webrtc::MediaStreamInterface *stream);
    virtual void onRemoveRemoteStream(Peer *conn, webrtc::MediaStreamInterface *stream);

    virtual void onStable(Peer *conn);
    virtual void onClosed(Peer *conn);
    virtual void onFailure(Peer *conn, const std::string &error);

    virtual void postAppMessage(std::string message, std::string from, std::string &room);
    // Arvind this could be used to change the resolution on the fly.
    // virtual void oncommand(const std::string& token, std::string& cmd,  std::string& para);


    web_rtc::MultiplexMediaCapturer _capturer;
};


}  // namespace web_rtc
}  // namespace base


#endif
