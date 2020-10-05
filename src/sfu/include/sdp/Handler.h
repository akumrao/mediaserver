#ifndef SDP_HANDLER_H
#define SDP_HANDLER_H


#include "sdp/RemoteSdp.h"
#include <json.hpp>
#include <unordered_map>
#include <string>
#include <atomic>
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
        void raiseRequest( nlohmann::json &param , nlohmann::json& trans,  std::function<void (const nlohmann::json& )> func );

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
        
    public:
        Peer *peer;
        
        std::string classtype;
        std::string constructor_name { "WebrtcTransport"};

    };

    class Consumers;
    class Producers:public Handler
    {
       
        public:
        
        Producers(Signaler *signaler, Peer *peer);
        
        ~Producers();

        void close_producer( const std::string &producerid);
        void runit(std::function<void (const std::string & )> cbAns);
        
//        struct MapProCon
//        {   //std::string answer;
//           // Consumers *cons{nullptr};
//            std::string consumerId;
//        };
        
        struct Producer
        {   //std::string answer;
            nlohmann::json producer;
            std::unordered_map < Consumers*, std::string > mapProCon;   // std::string consumerId;
        };
        

        void producer_getStats(const std::string& producerId); 
        void rtpObserver_addProducer( bool flag);
        void resume(const std::string& producerId, bool pause);
        
        std::unordered_map<std::string, Producer*>  mapProducer;
        // this is to store mid 0 and 1 so that audio and video are sequence during sdp generation.
        std::map<size_t, std::string>  mapProdMid; 
        
        
        
    private:
        void GetAnswer(std::string & kind , nlohmann::json &sendingRtpParameters,std::string mid, std::string reuseMid,  nlohmann::json &offerMediaObject );
        
        std::string cnameForProducers; 
        
    };

    class Consumers : public Handler
    {
        
    public:
        Consumers(Signaler *signaler, Peer * peer);
        ~Consumers();
      
        void runit(std::vector < Peer *> vecProdPeer);
       
        //void sendOffer(const std::string& id, const std::string&  mid , const std::string& kind, const nlohmann::json & rtpParameters, const std::string& partID , const std::string& remotePartID);

        void loadAnswer( std::string sdp);
        void resume( const std::string& consumerId , bool pause);

        void close_consumer(const std::string& producerid, const std::string& conumserid );
        void consumer_getStats( const std::string& consumerIds); 
         void sendSDP(std::string &from);
       // void onUnSubscribe(const std::string& producerPeer);
        void setPreferredLayers( nlohmann::json &layer);
        //std::atomic<uint8_t>  nodevice{0};
        
       
        struct Consumer
        {  // std::string offer;
            nlohmann::json consumer;
            Producers *producers;
        };
        std::map<int, std::string>  mapConMid;  // map mid with conusmer id
        std::unordered_map<std::string, Consumer*>  mapConsumer;  // map number of consumer Devices
        //std::unordered_map<std::string, std::pair<int, int > > mapProdDevs;  // map number of device mid used from producer 
                                // pair is for mid range 0 to maximum devices
        bool _probatorConsumerCreated{true};
    private:
       //int mid{0};
       // Producers *producers;

    };
    
    
} // namespace SdpParse

#endif
