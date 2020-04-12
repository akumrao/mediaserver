#define MS_CLASS "RTC::RtpEncodingParameters"
// #define MS_LOG_DEV_LEVEL 3

#include "LoggerTag.h"
#include "base/error.h"
#include "Utils.h"
#include "RTC/RtpDictionaries.h"
#include <exception>
#include <regex>

namespace RTC
{
	/* Instance methods. */

	RtpEncodingParameters::RtpEncodingParameters(json& data)
	{
		MS_TRACE();

		if (!data.is_object())
			base::uv::throwError("data is not an object");

		auto jsonSsrcIt             = data.find("ssrc");
		auto jsonRidIt              = data.find("rid");
		auto jsonCodecPayloadTypeIt = data.find("codecPayloadType");
		auto jsonRtxIt              = data.find("rtx");
		auto jsonMaxBitrateIt       = data.find("maxBitrate");
		auto jsonMaxFramerateIt     = data.find("maxFramerate");
		auto jsonDtxIt              = data.find("dtx");
		auto jsonScalabilityModeIt  = data.find("scalabilityMode");

		// ssrc is optional.
		
		if (
			jsonSsrcIt != data.end() &&
			Utils::Json::IsPositiveInteger(*jsonSsrcIt)
		)
		
		{
			this->ssrc = jsonSsrcIt->get<uint32_t>();
		}

		// rid is optional.
		if (jsonRidIt != data.end() && jsonRidIt->is_string())
			this->rid = jsonRidIt->get<std::string>();

		// codecPayloadType is optional.
		
		if (
			jsonCodecPayloadTypeIt != data.end() &&
			Utils::Json::IsPositiveInteger(*jsonCodecPayloadTypeIt)
		)
		
		{
			this->codecPayloadType    = jsonCodecPayloadTypeIt->get<uint8_t>();
			this->hasCodecPayloadType = true;
		}

		// rtx is optional.
		// This may throw.
		if (jsonRtxIt != data.end() && jsonRtxIt->is_object())
		{
			this->rtx    = RtpRtxParameters(*jsonRtxIt);
			this->hasRtx = true;
		}

		// maxBitrate is optional.
		
		if (
			jsonMaxBitrateIt != data.end() &&
			Utils::Json::IsPositiveInteger(*jsonMaxBitrateIt)
		)
		
		{
			this->maxBitrate = jsonMaxBitrateIt->get<uint32_t>();
		}

		// maxFramerate is optional.
		if (jsonMaxFramerateIt != data.end() && jsonMaxFramerateIt->is_number())
			this->maxFramerate = jsonMaxFramerateIt->get<double>();

		// dtx is optional.
		if (jsonDtxIt != data.end() && jsonDtxIt->is_boolean())
			this->dtx = jsonDtxIt->get<bool>();

		// scalabilityMode is optional.
		if (jsonScalabilityModeIt != data.end() && jsonScalabilityModeIt->is_string())
		{
			std::string scalabilityMode = jsonScalabilityModeIt->get<std::string>();

			static const std::regex ScalabilityModeRegex(
			  "^[LS]([1-9]\\d{0,1})T([1-9]\\d{0,1})(_KEY)?.*", std::regex_constants::ECMAScript);

			std::smatch match;

			std::regex_match(scalabilityMode, match, ScalabilityModeRegex);

			if (!match.empty())
			{
				this->scalabilityMode = scalabilityMode;

				try
				{
					this->spatialLayers  = std::stoul(match[1].str());
					this->temporalLayers = std::stoul(match[2].str());
					this->ksvc           = match.size() >= 4 && match[3].str() == "_KEY";
				}
				catch (std::exception& e)
				{
					base::uv::throwError("invalid scalabilityMode: " + std::string(e.what()));
				}
			}
		}
	}

	void RtpEncodingParameters::FillJson(json& jsonObject) const
	{
		MS_TRACE();

		// Force it to be an object even if no key/values are added below.
		jsonObject = json::object();

		// Add ssrc.
		if (this->ssrc != 0u)
			jsonObject["ssrc"] = this->ssrc;

		// Add rid.
		if (!this->rid.empty())
			jsonObject["rid"] = this->rid;

		// Add codecPayloadType.
		if (this->hasCodecPayloadType)
			jsonObject["codecPayloadType"] = this->codecPayloadType;

		// Add rtx.
		if (this->hasRtx)
			this->rtx.FillJson(jsonObject["rtx"]);

		// Add maxBitrate.
		if (this->maxBitrate != 0u)
			jsonObject["maxBitrate"] = this->maxBitrate;

		// Add maxFramerate.
		if (this->maxFramerate > 0)
			jsonObject["maxFramerate"] = this->maxFramerate;

		// Add dtx.
		if (this->dtx)
			jsonObject["dtx"] = this->dtx;

		// Add scalabilityMode.
		if (!this->scalabilityMode.empty())
		{
			jsonObject["scalabilityMode"] = this->scalabilityMode;
			jsonObject["spatialLayers"]   = this->spatialLayers;
			jsonObject["temporalLayers"]  = this->temporalLayers;
			jsonObject["ksvc"]            = this->ksvc;
		}
	}
} // namespace RTC
