#ifndef SDP_DEVICE_H
#define SDP_DEVICE_H

#include "sdp/RemoteSdp.h"
#include <json.hpp>
#include <map>
#include <string>
#include "sdp/Handler.h"

namespace SdpParse {
    struct Room;
    class Peer {
    public:
        Peer(Signaler *signaler, Room *room):signaler(signaler),room(room)
        {
            
        }

        ~Peer();

        bool IsLoaded() const;
        const nlohmann::json& GetRtpCapabilities() const;
        const nlohmann::json& GetSctpCapabilities() const;
        void Load( nlohmann::json routerRtpCapabilities, std::string sdp);
        
                
        bool CanProduce(const std::string& kind);

        nlohmann::json GetNativeSctpCapabilities();
        
        std::string participantID;
        std::string participantName;
        

    private:
        // Loaded flag.
        bool loaded{ false};
        // Extended RTP capabilities.
        nlohmann::json extendedRtpCapabilities;
        // Local RTP capabilities for receiving media.
        nlohmann::json recvRtpCapabilities;
        // Whether we can produce audio/video based on computed extended RTP capabilities.
        // clang-format off
        std::map<std::string, bool> canProduceByKind ={
            { "audio", false},
            { "video", false}
        };
        // clang-format on
        // Local SCTP capabilities.
        nlohmann::json sctpCapabilities;
        
      
            
    public:
        
    nlohmann::json sendingRtpParametersByKind;
    nlohmann::json sendingRemoteRtpParametersByKind ;
    nlohmann::json sdpObject;
    
    int reqId{1};
    
    void on_producer_offer( const nlohmann::json &sdp);
    void on_consumer_answer( const nlohmann::json &sdp);
   
    void onSubscribe();
    
    private:
        
     Producers *producers{nullptr};
     Consumers *consumers{nullptr};
     
     Room *room;
     Signaler *signaler;
     
    };
    
     
    class Peers
    {
    public:
        Peers(Signaler *signaler, Room *room):signaler(signaler),room(room)
        {
        }
        
        ~Peers();
        
        void on_producer_offer( std::string& participantID, const nlohmann::json &sdp);
        void on_consumer_answer( std::string& participantID, const nlohmann::json &sdp);
        
        void onSubscribe(std::string& participantID);
        
        std::map< std::string, Peer*> mapPeers;
        
        Room *room;
        Signaler *signaler;   
    };
    
    
} // namespace SdpParse

#endif
