#ifndef SDP_HANDLER_H
#define SDP_HANDLER_H


//#include "sdp/signaler.h"
#include "sdp/RemoteSdp.h"
#include <json.hpp>
#include <map>
#include <string>

namespace SdpParse {
    
    class Peer;
    class Signaler;
    class Handler
    {
    
    public:
                
        Handler(Peer * peer, std::string &peerID): peer(peer), peerID(peerID) 
        {
            
        }
        ~Handler()
        {
            if (remoteSdp) {
                delete remoteSdp;
                remoteSdp = nullptr;
            }
        }
        
        void transportCreate(Signaler *signal);
        void transportConnect(Signaler *signal);
       

        Sdp::RemoteSdp *remoteSdp{nullptr};
        void createSdp(const nlohmann::json& iceParameters, const nlohmann::json& iceCandidates, const nlohmann::json& dtlsParameters);
        void _setupTransport(const nlohmann::json & sdpObject, const std::string localDtlsRole);
        
        
    protected:
        Peer * peer;
        std::string peerID;
        std::string transportId;
        nlohmann::json dtlsParameters;
        bool forceTcp{false};
        static int ids;
        
    };

    class Producer:public Handler
    {
       
        public:
        
        Producer(Peer * peer, std::string &peerID);

        void runit(Signaler *signal);
        std::string answer;
        
        nlohmann::json producer;
        
    private:
        std::string GetAnswer();
        nlohmann::json sendingRtpParameters;
    
        
    };

    class Consumer : public Handler
    {
        
    public:
        Consumer(Peer * peer, std::string &peerID);
      
        void runit(Signaler *signal,  nlohmann::json & producer);

        std::string GetOffer(const std::string& id, const std::string& kind, const nlohmann::json & rtpParameters);

        void loadAnswer(Signaler *signal, std::string sdp);
        void resume(Signaler *signal , nlohmann::json & producer, bool pause );
        

        std::string offer;
    private:
        int mid{0};
      
        nlohmann::json consumer;

    };
    
    
} // namespace SdpParse

#endif
