#ifndef SDP_HANDLER_H
#define SDP_HANDLER_H


#include "sdp/RemoteSdp.h"
#include <json.hpp>
#include <map>
#include <string>

namespace SdpParse {
    
    class Peer;
    class Signaler;
    class Room;
    
    class Handler
    {
    
    public:
                
        Handler(Signaler *signaler,  Peer * peer):signaler(signaler),peer(peer)
        {
            
        }
        ~Handler()
        {
            if (remoteSdp) {
                delete remoteSdp;
                remoteSdp = nullptr;
            }
        }
        
        void transportCreate();
        void transportConnect(const nlohmann::json& sdpObject, const std::string& localDtlsRole);
        void raiseRequest( nlohmann::json &param , nlohmann::json& trans, nlohmann::json& ack_resp);

        Sdp::RemoteSdp *remoteSdp{nullptr};
        void createSdp(const nlohmann::json& iceParameters, const nlohmann::json& iceCandidates, const nlohmann::json& dtlsParameters);
        nlohmann::json  _setupTransport(const nlohmann::json & sdpObject, const std::string& localDtlsRole);
        
        bool transport_connect{false};
    protected:
      
        //std::string peerID;
        std::string transportId;
        //nlohmann::json dtlsParameters;
        bool forceTcp{false};
 
        Signaler *signaler;
       // std::string roomId;
        
        Peer *peer;
        
        std::string classtype;
        std::string constructor_name { "WebrtcTransport"};

    };

    class Producers:public Handler
    {
       
        public:
        
        Producers(Signaler *signaler, Peer *peer);
        
        ~Producers();

        void runit(std::string & answer );
        
        struct Producer
        {   //std::string answer;
            nlohmann::json producer;
        };
        

        void producer_getStats( nlohmann::json &stats); 
        void rtpObserver_addProducer( bool flag);

        
        std::map<std::string, Producer*>  mapProducer;
        // this is to store mid 0 and 1 so that audio and video are sequence during sdp generation.
        std::map<size_t, std::string>  mapProdMid;  
        
    private:
        std::string GetAnswer(std::string & kind , nlohmann::json &sendingRtpParameters, Sdp::RemoteSdp::MediaSectionIdx &mediaSectionIdx);
        
        std::string cnameForProducers; 
        
    };

    class Consumers : public Handler
    {
        
    public:
        Consumers(Signaler *signaler, Peer * peer);
        ~Consumers();
      
        void runit(std::string& offer,  Producers *producers);

        std::string GetOffer(const std::string& id, size_t  mid , const std::string& kind, const nlohmann::json & rtpParameters);

        void loadAnswer( std::string sdp);
        void resume( bool pause);

        void consumer_getStats( nlohmann::json &stats); 
        void setPreferredLayers( nlohmann::json &layer);
        

        struct Consumer
        {  // std::string offer;
            nlohmann::json consumer;
        };
        
        std::map<std::string, Consumer*>  mapConsumer;
        
    private:
        //int mid{0};
      
       // Producers *producers;

    };
    
    
} // namespace SdpParse

#endif
