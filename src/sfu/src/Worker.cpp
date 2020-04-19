
#include "Worker.h"
//#include "base/application.h"
#include "LoggerTag.h"
#include "base/error.h"
#include "Settings.h"
#include "Channel/Notifier.h"

/* Instance methods. */

Worker::Worker() //: channel(channel)
{

	// Set us as Channel's listener.
       // if(this->channel)
	//this->channel->SetListener(this);

	// Set the signals handler.
	this->signalsHandler = new SignalsHandler(this);

	// Add signals to handle.
	this->signalsHandler->AddSignal(SIGINT, "INT");
	this->signalsHandler->AddSignal(SIGTERM, "TERM");

	// Tell the Node process that we are running. // arvind
	//Channel::Notifier::Emit(std::to_string(Logger::pid), "running");


}

Worker::~Worker()
{
	

    if (!this->closed)
            Close();
}

void Worker::Close()
{
	

	if (this->closed)
		return;

	this->closed = true;

	// Delete the SignalsHandler.
	delete this->signalsHandler;

	// Delete all Routers.
	for (auto& kv : this->mapRouters)
	{
		auto* router = kv.second;

		delete router;
	}
	this->mapRouters.clear();

	// Close the Channel.
	//delete this->channel;
}

void Worker::FillJson(json& jsonObject) const
{
	const int64_t pid{ static_cast<int64_t>(uv_os_getpid()) }; //arvind

	// Add pid.
	jsonObject["pid"] = pid;

	// Add routerIds.
	jsonObject["routerIds"] = json::array();
	auto jsonRouterIdsIt    = jsonObject.find("routerIds");

	for (auto& kv : this->mapRouters)
	{
		auto& routerId = kv.first;

		jsonRouterIdsIt->emplace_back(routerId);
	}
}

void Worker::FillJsonResourceUsage(json& jsonObject) const
{
	

	int err;
	uv_rusage_t uvRusage;

	err = uv_getrusage(std::addressof(uvRusage));

	if (err != 0)
		base::uv::throwError("uv_getrusagerequest() failed:" , err);

	// Add ru_utime (uv_timeval_t, user CPU time used, converted to ms).
	jsonObject["ru_utime"] =
	  (uvRusage.ru_utime.tv_sec * static_cast<uint64_t>(1000)) + (uvRusage.ru_utime.tv_usec / 1000);

	// Add ru_stime (uv_timeval_t, system CPU time used, converted to ms).
	jsonObject["ru_stime"] =
	  (uvRusage.ru_stime.tv_sec * static_cast<uint64_t>(1000)) + (uvRusage.ru_stime.tv_usec / 1000);

	// Add ru_maxrss (uint64_t, maximum resident set size).
	jsonObject["ru_maxrss"] = uvRusage.ru_maxrss;

	// Add ru_ixrss (uint64_t, integral shared memory size).
	jsonObject["ru_ixrss"] = uvRusage.ru_ixrss;

	// Add ru_idrss (uint64_t, integral unshared data size).
	jsonObject["ru_idrss"] = uvRusage.ru_idrss;

	// Add ru_isrss (uint64_t, integral unshared stack size).
	jsonObject["ru_isrss"] = uvRusage.ru_isrss;

	// Add ru_minflt (uint64_t, page reclaims, soft page faults).
	jsonObject["ru_minflt"] = uvRusage.ru_minflt;

	// Add ru_majflt (uint64_t, page faults, hard page faults).
	jsonObject["ru_majflt"] = uvRusage.ru_majflt;

	// Add ru_nswap (uint64_t, swaps).
	jsonObject["ru_nswap"] = uvRusage.ru_nswap;

	// Add ru_inblock (uint64_t, block input operations).
	jsonObject["ru_inblock"] = uvRusage.ru_inblock;

	// Add ru_oublock (uint64_t, block output operations).
	jsonObject["ru_oublock"] = uvRusage.ru_oublock;

	// Add ru_msgsnd (uint64_t, IPC messages sent).
	jsonObject["ru_msgsnd"] = uvRusage.ru_msgsnd;

	// Add ru_msgrcv (uint64_t, IPC messages received).
	jsonObject["ru_msgrcv"] = uvRusage.ru_msgrcv;

	// Add ru_nsignals (uint64_t, signals received).
	jsonObject["ru_nsignals"] = uvRusage.ru_nsignals;

	// Add ru_nvcsw (uint64_t, voluntary context switches).
	jsonObject["ru_nvcsw"] = uvRusage.ru_nvcsw;

	// Add ru_nivcsw (uint64_t, involuntary context switches).
	jsonObject["ru_nivcsw"] = uvRusage.ru_nivcsw;
}

