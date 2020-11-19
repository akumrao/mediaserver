#include "RTC/UdpSocket.h"
#include "LoggerTag.h"
#include "net/PortManager.h"
#include <string>

namespace RTC
{
	/* Instance methods. */

	UdpSocket::UdpSocket(Listener* listener, std::string& ip)
	  : // This may throw.
	    base::net::UdpSocket::UdpSocket(base::net::PortManager::BindUdp(ip)), listener(listener)
	{
		
	}

	UdpSocket::~UdpSocket()
	{
		base::net::PortManager::UnbindUdp(this->localIp, this->localPort);
                LTrace( "rtc::~UdpSocket()")
	}

	void UdpSocket::UserOnUdpDatagramReceived(const char* data, size_t len, struct sockaddr* addr)
	{
		
                if (this->listener == nullptr)
		{
			MS_ERROR("no listener set");
			return;
		}

		// Notify the reader.
		this->listener->OnUdpSocketPacketReceived(this, (const uint8_t*)data, len, addr);
	}
        
        void UdpSocket::Send( const uint8_t* data, size_t len, const struct sockaddr* addr, onSendCallback cb)
        {
            

            int r = base::net::UdpSocket::send((const char*)data, len, addr); //arvind
            if(r==len && cb)
            {
                (cb)(true);
            }else if (cb)
                (cb)(false);

	}
        
} // namespace RTC
