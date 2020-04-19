

#include "RTC/Consumer.h"
#include "base/application.h"
#include "LoggerTag.h"
#include "base/error.h"
#include "Channel/Notifier.h"
#include <iterator> // std::ostream_iterator
#include <sstream>  // std::ostringstream

namespace RTC
{
	/* Instance methods. */

	Consumer::Consumer(const std::string& id, Listener* listener, json& data, RTC::RtpParameters::Type type)
	  : id(id), listener(listener), type(type)
	{
		MS_TRACE();

		auto jsonKindIt = data.find("kind");

		if (jsonKindIt == data.end() || !jsonKindIt->is_string())
			base::uv::throwError("missing kind");

		// This may throw.
		this->kind = RTC::Media::GetKind(jsonKindIt->get<std::string>());

		if (this->kind == RTC::Media::Kind::ALL)
			base::uv::throwError("invalid empty kind");

		auto jsonRtpParametersIt = data.find("rtpParameters");

		if (jsonRtpParametersIt == data.end() || !jsonRtpParametersIt->is_object())
			base::uv::throwError("missing rtpParameters");

		// This may throw.
		this->rtpParameters = RTC::RtpParameters(*jsonRtpParametersIt);

		if (this->rtpParameters.encodings.empty())
			base::uv::throwError("empty rtpParameters.encodings");

		// All encodings must have SSRCs.
		for (auto& encoding : this->rtpParameters.encodings)
		{
			if (encoding.ssrc == 0)
				base::uv::throwError("invalid encoding in rtpParameters (missing ssrc)");
			else if (encoding.hasRtx && encoding.rtx.ssrc == 0)
				base::uv::throwError("invalid encoding in rtpParameters (missing rtx.ssrc)");
		}

		auto jsonConsumableRtpEncodingsIt = data.find("consumableRtpEncodings");

		if (jsonConsumableRtpEncodingsIt == data.end() || !jsonConsumableRtpEncodingsIt->is_array())
			base::uv::throwError("missing consumableRtpEncodings");

		if (jsonConsumableRtpEncodingsIt->empty())
			base::uv::throwError("empty consumableRtpEncodings");

		this->consumableRtpEncodings.reserve(jsonConsumableRtpEncodingsIt->size());

		for (size_t i{ 0 }; i < jsonConsumableRtpEncodingsIt->size(); ++i)
		{
			auto& entry = (*jsonConsumableRtpEncodingsIt)[i];

			// This may throw due the constructor of RTC::RtpEncodingParameters.
			this->consumableRtpEncodings.emplace_back(entry);

			// Verify that it has ssrc field.
			auto& encoding = this->consumableRtpEncodings[i];

			if (encoding.ssrc == 0u)
				base::uv::throwError("wrong encoding in consumableRtpEncodings (missing ssrc)");
		}

		// Fill RTP header extension ids and their mapped values.
		// This may throw.
		for (auto& exten : this->rtpParameters.headerExtensions)
		{
			if (exten.id == 0u)
				base::uv::throwError("RTP extension id cannot be 0");

			if (this->rtpHeaderExtensionIds.ssrcAudioLevel == 0u && exten.type == RTC::RtpHeaderExtensionUri::Type::SSRC_AUDIO_LEVEL)
			{
				this->rtpHeaderExtensionIds.ssrcAudioLevel = exten.id;
			}

			if (this->rtpHeaderExtensionIds.videoOrientation == 0u && exten.type == RTC::RtpHeaderExtensionUri::Type::VIDEO_ORIENTATION)
			{
				this->rtpHeaderExtensionIds.videoOrientation = exten.id;
			}

			if (this->rtpHeaderExtensionIds.absSendTime == 0u && exten.type == RTC::RtpHeaderExtensionUri::Type::ABS_SEND_TIME)
			{
				this->rtpHeaderExtensionIds.absSendTime = exten.id;
			}

			if (this->rtpHeaderExtensionIds.transportWideCc01 == 0u && exten.type == RTC::RtpHeaderExtensionUri::Type::TRANSPORT_WIDE_CC_01)
			{
				this->rtpHeaderExtensionIds.transportWideCc01 = exten.id;
			}

			if (this->rtpHeaderExtensionIds.mid == 0u && exten.type == RTC::RtpHeaderExtensionUri::Type::MID)
			{
				this->rtpHeaderExtensionIds.mid = exten.id;
			}

			if (this->rtpHeaderExtensionIds.rid == 0u && exten.type == RTC::RtpHeaderExtensionUri::Type::RTP_STREAM_ID)
			{
				this->rtpHeaderExtensionIds.rid = exten.id;
			}

			if (this->rtpHeaderExtensionIds.rrid == 0u && exten.type == RTC::RtpHeaderExtensionUri::Type::REPAIRED_RTP_STREAM_ID)
			{
				this->rtpHeaderExtensionIds.rrid = exten.id;
			}
		}

		auto jsonPausedIt = data.find("paused");

		if (jsonPausedIt != data.end() && jsonPausedIt->is_boolean())
			this->paused = jsonPausedIt->get<bool>();

		// Fill supported codec payload types.
		for (auto& codec : this->rtpParameters.codecs)
		{
			if (codec.mimeType.IsMediaCodec())
				this->supportedCodecPayloadTypes.insert(codec.payloadType);
		}

		// Fill media SSRCs vector.
		for (auto& encoding : this->rtpParameters.encodings)
		{
			this->mediaSsrcs.push_back(encoding.ssrc);
		}

		// Fill RTX SSRCs vector.
		for (auto& encoding : this->rtpParameters.encodings)
		{
			if (encoding.hasRtx)
				this->rtxSsrcs.push_back(encoding.rtx.ssrc);
		}

		// Set the RTCP report generation interval.
		if (this->kind == RTC::Media::Kind::AUDIO)
			this->maxRtcpInterval = RTC::RTCP::MaxAudioIntervalMs;
		else
			this->maxRtcpInterval = RTC::RTCP::MaxVideoIntervalMs;
	}

