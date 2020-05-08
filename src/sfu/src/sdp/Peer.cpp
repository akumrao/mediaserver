
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
         
         producers =  new Producers(signaler,room , this );
         producers->runit();
          
        int sz =  producers->mapProducer.size(); 
        signaler->sendSDP("answer", producers->mapProducer.begin()->second->answer, participantID);
            
    }
     
    void Peer::onSubscribe( Peer *producerPeer)
    {
          if( producerPeer->producers)
          {
                consumers =  new Consumers(signaler,room , this, producerPeer->producers);
                consumers->runit();
                signaler->sendSDP("offer", consumers->mapConsumer.begin()->second->offer, participantID);
          }
    }
     
    Peer::~Peer()
    {
        if(producers)
        {
            delete producers;
            producers = nullptr;
        }
         
        if(consumers)
        {
            delete consumers;
            consumers = nullptr;
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
       
  
} // namespace SdpParse
