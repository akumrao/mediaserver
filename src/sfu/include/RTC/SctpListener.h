#ifndef MS_RTC_SCTP_LISTENER_HPP
#define MS_RTC_SCTP_LISTENER_HPP

#include "common.h"
#include "RTC/DataProducer.h"
#include <json.hpp>
#include <unordered_map>
#include "RTC/SctpAssociation.h"
using json = nlohmann::json;

namespace RTC
{
	class SctpListener
	{
	public:
		void FillJson(json& jsonObject) const;
		void AddDataProducer(RTC::SctpAssociation*, RTC::DataProducer* dataProducer);
		void RemoveDataProducer(RTC::DataProducer* dataProducer);
		RTC::DataProducer* GetDataProducer( RTC::SctpAssociation* sctpAssociation);

	public:
		// Table of streamId / DataProducer pairs.
		std::unordered_map< RTC::SctpAssociation* , RTC::DataProducer*> sctpTable;
	};
} // namespace RTC

#endif
