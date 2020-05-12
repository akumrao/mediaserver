
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
    
    void Peer::on_consumer_answer( const json &sdp)
    {
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
             
         producers =  new Producers(signaler,room , this );
         producers->runit(answer);
          
        //int sz =  producers->mapProducer.size(); 
        signaler->sendSDP("answer", answer, participantID);
            
    }
     
    void Peer::onSubscribe( Peer *producerPeer)
    {
          if(producerPeer->mapOtherConumers.find(participantID ) !=   producerPeer->mapOtherConumers.end() )
          {
            //  delete producerPeer->mapOtherConumers[ participantID ];
              producerPeer->mapOtherConumers.erase(participantID);
          }
          
          if(mapSelfConumers.find(producerPeer->participantID ) !=   mapSelfConumers.end() )
          {
              if(mapSelfConumers[ producerPeer->participantID ])
              {
                delete mapSelfConumers[ producerPeer->participantID ];
                mapSelfConumers[ producerPeer->participantID ]= nullptr;
              }
              mapSelfConumers.erase(producerPeer->participantID);
          }
          
          
          if( producerPeer->producers)
          {      std::string offer;
                consumers =  new Consumers(signaler,room , this, producerPeer->producers);
                consumers->runit(offer);
                signaler->sendSDP("offer", offer, participantID);
                producerPeer->mapOtherConumers[ participantID ] = consumers;
                mapSelfConumers[ producerPeer->participantID ] = consumers;
          }
    }
    
    void Peer::onDisconnect( Peers *peers)
    {
        for(auto & othCon : mapOtherConumers )
        {
//            if(othCon.second)
//            {
//                delete othCon.second;
//                mapOtherConumers[othCon.first] = nullptr;
//            }
            SInfo <<  "delte producer: " <<  participantID <<  " cusumer: " << othCon.first;
            mapOtherConumers.erase(othCon.first);

        }
        mapOtherConumers.clear();
         
         
        for(auto & selfCon : mapSelfConumers )
        {
           if(selfCon.second )
           {
                delete selfCon.second;
                mapSelfConumers[selfCon.first] = nullptr;
           }
           

          Peer *producerPeer =  peers->mapPeers[selfCon.first];

          if(producerPeer)
          if(producerPeer->mapOtherConumers.find(participantID ) !=   producerPeer->mapOtherConumers.end() )
          {
              SInfo <<  "delete producer: " <<  participantID <<  " cusumer: " << selfCon.first;

              producerPeer->mapOtherConumers.erase(participantID);
          }

        }
        mapSelfConumers.clear();
          
    }
     
     
    Peer::~Peer()
    {
        if(producers)
        {
            delete producers;
            producers = nullptr;
        }
         
//        if(consumers)
//        {
//            delete consumers;
//            consumers = nullptr;
//        }
    }
    
    
    void Peer::producer_getStats()
    {
           
         if(producers)
         {
           producers->producer_getStats();
         }
            
    }

    void Peer::rtpObserver_addProducer()
    {
         if(producers)
         {
           producers->rtpObserver_addProducer();
         }
            
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
    
    

    void Peers::on_producer_offer( std::string& participantID, const json &sdp)
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
          
    }
    
    void Peers::onSubscribe(std::string& participantID)
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

        for( auto & pr : mapPeers)
        {
           if( pr.first != participantID)  // do not stream your stream to youself
            peer->onSubscribe( pr.second);
            
        }
          
    }

    
    void Peers::on_consumer_answer( std::string& participantID, const json &sdp)
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

        peer->on_consumer_answer(sdp);
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

        peer->onDisconnect(this);
        delete peer;
        mapPeers.erase(participantID);
        
    }
    
    void Peers::producer_getStats( std::string& participantID)
    {
        Peer *peer;
        if (mapPeers.find(participantID) != mapPeers.end()) {
            peer = mapPeers[participantID];
        } else {
             SError << "Peer does not exist. Not a possible state. " << participantID ;
             return ;
        }

        peer->producer_getStats();
    
    }  

    void Peers::rtpObserver_addProducer( std::string& participantID)
    {
        Peer *peer;
        if (mapPeers.find(participantID) != mapPeers.end()) {
            peer = mapPeers[participantID];
        } else {
             SError << "Peer does not exist. Not a possible state. " << participantID ;
             return ;
        }

        peer->rtpObserver_addProducer();
    
    }  

    
  
} // namespace SdpParse