	Consumer::~Consumer()
	{
		MS_TRACE();
	}

	void Consumer::FillJson(json& jsonObject) const
	{
		MS_TRACE();

		// Add id.
		jsonObject["id"] = this->id;

		// Add kind.
		jsonObject["kind"] = RTC::Media::GetString(this->kind);

		// Add rtpParameters.
		this->rtpParameters.FillJson(jsonObject["rtpParameters"]);

		// Add type.
		jsonObject["type"] = RTC::RtpParameters::GetTypeString(this->type);

		// Add consumableRtpEncodings.
		jsonObject["consumableRtpEncodings"] = json::array();
		auto jsonConsumableRtpEncodingsIt    = jsonObject.find("consumableRtpEncodings");

		for (size_t i{ 0 }; i < this->consumableRtpEncodings.size(); ++i)
		{
			jsonConsumableRtpEncodingsIt->emplace_back(json::value_t::object);

			auto& jsonEntry = (*jsonConsumableRtpEncodingsIt)[i];
			auto& encoding  = this->consumableRtpEncodings[i];

			encoding.FillJson(jsonEntry);
		}

		// Add supportedCodecPayloadTypes.
		jsonObject["supportedCodecPayloadTypes"] = this->supportedCodecPayloadTypes;

		// Add paused.
		jsonObject["paused"] = this->paused;

		// Add producerPaused.
		jsonObject["producerPaused"] = this->producerPaused;

		// Add priority.
		jsonObject["priority"] = this->priority;

		// Add traceEventTypes.
		std::vector<std::string> traceEventTypes;
		std::ostringstream traceEventTypesStream;

		if (this->traceEventTypes.rtp)
			traceEventTypes.emplace_back("rtp");
		if (this->traceEventTypes.keyframe)
			traceEventTypes.emplace_back("keyframe");
		if (this->traceEventTypes.nack)
			traceEventTypes.emplace_back("nack");
		if (this->traceEventTypes.pli)
			traceEventTypes.emplace_back("pli");
		if (this->traceEventTypes.fir)
			traceEventTypes.emplace_back("fir");

		if (!traceEventTypes.empty())
		{
			std::copy(
			  traceEventTypes.begin(),
			  traceEventTypes.end() - 1,
			  std::ostream_iterator<std::string>(traceEventTypesStream, ","));
			traceEventTypesStream << traceEventTypes.back();
		}

		jsonObject["traceEventTypes"] = traceEventTypesStream.str();
	}

	void Consumer::HandleRequest(Channel::Request* request)
	{
		MS_TRACE();

		switch (request->methodId)
		{
			case Channel::Request::MethodId::CONSUMER_DUMP:
			{
				json data = json::object();

				FillJson(data);

				request->Accept(data);

				break;
			}

			case Channel::Request::MethodId::CONSUMER_GET_STATS:
			{
				json data = json::array();

				FillJsonStats(data);

				request->Accept(data);

				break;
			}

			case Channel::Request::MethodId::CONSUMER_PAUSE:
			{
				if (this->paused)
				{
					request->Accept();

					return;
				}

				bool wasActive = IsActive();

				this->paused = true;

				MS_DEBUG_DEV("Consumer paused [consumerId:%s]", this->id.c_str());

				if (wasActive)
					UserOnPaused();

				request->Accept();

				break;
			}

			case Channel::Request::MethodId::CONSUMER_RESUME:
			{
				if (!this->paused)
				{
					request->Accept();

					return;
				}

				this->paused = false;

				MS_DEBUG_DEV("Consumer resumed [consumerId:%s]", this->id.c_str());

				if (IsActive())
					UserOnResumed();

				request->Accept();

				break;
			}

			case Channel::Request::MethodId::CONSUMER_SET_PRIORITY:
			{
				auto jsonPriorityIt = request->data.find("priority");

				if (jsonPriorityIt == request->data.end() || !jsonPriorityIt->is_number())
					base::uv::throwError("wrong priority (not a number)");

				auto priority = jsonPriorityIt->get<uint8_t>();

				if (priority < 1u)
					base::uv::throwError("wrong priority (must be higher than 0)");

				this->priority = priority;

				json data = json::object();

				data["priority"] = this->priority;

				request->Accept(data);

				break;
			}

			case Channel::Request::MethodId::CONSUMER_ENABLE_TRACE_EVENT:
			{
				auto jsonTypesIt = request->data.find("types");

				// Disable all if no entries.
				if (jsonTypesIt == request->data.end() || !jsonTypesIt->is_array())
					base::uv::throwError("wrong types (not an array)");

				// Reset traceEventTypes.
				struct TraceEventTypes newTraceEventTypes;

				for (const auto& type : *jsonTypesIt)
				{
					if (!type.is_string())
						base::uv::throwError("wrong type (not a string)");

					std::string typeStr = type.get<std::string>();

					if (typeStr == "rtp")
						newTraceEventTypes.rtp = true;
					else if (typeStr == "keyframe")
						newTraceEventTypes.keyframe = true;
					else if (typeStr == "nack")
						newTraceEventTypes.nack = true;
					else if (typeStr == "pli")
						newTraceEventTypes.pli = true;
					else if (typeStr == "fir")
						newTraceEventTypes.fir = true;
				}

				this->traceEventTypes = newTraceEventTypes;

				request->Accept();

				break;
			}

			default:
			{
				base::uv::throwError("unknown method " + request->method);
			}
		}
	}