void Worker::SetNewRouterIdFromRequest(Channel::Request* request, std::string& routerId) const
{
	

	auto jsonRouterIdIt = request->internal.find("routerId");

	if (jsonRouterIdIt == request->internal.end() || !jsonRouterIdIt->is_string())
		base::uv::throwError("request has no internal.routerId");

	routerId.assign(jsonRouterIdIt->get<std::string>());

	if (this->mapRouters.find(routerId) != this->mapRouters.end())
		base::uv::throwError("a Router with same routerId already exists");
}

RTC::Router* Worker::GetRouterFromRequest(Channel::Request* request) const
{
	

	auto jsonRouterIdIt = request->internal.find("routerId");

	if (jsonRouterIdIt == request->internal.end() || !jsonRouterIdIt->is_string())
		base::uv::throwError("request has no internal.routerId");

	auto it = this->mapRouters.find(jsonRouterIdIt->get<std::string>());

	if (it == this->mapRouters.end())
		base::uv::throwError("Router not found");

	RTC::Router* router = it->second;

	return router;
}

 void Worker::OnChannelRequest(Channel::Request* request)
{
	MS_DEBUG_DEV(
	  "Channel request received method: ", request->method," id:" , request->id);

	switch (request->methodId)
	{
		case Channel::Request::MethodId::WORKER_DUMP:
		{
			json data = json::object();

			FillJson(data);

			request->Accept(data);

			break;
		}

		case Channel::Request::MethodId::WORKER_GET_RESOURCE_USAGE:
		{
			json data = json::object();

			FillJsonResourceUsage(data);

			request->Accept(data);

			break;
		}

		case Channel::Request::MethodId::WORKER_UPDATE_SETTINGS:
		{
			Settings::HandleRequest(request);

			break;
		}

		case Channel::Request::MethodId::WORKER_CREATE_ROUTER:
		{
			std::string routerId;

			// This may throw.
			SetNewRouterIdFromRequest(request, routerId);

			auto* router = new RTC::Router(routerId);

			this->mapRouters[routerId] = router;

			MS_DEBUG_DEV("Router created [routerId] ", routerId);

			request->Accept();

			break;
		}

		case Channel::Request::MethodId::ROUTER_CLOSE:
		{
			// This may throw.
			RTC::Router* router = GetRouterFromRequest(request);

			// Remove it from the map and delete it.
			this->mapRouters.erase(router->id);
			delete router;

			MS_DEBUG_DEV("Router closed [id:] ", router->id);

			request->Accept();

			break;
		}

		// Any other request must be delivered to the corresponding Router.
		default:
		{
			// This may throw.
			RTC::Router* router = GetRouterFromRequest(request);

			router->HandleRequest(request);

			break;
		}
	}
}

 void Worker::OnChannelClosed()
{
	MS_TRACE_STD();

	// If the pipe is remotely closed it may mean that sfuserver Node process
	// abruptly died (SIGKILL?) so we must die.
	MS_ERROR_STD("channel remotely closed, closing myself");

	Close();
}

 void Worker::OnSignal(SignalsHandler* /*signalsHandler*/, int signum)
{
	

	if (this->closed)
		return;

	switch (signum)
	{
		case SIGINT:
		{
			if (this->closed)
				return;

			MS_DEBUG_DEV("INT signal received, closing myself");

			Close();

			break;
		}

		case SIGTERM:
		{
			if (this->closed)
				return;

			MS_DEBUG_DEV("TERM signal received, closing myself");

			Close();

			break;
		}

		default:
		{
			MS_WARN_DEV("received a non handled signal [signum:] ", signum);
		}
	}
}
