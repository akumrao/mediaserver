#define MS_CLASS "Channel::Notifier"
// #define MS_LOG_DEV_LEVEL 3

#include "Channel/Notifier.h"
#include "LoggerTag.h"

namespace Channel
{
	/* Class variables. */


        SdpParse::Signaler* Notifier::m_sig{ nullptr };
	/* Static methods. */

	void Notifier::ClassInit( SdpParse::Signaler *sig)
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
                
                m_sig->postAppMessage(jsonNotification);
                return;
   
	}

	void Notifier::Emit(const std::string& targetId, const char* event, json& data)
	{       
                STrace <<  "Emit: " << targetId << " " << data.dump(4);

                json jsonNotification = json::object();

				//jsonNotification["targetId"] = targetId;
                jsonNotification["event"]    = event;
                jsonNotification["desc"]     = data;
		                
                if( std::string(event) == std::string("volumes"))
                {
                    jsonNotification["roomid"]= targetId;
                    jsonNotification["type"]= "soundlevel";
                }
                else
                {
                    int sizeit = m_sig->mapNotification.size();
                    
                    if( m_sig->mapNotification.find(targetId) != m_sig->mapNotification.end())
                    {
                        jsonNotification["roomid"]= m_sig->mapNotification[targetId];
                        jsonNotification["type"]= event;
                    }
                
                }
                
                
                
                m_sig->postAppMessage(jsonNotification);
                return;
		
	}
} // namespace Channel
