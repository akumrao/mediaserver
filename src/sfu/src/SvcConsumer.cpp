#define MS_CLASS "RTC::SvcConsumer"
// #define MS_LOG_DEV_LEVEL 3

#include "RTC/SvcConsumer.h"
#include "base/application.h"
#include "LoggerTag.h"
#include "base/error.h"
#include "Utils.h"
#include "Channel/Notifier.h"
#include "RTC/Codecs/Codecs.h"

namespace RTC
{
	/* Static. */

	static constexpr uint64_t BweDowngradeConservativeMs{ 10000u }; // In ms.
	static constexpr uint64_t BweDowngradeMinActiveMs{ 8000u };     // In ms.

	/* Instance methods. */

	SvcConsumer::SvcConsumer(const std::string& id, RTC::Consumer::Listener* listener, json& data)
	  : RTC::Consumer::Consumer(id, listener, data, RTC::RtpParameters::Type::SVC)
	{
		MS_TRACE();

		// Ensure there is a single encoding.
		if (this->consumableRtpEncodings.size() != 1u)
			base::uv::throwError("invalid consumableRtpEncodings with size != 1");

		auto& encoding = this->rtpParameters.encodings[0];

		// Ensure there are multiple spatial or temporal layers.
		if (encoding.spatialLayers < 2u && encoding.temporalLayers < 2u)
			base::uv::throwError("invalid number of layers");

		auto jsonPreferredLayersIt = data.find("preferredLayers");

		// Set preferredLayers (if given).
		if (jsonPreferredLayersIt != data.end() && jsonPreferredLayersIt->is_object())
		{
			auto jsonSpatialLayerIt  = jsonPreferredLayersIt->find("spatialLayer");
			auto jsonTemporalLayerIt = jsonPreferredLayersIt->find("temporalLayer");

			
			if (
				jsonSpatialLayerIt == jsonPreferredLayersIt->end() ||
				!Utils::Json::IsPositiveInteger(*jsonSpatialLayerIt)
			)
			
			{
				base::uv::throwError("missing preferredLayers.spatialLayer");
			}

			this->preferredSpatialLayer = jsonSpatialLayerIt->get<int16_t>();

			if (this->preferredSpatialLayer > encoding.spatialLayers - 1)
				this->preferredSpatialLayer = encoding.spatialLayers - 1;

			
			if (
				jsonTemporalLayerIt != jsonPreferredLayersIt->end() &&
				Utils::Json::IsPositiveInteger(*jsonTemporalLayerIt)
			)
			
			{
				this->preferredTemporalLayer = jsonTemporalLayerIt->get<int16_t>();

				if (this->preferredTemporalLayer > encoding.temporalLayers - 1)
					this->preferredTemporalLayer = encoding.temporalLayers - 1;
			}
			else
			{
				this->preferredTemporalLayer = encoding.temporalLayers - 1;
			}
		}
		else
		{
			// Initially set preferredSpatialLayer and preferredTemporalLayer to the
			// maximum value.
			this->preferredSpatialLayer  = encoding.spatialLayers - 1;
			this->preferredTemporalLayer = encoding.temporalLayers - 1;
		}

		// Create the encoding context.
		auto* mediaCodec = this->rtpParameters.GetCodecForEncoding(encoding);

		if (!RTC::Codecs::IsValidTypeForCodec(this->type, mediaCodec->mimeType))
		{
			base::uv::throwError(mediaCodec->mimeType.ToString() + " codec not supported for svc" );
		}

		RTC::Codecs::EncodingContext::Params params;

		params.spatialLayers  = encoding.spatialLayers;
		params.temporalLayers = encoding.temporalLayers;
		params.ksvc           = encoding.ksvc;

		this->encodingContext.reset(RTC::Codecs::GetEncodingContext(mediaCodec->mimeType, params));

		assertm(this->encodingContext, "no encoding context for this codec");

		// Create RtpStreamSend instance for sending a single stream to the remote.
		CreateRtpStream();
	}

	SvcConsumer::~SvcConsumer()
	{
		MS_TRACE();

		delete this->rtpStream;
	}

