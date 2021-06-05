

#include "RTC/DataConsumer.h"
#include "base/application.h"
#include "LoggerTag.h"
#include "base/error.h"
#include "Channel/Notifier.h"

namespace RTC
{
	/* Instance methods. */

	DataConsumer::DataConsumer(
	  const std::string& id, RTC::DataConsumer::Listener* listener, json& data, size_t maxSctpMessageSize)
	  : id(id), listener(listener), maxSctpMessageSize(maxSctpMessageSize)
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

	DataConsumer::~DataConsumer()
	{
		MS_TRACE();
	}

	void DataConsumer::FillJson(json& jsonObject) const
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

	void DataConsumer::FillJsonStats(json& jsonArray) const
	{
		MS_TRACE();

		jsonArray.emplace_back(json::value_t::object);
		auto& jsonObject = jsonArray[0];

		// Add type.
		jsonObject["type"] = "data-consumer";

		// Add timestamp.
		jsonObject["timestamp"] = base::Application::GetTimeMs();

		// Add label.
		jsonObject["label"] = this->label;

		// Add protocol.
		jsonObject["protocol"] = this->protocol;

		// Add messagesSent.
		jsonObject["messagesSent"] = this->messagesSent;

		// Add bytesSent.
		jsonObject["bytesSent"] = this->bytesSent;
	}

	void DataConsumer::HandleRequest(Channel::Request* request)
	{
		MS_TRACE();

		switch (request->methodId)
		{
			case Channel::Request::MethodId::DATA_CONSUMER_DUMP:
			{
				json data = json::object();

				FillJson(data);

				request->Accept(data);

				break;
			}

			case Channel::Request::MethodId::DATA_CONSUMER_GET_STATS:
			{
				json data = json::array();

				FillJsonStats(data);

				request->Accept(data);

				break;
			}

			default:
			{
				base::uv::throwError("unknown method " + request->method );
			}
		}
	}

	void DataConsumer::TransportConnected()
	{
		MS_TRACE();

		this->transportConnected = true;

		//MS_DEBUG_DEV("Transport connected [dataConsumerId:%s]", this->id.c_str());
		SInfo << "Transport connected dataConsumerId:" << this->id;
	}

	void DataConsumer::TransportDisconnected()
	{
		MS_TRACE();

		this->transportConnected = false;

	
		SInfo << "Transport disconnected dataConsumerId:" << this->id;
	}

	void DataConsumer::SctpAssociationConnected()
	{
		MS_TRACE();

		this->sctpAssociationConnected = true;

		SInfo << "SctpAssociation connected  dataConsumerId:" << this->id;
	}

	void DataConsumer::SctpAssociationClosed()
	{
		MS_TRACE();

		this->sctpAssociationConnected = false;

		//MS_DEBUG_DEV("SctpAssociation closed [dataConsumerId:%s]", this->id.c_str());
		SInfo << "SctpAssociation closed  dataConsumerId:" << this->id;
	}

	// The caller (Router) is supposed to proceed with the deletion of this DataConsumer
	// right after calling this method. Otherwise ugly things may happen.
	void DataConsumer::DataProducerClosed()
	{
		MS_TRACE();

		this->dataProducerClosed = true;

		MS_DEBUG_DEV("DataProducer closed [dataConsumerId:%s]", this->id.c_str());

		Channel::Notifier::Emit(this->id, "dataproducerclose");

		this->listener->OnDataConsumerDataProducerClosed(this);
	}

	void DataConsumer::SendSctpMessage(uint32_t ppid, const uint8_t* msg, size_t len)
	{
		MS_TRACE();
                
                SInfo << "SctpAssociation SendSctpMessage  dataConsumerId:" << this->id;

		if (!IsActive())
			return;

		if (len > this->maxSctpMessageSize)
		{
			MS_WARN_TAG(
			  sctp,
			  "given message exceeds maxSctpMessageSize value [maxSctpMessageSize:%zu, len:%zu]",
			  len,
			  this->maxSctpMessageSize);

			return;
		}

		this->messagesSent++;
		this->bytesSent += len;

		this->listener->OnDataConsumerSendSctpMessage(this, ppid, msg, len);
	}
} // namespace RTC
