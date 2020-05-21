#define MS_CLASS "Channel::Request"
// #define MS_LOG_DEV_LEVEL 3

#include "Channel/Request.h"
#include "LoggerTag.h"
#include "base/error.h"
#include "Utils.h"
#include "json/configuration.h"

namespace Channel
{
	/* Class variables. */

	
	std::unordered_map<std::string, Request::MethodId> Request::string2MethodId =
	{
		{ "worker.dump",                     Request::MethodId::WORKER_DUMP                        },
		{ "worker.getResourceUsage",         Request::MethodId::WORKER_GET_RESOURCE_USAGE          },
		{ "worker.updateSettings",           Request::MethodId::WORKER_UPDATE_SETTINGS             },
		{ "worker.createRouter",             Request::MethodId::WORKER_CREATE_ROUTER               },
		{ "router.close",                    Request::MethodId::ROUTER_CLOSE                       },
		{ "router.dump",                     Request::MethodId::ROUTER_DUMP                        },
		{ "router.createWebRtcTransport",    Request::MethodId::ROUTER_CREATE_WEBRTC_TRANSPORT     },
		{ "router.createPlainRtpTransport",  Request::MethodId::ROUTER_CREATE_PLAIN_RTP_TRANSPORT  },
		{ "router.createPipeTransport",      Request::MethodId::ROUTER_CREATE_PIPE_TRANSPORT       },
		{ "router.createAudioLevelObserver", Request::MethodId::ROUTER_CREATE_AUDIO_LEVEL_OBSERVER },
		{ "transport.close",                 Request::MethodId::TRANSPORT_CLOSE                    },
		{ "transport.dump",                  Request::MethodId::TRANSPORT_DUMP                     },
		{ "transport.getStats",              Request::MethodId::TRANSPORT_GET_STATS                },
		{ "transport.connect",               Request::MethodId::TRANSPORT_CONNECT                  },
		{ "transport.setMaxIncomingBitrate", Request::MethodId::TRANSPORT_SET_MAX_INCOMING_BITRATE },
		{ "transport.restartIce",            Request::MethodId::TRANSPORT_RESTART_ICE              },
		{ "transport.produce",               Request::MethodId::TRANSPORT_PRODUCE                  },
		{ "transport.consume",               Request::MethodId::TRANSPORT_CONSUME                  },
		{ "transport.produceData",           Request::MethodId::TRANSPORT_PRODUCE_DATA             },
		{ "transport.consumeData",           Request::MethodId::TRANSPORT_CONSUME_DATA             },
		{ "transport.enableTraceEvent",      Request::MethodId::TRANSPORT_ENABLE_TRACE_EVENT       },
		{ "producer.close",                  Request::MethodId::PRODUCER_CLOSE                     },
		{ "producer.dump",                   Request::MethodId::PRODUCER_DUMP                      },
		{ "producer.getStats",               Request::MethodId::PRODUCER_GET_STATS                 },
		{ "producer.pause",                  Request::MethodId::PRODUCER_PAUSE                     },
		{ "producer.resume" ,                Request::MethodId::PRODUCER_RESUME                    },
		{ "producer.enableTraceEvent",       Request::MethodId::PRODUCER_ENABLE_TRACE_EVENT        },
		{ "consumer.close",                  Request::MethodId::CONSUMER_CLOSE                     },
		{ "consumer.dump",                   Request::MethodId::CONSUMER_DUMP                      },
		{ "consumer.getStats",               Request::MethodId::CONSUMER_GET_STATS                 },
		{ "consumer.pause",                  Request::MethodId::CONSUMER_PAUSE                     },
		{ "consumer.resume",                 Request::MethodId::CONSUMER_RESUME                    },
		{ "consumer.setPreferredLayers",     Request::MethodId::CONSUMER_SET_PREFERRED_LAYERS      },
		{ "consumer.setPriority",            Request::MethodId::CONSUMER_SET_PRIORITY              },
		{ "consumer.requestKeyFrame",        Request::MethodId::CONSUMER_REQUEST_KEY_FRAME         },
		{ "consumer.enableTraceEvent",       Request::MethodId::CONSUMER_ENABLE_TRACE_EVENT        },
		{ "dataProducer.close",              Request::MethodId::DATA_PRODUCER_CLOSE                },
		{ "dataProducer.dump",               Request::MethodId::DATA_PRODUCER_DUMP                 },
		{ "dataProducer.getStats",           Request::MethodId::DATA_PRODUCER_GET_STATS            },
		{ "dataConsumer.close",              Request::MethodId::DATA_CONSUMER_CLOSE                },
		{ "dataConsumer.dump",               Request::MethodId::DATA_CONSUMER_DUMP                 },
		{ "dataConsumer.getStats",           Request::MethodId::DATA_CONSUMER_GET_STATS            },
		{ "rtpObserver.close",               Request::MethodId::RTP_OBSERVER_CLOSE                 },
		{ "rtpObserver.pause",               Request::MethodId::RTP_OBSERVER_PAUSE                 },
		{ "rtpObserver.resume",              Request::MethodId::RTP_OBSERVER_RESUME                },
		{ "rtpObserver.addProducer",         Request::MethodId::RTP_OBSERVER_ADD_PRODUCER          },
		{ "rtpObserver.removeProducer",      Request::MethodId::RTP_OBSERVER_REMOVE_PRODUCER       }
	};
	