	void SvcConsumer::FillJson(json& jsonObject) const
	{
		MS_TRACE();

		// Call the parent method.
		RTC::Consumer::FillJson(jsonObject);

		// Add rtpStream.
		this->rtpStream->FillJson(jsonObject["rtpStream"]);

		// Add preferredSpatialLayer.
		jsonObject["preferredSpatialLayer"] = this->preferredSpatialLayer;

		// Add targetSpatialLayer.
		jsonObject["targetSpatialLayer"] = this->encodingContext->GetTargetSpatialLayer();

		// Add currentSpatialLayer.
		jsonObject["currentSpatialLayer"] = this->encodingContext->GetCurrentSpatialLayer();

		// Add preferredTemporalLayer.
		jsonObject["preferredTemporalLayer"] = this->preferredTemporalLayer;

		// Add targetTemporalLayer.
		jsonObject["targetTemporalLayer"] = this->encodingContext->GetTargetTemporalLayer();

		// Add currentTemporalLayer.
		jsonObject["currentTemporalLayer"] = this->encodingContext->GetCurrentTemporalLayer();
	}

	void SvcConsumer::FillJsonStats(json& jsonArray) const
	{
		MS_TRACE();

		// Add stats of our send stream.
		jsonArray.emplace_back(json::value_t::object);
		this->rtpStream->FillJsonStats(jsonArray[0]);

		// Add stats of our recv stream.
		if (this->producerRtpStream)
		{
			jsonArray.emplace_back(json::value_t::object);
			this->producerRtpStream->FillJsonStats(jsonArray[1]);
		}
	}

	void SvcConsumer::FillJsonScore(json& jsonObject) const
	{
		MS_TRACE();

		jsonObject["score"] = this->rtpStream->GetScore();

		if (this->producerRtpStream)
			jsonObject["producerScore"] = this->producerRtpStream->GetScore();
		else
			jsonObject["producerScore"] = 0;
	}

	void SvcConsumer::HandleRequest(Channel::Request* request)
	{
		MS_TRACE();

		switch (request->methodId)
		{
			case Channel::Request::MethodId::CONSUMER_REQUEST_KEY_FRAME:
			{
				if (IsActive())
					RequestKeyFrame();

				request->Accept();

				break;
			}

			case Channel::Request::MethodId::CONSUMER_SET_PREFERRED_LAYERS:
			{
				auto previousPreferredSpatialLayer  = this->preferredSpatialLayer;
				auto previousPreferredTemporalLayer = this->preferredTemporalLayer;

				auto jsonSpatialLayerIt  = request->data.find("spatialLayer");
				auto jsonTemporalLayerIt = request->data.find("temporalLayer");

				// Spatial layer.
				
				if (
					jsonSpatialLayerIt == request->data.end() ||
					!Utils::Json::IsPositiveInteger(*jsonSpatialLayerIt)
				)
				
				{
					base::uv::throwError("missing spatialLayer");
				}

				this->preferredSpatialLayer = jsonSpatialLayerIt->get<int16_t>();

				if (this->preferredSpatialLayer > this->rtpStream->GetSpatialLayers() - 1)
					this->preferredSpatialLayer = this->rtpStream->GetSpatialLayers() - 1;

				// preferredTemporaLayer is optional.
				
				if (
					jsonTemporalLayerIt != request->data.end() &&
					Utils::Json::IsPositiveInteger(*jsonTemporalLayerIt)
				)
				
				{
					this->preferredTemporalLayer = jsonTemporalLayerIt->get<int16_t>();

					if (this->preferredTemporalLayer > this->rtpStream->GetTemporalLayers() - 1)
						this->preferredTemporalLayer = this->rtpStream->GetTemporalLayers() - 1;
				}
				else
				{
					this->preferredTemporalLayer = this->rtpStream->GetTemporalLayers() - 1;
				}

				MS_DEBUG_DEV(
				  "preferred layers changed [spatial:%" PRIi16 ", temporal:%" PRIi16 ", consumerId:%s]",
				  this->preferredSpatialLayer,
				  this->preferredTemporalLayer,
				  this->id.c_str());

				json data = json::object();

				data["spatialLayer"]  = this->preferredSpatialLayer;
				data["temporalLayer"] = this->preferredTemporalLayer;

				request->Accept(data);

				
				if (
					IsActive() &&
					(
						this->preferredSpatialLayer != previousPreferredSpatialLayer ||
						this->preferredTemporalLayer != previousPreferredTemporalLayer
					)
				)
				
				{
					MayChangeLayers(/*force*/ true);
				}

				break;
			}

			default:
			{
				// Pass it to the parent class.
				RTC::Consumer::HandleRequest(request);
			}
		}
	}

	void SvcConsumer::ProducerRtpStream(RTC::RtpStream* rtpStream, uint32_t /*mappedSsrc*/)
	{
		MS_TRACE();

		this->producerRtpStream = rtpStream;

		// Emit the score event.
		EmitScore();
	}

