#define MS_CLASS "RTC::PipeConsumer"
// #define MS_LOG_DEV_LEVEL 3

#include "RTC/PipeConsumer.h"
#include "base/application.h"
#include "LoggerTag.h"
#include "base/error.h"
#include "RTC/Codecs/Codecs.h"

namespace RTC
{
	/* Instance methods. */

	PipeConsumer::PipeConsumer(const std::string& id, RTC::Consumer::Listener* listener, json& data)
	  : RTC::Consumer::Consumer(id, listener, data, RTC::RtpParameters::Type::PIPE)
	{
		MS_TRACE();

		// Ensure there are as many encodings as consumable encodings.
		if (this->rtpParameters.encodings.size() != this->consumableRtpEncodings.size())
			base::uv::throwError("number of rtpParameters.encodings and consumableRtpEncodings do not match");

		auto& encoding   = this->rtpParameters.encodings[0];
		auto* mediaCodec = this->rtpParameters.GetCodecForEncoding(encoding);

		this->keyFrameSupported = RTC::Codecs::CanBeKeyFrame(mediaCodec->mimeType);

		// Create RtpStreamSend instances.
		CreateRtpStreams();
	}

	PipeConsumer::~PipeConsumer()
	{
		MS_TRACE();

		for (auto* rtpStream : this->rtpStreams)
		{
			delete rtpStream;
		}
		this->rtpStreams.clear();
		this->mapMappedSsrcRtpStream.clear();
	}

	void PipeConsumer::FillJson(json& jsonObject) const
	{
		MS_TRACE();

		// Call the parent method.
		RTC::Consumer::FillJson(jsonObject);

		// Add rtpStreams.
		jsonObject["rtpStreams"] = json::array();
		auto jsonRtpStreamsIt    = jsonObject.find("rtpStreams");

		for (auto* rtpStream : this->rtpStreams)
		{
			auto& jsonEntry = (*jsonRtpStreamsIt)[jsonRtpStreamsIt->size() - 1];

			jsonRtpStreamsIt->emplace_back(json::value_t::object);
			rtpStream->FillJson(jsonEntry);
		}
	}

	void PipeConsumer::FillJsonStats(json& jsonArray) const
	{
		MS_TRACE();

		// Add stats of our send streams.
		for (auto* rtpStream : this->rtpStreams)
		{
			jsonArray.emplace_back(json::value_t::object);
			rtpStream->FillJsonStats(jsonArray[jsonArray.size() - 1]);
		}
	}

	void PipeConsumer::FillJsonScore(json& jsonObject) const
	{
		MS_TRACE();

		// NOTE: Hardcoded values in PipeTransport.
		jsonObject["score"]         = 10;
		jsonObject["producerScore"] = 10;
	}

	void PipeConsumer::HandleRequest(Channel::Request* request)
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
				// Do nothing.

				request->Accept();

