
#include "sdp/Device.h"
#include "LoggerTag.h"
#include "base/error.h"
#include "sdp/ortc.h"
#include "sdptransform.hpp"
#include "sdp/Utils.h"


using json = nlohmann::json;


static json SctpNumStreams = {
    { "OS", 1024u},
    { "MIS", 1024u}
};

namespace SdpParse {

    /**
     * Whether the Device is loaded.
     */
    bool Device::IsLoaded() const {


        return this->loaded;
    }

    /**
     * RTP capabilities of the Device for receiving media.
     */
    const json& Device::GetRtpCapabilities() const {


        if (!this->loaded)
            MS_ABORT("not loaded");

        return this->recvRtpCapabilities;
    }

    /**
     * SCTP capabilities of the Device for receiving media.
     */
    const json& Device::GetSctpCapabilities() const {


        if (!this->loaded)
            MS_ABORT("not loaded");

        return this->sctpCapabilities;
    }

    /**
     * Initialize the Device.
     */
    void Device::Load(json routerRtpCapabilities, std::string sdp) {

        //if (this->loaded)
           // MS_ABORT("already loaded");

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

        this->loaded = true;
    }

    /**
     * Whether we can produce audio/video.
     *
     */
    bool Device::CanProduce(const std::string& kind) {
        if (!this->loaded) {
            MS_ABORT("not loaded");
        } else if (kind != "audio" && kind != "video") {
            MS_ABORT("invalid kind");
        }

        return this->canProduceByKind[kind];
    }

    json Device::GetNativeSctpCapabilities() {
        auto caps = json::object();
        caps["numStreams"] = SctpNumStreams;
        return caps;
    }
    
    void Device::_setupTransport(const std::string localDtlsRole  )
    {
   
        // Get our local DTLS parameters.
        dtlsParameters = Sdp::Utils::extractDtlsParameters(sdpObject);
        // Set our DTLS role.
        dtlsParameters["role"] = localDtlsRole;

       

        // Update the remote DTLS role in the SDP.
        remoteSdp->UpdateDtlsRole(
            localDtlsRole == "client" ? "server" : "client");

        // Need to tell the remote transport about our parameters.
       // await this.safeEmitAsPromise('@connect', { dtlsParameters });

       // this._transportReady = true;
    }
     

    void Device::createSdp(const json& iceParameters, const json& iceCandidates, const json& dtlsParameters) 
    {
        remoteSdp = new Sdp::RemoteSdp(iceParameters, iceCandidates, dtlsParameters, nullptr);
    }
           
    std::string Device::GetAnswer() {
        json sendingRtpParametersByKind = {
            { "audio", ortc::getSendingRtpParameters("audio", extendedRtpCapabilities)},
            { "video", ortc::getSendingRtpParameters("video", extendedRtpCapabilities)}
        };

        json sendingRemoteRtpParametersByKind = {
            { "audio", ortc::getSendingRemoteRtpParameters("audio", extendedRtpCapabilities)},
            { "video", ortc::getSendingRemoteRtpParameters("video", extendedRtpCapabilities)}
        };

        sendingRtpParameters = sendingRtpParametersByKind["video"];
        
        const Sdp::RemoteSdp::MediaSectionIdx mediaSectionIdx = remoteSdp->GetNextMediaSectionIdx();
        
       json& offerMediaObject = sdpObject["media"][mediaSectionIdx.idx];
          
       auto midIt   = offerMediaObject.find("mid");
        
       if( midIt == offerMediaObject.end())
       {
            SError << "Found no mid in SDP";
            throw "Found no mid in SDP";
       }
        
       sendingRtpParameters["mid"] = *midIt;
        
       sendingRtpParameters["rtcp"]["cname"] = Sdp::Utils::getCname(offerMediaObject);
       sendingRtpParameters["encodings"] = Sdp::Utils::getRtpEncodings(offerMediaObject);
        
        
        // If VP8 and there is effective simulcast, add scalabilityMode to each encoding.
       auto mimeType = sendingRtpParameters["codecs"][0]["mimeType"].get<std::string>();

       std::transform(mimeType.begin(), mimeType.end(), mimeType.begin(), ::tolower);

       if (
                sendingRtpParameters["encodings"].size() > 1 &&
                (mimeType == "video/vp8" || mimeType == "video/h264")
        )
        {
                for (auto& encoding : sendingRtpParameters["encodings"])
                {
                        encoding["scalabilityMode"] = "S1T3";
                }
        }
       
      // STrace << "sendingRtpParameters "  <<  sendingRtpParameters ;

        json *codecOptions = nullptr;
        
        _setupTransport("server");

        remoteSdp->Send(
                offerMediaObject,
                mediaSectionIdx.reuseMid,
                sendingRtpParameters,
                sendingRemoteRtpParametersByKind["video"],
                codecOptions);

        auto answer = remoteSdp->GetSdp();
        
        if (remoteSdp) {
                delete remoteSdp;
                remoteSdp = nullptr;
            }
        
        STrace << "andwer: " <<  answer ;
        return answer;
    }

    
    std::string Device::GetOffer(const std::string& id, const std::string& kind, const json& rtpParameters) {
       std::string localId;
       
       static int mid=0;  // wrong TBD

		// mid is optional, check whether it exists and is a non empty string.
        auto midIt = rtpParameters.find("mid");
        if (midIt != rtpParameters.end() && (midIt->is_string() && !midIt->get<std::string>().empty()))
            localId = midIt->get<std::string>();
        else
            localId = std::to_string(mid++);

        auto& cname = rtpParameters["rtcp"]["cname"];

        this->remoteSdp->Receive(localId, kind, rtpParameters, cname, id);

        auto offer = this->remoteSdp->GetSdp();
        
        if (remoteSdp) {
                delete remoteSdp;
                remoteSdp = nullptr;
            }
        
        STrace << "offer: " <<  offer ;
        return offer;
    } 
    