	void SvcConsumer::ProducerNewRtpStream(RTC::RtpStream* rtpStream, uint32_t /*mappedSsrc*/)
	{
		MS_TRACE();

		this->producerRtpStream = rtpStream;

		// Request a key frame.
		RequestKeyFrame();

		// Emit the score event.
		EmitScore();

		if (IsActive())
			MayChangeLayers();
	}

	void SvcConsumer::ProducerRtpStreamScore(
	  RTC::RtpStream* /*rtpStream*/, uint8_t score, uint8_t previousScore)
	{
		MS_TRACE();

		// Emit score event.
		EmitScore();

		if (RTC::Consumer::IsActive())
		{
			// Just check target layers if the stream has died or reborned.
			
			if (
				!this->externallyManagedBitrate ||
				(score == 0u || previousScore == 0u)
			)
			
			{
				MayChangeLayers();
			}
		}
	}

	void SvcConsumer::ProducerRtcpSenderReport(RTC::RtpStream* /*rtpStream*/, bool /*first*/)
	{
		MS_TRACE();

		// Do nothing.
	}

	uint8_t SvcConsumer::GetBitratePriority() const
	{
		MS_TRACE();

		assertm(this->externallyManagedBitrate, "bitrate is not externally managed");

		if (!IsActive())
			return 0u;

		return this->priority;
	}

	uint32_t SvcConsumer::IncreaseLayer(uint32_t bitrate, bool considerLoss)
	{
		MS_TRACE();

		assertm(this->externallyManagedBitrate, "bitrate is not externally managed");
		assertm(IsActive(), "should be active");

		if (this->producerRtpStream->GetScore() == 0u)
			return 0u;

		// If already in the preferred layers, do nothing.
		
		if (
			this->provisionalTargetSpatialLayer == this->preferredSpatialLayer &&
			this->provisionalTargetTemporalLayer == this->preferredTemporalLayer
		)
		
		{
			return 0u;
		}

		uint32_t virtualBitrate;

		if (considerLoss)
		{
			// Calculate virtual available bitrate based on given bitrate and our
			// packet lost.
			auto lossPercentage = this->rtpStream->GetLossPercentage();

			if (lossPercentage < 2)
				virtualBitrate = 1.08 * bitrate;
			else if (lossPercentage > 10)
				virtualBitrate = (1 - 0.5 * (lossPercentage / 100)) * bitrate;
			else
				virtualBitrate = bitrate;
		}
		else
		{
			virtualBitrate = bitrate;
		}

		uint32_t requiredBitrate{ 0u };
		int16_t spatialLayer{ 0 };
		int16_t temporalLayer{ 0 };
		auto nowMs = base::Application::GetTimeMs();

		for (; spatialLayer < this->producerRtpStream->GetSpatialLayers(); ++spatialLayer)
		{
			// If this is higher than current spatial layer and we moved to to current spatial
			// layer due to BWE limitations, check how much it has elapsed since then.
			if (nowMs - this->lastBweDowngradeAtMs < BweDowngradeConservativeMs)
			{
				if (this->provisionalTargetSpatialLayer > -1 && spatialLayer > this->encodingContext->GetCurrentSpatialLayer())
				{
					MS_DEBUG_DEV(
					  "avoid upgrading to spatial layer %" PRIi16 " due to recent BWE downgrade", spatialLayer);

					goto done;
				}
			}

			// Ignore spatial layers lower than the one we already have.
			if (spatialLayer < this->provisionalTargetSpatialLayer)
				continue;

			temporalLayer = 0;

			// Check bitrate of every temporal layer.
			for (; temporalLayer < this->producerRtpStream->GetTemporalLayers(); ++temporalLayer)
			{
				// Ignore temporal layers lower than the one we already have (taking into account
				// the spatial layer too).
				
				if (
					spatialLayer == this->provisionalTargetSpatialLayer &&
					temporalLayer <= this->provisionalTargetTemporalLayer
				)
				
				{
					continue;
				}

				requiredBitrate =
				  this->producerRtpStream->GetLayerBitrate(nowMs, spatialLayer, temporalLayer);

				MS_DEBUG_DEV(
				  "testing layers %" PRIi16 ":%" PRIi16 " [virtual bitrate:%" PRIu32
				  ", required bitrate:%" PRIu32 "]",
				  spatialLayer,
				  temporalLayer,
				  virtualBitrate,
				  requiredBitrate);

				// If active layer, end iterations here. Otherwise move to next spatial layer.
				if (requiredBitrate)
					goto done;
				else
					break;
			}

			// If this is the preferred or higher spatial layer, take it and exit.
			if (spatialLayer >= this->preferredSpatialLayer)
				break;
		}

	done:

		// No higher active layers found.
		if (!requiredBitrate)
			return 0u;

		// No luck.
		if (requiredBitrate > virtualBitrate)
			return 0u;

		// Set provisional layers.
		this->provisionalTargetSpatialLayer  = spatialLayer;
		this->provisionalTargetTemporalLayer = temporalLayer;

		MS_DEBUG_DEV(
		  "upgrading to layers %" PRIi16 ":%" PRIi16 " [virtual bitrate:%" PRIu32
		  ", required bitrate:%" PRIu32 "]",
		  this->provisionalTargetSpatialLayer,
		  this->provisionalTargetTemporalLayer,
		  virtualBitrate,
		  requiredBitrate);

		if (requiredBitrate <= bitrate)
			return requiredBitrate;
		else if (requiredBitrate <= virtualBitrate)
			return bitrate;
		else
			return requiredBitrate; // NOTE: This cannot happen.
	}