	void Consumer::TransportConnected()
	{
		MS_TRACE();

		this->transportConnected = true;

		MS_DEBUG_DEV("Transport connected [consumerId:%s]", this->id.c_str());

		UserOnTransportConnected();
	}

	void Consumer::TransportDisconnected()
	{
		MS_TRACE();

		this->transportConnected = false;

		MS_DEBUG_DEV("Transport disconnected [consumerId:%s]", this->id.c_str());

		UserOnTransportDisconnected();
	}

	void Consumer::ProducerPaused()
	{
		MS_TRACE();

		if (this->producerPaused)
			return;

		bool wasActive = IsActive();

		this->producerPaused = true;

		MS_DEBUG_DEV("Producer paused [consumerId:%s]", this->id.c_str());

		if (wasActive)
			UserOnPaused();

		Channel::Notifier::Emit(this->id, "producerpause");
	}

	void Consumer::ProducerResumed()
	{
		MS_TRACE();

		if (!this->producerPaused)
			return;

		this->producerPaused = false;

		MS_DEBUG_DEV("Producer resumed [consumerId:%s]", this->id.c_str());

		if (IsActive())
			UserOnResumed();

		Channel::Notifier::Emit(this->id, "producerresume");
	}

	// The caller (Router) is supposed to proceed with the deletion of this Consumer
	// right after calling this method. Otherwise ugly things may happen.
	void Consumer::ProducerClosed()
	{
		MS_TRACE();

		this->producerClosed = true;

		MS_DEBUG_DEV("Producer closed [consumerId:%s]", this->id.c_str());

		Channel::Notifier::Emit(this->id, "producerclose");

		this->listener->OnConsumerProducerClosed(this);
	}

	void Consumer::EmitTraceEventRtpAndKeyFrameTypes(RTC::RtpPacket* packet, bool isRtx) const
	{
		MS_TRACE();

		if (this->traceEventTypes.keyframe && packet->IsKeyFrame())
		{
			json data = json::object();

			data["type"]      = "keyframe";
			data["timestamp"] = base::Application::GetTimeMs();
			data["direction"] = "out";

			packet->FillJson(data["info"]);

			if (isRtx)
				data["info"]["isRtx"] = true;

			Channel::Notifier::Emit(this->id, "trace", data);
		}
		else if (this->traceEventTypes.rtp)
		{
			json data = json::object();

			data["type"]      = "rtp";
			data["timestamp"] = base::Application::GetTimeMs();
			data["direction"] = "out";

			packet->FillJson(data["info"]);

			if (isRtx)
				data["info"]["isRtx"] = true;

			Channel::Notifier::Emit(this->id, "trace", data);
		}
	}

	void Consumer::EmitTraceEventPliType(uint32_t ssrc) const
	{
		MS_TRACE();

		if (!this->traceEventTypes.pli)
			return;

		json data = json::object();

		data["type"]         = "pli";
		data["timestamp"]    = base::Application::GetTimeMs();
		data["direction"]    = "in";
		data["info"]["ssrc"] = ssrc;

		Channel::Notifier::Emit(this->id, "trace", data);
	}

	void Consumer::EmitTraceEventFirType(uint32_t ssrc) const
	{
		MS_TRACE();

		if (!this->traceEventTypes.fir)
			return;

		json data = json::object();

		data["type"]         = "fir";
		data["timestamp"]    = base::Application::GetTimeMs();
		data["direction"]    = "in";
		data["info"]["ssrc"] = ssrc;

		Channel::Notifier::Emit(this->id, "trace", data);
	}

	void Consumer::EmitTraceEventNackType() const
	{
		MS_TRACE();

		if (!this->traceEventTypes.nack)
			return;

		json data = json::object();

		data["type"]      = "nack";
		data["timestamp"] = base::Application::GetTimeMs();
		data["direction"] = "in";
		data["info"]      = json::object();

		Channel::Notifier::Emit(this->id, "trace", data);
	}
} // namespace RTC
