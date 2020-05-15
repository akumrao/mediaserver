

#ifndef SFU_Signaler_H
#define SFU_Signaler_H

#include "Worker.h"
#include "socketio/socketioClient.h"

//#include "sdp/Handler.h"

using namespace base::sockio;


namespace SdpParse {

    class Rooms;
    
        class Signaler {
        public:
            Signaler();
            ~Signaler();

            void startStreaming(const std::string& file, bool loop = true);
            void connect(const std::string& host, const uint16_t port);
            
            Worker *worker{nullptr};
            
            std::string  sfuID;
            
            std::map< std::string, std::map<std::string, std::string> > mapNotification;
            
        protected:

            // /// PeerManager interface
             
            // void sendCandidate(wrtc::Peer* conn, const std::string& mid, int mlineindex, const std::string& sdp) override;
            // void onAddRemoteStream(wrtc::Peer* conn, webrtc::MediaStreamInterface* stream) override;
            // void onRemoveRemoteStream(wrtc::Peer* conn, webrtc::MediaStreamInterface* stream) override;
            // void onStable(wrtc::Peer* conn) override;
            // void onClosed(wrtc::Peer* conn) override;
            // void onFailure(wrtc::Peer* conn, const std::string& error) override;

        public:
            void sendSDP( const std::string& type, const std::string& sdp,  std::string & remoteID);
            void postMessage(const json& m);
            void postAppMessage(const json& m);
                      // void onPeerConnected(std::string& peerID);
            void onPeerMessage(std::string &room , json const& m);
           // void onPeerDiconnected(std::string& peerID);

            
                /// PeerManager interface
            //void sendSDP(wrtc::Peer* conn, const std::string& type, const std::string& sdp) override;
           // void sendCandidate(wrtc::Peer* conn, const std::string& mid, int mlineindex, const std::string& sdp) override;
      
            //void recvSDP(const std::string& token, const json& data);
            void recvCandidate(const std::string& token, const json& data);
            
           // void onPeerConnected(std::string& peerID);
                        
            //void onffer(std::string &room,  std::string& participant, const json &sdp);
           
            //void onPeerDiconnected(std::string &room,  std::string& peerID);
            
            void request(string const& room, json const& data, bool isAck, json & ack_resp);

        protected:
#if USE_SSL
            //  SocketioSecClient *client;
#else
            SocketioClient *client;
            Socket *socket;

#endif
//            wrtc::MultiplexMediaCapturer _capturer;
//            wrtc::PeerFactoryContext _context;

            //socket* socket{nullptr};

            bool isChannelReady{false};
            bool isInitiator{false};
            bool isStarted{false};
            
            Rooms *rooms{nullptr};

            int reqId{1};
        };


} // namespace SdpParse {


#endif