	void SvcConsumer::ApplyLayers()
	{
		MS_TRACE();

		assertm(this->externallyManagedBitrate, "bitrate is not externally managed");
		assertm(IsActive(), "should be active");

		auto provisionalTargetSpatialLayer  = this->provisionalTargetSpatialLayer;
		auto provisionalTargetTemporalLayer = this->provisionalTargetTemporalLayer;

		// Reset provisional target layers.
		this->provisionalTargetSpatialLayer  = -1;
		this->provisionalTargetTemporalLayer = -1;

		if (!IsActive())
			return;

		
		if (
			provisionalTargetSpatialLayer != this->encodingContext->GetTargetSpatialLayer() ||
			provisionalTargetTemporalLayer != this->encodingContext->GetTargetTemporalLayer()
		)
		
		{
			UpdateTargetLayers(provisionalTargetSpatialLayer, provisionalTargetTemporalLayer);

			// If this looks like a spatial layer downgrade due to BWE limitations, set member.
			
			if (
				this->rtpStream->GetActiveMs() > BweDowngradeMinActiveMs &&
				this->encodingContext->GetTargetSpatialLayer() < this->encodingContext->GetCurrentSpatialLayer() &&
				this->encodingContext->GetCurrentSpatialLayer() <= this->preferredSpatialLayer
			)
			
			{
				MS_DEBUG_DEV(
				  "possible target spatial layer downgrade (from %" PRIi16 " to %" PRIi16
				  ") due to BWE limitation",
				  this->encodingContext->GetCurrentSpatialLayer(),
				  this->encodingContext->GetTargetSpatialLayer());

				this->lastBweDowngradeAtMs = base::Application::GetTimeMs();
			}
		}
	}

	uint32_t SvcConsumer::GetDesiredBitrate() const
	{
		MS_TRACE();

		assertm(this->externallyManagedBitrate, "bitrate is not externally managed");

		if (!IsActive())
			return 0u;

		int16_t desiredSpatialLayer{ -1 };
		int16_t desiredTemporalLayer{ -1 };
		uint32_t desiredBitrate{ 0u };
		auto nowMs = base::Application::GetTimeMs();
		int16_t spatialLayer{ 0 };

		for (; spatialLayer < this->producerRtpStream->GetSpatialLayers(); ++spatialLayer)
		{
			int16_t temporalLayer{ 0 };

			// Check bitrate of every temporal layer.
			for (; temporalLayer < this->producerRtpStream->GetTemporalLayers(); ++temporalLayer)
			{
				auto bitrate = this->producerRtpStream->GetBitrate(nowMs, spatialLayer, temporalLayer);

				// If layer is not active move to next spatial layer.
				if (bitrate == 0u)
					break;

				// Set desired target layers and bitrate.
				desiredSpatialLayer  = spatialLayer;
				desiredTemporalLayer = temporalLayer;
				desiredBitrate       = bitrate;
			}
		}

		// No luck.
		if (desiredSpatialLayer == -1)
			return 0u;

		MS_DEBUG_2TAGS(
		  bwe,
		  svc,
		  "[current layers:%" PRIi16 ":%" PRIi16 ", desired layers:%" PRIi16 ":%" PRIi16
		  ", desired bitrate:%" PRIu32 ", consumerId:%s]",
		  this->encodingContext->GetCurrentSpatialLayer(),
		  this->encodingContext->GetCurrentTemporalLayer(),
		  desiredSpatialLayer,
		  desiredTemporalLayer,
		  desiredBitrate,
		  this->id.c_str());

		return desiredBitrate;
	}

