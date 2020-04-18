#define MS_CLASS "Channel::Notifier"
// #define MS_LOG_DEV_LEVEL 3

#include "Channel/Notifier.h"
#include "LoggerTag.h"

namespace Channel
{
	/* Class variables. */


        base::wrtc::Signaler* Notifier::m_sig{ nullptr };
	/* Static methods. */

	void Notifier::ClassInit( base::wrtc::Signaler *sig)
	{
		MS_TRACE();
                
                m_sig = sig;
//		Notifier::channel = channel;
	}

	void Notifier::Emit(const std::string& targetId, const char* event)
	{
            

                json jsonNotification = json::object();

		jsonNotification["targetId"] = targetId;
		jsonNotification["event"]    = event;
                
                m_sig->postMessage(jsonNotification);
                return;

             
	}

	void Notifier::Emit(const std::string& targetId, const char* event, json& data)
	{

                json jsonNotification = json::object();

		jsonNotification["targetId"] = targetId;
		jsonNotification["event"]    = event;
		jsonNotification["data"]     = data;
                
                m_sig->postMessage(jsonNotification);
                return;
		
	}
} // namespace Channel
