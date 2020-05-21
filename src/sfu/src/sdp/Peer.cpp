
#include "sdp/Peer.h"
#include "LoggerTag.h"
#include "base/error.h"
#include "sdp/ortc.h"
#include "sdptransform.hpp"
#include "sdp/Utils.h"
#include "base/uuid.h"
#include "sdp/signaler.h"
using json = nlohmann::json;


static json SctpNumStreams = {
    { "OS", 1024u},
    { "MIS", 1024u}
};

namespace SdpParse {

    /**
     * Whether the Peer is loaded.
     */
    bool Peer::IsLoaded() const {


        return this->loaded;
    }

    /**
     * RTP capabilities of the Peer for receiving media.
     */
    const json& Peer::GetRtpCapabilities() const {


        if (!this->loaded)
            MS_ABORT("not loaded");

        return this->recvRtpCapabilities;
    }

    /**
     * SCTP capabilities of the Peer for receiving media.
     */
    const json& Peer::GetSctpCapabilities() const {


        if (!this->loaded)
            MS_ABORT("not loaded");

        return this->sctpCapabilities;
    }

    
    Peer::Peer(Signaler *signaler, std::string &roomId):signaler(signaler),roomId(roomId)
    {
        //consumers =  new Consumers(signaler, this);
    }

    ////////////////////////
    /**
     * Initialize the Peer.
     */
    void Peer::Load(json routerRtpCapabilities, std::string sdp) {

        if (this->loaded)
            MS_ABORT("already loaded");

      //  LDebug("got sdpObject: ", routerRtpCapabilities.dump(4));
        // This may throw.
        ortc::validateRtpCapabilities(routerRtpCapabilities);

        // Get Native RTP capabilities.
        //auto nativeRtpCapabilities = Handler::GetNativeRtpCapabilities(peerConnectionOptions);
        sdpObject = sdptransform::parse(sdp);

        
        //LDebug("got sdpObject: ", sdpObject.dump(4));

        auto nativeRtpCapabilities = Sdp::Utils::extractRtpCapabilities(sdpObject);



        //LDebug("got native RTP capabilities: ", nativeRtpCapabilities.dump(4));

        // This may throw.
        ortc::validateRtpCapabilities(nativeRtpCapabilities);

        // Get extended RTP capabilities.
        this->extendedRtpCapabilities =
                ortc::getExtendedRtpCapabilities(nativeRtpCapabilities, routerRtpCapabilities);

        //LDebug("got extended RTP capabilities: ", this->extendedRtpCapabilities.dump(4));

        // Check whether we can produce audio/video.
        this->canProduceByKind["audio"] = ortc::canSend("audio", this->extendedRtpCapabilities);
        this->canProduceByKind["video"] = ortc::canSend("video", this->extendedRtpCapabilities);

        // Generate our receiving RTP capabilities for receiving media.
        this->recvRtpCapabilities = ortc::getRecvRtpCapabilities(this->extendedRtpCapabilities);

       // LDebug("got receiving RTP capabilities: ", this->recvRtpCapabilities.dump(4));

        // This may throw.
        ortc::validateRtpCapabilities(this->recvRtpCapabilities);

        // Generate our SCTP capabilities.
        this->sctpCapabilities = GetNativeSctpCapabilities();

        //LDebug("got receiving SCTP capabilities: ", this->sctpCapabilities.dump(4));

        // This may throw.
        ortc::validateSctpCapabilities(this->sctpCapabilities);

       // LDebug("succeeded");

        
       sendingRtpParametersByKind = {
            { "audio", ortc::getSendingRtpParameters("audio", extendedRtpCapabilities)},
            { "video", ortc::getSendingRtpParameters("video", extendedRtpCapabilities)}
        };

        sendingRemoteRtpParametersByKind = {
            { "audio", ortc::getSendingRemoteRtpParameters("audio", extendedRtpCapabilities)},
            { "video", ortc::getSendingRemoteRtpParameters("video", extendedRtpCapabilities)}
        };
        
        
        this->loaded = true;
    }