	void SvcConsumer::SendRtpPacket(RTC::RtpPacket* packet)
	{
		MS_TRACE();

		if (!IsActive())
			return;

		
		if (
			this->encodingContext->GetTargetSpatialLayer() == -1 ||
			this->encodingContext->GetTargetTemporalLayer() == -1
		)
		
		{
			return;
		}

		auto payloadType = packet->GetPayloadType();

		// NOTE: This may happen if this Consumer supports just some codecs of those
		// in the corresponding Producer.
		if (this->supportedCodecPayloadTypes.find(payloadType) == this->supportedCodecPayloadTypes.end())
		{
			MS_DEBUG_DEV("payload type not supported [payloadType:%" PRIu8 "]", payloadType);

			return;
		}

		// If we need to sync and this is not a key frame, ignore the packet.
		if (this->syncRequired && !packet->IsKeyFrame())
			return;

		// Whether this is the first packet after re-sync.
		bool isSyncPacket = this->syncRequired;

		// Sync sequence number and timestamp if required.
		if (isSyncPacket)
		{
			if (packet->IsKeyFrame())
				MS_DEBUG_TAG(rtp, "sync key frame received");

			this->rtpSeqManager.Sync(packet->GetSequenceNumber() - 1);

			this->syncRequired = false;
		}

		auto previousSpatialLayer  = this->encodingContext->GetCurrentSpatialLayer();
		auto previousTemporalLayer = this->encodingContext->GetCurrentTemporalLayer();

		if (!packet->ProcessPayload(this->encodingContext.get()))
		{
			this->rtpSeqManager.Drop(packet->GetSequenceNumber());

			return;
		}

		
		if (
			previousSpatialLayer != this->encodingContext->GetCurrentSpatialLayer() ||
			previousTemporalLayer != this->encodingContext->GetCurrentTemporalLayer()
		)
		
		{
			// Emit the layersChange event.
			EmitLayersChange();
		}

		// Update RTP seq number and timestamp based on NTP offset.
		uint16_t seq;

		this->rtpSeqManager.Input(packet->GetSequenceNumber(), seq);

		// Save original packet fields.
		auto origSsrc = packet->GetSsrc();
		auto origSeq  = packet->GetSequenceNumber();

		// Rewrite packet.
		packet->SetSsrc(this->rtpParameters.encodings[0].ssrc);
		packet->SetSequenceNumber(seq);

		if (isSyncPacket)
		{
			MS_DEBUG_TAG(
			  rtp,
			  "sending sync packet [ssrc:%" PRIu32 ", seq:%" PRIu16 ", ts:%" PRIu32
			  "] from original [seq:%" PRIu16 "]",
			  packet->GetSsrc(),
			  packet->GetSequenceNumber(),
			  packet->GetTimestamp(),
			  origSeq);
		}

		// Process the packet.
		if (this->rtpStream->ReceivePacket(packet))
		{
			// Send the packet.
			this->listener->OnConsumerSendRtpPacket(this, packet);

			// May emit 'trace' event.
			EmitTraceEventRtpAndKeyFrameTypes(packet);
		}
		else
		{
			MS_WARN_TAG(
			  rtp,
			  "failed to send packet [ssrc:%" PRIu32 ", seq:%" PRIu16 ", ts:%" PRIu32
			  "] from original [ssrc:%" PRIu32 ", seq:%" PRIu16 "]",
			  packet->GetSsrc(),
			  packet->GetSequenceNumber(),
			  packet->GetTimestamp(),
			  origSsrc,
			  origSeq);
		}

		// Restore packet fields.
		packet->SetSsrc(origSsrc);
		packet->SetSequenceNumber(origSeq);

		// Restore the original payload if needed.
		packet->RestorePayload();
	}

	void SvcConsumer::GetRtcp(
	  RTC::RTCP::CompoundPacket* packet, RTC::RtpStreamSend* rtpStream, uint64_t nowMs)
	{
		MS_TRACE();

		assertm(rtpStream == this->rtpStream, "RTP stream does not match");

		if (static_cast<float>((nowMs - this->lastRtcpSentTime) * 1.15) < this->maxRtcpInterval)
			return;

		auto* report = this->rtpStream->GetRtcpSenderReport(nowMs);

		if (!report)
			return;

		packet->AddSenderReport(report);

		// Build SDES chunk for this sender.
		auto* sdesChunk = this->rtpStream->GetRtcpSdesChunk();

		packet->AddSdesChunk(sdesChunk);

		this->lastRtcpSentTime = nowMs;
	}

