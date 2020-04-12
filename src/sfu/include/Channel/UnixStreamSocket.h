#ifndef MS_CHANNEL_UNIX_STREAM_SOCKET_HPP
#define MS_CHANNEL_UNIX_STREAM_SOCKET_HPP

#include "common.h"
#include "Channel/Request.h"
#include "handles/UnixStreamSocket.h"
#include <json.hpp>

namespace Channel
{
	class ConsumerSocket : public ::UnixStreamSocket
	{
	public:
		class Listener
		{
		public:
			virtual void OnConsumerSocketMessage(ConsumerSocket* consumerSocket, json& jsonMessage) = 0;
			virtual void OnConsumerSocketClosed(ConsumerSocket* consumerSocket)                     = 0;
		};

	public:
		ConsumerSocket(int fd, size_t bufferSize, Listener* listener);
		/* Pure virtual methods inherited from ::UnixStreamSocket. */
	public:
		void UserOnUnixStreamRead() override;
		void UserOnUnixStreamSocketClosed() override;

	private:
		// Passed by argument.
		Listener* listener{ nullptr };
		// Others.
		size_t msgStart{ 0 }; // Where the latest message starts.
	};

	class ProducerSocket : public ::UnixStreamSocket
	{
	public:
		ProducerSocket(int fd, size_t bufferSize);
		/* Pure virtual methods inherited from ::UnixStreamSocket. */
	public:
		void UserOnUnixStreamRead() override
		{
		}
		void UserOnUnixStreamSocketClosed() override
		{
		}
	};

	class UnixStreamSocket : public ConsumerSocket::Listener
	{
	public:
		class Listener
		{
		public:
			virtual void OnChannelRequest(Channel::UnixStreamSocket* channel, Channel::Request* request) = 0;
			virtual void OnChannelClosed(Channel::UnixStreamSocket* channel) = 0;
		};

	public:
		explicit UnixStreamSocket(int consumerFd, int producerFd);
		virtual ~UnixStreamSocket();

	public:
		void SetListener(Listener* listener);
		void Send(json& jsonMessage);
		void SendLog(char* nsPayload, size_t nsPayloadLen);
		void SendBinary(const uint8_t* nsPayload, size_t nsPayloadLen);

		/* Pure virtual methods inherited from ConsumerSocket::Listener. */
	public:
		void OnConsumerSocketMessage(ConsumerSocket* consumerSocket, json& jsonMessage) override;
		void OnConsumerSocketClosed(ConsumerSocket* consumerSocket) override;

	private:
		// Passed by argument.
		Listener* listener{ nullptr };
		// Others.
		ConsumerSocket consumerSocket;
		ProducerSocket producerSocket;
	};
} // namespace Channel

#endif
