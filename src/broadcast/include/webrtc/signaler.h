

#ifndef WebRTCStreamer_Signaler_H
#define WebRTCStreamer_Signaler_H


#include "socketio/socketioClient.h"
#include "webrtc/peermanager.h"

#define JOIN_ROOM "VideoEdgeWebRTC"

namespace base
{
namespace web_rtc
{

class Signaler : public web_rtc::PeerManager
{
public:
    Signaler();
    ~Signaler();

    void startStreaming(const std::string &dir, const std::string &file, const std::string &type, bool looping);
    void connect(const std::string &host, const uint16_t port, const std::string room);
    // void closeCamera(std::string &cam ,  std::string  reason );
    void postcloseCamera(std::string &trackInfo, std::string reason);
    void postAppMessage(std::string message, std::string from, std::string &room);

protected:
    /// PeerManager interface
    void sendSDP(web_rtc::Peer *conn, const std::string &type, const std::string &sdp) override;
    void sendCandidate(
        web_rtc::Peer *conn, const std::string &mid, int mlineindex, const std::string &sdp) override;
    void onAddRemoteStream(web_rtc::Peer *conn, webrtc::MediaStreamInterface *stream) override;
    void onRemoveRemoteStream(web_rtc::Peer *conn, webrtc::MediaStreamInterface *stream) override;

    void onPeerCommand(
        std::string &peerID, st_track &camT, std::string &cmd, const json &trackids, const json &action);

    void onStable(web_rtc::Peer *conn) override;
    void onClosed(web_rtc::Peer *conn) override;
    void onFailure(web_rtc::Peer *conn, const std::string &error) override;

    void postMessage(const json &m);
    // void syncMessage(const ipc::Action& action);

    // void onPeerConnected(std::string& peerID,  st_track &trackInfo, std::string &room);
    void onPeerOffer(std::string &peerID, st_track &trackInfo, std::string &room);
    void onPeerMessage(std::string &name, json const &m);
    void onPeerDiconnected(std::string &peerID);

protected:
#if USE_SSL
    //  SocketioSecClient *client;
#else
    sockio::SocketioClient *client;
    sockio::Socket *socket;
    // std::string peerID;
    // std::string remotePeerID;
#endif

    web_rtc::PeerFactoryContext _context;

    // socket* socket{nullptr};

    // std::string room;
    bool isChannelReady{false};
    bool isInitiator{false};
    bool isStarted{false};
};

}  // namespace web_rtc
}  // namespace base


#endif