	void SvcConsumer::NeedWorstRemoteFractionLost(uint32_t /*mappedSsrc*/, uint8_t& worstRemoteFractionLost)
	{
		MS_TRACE();

		if (!IsActive())
			return;

		auto fractionLost = this->rtpStream->GetFractionLost();

		// If our fraction lost is worse than the given one, update it.
		if (fractionLost > worstRemoteFractionLost)
			worstRemoteFractionLost = fractionLost;
	}

	void SvcConsumer::ReceiveNack(RTC::RTCP::FeedbackRtpNackPacket* nackPacket)
	{
		MS_TRACE();

		if (!IsActive())
			return;

		// May emit 'trace' event.
		EmitTraceEventNackType();

		this->rtpStream->ReceiveNack(nackPacket);
	}

	void SvcConsumer::ReceiveKeyFrameRequest(RTC::RTCP::FeedbackPs::MessageType messageType, uint32_t ssrc)
	{
		MS_TRACE();

		switch (messageType)
		{
			case RTC::RTCP::FeedbackPs::MessageType::PLI:
			{
				EmitTraceEventPliType(ssrc);

				break;
			}

			case RTC::RTCP::FeedbackPs::MessageType::FIR:
			{
				EmitTraceEventFirType(ssrc);

				break;
			}

			default:;
		}

		this->rtpStream->ReceiveKeyFrameRequest(messageType);

		if (IsActive())
			RequestKeyFrame();
	}

	void SvcConsumer::ReceiveRtcpReceiverReport(RTC::RTCP::ReceiverReport* report)
	{
		MS_TRACE();

		this->rtpStream->ReceiveRtcpReceiverReport(report);
	}

	uint32_t SvcConsumer::GetTransmissionRate(uint64_t nowMs)
	{
		MS_TRACE();

		if (!IsActive())
			return 0u;

		return this->rtpStream->GetBitrate(nowMs);
	}

	float SvcConsumer::GetRtt() const
	{
		MS_TRACE();

		return this->rtpStream->GetRtt();
	}

	void SvcConsumer::UserOnTransportConnected()
	{
		MS_TRACE();

		this->syncRequired = true;

		if (IsActive())
			MayChangeLayers();
	}

	void SvcConsumer::UserOnTransportDisconnected()
	{
		MS_TRACE();

		this->lastBweDowngradeAtMs = 0u;

		this->rtpStream->Pause();

		UpdateTargetLayers(-1, -1);
	}

	void SvcConsumer::UserOnPaused()
	{
		MS_TRACE();

		this->lastBweDowngradeAtMs = 0u;

		this->rtpStream->Pause();

		UpdateTargetLayers(-1, -1);

		if (this->externallyManagedBitrate)
			this->listener->OnConsumerNeedZeroBitrate(this);
	}

	void SvcConsumer::UserOnResumed()
	{
		MS_TRACE();

		this->syncRequired = true;

		if (IsActive())
			MayChangeLayers();
	}

