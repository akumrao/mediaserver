

#ifndef WebRTCStreamer_Signaler_H
#define WebRTCStreamer_Signaler_H


#include "socketio/socketioClient.h"
#include "webrtc/peermanager.h"
#include "webrtc/multiplexmediacapturer.h"
//#include "rtc_base/ssladapter.h"



namespace base {
    namespace wrtc {

        class Signaler : public wrtc::PeerManager {
        public:
            Signaler();
            ~Signaler();

            void startStreaming(const std::string& dir, const std::string& file,  const std::string& type ,  bool looping);
            void connect(const std::string& host, const uint16_t port, const std::string room);
            void closeCamera(std::string &cam);
            
        protected:

            /// PeerManager interface
            void sendSDP(wrtc::Peer* conn, const std::string& type, const std::string& sdp) override;
            void sendCandidate(wrtc::Peer* conn, const std::string& mid, int mlineindex, const std::string& sdp) override;
            void onAddRemoteStream(wrtc::Peer* conn, webrtc::MediaStreamInterface* stream) override;
            void onRemoveRemoteStream(wrtc::Peer* conn, webrtc::MediaStreamInterface* stream) override;
            void onStable(wrtc::Peer* conn) override;
            void onClosed(wrtc::Peer* conn) override;
            void onFailure(wrtc::Peer* conn, const std::string& error) override;

            void postMessage(const json& m);
            //void syncMessage(const ipc::Action& action);

            void onPeerConnected(std::string& peerID,  std::string &cam, std::string &room);
            void onPeerMessage(std::string &name , json const& m);
            void onPeerDiconnected(std::string& peerID);
            void postAppMessage(std::string message , std::string from , std::string &room);



        protected:
#if USE_SSL
            //  SocketioSecClient *client;
#else
            sockio::SocketioClient *client;
            sockio::Socket *socket;
            //std::string peerID;
           // std::string remotePeerID;
#endif
            wrtc::MultiplexMediaCapturer _capturer;
            wrtc::PeerFactoryContext _context;

            //socket* socket{nullptr};

           // std::string room;
            bool isChannelReady{false};
            bool isInitiator{false};
            bool isStarted{false};

        };

    }
} // namespace base


#endif
