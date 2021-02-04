#ifndef MS_WORKER_HPP
#define MS_WORKER_HPP

#include "common.h"
#include "Channel/Request.h"
//#include "Channel/UnixStreamSocket.h"
#include "RTC/Router.h"
#include "base/signalsHandler.h"
#include <json.hpp>
#include <string>
#include <unordered_map>

#include "base/thread.h"
#include "base/queue.h"

using json = nlohmann::json;

class Worker : public base::Thread
{
public:
	 Worker();
	~Worker();

     
        base::Queue<Channel::Request*> qEvent;
        
        inline void  dispatch();
private:
	void Close();
       	void FillJson(json& jsonObject) const;
	void FillJsonResourceUsage(json& jsonObject) const;
	void SetNewRouterIdFromRequest(Channel::Request* request, std::string& routerId) const;
	RTC::Router* GetRouterFromRequest(Channel::Request* request) const;
        void OnChannelRequest(Channel::Request* request);
        //void dispatch(T& item);

	/* Methods inherited from Channel::lUnixStreamSocket::Listener. */
public:
        void run();
	void OnChannelClosed() ;

	/* Methods inherited from SignalsHandler::Listener. */
public:
	//void OnSignal(SignalsHandler* signalsHandler, int signum) override;
        std::unordered_map<std::string, RTC::Router*> mapRouters;

private:
	// Passed by argument.
	//Channel::UnixStreamSocket* channel{ nullptr };
	// Allocated by this.
	//SignalsHandler* signalsHandler{ nullptr };
	
	// Others.
	bool closed{ false };
        
        uv_idle_t *idler{nullptr};
        
        std::mutex _mutex;
   
};

#endif
