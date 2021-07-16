#ifndef MS_RTC_DATA_PRODUCER_HPP
#define MS_RTC_DATA_PRODUCER_HPP

#include "common.h"
#include "Channel/Request.h"
#include "RTC/SctpDictionaries.h"
#include <json.hpp>
#include <string>

namespace RTC
{
	class DataProducer
	{
	public:
		class Listener
		{
		public:
			virtual void OnDataProducerSctpMessageReceived(
			  RTC::DataProducer* dataProducer, uint32_t ppid, const uint8_t* msg, size_t len) = 0;
		};

	public:
		DataProducer(const std::string& id, RTC::DataProducer::Listener* listener, json& data);
		virtual ~DataProducer();

	public:
		void FillJson(json& jsonObject) const;
		void FillJsonStats(json& jsonArray) const;
		void HandleRequest(Channel::Request* request);
		 RTC::SctpStreamParameters& GetSctpStreamParameters() ;
		void ReceiveSctpMessage(uint32_t ppid, const uint8_t* msg, size_t len);

	public:
		// Passed by argument.
		const std::string id;
                std::string label;
		std::string protocol;

	private:
		// Passed by argument.
		RTC::DataProducer::Listener* listener{ nullptr };
		// Others.
		RTC::SctpStreamParameters sctpStreamParameters;
		size_t messagesReceived{ 0 };
		size_t bytesReceived{ 0 };
	};

	/* Inline methods. */

	inline  RTC::SctpStreamParameters& DataProducer::GetSctpStreamParameters() 
	{
		return this->sctpStreamParameters;
	}
} // namespace RTC

#endif
