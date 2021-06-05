#define MS_CLASS "RTC::SctpStreamParameters"
// #define MS_LOG_DEV_LEVEL 3

#include "LoggerTag.h"
#include "base/error.h"
#include "Utils.h"
#include "RTC/SctpDictionaries.h"

namespace RTC
{
	/* Instance methods. */

	SctpStreamParameters::SctpStreamParameters(json& data)
	{
		MS_TRACE();
                
                STrace << "SctpStreamParameters() " <<  this->streamId;

		if (!data.is_object())
			base::uv::throwError("data is not an object");

		auto jsonStreamIdIt          = data.find("streamId");
		auto jsonOrderedIdIt         = data.find("ordered");
		auto jsonMaxPacketLifeTimeIt = data.find("maxPacketLifeTime");
		auto jsonMaxRetransmitsIt    = data.find("maxRetransmits");

		// streamId is mandatory.
		if (jsonStreamIdIt == data.end() || !Utils::Json::IsPositiveInteger(*jsonStreamIdIt))
			base::uv::throwError("missing streamId");

		this->streamId = jsonStreamIdIt->get<uint16_t>();

		if (this->streamId > 65534)
			base::uv::throwError("streamId must not be greater than 65534");

		// ordered is optional.
		bool orderedGiven = false;

		if (jsonOrderedIdIt != data.end() && jsonOrderedIdIt->is_boolean())
		{
			orderedGiven  = true;
			this->ordered = jsonOrderedIdIt->get<bool>();
		}

		// maxPacketLifeTime is optional.
		
		if (
			jsonMaxPacketLifeTimeIt != data.end() &&
			Utils::Json::IsPositiveInteger(*jsonMaxPacketLifeTimeIt)
		)
		
		{
			this->maxPacketLifeTime = jsonMaxPacketLifeTimeIt->get<uint16_t>();
		}

		// maxRetransmits is optional.
		
		if (
			jsonMaxRetransmitsIt != data.end() &&
			Utils::Json::IsPositiveInteger(*jsonMaxRetransmitsIt)
		)
		
		{
			this->maxRetransmits = jsonMaxRetransmitsIt->get<uint16_t>();
		}

		if (this->maxPacketLifeTime && this->maxRetransmits)
			base::uv::throwError("cannot provide both maxPacketLifeTime and maxRetransmits");

		
		if (
			orderedGiven &&
			this->ordered &&
			(this->maxPacketLifeTime || this->maxRetransmits)
		)
		
		{
			base::uv::throwError("cannot be ordered with maxPacketLifeTime or maxRetransmits");
		}
		else if (!orderedGiven && (this->maxPacketLifeTime || this->maxRetransmits))
		{
			this->ordered = false;
		}
	}

	void SctpStreamParameters::FillJson(json& jsonObject) const
	{
		MS_TRACE();
                
                SInfo << "SctpStreamParameters::FillJson " <<  this->streamId;

		// Add streamId.
		jsonObject["streamId"] = this->streamId;

		// Add ordered.
		jsonObject["ordered"] = this->ordered;

		// Add maxPacketLifeTime.
		if (this->maxPacketLifeTime)
			jsonObject["maxPacketLifeTime"] = this->maxPacketLifeTime;

		// Add maxRetransmits.
		if (this->maxRetransmits)
			jsonObject["maxRetransmits"] = this->maxRetransmits;
	}
} // namespace RTC
