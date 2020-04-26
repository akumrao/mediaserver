

#ifndef SFU_Signaler_H
#define SFU_Signaler_H

#include "Worker.h"
#include "socketio/socketioClient.h"
#include "sdp/Device.h"

using namespace base::sockio;

namespace base {
    namespace wrtc {

        class Signaler {
        public:
            Signaler();
            ~Signaler();

            void startStreaming(const std::string& file, bool loop = true);
            void connect(const std::string& host, const uint16_t port, const std::string room);
            
            Worker *worker{nullptr};
            

        protected:

            // /// PeerManager interface
            // void sendSDP(wrtc::Peer* conn, const std::string& type, const std::string& sdp) override;
            // void sendCandidate(wrtc::Peer* conn, const std::string& mid, int mlineindex, const std::string& sdp) override;
            // void onAddRemoteStream(wrtc::Peer* conn, webrtc::MediaStreamInterface* stream) override;
            // void onRemoveRemoteStream(wrtc::Peer* conn, webrtc::MediaStreamInterface* stream) override;
            // void onStable(wrtc::Peer* conn) override;
            // void onClosed(wrtc::Peer* conn) override;
            // void onFailure(wrtc::Peer* conn, const std::string& error) override;

        public:
            void postMessage(const json& m);
            void postAppMessage(const json& m);
            //void syncMessage(const ipc::Action& action);

           // void onPeerConnected(std::string& peerID);
            void onPeerMessage(json const& m);
           // void onPeerDiconnected(std::string& peerID);

           
            
                /// PeerManager interface
            //void sendSDP(wrtc::Peer* conn, const std::string& type, const std::string& sdp) override;
           // void sendCandidate(wrtc::Peer* conn, const std::string& mid, int mlineindex, const std::string& sdp) override;
      
            void recvSDP(const std::string& token, const json& data);
            void recvCandidate(const std::string& token, const json& data);
            
            void onPeerConnected(std::string& peerID, const json &sdp);
           
            void onPeerDiconnected(std::string& peerID);
            
            
            
            void request(string const& name, json const& data, bool isAck, json & ack_resp);

        protected:
#if USE_SSL
            //  SocketioSecClient *client;
#else
            SocketioClient *client;
            Socket *socket;
            std::string peerID;
            std::string remotePeerID;
#endif
//            wrtc::MultiplexMediaCapturer _capturer;
//            wrtc::PeerFactoryContext _context;

            //socket* socket{nullptr};

            std::string room;
            bool isChannelReady{false};
            bool isInitiator{false};
            bool isStarted{false};
            
            SdpParse::Device device;
            

        };

    }
} // namespace base


#endif