	void SvcConsumer::CreateRtpStream()
	{
		MS_TRACE();

		auto& encoding   = this->rtpParameters.encodings[0];
		auto* mediaCodec = this->rtpParameters.GetCodecForEncoding(encoding);

		MS_DEBUG_TAG(
		  rtp, "[ssrc:%" PRIu32 ", payloadType:%" PRIu8 "]", encoding.ssrc, mediaCodec->payloadType);

		// Set stream params.
		RTC::RtpStream::Params params;

		params.ssrc           = encoding.ssrc;
		params.payloadType    = mediaCodec->payloadType;
		params.mimeType       = mediaCodec->mimeType;
		params.clockRate      = mediaCodec->clockRate;
		params.cname          = this->rtpParameters.rtcp.cname;
		params.spatialLayers  = encoding.spatialLayers;
		params.temporalLayers = encoding.temporalLayers;

		// Check in band FEC in codec parameters.
		if (mediaCodec->parameters.HasInteger("useinbandfec") && mediaCodec->parameters.GetInteger("useinbandfec") == 1)
		{
			MS_DEBUG_TAG(rtp, "in band FEC enabled");

			params.useInBandFec = true;
		}

		// Check DTX in codec parameters.
		if (mediaCodec->parameters.HasInteger("usedtx") && mediaCodec->parameters.GetInteger("usedtx") == 1)
		{
			MS_DEBUG_TAG(rtp, "DTX enabled");

			params.useDtx = true;
		}

		// Check DTX in the encoding.
		if (encoding.dtx)
		{
			MS_DEBUG_TAG(rtp, "DTX enabled");

			params.useDtx = true;
		}

		for (auto& fb : mediaCodec->rtcpFeedback)
		{
			if (!params.useNack && fb.type == "nack" && fb.parameter == "")
			{
				MS_DEBUG_2TAGS(rtp, rtcp, "NACK supported");

				params.useNack = true;
			}
			else if (!params.usePli && fb.type == "nack" && fb.parameter == "pli")
			{
				MS_DEBUG_2TAGS(rtp, rtcp, "PLI supported");

				params.usePli = true;
			}
			else if (!params.useFir && fb.type == "ccm" && fb.parameter == "fir")
			{
				MS_DEBUG_2TAGS(rtp, rtcp, "FIR supported");

				params.useFir = true;
			}
		}

		// Create a RtpStreamSend for sending a single media stream.
		size_t bufferSize = params.useNack ? 600u : 0u;

		this->rtpStream = new RTC::RtpStreamSend(this, params, bufferSize);
		this->rtpStreams.push_back(this->rtpStream);

		// If the Consumer is paused, tell the RtpStreamSend.
		if (IsPaused() || IsProducerPaused())
			this->rtpStream->Pause();

		auto* rtxCodec = this->rtpParameters.GetRtxCodecForEncoding(encoding);

		if (rtxCodec && encoding.hasRtx)
			this->rtpStream->SetRtx(rtxCodec->payloadType, encoding.rtx.ssrc);
	}

	void SvcConsumer::RequestKeyFrame()
	{
		MS_TRACE();

		if (this->kind != RTC::Media::Kind::VIDEO)
			return;

		auto mappedSsrc = this->consumableRtpEncodings[0].ssrc;

		this->listener->OnConsumerKeyFrameRequested(this, mappedSsrc);
	}

	void SvcConsumer::MayChangeLayers(bool force)
	{
		MS_TRACE();

		int16_t newTargetSpatialLayer;
		int16_t newTargetTemporalLayer;

		if (RecalculateTargetLayers(newTargetSpatialLayer, newTargetTemporalLayer))
		{
			// If bitrate externally managed, don't bother the transport unless
			// the newTargetSpatialLayer has changed (or force is true).
			// This is because, if bitrate is externally managed, the target temporal
			// layer is managed by the available given bitrate so the transport
			// will let us change it when it considers.
			if (this->externallyManagedBitrate)
			{
				if (newTargetSpatialLayer != this->encodingContext->GetTargetSpatialLayer() || force)
					this->listener->OnConsumerNeedBitrateChange(this);
			}
			else
			{
				UpdateTargetLayers(newTargetSpatialLayer, newTargetTemporalLayer);
			}
		}
	}

	bool SvcConsumer::RecalculateTargetLayers(
	  int16_t& newTargetSpatialLayer, int16_t& newTargetTemporalLayer) const
	{
		MS_TRACE();

		// Start with no layers.
		newTargetSpatialLayer  = -1;
		newTargetTemporalLayer = -1;

		auto nowMs = base::Application::GetTimeMs();
		int16_t spatialLayer{ 0 };

		if (!this->producerRtpStream)
			goto done;

		if (this->producerRtpStream->GetScore() == 0u)
			goto done;

		for (; spatialLayer < this->producerRtpStream->GetSpatialLayers(); ++spatialLayer)
		{
			// If this is higher than current spatial layer and we moved to to current spatial
			// layer due to BWE limitations, check how much it has elapsed since then.
			if (nowMs - this->lastBweDowngradeAtMs < BweDowngradeConservativeMs)
			{
				if (newTargetSpatialLayer > -1 && spatialLayer > this->encodingContext->GetCurrentSpatialLayer())
					continue;
			}

			if (!this->producerRtpStream->GetSpatialLayerBitrate(nowMs, spatialLayer))
				continue;

			newTargetSpatialLayer = spatialLayer;

			// If this is the preferred or higher spatial layer and has bitrate,
			// take it and exit.
			if (spatialLayer >= this->preferredSpatialLayer)
				break;
		}

		if (newTargetSpatialLayer != -1)
		{
			if (newTargetSpatialLayer == this->preferredSpatialLayer)
				newTargetTemporalLayer = this->preferredTemporalLayer;
			else if (newTargetSpatialLayer < this->preferredSpatialLayer)
				newTargetTemporalLayer = this->rtpStream->GetTemporalLayers() - 1;
			else
				newTargetTemporalLayer = 0;
		}

	done:

		// Return true if any target layer changed.
		
		return (
			newTargetSpatialLayer != this->encodingContext->GetTargetSpatialLayer() ||
			newTargetTemporalLayer != this->encodingContext->GetTargetTemporalLayer()
		);
		
	}