				break;
			}

			default:
			{
				// Pass it to the parent class.
				RTC::Consumer::HandleRequest(request);
			}
		}
	}

	void PipeConsumer::ProducerRtpStream(RTC::RtpStream* /*rtpStream*/, uint32_t /*mappedSsrc*/)
	{
		MS_TRACE();

		// Do nothing.
	}

	void PipeConsumer::ProducerNewRtpStream(RTC::RtpStream* /*rtpStream*/, uint32_t /*mappedSsrc*/)
	{
		MS_TRACE();

		// Do nothing.
	}

	void PipeConsumer::ProducerRtpStreamScore(
	  RTC::RtpStream* /*rtpStream*/, uint8_t /*score*/, uint8_t /*previousScore*/)
	{
		MS_TRACE();

		// Do nothing.
	}

	void PipeConsumer::ProducerRtcpSenderReport(RTC::RtpStream* /*rtpStream*/, bool /*first*/)
	{
		MS_TRACE();

		// Do nothing.
	}

	uint8_t PipeConsumer::GetBitratePriority() const
	{
		MS_TRACE();

		// PipeConsumer does not play the BWE game.
		return 0u;
	}

	uint32_t PipeConsumer::IncreaseLayer(uint32_t /*bitrate*/, bool /*considerLoss*/)
	{
		MS_TRACE();

		// PipeConsumer does not play the BWE game.
		return 0u;
	}

	void PipeConsumer::ApplyLayers()
	{
		MS_TRACE();

		// PipeConsumer does not play the BWE game.
	}

	uint32_t PipeConsumer::GetDesiredBitrate() const
	{
		MS_TRACE();

		// PipeConsumer does not play the BWE game.
		return 0u;
	}

	void PipeConsumer::SendRtpPacket(RTC::RtpPacket* packet)
	{
		MS_TRACE();

		if (!IsActive())
			return;

		auto payloadType = packet->GetPayloadType();

		// NOTE: This may happen if this Consumer supports just some codecs of those
		// in the corresponding Producer.
		if (this->supportedCodecPayloadTypes.find(payloadType) == this->supportedCodecPayloadTypes.end())
		{
			MS_DEBUG_DEV("payload type not supported [payloadType:%" PRIu8 "]", payloadType);

			return;
		}

		auto* rtpStream     = this->mapMappedSsrcRtpStream.at(packet->GetSsrc());
		auto& syncRequired  = this->mapRtpStreamSyncRequired.at(rtpStream);
		auto& rtpSeqManager = this->mapRtpStreamRtpSeqManager.at(rtpStream);

		// If we need to sync, support key frames and this is not a key frame, ignore
		// the packet.
		if (syncRequired && this->keyFrameSupported && !packet->IsKeyFrame())
			return;

		// Whether this is the first packet after re-sync.
		bool isSyncPacket = syncRequired;

		// Sync sequence number and timestamp if required.
		if (isSyncPacket)
		{
			if (packet->IsKeyFrame())
				MS_DEBUG_TAG(rtp, "sync key frame received");

			rtpSeqManager.Sync(packet->GetSequenceNumber() - 1);

			syncRequired = false;
		}

		// Update RTP seq number and timestamp.
		uint16_t seq;

		rtpSeqManager.Input(packet->GetSequenceNumber(), seq);

		// Save original packet fields.
		auto origSeq = packet->GetSequenceNumber();

		// Rewrite packet.
		// NOTE: Do not override the ssrc because we want to honor the consumable ssrcs.
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
		if (rtpStream->ReceivePacket(packet))
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
			  "] from original [seq:%" PRIu16 "]",
			  packet->GetSsrc(),
			  packet->GetSequenceNumber(),
			  packet->GetTimestamp(),
			  origSeq);
		}

		// Restore packet fields.
		packet->SetSequenceNumber(origSeq);
	}

	void PipeConsumer::GetRtcp(
	  RTC::RTCP::CompoundPacket* packet, RTC::RtpStreamSend* rtpStream, uint64_t nowMs)
	{
		MS_TRACE();

		assertm(
		  std::find(this->rtpStreams.begin(), this->rtpStreams.end(), rtpStream) != this->rtpStreams.end(),
		  "RTP stream does exist");

		// Special condition for PipeConsumer since this method will be called in a loop for
		// each stream in this PipeConsumer.
		
		if (
			nowMs != this->lastRtcpSentTime &&
			static_cast<float>((nowMs - this->lastRtcpSentTime) * 1.15) < this->maxRtcpInterval
		)
		
		{
			return;
		}

		auto* report = rtpStream->GetRtcpSenderReport(nowMs);

		if (!report)
			return;

		packet->AddSenderReport(report);

		// Build SDES chunk for this sender.
		auto* sdesChunk = rtpStream->GetRtcpSdesChunk();

		packet->AddSdesChunk(sdesChunk);

		this->lastRtcpSentTime = nowMs;
	}

	void PipeConsumer::NeedWorstRemoteFractionLost(uint32_t /*mappedSsrc*/, uint8_t& worstRemoteFractionLost)
	{
		MS_TRACE();

		if (!IsActive())
			return;

		for (auto* rtpStream : this->rtpStreams)
		{
			auto fractionLost = rtpStream->GetFractionLost();

			// If our fraction lost is worse than the given one, update it.
			if (fractionLost > worstRemoteFractionLost)
				worstRemoteFractionLost = fractionLost;
		}
	}

	void PipeConsumer::ReceiveNack(RTC::RTCP::FeedbackRtpNackPacket* /*nackPacket*/)
	{
		MS_TRACE();

		// Do nothing since we do not enable NACK.
	}

	void PipeConsumer::ReceiveKeyFrameRequest(RTC::RTCP::FeedbackPs::MessageType messageType, uint32_t ssrc)
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

		auto* rtpStream = this->mapMappedSsrcRtpStream.at(ssrc);

		rtpStream->ReceiveKeyFrameRequest(messageType);

		if (IsActive())
			RequestKeyFrame();
	}

	void PipeConsumer::ReceiveRtcpReceiverReport(RTC::RTCP::ReceiverReport* report)
	{
		MS_TRACE();

		auto* rtpStream = this->mapMappedSsrcRtpStream.at(report->GetSsrc());

		rtpStream->ReceiveRtcpReceiverReport(report);
	}

	uint32_t PipeConsumer::GetTransmissionRate(uint64_t nowMs)
	{
		MS_TRACE();

		if (!IsActive())
			return 0u;

		uint32_t rate{ 0u };

		for (auto* rtpStream : this->rtpStreams)
		{
			rate += rtpStream->GetBitrate(nowMs);
		}

		return rate;
	}

	float PipeConsumer::GetRtt() const
	{
		MS_TRACE();

		float rtt{ 0 };

		for (auto* rtpStream : this->rtpStreams)
		{
			if (rtpStream->GetRtt() > rtt)
				rtt = rtpStream->GetRtt();
		}

		return rtt;
	}

	void PipeConsumer::UserOnTransportConnected()
	{
		MS_TRACE();

		for (auto& kv : this->mapRtpStreamSyncRequired)
		{
			kv.second = true;
		}

		if (IsActive())
		{
			for (auto* rtpStream : this->rtpStreams)
			{
				rtpStream->Resume();
			}

			RequestKeyFrame();
		}
	}

	void PipeConsumer::UserOnTransportDisconnected()
	{
		MS_TRACE();

		for (auto* rtpStream : this->rtpStreams)
		{
			rtpStream->Pause();
		}
	}

	void PipeConsumer::UserOnPaused()
	{
		MS_TRACE();

		for (auto* rtpStream : this->rtpStreams)
		{
			rtpStream->Pause();
		}
	}

	void PipeConsumer::UserOnResumed()
	{
		MS_TRACE();

		for (auto& kv : this->mapRtpStreamSyncRequired)
		{
			kv.second = true;
		}

		if (IsActive())
		{
			for (auto* rtpStream : this->rtpStreams)
			{
				rtpStream->Resume();
			}

			RequestKeyFrame();
		}
	}

	void PipeConsumer::CreateRtpStreams()
	{
		MS_TRACE();

		// NOTE: Here we know that SSRCs in Consumer's rtpParameters must be the same
		// as in the given consumableRtpEncodings.
		for (auto& encoding : this->rtpParameters.encodings)
		{
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
				// NOTE: Do not consider NACK in PipeConsumer.

				if (!params.usePli && fb.type == "nack" && fb.parameter == "pli")
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
			// NOTE: PipeConsumer does not support NACK.
			size_t bufferSize{ 0u };
			auto* rtpStream = new RTC::RtpStreamSend(this, params, bufferSize);

			// If the Consumer is paused, tell the RtpStreamSend.
			if (IsPaused() || IsProducerPaused())
				rtpStream->Pause();

			auto* rtxCodec = this->rtpParameters.GetRtxCodecForEncoding(encoding);

			if (rtxCodec && encoding.hasRtx)
				rtpStream->SetRtx(rtxCodec->payloadType, encoding.rtx.ssrc);

			this->rtpStreams.push_back(rtpStream);
			this->mapMappedSsrcRtpStream[encoding.ssrc] = rtpStream;
			this->mapRtpStreamSyncRequired[rtpStream]   = false;
			this->mapRtpStreamRtpSeqManager[rtpStream];
		}
	}

	void PipeConsumer::RequestKeyFrame()
	{
		MS_TRACE();

		if (this->kind != RTC::Media::Kind::VIDEO)
			return;

		for (auto& consumableRtpEncoding : this->consumableRtpEncodings)
		{
			auto mappedSsrc = consumableRtpEncoding.ssrc;

			this->listener->OnConsumerKeyFrameRequested(this, mappedSsrc);
		}
	}

	inline void PipeConsumer::OnRtpStreamScore(
	  RTC::RtpStream* /*rtpStream*/, uint8_t /*score*/, uint8_t /*previousScore*/)
	{
		MS_TRACE();

		// Do nothing.
	}

	inline void PipeConsumer::OnRtpStreamRetransmitRtpPacket(
	  RTC::RtpStreamSend* rtpStream, RTC::RtpPacket* packet)
	{
		MS_TRACE();

		this->listener->OnConsumerRetransmitRtpPacket(this, packet);

		// May emit 'trace' event.
		EmitTraceEventRtpAndKeyFrameTypes(packet, rtpStream->HasRtx());
	}
} // namespace RTC
