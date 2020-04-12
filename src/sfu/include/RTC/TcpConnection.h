#ifndef MS_RTC_TCP_CONNECTION_HPP
#define MS_RTC_TCP_CONNECTION_HPP

#include "common.h"
#include "handles/TcpConnection.h"

namespace RTC
{
	class TcpConnection : public ::TcpConnection
	{
	public:
		class Listener
		{
		public:
			virtual void OnTcpConnectionPacketReceived(
			  RTC::TcpConnection* connection, const uint8_t* data, size_t len) = 0;
		};

	public:
		TcpConnection(Listener* listener, size_t bufferSize);
		~TcpConnection() override;

	public:
		void Send(const uint8_t* data, size_t len, ::TcpConnection::onSendCallback* cb);

		/* Pure virtual methods inherited from ::TcpConnection. */
	public:
		void UserOnTcpConnectionRead() override;

	private:
		// Passed by argument.
		Listener* listener{ nullptr };
		// Others.
		size_t frameStart{ 0 }; // Where the latest frame starts.
	};
} // namespace RTC

#endif
