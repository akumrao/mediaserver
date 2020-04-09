#define MS_CLASS "RTC::RtpCodecParameters"
// #define MS_LOG_DEV_LEVEL 3

#include "LoggerTag.h"
#include "base/error.h"
#include "Utils.h"
#include "RTC/RtpDictionaries.h"

namespace RTC
{
	/* Instance methods. */

	RtpCodecParameters::RtpCodecParameters(json& data)
	{
		MS_TRACE();

		if (!data.is_object())
			base::uv::throwError("data is not an object");

		auto jsonMimeTypeIt     = data.find("mimeType");
		auto jsonPayloadTypeIt  = data.find("payloadType");
		auto jsonClockRateIt    = data.find("clockRate");
		auto jsonChannelsIt     = data.find("channels");
		auto jsonParametersIt   = data.find("parameters");
		auto jsonRtcpFeedbackIt = data.find("rtcpFeedback");

		// mimeType is mandatory.
		if (jsonMimeTypeIt == data.end() || !jsonMimeTypeIt->is_string())
			base::uv::throwError("missing mimeType");

		// Set MIME field.
		// This may throw.
		this->mimeType.SetMimeType(jsonMimeTypeIt->get<std::string>());

		// payloadType is mandatory.
		
		if (
			jsonPayloadTypeIt == data.end() ||
			!Utils::Json::IsPositiveInteger(*jsonPayloadTypeIt)
		)
		
		{
			base::uv::throwError("missing payloadType");
		}

		this->payloadType = jsonPayloadTypeIt->get<uint8_t>();

		// clockRate is mandatory.
		
		if (
			jsonClockRateIt == data.end() ||
			!Utils::Json::IsPositiveInteger(*jsonClockRateIt)
		)
		
		{
			base::uv::throwError("missing clockRate");
		}

		this->clockRate = jsonClockRateIt->get<uint32_t>();

		// channels is optional.
		
		if (
			jsonChannelsIt != data.end() &&
			Utils::Json::IsPositiveInteger(*jsonChannelsIt)
		)
		
		{
			this->channels = jsonChannelsIt->get<uint8_t>();
		}

		// parameters is optional.
		if (jsonParametersIt != data.end() && jsonParametersIt->is_object())
			this->parameters.Set(*jsonParametersIt);

		// rtcpFeedback is optional.
		if (jsonRtcpFeedbackIt != data.end() && jsonRtcpFeedbackIt->is_array())
		{
			this->rtcpFeedback.reserve(jsonRtcpFeedbackIt->size());

			for (auto& entry : *jsonRtcpFeedbackIt)
			{
				// This may throw due the constructor of RTC::RtcpFeedback.
				this->rtcpFeedback.emplace_back(entry);
			}
		}

		// Check codec.
		CheckCodec();
	}

	void RtpCodecParameters::FillJson(json& jsonObject) const
	{
		MS_TRACE();

		// Add mimeType.
		jsonObject["mimeType"] = this->mimeType.ToString();

		// Add payloadType.
		jsonObject["payloadType"] = this->payloadType;

		// Add clockRate.
		jsonObject["clockRate"] = this->clockRate;

		// Add channels.
		if (this->channels > 1)
			jsonObject["channels"] = this->channels;

		// Add parameters.
		this->parameters.FillJson(jsonObject["parameters"]);

		// Add rtcpFeedback.
		jsonObject["rtcpFeedback"] = json::array();
		auto jsonRtcpFeedbackIt    = jsonObject.find("rtcpFeedback");

		for (size_t i{ 0 }; i < this->rtcpFeedback.size(); ++i)
		{
			jsonRtcpFeedbackIt->emplace_back(json::value_t::object);

			auto& jsonEntry = (*jsonRtcpFeedbackIt)[i];
			auto& fb        = this->rtcpFeedback[i];

			fb.FillJson(jsonEntry);
		}
	}

	inline void RtpCodecParameters::CheckCodec()
	{
		MS_TRACE();

		static std::string aptString{ "apt" };

		// Check per MIME parameters and set default values.
		switch (this->mimeType.subtype)
		{
			case RTC::RtpCodecMimeType::Subtype::RTX:
			{
				// A RTX codec must have 'apt' parameter.
				if (!this->parameters.HasPositiveInteger(aptString))
					base::uv::throwError("missing apt parameter in RTX codec");

				break;
			}

			default:;
		}
	}
} // namespace RTC
