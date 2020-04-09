#define MS_CLASS "Channel::Notifier"
// #define MS_LOG_DEV_LEVEL 3

#include "Channel/Notifier.h"
#include "LoggerTag.h"

namespace Channel
{
	/* Class variables. */

	Channel::UnixStreamSocket* Notifier::channel{ nullptr };
        base::wrtc::Signaler* Notifier::m_sig{ nullptr };
	/* Static methods. */

	void Notifier::ClassInit(Channel::UnixStreamSocket* channel, base::wrtc::Signaler *sig)
	{
		MS_TRACE();
                
                m_sig = sig;
		Notifier::channel = channel;
	}

	void Notifier::Emit(const std::string& targetId, const char* event)
	{
            
             if(!Notifier::channel)
            {
                json jsonNotification = json::object();

		jsonNotification["targetId"] = targetId;
		jsonNotification["event"]    = event;
                
                m_sig->postMessage(jsonNotification);
                return;
            }
             
		MS_TRACE();

		assertm(Notifier::channel != nullptr, "channel unset");

		json jsonNotification = json::object();

		jsonNotification["targetId"] = targetId;
		jsonNotification["event"]    = event;

		Notifier::channel->Send(jsonNotification);
	}

	void Notifier::Emit(const std::string& targetId, const char* event, json& data)
	{
            if(!Notifier::channel)
            {
                json jsonNotification = json::object();

		jsonNotification["targetId"] = targetId;
		jsonNotification["event"]    = event;
		jsonNotification["data"]     = data;
                
                m_sig->postMessage(jsonNotification);
                return;
            }
		MS_TRACE();

		assertm(Notifier::channel != nullptr, "channel unset");

		json jsonNotification = json::object();

		jsonNotification["targetId"] = targetId;
		jsonNotification["event"]    = event;
		jsonNotification["data"]     = data;

		Notifier::channel->Send(jsonNotification);
	}
} // namespace Channel
