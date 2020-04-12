#define MS_CLASS "RTC::RtcpParameters"
// #define MS_LOG_DEV_LEVEL 3

#include "LoggerTag.h"
#include "base/error.h"
#include "Utils.h"
#include "RTC/RtpDictionaries.h"

namespace RTC
{
	/* Instance methods. */

	RtcpParameters::RtcpParameters(json& data)
	{
		MS_TRACE();

		if (!data.is_object())
			base::uv::throwError("data is not an object");

		auto jsonCnameIt       = data.find("cname");
		auto jsonSsrcIt        = data.find("ssrc");
		auto jsonRedicedSizeIt = data.find("reducedSize");

		// cname is optional.
		if (jsonCnameIt != data.end() && jsonCnameIt->is_string())
			this->cname = jsonCnameIt->get<std::string>();

		// ssrc is optional.
		
		if (
			jsonSsrcIt != data.end() &&
			Utils::Json::IsPositiveInteger(*jsonSsrcIt)
		)
		
		{
			this->ssrc = jsonSsrcIt->get<uint32_t>();
		}

		// reducedSize is optional.
		if (jsonRedicedSizeIt != data.end() && jsonRedicedSizeIt->is_boolean())
			this->reducedSize = jsonRedicedSizeIt->get<bool>();
	}

	void RtcpParameters::FillJson(json& jsonObject) const
	{
		MS_TRACE();

		// Add cname.
		if (!this->cname.empty())
			jsonObject["cname"] = this->cname;

		// Add ssrc.
		if (this->ssrc != 0u)
			jsonObject["ssrc"] = this->ssrc;

		// Add reducedSize.
		jsonObject["reducedSize"] = this->reducedSize;
	}
} // namespace RTC
