#ifndef MS_RTC_TCP_CONNECTION_HPP
#define MS_RTC_TCP_CONNECTION_HPP

#include "common.h"
#include "net/netInterface.h"
#include "net/TcpConnection.h"

namespace RTC
{
	class TcpConnection : public base::net::TcpConnectionBase
	{
	public:
//		class Listener
//		{
//		public:
//			virtual void OnTcpConnectionPacketReceived(
//			  RTC::TcpConnection* connection, const uint8_t* data, size_t len) = 0;
//		};

	public:
		TcpConnection(Listener* listener);
		~TcpConnection() override;

	public:
                using onSendCallback = const std::function<void(bool sent)>;
		void Send(const uint8_t* data, size_t len, onSendCallback cb);

		/* Pure virtual methods inherited from ::TcpConnection. */
	public:
		//void UserOnTcpConnectionRead() override;
               void on_read( const char* data, size_t len) override;
               void on_close() override;
	 

        private:
            // Passed by argument.
            Listener* listener{ nullptr};
            // Others.
        private:
            size_t frameStart{ 0}; // Where the latest frame starts.

	};
} // namespace RTC

#endif
