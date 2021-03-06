#ifndef SDP_DEVICE_H
#define SDP_DEVICE_H

#include "sdp/RemoteSdp.h"
#include <json.hpp>
#include <unordered_map>
#include <string>
#include "sdp/Handler.h"

namespace SdpParse {
    class Peers;
    struct Room;
    class Peer {
    public:
        Peer(Signaler *signaler, std::string &roomId);
       

        ~Peer();

        bool IsLoaded() const;
        const nlohmann::json& GetRtpCapabilities() const;
        const nlohmann::json& GetSctpCapabilities() const;
        void Load( nlohmann::json routerRtpCapabilities, std::string sdp);
        
                
        bool CanProduce(const std::string& kind);

        nlohmann::json GetNativeSctpCapabilities();
        
        std::string participantID;
        std::string participantName;
        
        
        std::map<std::string, bool> canProduceByKind ={
            { "audio", false},
            { "video", false}
        };
        

    private:
        // Loaded flag.
        bool loaded{ false};
        // Extended RTP capabilities.
        nlohmann::json extendedRtpCapabilities;
        // Local RTP capabilities for receiving media.
        nlohmann::json recvRtpCapabilities;
        // Whether we can produce audio/video based on computed extended RTP capabilities.
        // Local SCTP capabilities.
        nlohmann::json sctpCapabilities;
        
      
            
    public:
        std::string roomId;
        
    nlohmann::json sendingRtpParametersByKind;
    nlohmann::json sendingRemoteRtpParametersByKind ;
    nlohmann::json sdpObject;
    
    int getMaxMid( ){return maxMid;}
    
    
    void on_producer_offer( const nlohmann::json &sdp);
    void on_consumer_answer( std::string& to,const nlohmann::json &sdp);
    void onSubscribe( const std::vector < Peer *> &vecProdPeer );
    //void onUnSubscribe( const std::string& producerPeer);
    void onDisconnect();
    
    void producer_getStats(const std::string& producerId); 
    void consumer_getStats(const std::string& consumerId); 
    void rtpObserver_addProducer( bool flag);
    void setPreferredLayers( nlohmann::json &layer);

    Producers *producers{nullptr};
    Consumers *consumers{nullptr};
    
    private:
        
     //std::map<std::string, Consumers* >  mapOtherConumers;
     std::unordered_map<std::string, Consumers* >  mapSelfConumers;
     
     int maxMid;
     
     Room *room;
     Signaler *signaler;
     
    };
    
     
    class Peers
    {
    public:
        Peers(Signaler *signaler):signaler(signaler)
        {
        }
        
        ~Peers();
        
        void on_producer_offer(std::string &room, std::string& participantID, std::string& user, const nlohmann::json &sdp);
        void on_consumer_answer( std::string& participantID, std::string& to, const nlohmann::json &sdp);
        
        void onSubscribe(std::string &room, std::string& participantID, std::string& user, const nlohmann::json& peerPartiID);
        
        void onDisconnect( std::string& participantID);
        
        void resume(std::string& participantID, std::string& handlerId,  bool flag , bool producer);
    
        void producer_getStats( std::string& participantID, const std::string& producerId); 
        void consumer_getStats( std::string& participantID, const std::string& consumerId); 
        void rtpObserver_addProducer( std::string& participantID, bool flag);
        void setPreferredLayers( std::string& participantID,  nlohmann::json &layer);
        

        std::unordered_map< std::string, Peer*> mapPeers;
        
  
        Signaler *signaler;   
    };
    
    
} // namespace SdpParse

#endif
