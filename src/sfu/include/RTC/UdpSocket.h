#ifndef MS_RTC_UDP_SOCKET_HPP
#define MS_RTC_UDP_SOCKET_HPP

#include "common.h"
#include "net/UdpSocket.h"
#include <string>

namespace RTC
{
	class UdpSocket : public base::net::UdpSocket
	{
	public:
		class Listener
		{
		public:
			virtual void OnUdpSocketPacketReceived(
			  RTC::UdpSocket* socket, const uint8_t* data, size_t len, const struct sockaddr* remoteAddr) = 0;
		};
        protected:
	using onSendCallback = const std::function<void(bool sent)>;

	public:
		UdpSocket(Listener* listener, std::string& ip);
		~UdpSocket() override;
                
                void Send(const uint8_t* data, size_t len, const struct sockaddr* addr, onSendCallback* cb);

		/* Pure virtual methods inherited from ::UdpSocket. */
	public:
		void UserOnUdpDatagramReceived(const char* data, size_t len, struct sockaddr* addr) override;
                

        
	private:
		// Passed by argument.
		Listener* listener{ nullptr };
                onSendCallback* m_cb{ nullptr };
	};
} // namespace RTC

#endif