    /**
     * Whether we can produce audio/video.
     *
     */
    bool Peer::CanProduce(const std::string& kind) {
        if (!this->loaded) {
            MS_ABORT("not loaded");
        } else if (kind != "audio" && kind != "video") {
            MS_ABORT("invalid kind");
        }

        return this->canProduceByKind[kind];
    }

    json Peer::GetNativeSctpCapabilities() {
        auto caps = json::object();
        caps["numStreams"] = SctpNumStreams;
        return caps;
    }
    
    void Peer::on_consumer_answer( std::string& to, const json &sdp)
    {   
        if( mapSelfConumers.find(to) == mapSelfConumers.end())
        {
            SError << "Could not find consumer:  " << to;
        }
        Consumers *consumers = mapSelfConumers[to];
        if(!consumers->transport_connect)
         consumers->loadAnswer(sdp["sdp"].get<std::string>());   
    }
      
    void Peer::on_producer_offer( const json &sdp)
    {
         Load(Settings::configuration.routerCapabilities, sdp["sdp"].get<std::string>());
         
         std::string answer;
         
         if(producers)
         {
             delete producers;
             producers = nullptr;
         }
             
         producers =  new Producers(signaler, this );
         producers->runit(  [&]( const std::string &answer)
         {
          signaler->sendSDP("answer", answer, participantID,participantID);
         });
       
    }
     
    void Peer::onSubscribe( Peer *producerPeer)
    {
        if( producerPeer->producers)
        {
            Consumers *consumers;
            if( mapSelfConumers.find(producerPeer->participantID ) == mapSelfConumers.end())
            {
                consumers = new Consumers(signaler, this);
                mapSelfConumers[ producerPeer->participantID ] = consumers;
                 
                 //SInfo << "mapSelfConumers " <<  "participantID: " << participantID   << " for " << producerPeer->participantID; 
            }
            else
            {
                SError << "found mapSelfConumers " <<  "participantID: " << participantID   << " for " << producerPeer->participantID; 

               consumers = mapSelfConumers[producerPeer->participantID ];
            }
               
               
            consumers->runit( producerPeer->producers, [&]( const std::string &offer)
            {
                
              signaler->sendSDP("offer", offer, participantID, producerPeer->participantID);
           
             // SInfo << "sendSDP offer " <<  "participantID: " << participantID   << " for " << producerPeer->participantID;
                
            });     
            
        }
    }
    
    void Peer::onDisconnect( )
    {

        for(auto & selfCon : mapSelfConumers )
        {
           if(selfCon.second )
           {
                delete selfCon.second;
                mapSelfConumers[selfCon.first] = nullptr;
           }
           // FOr P1  = c2 c3 others map
           // for P1  = c2 c3  self map
           SInfo <<  "Erase form self consumermap : " <<  participantID <<  " cusumer: " << selfCon.first;
           mapSelfConumers.erase(selfCon.first);

        }
        mapSelfConumers.clear();
        
        if(producers)
        {
            delete producers;
            producers = nullptr;
        }
          
    }
     
     
    Peer::~Peer()
    {
        onDisconnect( );
    }
    

    void Peer::producer_getStats( nlohmann::json &stats)
    {

       if(producers)
         {
           producers->producer_getStats(stats);
         }

    }
    void  Peer::rtpObserver_addProducer( bool flag)
    {
        if(producers)
         {
           producers->rtpObserver_addProducer(flag);
         }
    }

    void  Peer::consumer_getStats( nlohmann::json &stats)
    {
//        if(consumers)
//         {
//           consumers->consumer_getStats(stats);
//         }

    }
    void  Peer::setPreferredLayers( nlohmann::json &layer)
    {
//
//       if(consumers)
//         {
//           consumers->setPreferredLayers(layer);
//         }
    }
    
    
    /*****************************************************************************
     Peers
    *********************************************************************************/
    Peers::~Peers()
    {
        for(auto & peer : mapPeers  )
        {
            delete peer.second;
        }
    }
    

    void Peers::on_producer_offer( std::string &room, std::string& participantID, const json &sdp)
    {
        Peer *peer;
        if (mapPeers.find(participantID) != mapPeers.end()) {
            SWarn << "Peer already exist " << participantID ;
            peer = mapPeers[participantID];
        } else {
            peer = new Peer( signaler, room);
            peer->participantID = participantID;
            peer->participantName = participantID;
            mapPeers[participantID] = peer;
        }
        peer->on_producer_offer(sdp);
        
        //onSubscribe(participantID,"");
    }
    