	/* Instance methods. */

	Request::Request(json& jsonRequest, std::function<void (json& )> func): cb(func) //: channel(channel)
	{
		MS_TRACE();

		auto jsonIdIt = jsonRequest.find("id");

		if (jsonIdIt == jsonRequest.end() || !Utils::Json::IsPositiveInteger(*jsonIdIt))
			base::uv::throwError("missing id");

		this->id = jsonIdIt->get<uint32_t>();

		auto jsonMethodIt = jsonRequest.find("method");

		if (jsonMethodIt == jsonRequest.end() || !jsonMethodIt->is_string())
			base::uv::throwError("missing method");

		this->method = jsonMethodIt->get<std::string>();

		auto methodIdIt = Request::string2MethodId.find(this->method);

		if (methodIdIt == Request::string2MethodId.end())
		{
			Error("unknown method");

			base::uv::throwError("unknown method " + this->method);
		}

		this->methodId = methodIdIt->second;

		auto jsonInternalIt = jsonRequest.find("internal");

		if (jsonInternalIt != jsonRequest.end() && jsonInternalIt->is_object())
			this->internal = *jsonInternalIt;
		else
			this->internal = json::object();

		auto jsonDataIt = jsonRequest.find("data");

		if (jsonDataIt != jsonRequest.end() && jsonDataIt->is_object())
			this->data = *jsonDataIt;
		else
			this->data = json::object();
	}

	Request::~Request()
	{
		MS_TRACE();
	}

	void Request::Accept()
	{
		MS_TRACE();

		assertm(!this->replied, "request already replied");

		this->replied = true;

		json jsonResponse = json::object();

		jsonResponse["id"]       = this->id;
		jsonResponse["accepted"] = true;

                if(cb)
                cb(jsonResponse );
		//this->channel->Send(jsonResponse);
	}

	void Request::Accept(json& data)
	{
		MS_TRACE();

		assertm(!this->replied, "request already replied");

		this->replied = true;

		json jsonResponse = json::object();

		jsonResponse["id"]       = this->id;
		jsonResponse["accepted"] = true;

		if (data.is_structured())
		jsonResponse["data"] = data;
                if(cb) 
                cb(jsonResponse );
	}

	void Request::Error(const char* reason)
	{
		MS_TRACE();

		assertm(!this->replied, "request already replied");

		this->replied = true;

		json jsonResponse = json::object();

		jsonResponse["id"]    = this->id;
		jsonResponse["error"] = "Error";

		if (reason != nullptr)
			jsonResponse["reason"] = reason;
                
		SError << jsonResponse.dump(4);
                 
                cb(jsonResponse );
                
	}

	void Request::TypeError(const char* reason)
	{
		MS_TRACE();

		assertm(!this->replied, "request already replied");

		this->replied = true;

		json  jsonResponse = json::object();

		jsonResponse["id"]    = this->id;
		jsonResponse["error"] = "TypeError";

		if (reason != nullptr)
			jsonResponse["reason"] = reason;

                SError << jsonResponse.dump(4);
                 
                cb(jsonResponse );
	}
} // namespace Channel