    //	SendTransport* Device::CreateSendTransport(
    //	  SendTransport::Listener* listener,
    //	  const std::string& id,
    //	  const json& iceParameters,
    //	  const json& iceCandidates,
    //	  const json& dtlsParameters,
    //	  const json& sctpParameters,
    //	  const PeerConnection::Options* peerConnectionOptions,
    //	  const json& appData) const
    //	{
    //		
    //
    //		if (!this->loaded)
    //			MS_ABORT("not loaded");
    //		else if (!appData.is_object())
    //			MS_ABORT("appData must be a JSON object");
    //
    //		// Validate arguments.
    //		ortc::validateIceParameters(const_cast<json&>(iceParameters));
    //		ortc::validateIceCandidates(const_cast<json&>(iceCandidates));
    //		ortc::validateDtlsParameters(const_cast<json&>(dtlsParameters));
    //
    //		if (!sctpParameters.is_null())
    //			ortc::validateSctpParameters(const_cast<json&>(sctpParameters));
    //
    //		// Create a new Transport.
    //		auto* transport = new SendTransport(
    //		  listener,
    //		  id,
    //		  iceParameters,
    //		  iceCandidates,
    //		  dtlsParameters,
    //		  sctpParameters,
    //		  peerConnectionOptions,
    //		  &this->extendedRtpCapabilities,
    //		  &this->canProduceByKind,
    //		  appData);
    //
    //		return transport;
    //	}
    //
    //	SendTransport* Device::CreateSendTransport(
    //	  SendTransport::Listener* listener,
    //	  const std::string& id,
    //	  const json& iceParameters,
    //	  const json& iceCandidates,
    //	  const json& dtlsParameters,
    //	  const PeerConnection::Options* peerConnectionOptions,
    //	  const json& appData) const
    //	{
    //		
    //
    //		return Device::CreateSendTransport(
    //		  listener, id, iceParameters, iceCandidates, dtlsParameters, nullptr, peerConnectionOptions, appData);
    //	}
    //
    //	RecvTransport* Device::CreateRecvTransport(
    //	  Transport::Listener* listener,
    //	  const std::string& id,
    //	  const json& iceParameters,
    //	  const json& iceCandidates,
    //	  const json& dtlsParameters,
    //	  const json& sctpParameters,
    //	  const PeerConnection::Options* peerConnectionOptions,
    //	  const json& appData) const
    //	{
    //		
    //
    //		if (!this->loaded)
    //			MS_ABORT("not loaded");
    //		else if (!appData.is_object())
    //			MS_ABORT("appData must be a JSON object");
    //
    //		// Validate arguments.
    //		ortc::validateIceParameters(const_cast<json&>(iceParameters));
    //		ortc::validateIceCandidates(const_cast<json&>(iceCandidates));
    //		ortc::validateDtlsParameters(const_cast<json&>(dtlsParameters));
    //
    //		if (!sctpParameters.is_null())
    //			ortc::validateSctpParameters(const_cast<json&>(sctpParameters));
    //
    //		// Create a new Transport.
    //		auto* transport = new RecvTransport(
    //		  listener,
    //		  id,
    //		  iceParameters,
    //		  iceCandidates,
    //		  dtlsParameters,
    //		  sctpParameters,
    //		  peerConnectionOptions,
    //		  &this->extendedRtpCapabilities,
    //		  appData);
    //
    //		return transport;
    //	}
    //
    //	RecvTransport* Device::CreateRecvTransport(
    //	  Transport::Listener* listener,
    //	  const std::string& id,
    //	  const json& iceParameters,
    //	  const json& iceCandidates,
    //	  const json& dtlsParameters,
    //	  const PeerConnection::Options* peerConnectionOptions,
    //	  const json& appData) const
    //	{
    //		
    //
    //		return Device::CreateRecvTransport(
    //		  listener, id, iceParameters, iceCandidates, dtlsParameters, nullptr, peerConnectionOptions, appData);
    //	}
} // namespace SdpParse
