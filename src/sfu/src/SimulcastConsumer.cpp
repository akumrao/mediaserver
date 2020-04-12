#define MS_CLASS "RTC::SimulcastConsumer"
// #define MS_LOG_DEV_LEVEL 3

#include "RTC/SimulcastConsumer.h"
#include "base/application.h"
#include "LoggerTag.h"
#include "base/error.h"
#include "Utils.h"
#include "Channel/Notifier.h"
#include "RTC/Codecs/Codecs.h"

namespace RTC
{
	/* Static. */

	static constexpr uint8_t StreamGoodScore{ 5u };
	static constexpr uint64_t StreamMinActiveMs{ 2000u };           // In ms.
	static constexpr uint64_t BweDowngradeConservativeMs{ 10000u }; // In ms.
	static constexpr uint64_t BweDowngradeMinActiveMs{ 8000u };     // In ms.

	/* Instance methods. */

	SimulcastConsumer::SimulcastConsumer(
	  const std::string& id, RTC::Consumer::Listener* listener, json& data)
	  : RTC::Consumer::Consumer(id, listener, data, RTC::RtpParameters::Type::SIMULCAST)
	{
		

		// Ensure there are N > 1 encodings.
		if (this->consumableRtpEncodings.size() <= 1u)
			base::uv::throwError("invalid consumableRtpEncodings with size <= 1");

		auto& encoding = this->rtpParameters.encodings[0];

		// Ensure there are as many spatial layers as encodings.
		if (encoding.spatialLayers != this->consumableRtpEncodings.size())
		{
			base::uv::throwError("encoding.spatialLayers does not match number of consumableRtpEncodings");
		}

		auto jsonPreferredLayersIt = data.find("preferredLayers");

		// Fill mapMappedSsrcSpatialLayer.
		for (size_t idx{ 0u }; idx < this->consumableRtpEncodings.size(); ++idx)
		{
			auto& encoding = this->consumableRtpEncodings[idx];

			this->mapMappedSsrcSpatialLayer[encoding.ssrc] = static_cast<int16_t>(idx);
		}

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

		// Reserve space for the Producer RTP streams by filling all the possible
		// entries with nullptr.
		this->producerRtpStreams.insert(
		  this->producerRtpStreams.begin(), this->consumableRtpEncodings.size(), nullptr);

		// Create the encoding context.
		auto* mediaCodec = this->rtpParameters.GetCodecForEncoding(encoding);

		if (!RTC::Codecs::IsValidTypeForCodec(this->type, mediaCodec->mimeType))
		{
			base::uv::throwError(  mediaCodec->mimeType.ToString() + " codec not supported for simulcast.");
		}

		RTC::Codecs::EncodingContext::Params params;

		params.spatialLayers  = encoding.spatialLayers;
		params.temporalLayers = encoding.temporalLayers;

		this->encodingContext.reset(RTC::Codecs::GetEncodingContext(mediaCodec->mimeType, params));

		assertm(this->encodingContext, "no encoding context for this codec");

		// Create RtpStreamSend instance for sending a single stream to the remote.
		CreateRtpStream();
	}

	SimulcastConsumer::~SimulcastConsumer()
	{
		

		delete this->rtpStream;
	}

	void SimulcastConsumer::FillJson(json& jsonObject) const
	{
		

		// Call the parent method.
		RTC::Consumer::FillJson(jsonObject);

		// Add rtpStream.
		this->rtpStream->FillJson(jsonObject["rtpStream"]);

		// Add preferredSpatialLayer.
		jsonObject["preferredSpatialLayer"] = this->preferredSpatialLayer;

		// Add targetSpatialLayer.
		jsonObject["targetSpatialLayer"] = this->targetSpatialLayer;

		// Add currentSpatialLayer.
		jsonObject["currentSpatialLayer"] = this->currentSpatialLayer;

		// Add preferredTemporalLayer.
		jsonObject["preferredTemporalLayer"] = this->preferredTemporalLayer;

		// Add targetTemporalLayer.
		jsonObject["targetTemporalLayer"] = this->targetTemporalLayer;

		// Add currentTemporalLayer.
		jsonObject["currentTemporalLayer"] = this->encodingContext->GetCurrentTemporalLayer();
	}

	void SimulcastConsumer::FillJsonStats(json& jsonArray) const
	{
		

		// Add stats of our send stream.
		jsonArray.emplace_back(json::value_t::object);
		this->rtpStream->FillJsonStats(jsonArray[0]);

		// Add stats of our recv stream.
		auto* producerCurrentRtpStream = GetProducerCurrentRtpStream();

		if (producerCurrentRtpStream)
		{
			jsonArray.emplace_back(json::value_t::object);
			producerCurrentRtpStream->FillJsonStats(jsonArray[1]);
		}
	}

