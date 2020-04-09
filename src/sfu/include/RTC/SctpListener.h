#ifndef MS_RTC_SCTP_LISTENER_HPP
#define MS_RTC_SCTP_LISTENER_HPP

#include "common.h"
#include "RTC/DataProducer.h"
#include <json.hpp>
#include <unordered_map>

using json = nlohmann::json;

namespace RTC
{
	class SctpListener
	{
	public:
		void FillJson(json& jsonObject) const;
		void AddDataProducer(RTC::DataProducer* dataProducer);
		void RemoveDataProducer(RTC::DataProducer* dataProducer);
		RTC::DataProducer* GetDataProducer(uint16_t streamId);

	public:
		// Table of streamId / DataProducer pairs.
		std::unordered_map<uint16_t, RTC::DataProducer*> streamIdTable;
	};
} // namespace RTC

#endif