	void SvcConsumer::UpdateTargetLayers(int16_t newTargetSpatialLayer, int16_t newTargetTemporalLayer)
	{
		MS_TRACE();

		if (newTargetSpatialLayer == -1)
		{
			// Unset current and target layers.
			this->encodingContext->SetTargetSpatialLayer(-1);
			this->encodingContext->SetCurrentSpatialLayer(-1);
			this->encodingContext->SetTargetTemporalLayer(-1);
			this->encodingContext->SetCurrentTemporalLayer(-1);

			MS_DEBUG_TAG(
			  svc, "target layers changed [spatial:-1, temporal:-1, consumerId:%s]", this->id.c_str());

			EmitLayersChange();

			return;
		}

		this->encodingContext->SetTargetSpatialLayer(newTargetSpatialLayer);
		this->encodingContext->SetTargetTemporalLayer(newTargetTemporalLayer);

		MS_DEBUG_TAG(
		  svc,
		  "target layers changed [spatial:%" PRIi16 ", temporal:%" PRIi16 ", consumerId:%s]",
		  newTargetSpatialLayer,
		  newTargetTemporalLayer,
		  this->id.c_str());

		// Target spatial layer has changed.
		if (newTargetSpatialLayer != this->encodingContext->GetCurrentSpatialLayer())
		{
			// In K-SVC always ask for a keyframe when changing target spatial layer.
			if (this->encodingContext->IsKSvc())
			{
				MS_DEBUG_DEV("K-SVC: requesting keyframe to target spatial change");

				RequestKeyFrame();
			}
			// In full SVC just ask for a keyframe when upgrading target spatial layer.
			// NOTE: This is because nobody implements RTCP LRR yet.
			else if (newTargetSpatialLayer > this->encodingContext->GetCurrentSpatialLayer())
			{
				MS_DEBUG_DEV("full SVC: requesting keyframe to target spatial upgrade");

				RequestKeyFrame();
			}
		}
	}

	inline void SvcConsumer::EmitScore() const
	{
		MS_TRACE();

		json data = json::object();

		FillJsonScore(data);

		Channel::Notifier::Emit(this->id, "score", data);
	}

	inline void SvcConsumer::EmitLayersChange() const
	{
		MS_TRACE();

		MS_DEBUG_DEV(
		  "current layers changed to [spatial:%" PRIi16 ", temporal:%" PRIi16 ", consumerId:%s]",
		  this->encodingContext->GetCurrentSpatialLayer(),
		  this->encodingContext->GetCurrentTemporalLayer(),
		  this->id.c_str());

		json data = json::object();

		if (this->encodingContext->GetCurrentSpatialLayer() >= 0)
		{
			data["spatialLayer"]  = this->encodingContext->GetCurrentSpatialLayer();
			data["temporalLayer"] = this->encodingContext->GetCurrentTemporalLayer();
		}
		else
		{
			data = nullptr;
		}

		Channel::Notifier::Emit(this->id, "layerschange", data);
	}

	inline void SvcConsumer::OnRtpStreamScore(
	  RTC::RtpStream* /*rtpStream*/, uint8_t /*score*/, uint8_t /*previousScore*/)
	{
		MS_TRACE();

		// Emit the score event.
		EmitScore();

		if (IsActive())
		{
			// Just check target layers if our bitrate is not externally managed.
			// NOTE: For now this is a bit useless since, when locally managed, we do
			// not check the Consumer score at all.
			if (!this->externallyManagedBitrate)
				MayChangeLayers();
		}
	}

	inline void SvcConsumer::OnRtpStreamRetransmitRtpPacket(
	  RTC::RtpStreamSend* /*rtpStream*/, RTC::RtpPacket* packet)
	{
		MS_TRACE();

		this->listener->OnConsumerRetransmitRtpPacket(this, packet);

		// May emit 'trace' event.
		EmitTraceEventRtpAndKeyFrameTypes(packet, this->rtpStream->HasRtx());
	}
} // namespace RTC
