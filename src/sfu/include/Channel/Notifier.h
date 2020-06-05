#ifndef MS_CHANNEL_NOTIFIER_HPP
#define MS_CHANNEL_NOTIFIER_HPP

#include "common.h"
//#include "Channel/UnixStreamSocket.h"
#include <json.hpp>
#include <string>
#include "sdp/signaler.h"

namespace Channel
{
	class Notifier
	{
	public:
		static void ClassInit( SdpParse::Signaler *sig);
		static void Emit(const std::string& targetId, const char* event);
		static void Emit(const std::string& targetId, const char* event, json& data);

	public:
		// Passed by argument.
		//static Channel::UnixStreamSocket* channel;
                static  SdpParse::Signaler *m_sig;
	};
} // namespace Channel

#endif
