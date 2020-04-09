#define MS_CLASS "RTC::RtcpFeedback"
// #define MS_LOG_DEV_LEVEL 3

#include "LoggerTag.h"
#include "base/error.h"
#include "RTC/RtpDictionaries.h"

namespace RTC
{
	/* Instance methods. */

	RtcpFeedback::RtcpFeedback(json& data)
	{
		MS_TRACE();

		if (!data.is_object())
			base::uv::throwError("data is not an object");

		auto jsonTypeIt      = data.find("type");
		auto jsonParameterIt = data.find("parameter");

		// type is mandatory.
		if (jsonTypeIt == data.end() || !jsonTypeIt->is_string())
			base::uv::throwError("missing type");

		this->type = jsonTypeIt->get<std::string>();

		// parameter is optional.
		if (jsonParameterIt != data.end() && jsonParameterIt->is_string())
			this->parameter = jsonParameterIt->get<std::string>();
	}

	void RtcpFeedback::FillJson(json& jsonObject) const
	{
		MS_TRACE();

		// Add type.
		jsonObject["type"] = this->type;

		// Add parameter (optional).
		if (this->parameter.length() > 0)
			jsonObject["parameter"] = this->parameter;
	}
} // namespace RTC
