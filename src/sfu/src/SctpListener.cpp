#define MS_CLASS "RTC::SctpListener"
// #define MS_LOG_DEV_LEVEL 3

#include "RTC/SctpListener.h"
#include "LoggerTag.h"
#include "base/error.h"
#include "RTC/DataProducer.h"

namespace RTC
{
	/* Instance methods. */

	void SctpListener::FillJson(json& jsonObject) const
	{
            SInfo << "SctpListener::FillJson";
		MS_TRACE();

		jsonObject["streamIdTable"] = json::object();

		auto jsonStreamIdTableIt = jsonObject.find("streamIdTable");

		// Add streamIdTable.
		for (auto& kv : this->sctpTable)
		{
			auto streamId      = kv.first;
			auto* dataProducer = kv.second;  // arvind following line comment might cause problem at statistic collection of sctp

			///(*jsonStreamIdTableIt)[std::to_string(streamId)] = dataProducer->id;  // arvind work pendign
		}
	}

	void SctpListener::AddDataProducer(RTC::SctpAssociation* sctpAssociation, RTC::DataProducer* dataProducer)
	{
		MS_TRACE();

		auto& sctpParameters = dataProducer->GetSctpStreamParameters();

		// Add entries into the streamIdTable.
		//auto streamId = sctpParameters.streamId;
                
                SInfo << "SctpListener::AddDataProducer sctpAssociation " << sctpAssociation ;

		if (this->sctpTable.find(sctpAssociation) == this->sctpTable.end())
		{
			this->sctpTable[sctpAssociation] = dataProducer;
		}
		else
		{
			base::uv::throwError("streamId already exists in SCTP listener ", 0);
		}
	}

	void SctpListener::RemoveDataProducer(RTC::DataProducer* dataProducer)
	{
		MS_TRACE();
                SInfo << "SctpListener::RemoveDataProducer "  ;
		// Remove from the listener table all entries pointing to the DataProducer.
		for (auto it = this->sctpTable.begin(); it != this->sctpTable.end();)
		{
			if (it->second == dataProducer)
				it = this->sctpTable.erase(it);
			else
				++it;
		}
	}

	RTC::DataProducer* SctpListener::GetDataProducer( RTC::SctpAssociation* sctpAssociation)
	{
		MS_TRACE();
                SInfo << "SctpListener::GetDataProducer streamId " << sctpAssociation ;
		auto it = this->sctpTable.find(sctpAssociation);
                
               // int xn = this->streamIdTable.size();

		if (it != this->sctpTable.end())
		{
			auto* dataProducer = it->second;

			return dataProducer;
		}

		return nullptr;
	}
} // namespace RTC