	void SimulcastConsumer::FillJsonScore(json& jsonObject) const
	{
		

		auto* producerCurrentRtpStream = GetProducerCurrentRtpStream();

		jsonObject["score"] = this->rtpStream->GetScore();

		if (producerCurrentRtpStream)
			jsonObject["producerScore"] = producerCurrentRtpStream->GetScore();
		else
			jsonObject["producerScore"] = 0;
	}

	void SimulcastConsumer::HandleRequest(Channel::Request* request)
	{
		

		switch (request->methodId)
		{
			case Channel::Request::MethodId::CONSUMER_REQUEST_KEY_FRAME:
			{
				if (IsActive())
					RequestKeyFrames();

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

	void SimulcastConsumer::ProducerRtpStream(RTC::RtpStream* rtpStream, uint32_t mappedSsrc)
	{
		

		auto it = this->mapMappedSsrcSpatialLayer.find(mappedSsrc);

		assertm(it != this->mapMappedSsrcSpatialLayer.end(), "unknown mappedSsrc");

		int16_t spatialLayer = it->second;

		this->producerRtpStreams[spatialLayer] = rtpStream;
	}

	void SimulcastConsumer::ProducerNewRtpStream(RTC::RtpStream* rtpStream, uint32_t mappedSsrc)
	{
		

		auto it = this->mapMappedSsrcSpatialLayer.find(mappedSsrc);

		assertm(it != this->mapMappedSsrcSpatialLayer.end(), "unknown mappedSsrc");

		int16_t spatialLayer = it->second;

		this->producerRtpStreams[spatialLayer] = rtpStream;

		if (IsActive())
			MayChangeLayers();
	}

	void SimulcastConsumer::ProducerRtpStreamScore(
	  RTC::RtpStream* rtpStream, uint8_t score, uint8_t previousScore)
	{
		

		// Emit score event only if the stream whose score changed is the current one.
		if (rtpStream == GetProducerCurrentRtpStream())
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

	void SimulcastConsumer::ProducerRtcpSenderReport(RTC::RtpStream* rtpStream, bool first)
	{
		

		// Just interested if this is the first Sender Report for a RTP stream.
		if (!first)
			return;

		MS_DEBUG_TAG(simulcast, "first SenderReport [ssrc:%" PRIu32 "]", rtpStream->GetSsrc());

		// If our current selected RTP stream does not yet have SR, do nothing since
		// we know we won't be able to switch.
		auto* producerCurrentRtpStream = GetProducerCurrentRtpStream();

		if (!producerCurrentRtpStream || !producerCurrentRtpStream->GetSenderReportNtpMs())
			return;

		if (IsActive())
			MayChangeLayers();
	}

	uint8_t SimulcastConsumer::GetBitratePriority() const
	{
		

		assertm(this->externallyManagedBitrate, "bitrate is not externally managed");

		if (!IsActive())
			return 0u;

		return this->priority;
	}

	uint32_t SimulcastConsumer::IncreaseLayer(uint32_t bitrate, bool considerLoss)
	{
		

		assertm(this->externallyManagedBitrate, "bitrate is not externally managed");
		assertm(IsActive(), "should be active");

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

		for (size_t sIdx{ 0u }; sIdx < this->producerRtpStreams.size(); ++sIdx)
		{
			spatialLayer = static_cast<int16_t>(sIdx);

			// If this is higher than current spatial layer and we moved to to current spatial
			// layer due to BWE limitations, check how much it has elapsed since then.
			if (nowMs - this->lastBweDowngradeAtMs < BweDowngradeConservativeMs)
			{
				if (this->provisionalTargetSpatialLayer > -1 && spatialLayer > this->currentSpatialLayer)
				{
					MS_DEBUG_DEV(
					  "avoid upgrading to spatial layer %" PRIi16 " due to recent BWE downgrade", spatialLayer);

					goto done;
				}
			}

			// Ignore spatial layers lower than the one we already have.
			if (spatialLayer < this->provisionalTargetSpatialLayer)
				continue;

			// This can be null.
			auto* producerRtpStream = this->producerRtpStreams.at(spatialLayer);

			// Producer stream does not exist or it's not good. Ignore.
			if (!producerRtpStream || producerRtpStream->GetScore() < StreamGoodScore)
				continue;

			// If the stream has not been active time enough and we have an active one
			// already, move to the next spatial layer.
			
			if (
				spatialLayer != this->provisionalTargetSpatialLayer &&
				this->provisionalTargetSpatialLayer != -1 &&
				producerRtpStream->GetActiveMs() < StreamMinActiveMs
			)
			
			{
				continue;
			}

			// We may not yet switch to this spatial layer.
			if (!CanSwitchToSpatialLayer(spatialLayer))
				continue;

			temporalLayer = 0;

			// Check bitrate of every temporal layer.
			for (; temporalLayer < producerRtpStream->GetTemporalLayers(); ++temporalLayer)
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

				requiredBitrate = producerRtpStream->GetLayerBitrate(nowMs, 0, temporalLayer);

				// This is simulcast so we must substract the bitrate of the current temporal
				// spatial layer if this is the temporal layer 0 of a higher spatial layer.
				//
				
				if (
					temporalLayer == 0 &&
					this->provisionalTargetSpatialLayer > -1 &&
					spatialLayer > this->provisionalTargetSpatialLayer
				)
				
				{
					auto* provisionalProducerRtpStream =
					  this->producerRtpStreams.at(this->provisionalTargetSpatialLayer);
					auto provisionalRequiredBitrate = provisionalProducerRtpStream->GetLayerBitrate(
					  nowMs, 0, this->provisionalTargetTemporalLayer);

					if (requiredBitrate > provisionalRequiredBitrate)
						requiredBitrate -= provisionalRequiredBitrate;
					else
						requiredBitrate = 1u; // Don't set 0 since it would be ignored.
				}

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
		  "setting provisional layers to %" PRIi16 ":%" PRIi16 " [virtual bitrate:%" PRIu32
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

	void SimulcastConsumer::ApplyLayers()
	{
		

		assertm(this->externallyManagedBitrate, "bitrate is not externally managed");
		assertm(IsActive(), "should be active");

		auto provisionalTargetSpatialLayer  = this->provisionalTargetSpatialLayer;
		auto provisionalTargetTemporalLayer = this->provisionalTargetTemporalLayer;

		// Reset provisional target layers.
		this->provisionalTargetSpatialLayer  = -1;
		this->provisionalTargetTemporalLayer = -1;

		
		if (
			provisionalTargetSpatialLayer != this->targetSpatialLayer ||
			provisionalTargetTemporalLayer != this->targetTemporalLayer
		)
		
		{
			UpdateTargetLayers(provisionalTargetSpatialLayer, provisionalTargetTemporalLayer);

			// If this looks like a spatial layer downgrade due to BWE limitations, set member.
			
			if (
				this->rtpStream->GetActiveMs() > BweDowngradeMinActiveMs &&
				this->targetSpatialLayer < this->currentSpatialLayer &&
				this->currentSpatialLayer <= this->preferredSpatialLayer
			)
			
			{
				MS_DEBUG_DEV(
				  "possible target spatial layer downgrade (from %" PRIi16 " to %" PRIi16
				  ") due to BWE limitation",
				  this->currentSpatialLayer,
				  this->targetSpatialLayer);

				this->lastBweDowngradeAtMs = base::Application::GetTimeMs();
			}
		}
	}

	uint32_t SimulcastConsumer::GetDesiredBitrate() const
	{
		

		assertm(this->externallyManagedBitrate, "bitrate is not externally managed");

		if (!IsActive())
			return 0u;

		int16_t desiredSpatialLayer{ -1 };
		int16_t desiredTemporalLayer{ -1 };
		uint32_t desiredBitrate{ 0u };
		uint8_t maxProducerScore{ 0u };
		auto nowMs = base::Application::GetTimeMs();

		for (size_t sIdx{ 0u }; sIdx < this->producerRtpStreams.size(); ++sIdx)
		{
			auto spatialLayer       = static_cast<int16_t>(sIdx);
			auto* producerRtpStream = this->producerRtpStreams.at(sIdx);
			auto producerScore      = producerRtpStream ? producerRtpStream->GetScore() : 0u;

			// Ignore spatial layers for non existing producer streams or for those
			// with score 0.
			if (producerScore == 0u)
				continue;

			// If the stream has not been active time enough and we have an active one
			// already, move to the next spatial layer.
			if (desiredBitrate > 0 && producerRtpStream->GetActiveMs() < StreamMinActiveMs)
				continue;

			// We may not yet switch to this spatial layer.
			if (!CanSwitchToSpatialLayer(spatialLayer))
				continue;

			// If the stream score is worse than the best seen and not good enough, ignore
			// this stream.
			if (producerScore < maxProducerScore && producerScore < StreamGoodScore)
				continue;

			maxProducerScore = producerScore;

			int16_t temporalLayer{ 0 };

			// Check bitrate of every temporal layer.
			for (; temporalLayer < producerRtpStream->GetTemporalLayers(); ++temporalLayer)
			{
				auto bitrate = producerRtpStream->GetBitrate(nowMs, 0u, temporalLayer);

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
		  simulcast,
		  "[current layers:%" PRIi16 ":%" PRIi16 ", desired layers:%" PRIi16 ":%" PRIi16
		  ", desired bitrate:%" PRIu32 ", consumerId:%s]",
		  this->currentSpatialLayer,
		  this->encodingContext->GetCurrentTemporalLayer(),
		  desiredSpatialLayer,
		  desiredTemporalLayer,
		  desiredBitrate,
		  this->id.c_str());

		return desiredBitrate;
	}

	void SimulcastConsumer::SendRtpPacket(RTC::RtpPacket* packet)
	{
		

		if (!IsActive())
			return;

		if (this->targetTemporalLayer == -1)
			return;

		auto payloadType = packet->GetPayloadType();

		// NOTE: This may happen if this Consumer supports just some codecs of those
		// in the corresponding Producer.
		if (this->supportedCodecPayloadTypes.find(payloadType) == this->supportedCodecPayloadTypes.end())
		{
			MS_DEBUG_DEV("payload type not supported [payloadType:%" PRIu8 "]", payloadType);

			return;
		}

		auto spatialLayer = this->mapMappedSsrcSpatialLayer.at(packet->GetSsrc());
		bool shouldSwitchCurrentSpatialLayer{ false };

		// Check whether this is the packet we are waiting for in order to update
		// the current spatial layer.
		if (this->currentSpatialLayer != this->targetSpatialLayer && spatialLayer == this->targetSpatialLayer)
		{
			// Ignore if not a key frame.
			if (!packet->IsKeyFrame())
				return;

			shouldSwitchCurrentSpatialLayer = true;

			// Need to resync the stream.
			this->syncRequired = true;
		}
		// If the packet belongs to different spatial layer than the one being sent,
		// drop it.
		else if (spatialLayer != this->currentSpatialLayer)
		{
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

			uint32_t tsOffset{ 0u };

			// Sync our RTP stream's RTP timestamp.
			if (spatialLayer == this->tsReferenceSpatialLayer)
			{
				tsOffset = 0u;
			}
			// If this is not the RTP stream we use as TS reference, do NTP based RTP TS synchronization.
			else
			{
				auto* producerTsReferenceRtpStream = GetProducerTsReferenceRtpStream();
				auto* producerTargetRtpStream      = GetProducerTargetRtpStream();

				// NOTE: If we are here is because we have Sender Reports for both the
				// TS reference stream and the target one.
				assertm(
				  producerTsReferenceRtpStream->GetSenderReportNtpMs(),
				  "no Sender Report for TS reference RTP stream");
				assertm(
				  producerTargetRtpStream->GetSenderReportNtpMs(), "no Sender Report for current RTP stream");

				// Calculate NTP and TS stuff.
				auto ntpMs1 = producerTsReferenceRtpStream->GetSenderReportNtpMs();
				auto ts1    = producerTsReferenceRtpStream->GetSenderReportTs();
				auto ntpMs2 = producerTargetRtpStream->GetSenderReportNtpMs();
				auto ts2    = producerTargetRtpStream->GetSenderReportTs();
				int64_t diffMs;

				if (ntpMs2 >= ntpMs1)
					diffMs = ntpMs2 - ntpMs1;
				else
					diffMs = -1 * (ntpMs1 - ntpMs2);

				int64_t diffTs  = diffMs * this->rtpStream->GetClockRate() / 1000;
				uint32_t newTs2 = ts2 - diffTs;

				// Apply offset. This is the difference that later must be removed from the
				// sending RTP packet.
				tsOffset = newTs2 - ts1;
			}

			// When switching to a new stream it may happen that the timestamp of this
			// key frame is lower than the highest timestamp sent to the remote endpoint.
			// If so, apply an extra offset to "fix" it for the whole live of this selected
			// Producer stream.
			//
			
			if (
				shouldSwitchCurrentSpatialLayer &&
				(packet->GetTimestamp() - tsOffset <= this->rtpStream->GetMaxPacketTs())
			)
			
			{
				// Max delay in ms we allow for the stream when switching.
				// https://en.wikipedia.org/wiki/Audio-to-video_synchronization#Recommendations
				static const uint32_t MaxExtraOffsetMs{ 75u };

				int64_t maxTsExtraOffset = MaxExtraOffsetMs * this->rtpStream->GetClockRate() / 1000;
				uint32_t tsExtraOffset =
				  this->rtpStream->GetMaxPacketTs() - packet->GetTimestamp() + tsOffset;

				// NOTE: Don't ask for a key frame if already done.
				if (this->keyFrameForTsOffsetRequested)
				{
					// Give up and use the theoretical offset.
					if (tsExtraOffset > maxTsExtraOffset)
					{
						MS_WARN_TAG(
						  simulcast,
						  "giving up on proper stream switching after got a requested keyframe for which still too high RTP timestamp extra offset is needed (%" PRIu32
						  ")",
						  tsExtraOffset);

						tsExtraOffset = 1u;
					}
				}
				else if (tsExtraOffset > maxTsExtraOffset)
				{
					MS_WARN_TAG(
					  simulcast,
					  "cannot switch stream due to too high RTP timestamp extra offset needed (%" PRIu32
					  "), requesting keyframe",
					  tsExtraOffset);

					RequestKeyFrameForTargetSpatialLayer();

					this->keyFrameForTsOffsetRequested = true;

					return;
				}
				// It's common that, when switching spatial layer, the resulting TS for the
				// outgoing packet matches the highest seen in the previous stream. Fix it.
				else if (tsExtraOffset == 0u)
				{
					tsExtraOffset = 1u;
				}

				if (tsExtraOffset > 0u)
				{
					MS_DEBUG_TAG(
					  simulcast,
					  "RTP timestamp extra offset generated for stream switching: %" PRIu32,
					  tsExtraOffset);

					// Increase the timestamp offset for the whole life of this Producer stream
					// (until switched to a different one).
					tsOffset -= tsExtraOffset;
				}
			}

			this->tsOffset = tsOffset;

			// Sync our RTP stream's sequence number.
			this->rtpSeqManager.Sync(packet->GetSequenceNumber() - 1);

			this->encodingContext->SyncRequired();

			this->syncRequired                 = false;
			this->keyFrameForTsOffsetRequested = false;
		}

		if (shouldSwitchCurrentSpatialLayer)
		{
			// Update current spatial layer.
			this->currentSpatialLayer = this->targetSpatialLayer;

			// Update target and current temporal layer.
			this->encodingContext->SetTargetTemporalLayer(this->targetTemporalLayer);
			this->encodingContext->SetCurrentTemporalLayer(packet->GetTemporalLayer());

			// Reset the score of our RtpStream to 10.
			this->rtpStream->ResetScore(10u, /*notify*/ false);

			// Emit the layersChange event.
			EmitLayersChange();

			// Emit the score event.
			EmitScore();

			// Rewrite payload if needed.
			packet->ProcessPayload(this->encodingContext.get());
		}
		else
		{
			auto previousTemporalLayer = this->encodingContext->GetCurrentTemporalLayer();

			// Rewrite payload if needed. Drop packet if necessary.
			if (!packet->ProcessPayload(this->encodingContext.get()))
			{
				this->rtpSeqManager.Drop(packet->GetSequenceNumber());

				return;
			}

			if (previousTemporalLayer != this->encodingContext->GetCurrentTemporalLayer())
				EmitLayersChange();
		}

		// Update RTP seq number and timestamp based on NTP offset.
		uint16_t seq;
		uint32_t timestamp = packet->GetTimestamp() - this->tsOffset;

		this->rtpSeqManager.Input(packet->GetSequenceNumber(), seq);

		// Save original packet fields.
		auto origSsrc      = packet->GetSsrc();
		auto origSeq       = packet->GetSequenceNumber();
		auto origTimestamp = packet->GetTimestamp();

		// Rewrite packet.
		packet->SetSsrc(this->rtpParameters.encodings[0].ssrc);
		packet->SetSequenceNumber(seq);
		packet->SetTimestamp(timestamp);

		if (isSyncPacket)
		{
			MS_DEBUG_TAG(
			  rtp,
			  "sending sync packet [ssrc:%" PRIu32 ", seq:%" PRIu16 ", ts:%" PRIu32
			  "] from original [ssrc:%" PRIu32 ", seq:%" PRIu16 ", ts:%" PRIu32 "]",
			  packet->GetSsrc(),
			  packet->GetSequenceNumber(),
			  packet->GetTimestamp(),
			  origSsrc,
			  origSeq,
			  origTimestamp);
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
			  "] from original [ssrc:%" PRIu32 ", seq:%" PRIu16 ", ts:%" PRIu32 "]",
			  packet->GetSsrc(),
			  packet->GetSequenceNumber(),
			  packet->GetTimestamp(),
			  origSsrc,
			  origSeq,
			  origTimestamp);
		}

		// Restore packet fields.
		packet->SetSsrc(origSsrc);
		packet->SetSequenceNumber(origSeq);
		packet->SetTimestamp(origTimestamp);

		// Restore the original payload if needed.
		packet->RestorePayload();
	}

	void SimulcastConsumer::GetRtcp(
	  RTC::RTCP::CompoundPacket* packet, RTC::RtpStreamSend* rtpStream, uint64_t nowMs)
	{
		

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

	void SimulcastConsumer::NeedWorstRemoteFractionLost(
	  uint32_t /*mappedSsrc*/, uint8_t& worstRemoteFractionLost)
	{
		

		if (!IsActive())
			return;

		auto fractionLost = this->rtpStream->GetFractionLost();

		// If our fraction lost is worse than the given one, update it.
		if (fractionLost > worstRemoteFractionLost)
			worstRemoteFractionLost = fractionLost;
	}

	void SimulcastConsumer::ReceiveNack(RTC::RTCP::FeedbackRtpNackPacket* nackPacket)
	{
		

		if (!IsActive())
			return;

		// May emit 'trace' event.
		EmitTraceEventNackType();

		this->rtpStream->ReceiveNack(nackPacket);
	}

	void SimulcastConsumer::ReceiveKeyFrameRequest(
	  RTC::RTCP::FeedbackPs::MessageType messageType, uint32_t ssrc)
	{
		

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
			RequestKeyFrameForCurrentSpatialLayer();
	}

	void SimulcastConsumer::ReceiveRtcpReceiverReport(RTC::RTCP::ReceiverReport* report)
	{
		

		this->rtpStream->ReceiveRtcpReceiverReport(report);
	}

	uint32_t SimulcastConsumer::GetTransmissionRate(uint64_t nowMs)
	{
		

		if (!IsActive())
			return 0u;

		return this->rtpStream->GetBitrate(nowMs);
	}

	float SimulcastConsumer::GetRtt() const
	{
		

		return this->rtpStream->GetRtt();
	}

	void SimulcastConsumer::UserOnTransportConnected()
	{
		

		this->syncRequired                 = true;
		this->keyFrameForTsOffsetRequested = false;

		if (IsActive())
			MayChangeLayers();
	}

	void SimulcastConsumer::UserOnTransportDisconnected()
	{
		

		this->lastBweDowngradeAtMs = 0u;

		this->rtpStream->Pause();

		UpdateTargetLayers(-1, -1);
	}

	void SimulcastConsumer::UserOnPaused()
	{
		

		this->lastBweDowngradeAtMs = 0u;

		this->rtpStream->Pause();

		UpdateTargetLayers(-1, -1);

		if (this->externallyManagedBitrate)
			this->listener->OnConsumerNeedZeroBitrate(this);
	}

	void SimulcastConsumer::UserOnResumed()
	{
		

		this->syncRequired                 = true;
		this->keyFrameForTsOffsetRequested = false;

		if (IsActive())
			MayChangeLayers();
	}

	void SimulcastConsumer::CreateRtpStream()
	{
		

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

	void SimulcastConsumer::RequestKeyFrames()
	{
		

		if (this->kind != RTC::Media::Kind::VIDEO)
			return;

		auto* producerTargetRtpStream  = GetProducerTargetRtpStream();
		auto* producerCurrentRtpStream = GetProducerCurrentRtpStream();

		if (producerTargetRtpStream)
		{
			auto mappedSsrc = this->consumableRtpEncodings[this->targetSpatialLayer].ssrc;

			this->listener->OnConsumerKeyFrameRequested(this, mappedSsrc);
		}

		if (producerCurrentRtpStream && producerCurrentRtpStream != producerTargetRtpStream)
		{
			auto mappedSsrc = this->consumableRtpEncodings[this->currentSpatialLayer].ssrc;

			this->listener->OnConsumerKeyFrameRequested(this, mappedSsrc);
		}
	}

	void SimulcastConsumer::RequestKeyFrameForTargetSpatialLayer()
	{
		

		if (this->kind != RTC::Media::Kind::VIDEO)
			return;

		auto* producerTargetRtpStream = GetProducerTargetRtpStream();

		if (!producerTargetRtpStream)
			return;

		auto mappedSsrc = this->consumableRtpEncodings[this->targetSpatialLayer].ssrc;

		this->listener->OnConsumerKeyFrameRequested(this, mappedSsrc);
	}

	void SimulcastConsumer::RequestKeyFrameForCurrentSpatialLayer()
	{
		

		if (this->kind != RTC::Media::Kind::VIDEO)
			return;

		auto* producerCurrentRtpStream = GetProducerCurrentRtpStream();

		if (!producerCurrentRtpStream)
			return;

		auto mappedSsrc = this->consumableRtpEncodings[this->currentSpatialLayer].ssrc;

		this->listener->OnConsumerKeyFrameRequested(this, mappedSsrc);
	}

	void SimulcastConsumer::MayChangeLayers(bool force)
	{
		

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
				if (newTargetSpatialLayer != this->targetSpatialLayer || force)
					this->listener->OnConsumerNeedBitrateChange(this);
			}
			else
			{
				UpdateTargetLayers(newTargetSpatialLayer, newTargetTemporalLayer);
			}
		}
	}

	bool SimulcastConsumer::RecalculateTargetLayers(
	  int16_t& newTargetSpatialLayer, int16_t& newTargetTemporalLayer) const
	{
		

		// Start with no layers.
		newTargetSpatialLayer  = -1;
		newTargetTemporalLayer = -1;

		uint8_t maxProducerScore{ 0u };
		auto nowMs = base::Application::GetTimeMs();

		for (size_t sIdx{ 0u }; sIdx < this->producerRtpStreams.size(); ++sIdx)
		{
			auto spatialLayer       = static_cast<int16_t>(sIdx);
			auto* producerRtpStream = this->producerRtpStreams.at(sIdx);
			auto producerScore      = producerRtpStream ? producerRtpStream->GetScore() : 0u;

			// If this is higher than current spatial layer and we moved to to current spatial
			// layer due to BWE limitations, check how much it has elapsed since then.
			if (nowMs - this->lastBweDowngradeAtMs < BweDowngradeConservativeMs)
			{
				if (newTargetSpatialLayer > -1 && spatialLayer > this->currentSpatialLayer)
					continue;
			}

			// Ignore spatial layers for non existing Producer streams or for those
			// with score 0.
			if (producerScore == 0u)
				continue;

			// If the stream has not been active time enough and we have an active one
			// already, move to the next spatial layer.
			// NOTE: Require bitrate externally managed for this.
			
			if (
				this->externallyManagedBitrate &&
				newTargetSpatialLayer != -1 &&
				producerRtpStream->GetActiveMs() < StreamMinActiveMs
			)
			
			{
				continue;
			}

			// We may not yet switch to this spatial layer.
			if (!CanSwitchToSpatialLayer(spatialLayer))
				continue;

			// If the stream score is worse than the best seen and not good enough, ignore
			// this stream.
			if (producerScore < maxProducerScore && producerScore < StreamGoodScore)
				continue;

			newTargetSpatialLayer = spatialLayer;
			maxProducerScore      = producerScore;

			// If this is the preferred or higher spatial layer and has good score,
			// take it and exit.
			if (spatialLayer >= this->preferredSpatialLayer && producerScore >= StreamGoodScore)
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

		// Return true if any target layer changed.
		
		return (
			newTargetSpatialLayer != this->targetSpatialLayer ||
			newTargetTemporalLayer != this->targetTemporalLayer
		);
		
	}

	void SimulcastConsumer::UpdateTargetLayers(int16_t newTargetSpatialLayer, int16_t newTargetTemporalLayer)
	{
		

		// If we don't have yet a RTP timestamp reference, set it now.
		if (newTargetSpatialLayer != -1 && this->tsReferenceSpatialLayer == -1)
		{
			MS_DEBUG_TAG(
			  simulcast, "using spatial layer %" PRIi16 " as RTP timestamp reference", newTargetSpatialLayer);

			this->tsReferenceSpatialLayer = newTargetSpatialLayer;
		}

		if (newTargetSpatialLayer == -1)
		{
			// Unset current and target layers.
			this->targetSpatialLayer  = -1;
			this->targetTemporalLayer = -1;
			this->currentSpatialLayer = -1;

			this->encodingContext->SetTargetTemporalLayer(-1);
			this->encodingContext->SetCurrentTemporalLayer(-1);

			MS_DEBUG_TAG(
			  simulcast, "target layers changed [spatial:-1, temporal:-1, consumerId:%s]", this->id.c_str());

			EmitLayersChange();

			return;
		}

		this->targetSpatialLayer  = newTargetSpatialLayer;
		this->targetTemporalLayer = newTargetTemporalLayer;

		// If the new target spatial layer matches the current one, apply the new
		// target temporal layer now.
		if (this->targetSpatialLayer == this->currentSpatialLayer)
			this->encodingContext->SetTargetTemporalLayer(this->targetTemporalLayer);

		MS_DEBUG_TAG(
		  simulcast,
		  "target layers changed [spatial:%" PRIi16 ", temporal:%" PRIi16 ", consumerId:%s]",
		  this->targetSpatialLayer,
		  this->targetTemporalLayer,
		  this->id.c_str());

		// If the target spatial layer is different than the current one, request
		// a key frame.
		if (this->targetSpatialLayer != this->currentSpatialLayer)
			RequestKeyFrameForTargetSpatialLayer();
	}

	inline bool SimulcastConsumer::CanSwitchToSpatialLayer(int16_t spatialLayer) const
	{

		// This method assumes that the caller has verified that there is a valid
		// Producer RtpStream for the given spatial layer.
		assertm(
		  this->producerRtpStreams.at(spatialLayer),  "no Producer RtpStream for the given spatialLayer");

		// We can switch to the given spatial layer if:
		// - we don't have any TS reference spatial layer yet, or
		// - the given spatial layer matches the TS reference spatial layer, or
		// - both , the RTP streams of our TS reference spatial layer and the given
		//   spatial layer, have Sender Report.
		//
		
		return (
			this->tsReferenceSpatialLayer == -1 ||
			spatialLayer == this->tsReferenceSpatialLayer ||
			(
				GetProducerTsReferenceRtpStream()->GetSenderReportNtpMs() &&
				this->producerRtpStreams.at(spatialLayer)->GetSenderReportNtpMs()
			)
		);
		
	}

	inline void SimulcastConsumer::EmitScore() const
	{
		

		json data = json::object();

		FillJsonScore(data);

		Channel::Notifier::Emit(this->id, "score", data);
	}

	inline void SimulcastConsumer::EmitLayersChange() const
	{
		

		MS_DEBUG_DEV(
		  "current layers changed to [spatial:%" PRIi16 ", temporal:%" PRIi16 ", consumerId:%s]",
		  this->currentSpatialLayer,
		  this->encodingContext->GetCurrentTemporalLayer(),
		  this->id.c_str());

		json data = json::object();

		if (this->currentSpatialLayer >= 0)
		{
			data["spatialLayer"]  = this->currentSpatialLayer;
			data["temporalLayer"] = this->encodingContext->GetCurrentTemporalLayer();
		}
		else
		{
			data = nullptr;
		}

		Channel::Notifier::Emit(this->id, "layerschange", data);
	}

	inline RTC::RtpStream* SimulcastConsumer::GetProducerCurrentRtpStream() const
	{
		

		if (this->currentSpatialLayer == -1)
			return nullptr;

		// This may return nullptr.
		return this->producerRtpStreams.at(this->currentSpatialLayer);
	}

	inline RTC::RtpStream* SimulcastConsumer::GetProducerTargetRtpStream() const
	{
		

		if (this->targetSpatialLayer == -1)
			return nullptr;

		// This may return nullptr.
		return this->producerRtpStreams.at(this->targetSpatialLayer);
	}

	inline RTC::RtpStream* SimulcastConsumer::GetProducerTsReferenceRtpStream() const
	{
		

		if (this->tsReferenceSpatialLayer == -1)
			return nullptr;

		// This may return nullptr.
		return this->producerRtpStreams.at(this->tsReferenceSpatialLayer);
	}

	inline void SimulcastConsumer::OnRtpStreamScore(
	  RTC::RtpStream* /*rtpStream*/, uint8_t /*score*/, uint8_t /*previousScore*/)
	{
		

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

	inline void SimulcastConsumer::OnRtpStreamRetransmitRtpPacket(
	  RTC::RtpStreamSend* /*rtpStream*/, RTC::RtpPacket* packet)
	{
		

		this->listener->OnConsumerRetransmitRtpPacket(this, packet);

		// May emit 'trace' event.
		EmitTraceEventRtpAndKeyFrameTypes(packet, this->rtpStream->HasRtx());
	}
} // namespace RTC
