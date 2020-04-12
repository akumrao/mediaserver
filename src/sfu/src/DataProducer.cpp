#define MS_CLASS "RTC::DataProducer"
// #define MS_LOG_DEV_LEVEL 3

#include "RTC/DataProducer.h"
#include "base/application.h"
#include "LoggerTag.h"
#include "base/error.h"

namespace RTC
{
	/* Instance methods. */

	DataProducer::DataProducer(const std::string& id, RTC::DataProducer::Listener* listener, json& data)
	  : id(id), listener(listener)
	{
		MS_TRACE();

		auto jsonSctpStreamParametersIt = data.find("sctpStreamParameters");
		auto jsonLabelIt                = data.find("label");
		auto jsonProtocolIt             = data.find("protocol");

		if (jsonSctpStreamParametersIt == data.end() || !jsonSctpStreamParametersIt->is_object())
		{
			base::uv::throwError("missing sctpStreamParameters");
		}

		// This may throw.
		this->sctpStreamParameters = RTC::SctpStreamParameters(*jsonSctpStreamParametersIt);

		if (jsonLabelIt != data.end() && jsonLabelIt->is_string())
			this->label = jsonLabelIt->get<std::string>();

		if (jsonProtocolIt != data.end() && jsonProtocolIt->is_string())
			this->protocol = jsonProtocolIt->get<std::string>();
	}

	DataProducer::~DataProducer()
	{
		MS_TRACE();
	}

	void DataProducer::FillJson(json& jsonObject) const
	{
		MS_TRACE();

		// Add id.
		jsonObject["id"] = this->id;

		// Add sctpStreamParameters.
		this->sctpStreamParameters.FillJson(jsonObject["sctpStreamParameters"]);

		// Add label.
		jsonObject["label"] = this->label;

		// Add protocol.
		jsonObject["protocol"] = this->protocol;
	}

	void DataProducer::FillJsonStats(json& jsonArray) const
	{
		MS_TRACE();

		jsonArray.emplace_back(json::value_t::object);
		auto& jsonObject = jsonArray[0];

		// Add type.
		jsonObject["type"] = "data-producer";

		// Add timestamp.
		jsonObject["timestamp"] = base::Application::GetTimeMs();

		// Add label.
		jsonObject["label"] = this->label;

		// Add protocol.
		jsonObject["protocol"] = this->protocol;

		// Add messagesReceived.
		jsonObject["messagesReceived"] = this->messagesReceived;

		// Add bytesReceived.
		jsonObject["bytesReceived"] = this->bytesReceived;
	}

	void DataProducer::HandleRequest(Channel::Request* request)
	{
		MS_TRACE();

		switch (request->methodId)
		{
			case Channel::Request::MethodId::DATA_PRODUCER_DUMP:
			{
				json data = json::object();

				FillJson(data);

				request->Accept(data);

				break;
			}

			case Channel::Request::MethodId::DATA_PRODUCER_GET_STATS:
			{
				json data = json::array();

				FillJsonStats(data);

				request->Accept(data);

				break;
			}

			default:
			{
				base::uv::throwError("unknown method " + request->method);
			}
		}
	}

	void DataProducer::ReceiveSctpMessage(uint32_t ppid, const uint8_t* msg, size_t len)
	{
		MS_TRACE();

		this->messagesReceived++;
		this->bytesReceived += len;

		this->listener->OnDataProducerSctpMessageReceived(this, ppid, msg, len);
	}
} // namespace RTC
