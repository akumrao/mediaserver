
#include "sdp/Device.h"
#include "LoggerTag.h"
#include "base/error.h"
#include "sdp/ortc.h"
#include "sdptransform.hpp"
#include "sdp/Utils.h"
#include "sdp/RemoteSdp.h"

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

        if (this->loaded)
            MS_ABORT("already loaded");

        // This may throw.
        ortc::validateRtpCapabilities(routerRtpCapabilities);

        // Get Native RTP capabilities.
        //auto nativeRtpCapabilities = Handler::GetNativeRtpCapabilities(peerConnectionOptions);
        sdpObject = sdptransform::parse(sdp);




        auto nativeRtpCapabilities = Sdp::Utils::extractRtpCapabilities(sdpObject);



        auto dtlsParameters = Sdp::Utils::extractDtlsParameters(sdpObject);
        // Set our DTLS role.
        dtlsParameters["role"] = "server";



        LDebug("got native RTP capabilities: ", nativeRtpCapabilities.dump(4));

        // This may throw.
        ortc::validateRtpCapabilities(nativeRtpCapabilities);

        // Get extended RTP capabilities.
        this->extendedRtpCapabilities =
                ortc::getExtendedRtpCapabilities(nativeRtpCapabilities, routerRtpCapabilities);

        LDebug("got extended RTP capabilities: ", this->extendedRtpCapabilities.dump(4));

        // Check whether we can produce audio/video.
        this->canProduceByKind["audio"] = ortc::canSend("audio", this->extendedRtpCapabilities);
        this->canProduceByKind["video"] = ortc::canSend("video", this->extendedRtpCapabilities);

        // Generate our receiving RTP capabilities for receiving media.
        this->recvRtpCapabilities = ortc::getRecvRtpCapabilities(this->extendedRtpCapabilities);

        LDebug("got receiving RTP capabilities: ", this->recvRtpCapabilities.dump(4));

        // This may throw.
        ortc::validateRtpCapabilities(this->recvRtpCapabilities);

        // Generate our SCTP capabilities.
        this->sctpCapabilities = GetNativeSctpCapabilities();

        LDebug("got receiving SCTP capabilities: ", this->sctpCapabilities.dump(4));

        // This may throw.
        ortc::validateSctpCapabilities(this->sctpCapabilities);

        LDebug("succeeded");

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

    void Device::GetAnswer(const json& iceParameters, const json& iceCandidates, const json& dtlsParameters) {
        json sendingRtpParametersByKind = {
            { "audio", ortc::getSendingRtpParameters("audio", extendedRtpCapabilities)},
            { "video", ortc::getSendingRtpParameters("video", extendedRtpCapabilities)}
        };

        json sendingRemoteRtpParametersByKind = {
            { "audio", ortc::getSendingRemoteRtpParameters("audio", extendedRtpCapabilities)},
            { "video", ortc::getSendingRemoteRtpParameters("video", extendedRtpCapabilities)}
        };

        json& sendingRtpParameters = sendingRtpParametersByKind["video"];




        Sdp::RemoteSdp *remoteSdp = new Sdp::RemoteSdp(iceParameters, iceCandidates, dtlsParameters, nullptr);

        const Sdp::RemoteSdp::MediaSectionIdx mediaSectionIdx = remoteSdp->GetNextMediaSectionIdx();

        json& offerMediaObject = sdpObject["media"][mediaSectionIdx.idx];

        json *codecOptions = nullptr;


        remoteSdp->Send(
                offerMediaObject,
                mediaSectionIdx.reuseMid,
                sendingRtpParameters,
                sendingRemoteRtpParametersByKind["video"],
                codecOptions);

        auto answer = remoteSdp->GetSdp();


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