    void Peers::onSubscribe(std::string &room, std::string& participantID, const json& peerPartiID)
    {
        Peer *peer;
        if (mapPeers.find(participantID) != mapPeers.end()) {
            SInfo << "Peer already exist " << participantID ;
            peer = mapPeers[participantID];
        } else {
            
            SInfo << " New Peer " << participantID ;
            peer = new Peer( signaler, room);
            peer->participantID = participantID;
            peer->participantName = participantID;
            mapPeers[participantID] = peer;
        }

//        if(peerPartiID.empty())
//        {
//            for( auto & pr : mapPeers)
//            {
//               if( pr.first != participantID)  // do not stream your stream to youself
//                peer->onSubscribe( pr.second);
//            }
//        }
//        else
        {
            // int x = 0;
            for( auto &id : peerPartiID)
            {
                if(mapPeers.find(id)  != mapPeers.end() )
                {
                 // SInfo  << " peerids "  << id;
                 // if(++x == 2  )
                  peer->onSubscribe(mapPeers[id]);
                }
            }
        }
    }

    
    void Peers::on_consumer_answer( std::string& participantID,  std::string& to, const json &sdp)
    {
        Peer *peer;
        if (mapPeers.find(participantID) != mapPeers.end()) {
            peer = mapPeers[participantID];
        } else {
             SError << "Peer does not exist. Not a possible state. " << participantID ;
//            peer = new Peer( signaler, room);
//            peer->participantID = participantID;
//            peer->participantName = participantID;
//            mapPeers[participantID] = peer;
             return ;
        }

        peer->on_consumer_answer(to, sdp);
    }    

    
    void Peers::onDisconnect( std::string& participantID)
    {
        Peer *peer;
        if (mapPeers.find(participantID) != mapPeers.end()) {
            peer = mapPeers[participantID];
        } else {
             SError << "Peer does not exist. Not a possible state. " << participantID ;
             return ;
        }

        peer->onDisconnect();
        delete peer;
        mapPeers.erase(participantID);
        
    }
    

    void  Peers::producer_getStats( std::string& participantID, nlohmann::json &stats)
    {

        Peer *peer;
        if (mapPeers.find(participantID) != mapPeers.end()) {
            peer = mapPeers[participantID];
        } else {
             SError << "Peer does not exist. Not a possible state. " << participantID ;
             return ;
        }

        peer->producer_getStats(stats);

    }
    void  Peers::rtpObserver_addProducer( std::string& participantID, bool flag)
    {


        Peer *peer;
        if (mapPeers.find(participantID) != mapPeers.end()) {
            peer = mapPeers[participantID];
        } else {
             SError << "Peer does not exist. Not a possible state. " << participantID ;
             return ;
        }

        peer->rtpObserver_addProducer(flag);

    }

    void  Peers::consumer_getStats( std::string& participantID, nlohmann::json &stats)
    {

        Peer *peer;
        if (mapPeers.find(participantID) != mapPeers.end()) {
            peer = mapPeers[participantID];
        } else {
             SError << "Peer does not exist. Not a possible state. " << participantID ;
             return ;
        }

        peer->consumer_getStats(stats);

    }
    void  Peers::setPreferredLayers( std::string& participantID,  nlohmann::json &layer)
    {


        Peer *peer;
        if (mapPeers.find(participantID) != mapPeers.end()) {
            peer = mapPeers[participantID];
        } else {
             SError << "Peer does not exist. Not a possible state. " << participantID ;
             return ;
        }

        peer->setPreferredLayers(layer);

    }

    void Peers::resume(std::string& participantID, std::string& consumerID,  bool flag)
    {
//         Peer *peer;
//        if (mapPeers.find(participantID) != mapPeers.end()) {
//            peer = mapPeers[participantID];
//        } else {
//             SError << "Peer does not exist. Not a possible state. " << participantID ;
//             return ;
//        }
//
//        peer->consumers->resume(consumerID,flag );

    }
  
} // namespace SdpParse
