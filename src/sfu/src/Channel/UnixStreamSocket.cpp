#define MS_CLASS "Channel::UnixStreamSocket"
// #define MS_LOG_DEV_LEVEL 3

#include "Channel/UnixStreamSocket.h"
#include "LoggerTag.h"
#include "base/error.h"
#include <cmath>   // std::ceil()
#include <cstdio>  // sprintf()
#include <cstring> // std::memcpy(), std::memmove()

#include "netstring.h"
int netstring_read1(char *buffer, size_t buffer_length,
		   char **netstring_start, size_t *netstring_length) {
  int i;
  size_t len = 0;

  /* Write default values for outputs */
  *netstring_start = NULL; *netstring_length = 0;

  /* Make sure buffer is big enough. Minimum size is 3. */
  if (buffer_length < 3) return NETSTRING_ERROR_TOO_SHORT;

  /* No leading zeros allowed! */
  if (buffer[0] == '0' && isdigit(buffer[1]))
    return NETSTRING_ERROR_LEADING_ZERO;

  /* The netstring must start with a number */
  if (!isdigit(buffer[0])) return NETSTRING_ERROR_NO_LENGTH;

  /* Read the number of bytes */
  for (i = 0; i < buffer_length && isdigit(buffer[i]); i++) {
    /* Error if more than 9 digits */
    if (i >= 9) return NETSTRING_ERROR_TOO_LONG;
    /* Accumulate each digit, assuming ASCII. */
    len = len*10 + (buffer[i] - '0');
  }

  /* Check buffer length once and for all. Specifically, we make sure
     that the buffer is longer than the number we've read, the length
     of the string itself, and the colon and comma. */
  if (i + len + 1 >= buffer_length) return NETSTRING_ERROR_TOO_SHORT;

  /* Read the colon */
  if (buffer[i++] != ':') return NETSTRING_ERROR_NO_COLON;
  
  /* Test for the trailing comma, and set the return values */
  if (buffer[i + len] != ',') return NETSTRING_ERROR_NO_COMMA;
  *netstring_start = &buffer[i]; *netstring_length = len;

  return 0;
}


namespace Channel
{
	/* Static. */

	// netstring length for a 4194304 bytes payload.
	static constexpr size_t NsMessageMaxLen{ 4194313 };
	static constexpr size_t NsPayloadMaxLen{ 4194304 };
	static uint8_t WriteBuffer[NsMessageMaxLen];

	/* Instance methods. */
	UnixStreamSocket::UnixStreamSocket(int consumerFd, int producerFd)
	  : consumerSocket(consumerFd, NsMessageMaxLen, this), producerSocket(producerFd, NsMessageMaxLen)
	{
		MS_TRACE_STD();
	}

	UnixStreamSocket ::~UnixStreamSocket()
	{
		MS_TRACE();
	}

	void UnixStreamSocket::SetListener(Listener* listener)
	{
		MS_TRACE_STD();

		this->listener = listener;
	}

	void UnixStreamSocket::Send(json& jsonMessage)
	{
		if (this->producerSocket.IsClosed())
			return;

		std::string nsPayload = jsonMessage.dump();
		size_t nsPayloadLen   = nsPayload.length();
		size_t nsNumLen;
		size_t nsLen;

		if (nsPayloadLen > NsPayloadMaxLen)
		{
			MS_ERROR_STD("mesage too big");

			return;
		}

		if (nsPayloadLen == 0)
		{
			nsNumLen       = 1;
			WriteBuffer[0] = '0';
			WriteBuffer[1] = ':';
			WriteBuffer[2] = ',';
		}
		else
		{
			nsNumLen = static_cast<size_t>(std::ceil(std::log10(static_cast<double>(nsPayloadLen) + 1)));
			std::sprintf(reinterpret_cast<char*>(WriteBuffer), "%zu:", nsPayloadLen);
			std::memcpy(WriteBuffer + nsNumLen + 1, nsPayload.c_str(), nsPayloadLen);
			WriteBuffer[nsNumLen + nsPayloadLen + 1] = ',';
		}

		nsLen = nsNumLen + nsPayloadLen + 2;

		this->producerSocket.Write(WriteBuffer, nsLen);
	}

	void UnixStreamSocket::SendLog(char* nsPayload, size_t nsPayloadLen)
	{
		if (this->producerSocket.IsClosed())
			return;

		// MS_TRACE_STD();

		size_t nsNumLen;
		size_t nsLen;

		if (nsPayloadLen > NsPayloadMaxLen)
		{
			MS_ERROR_STD("mesage too big");

			return;
		}

		if (nsPayloadLen == 0)
		{
			nsNumLen       = 1;
			WriteBuffer[0] = '0';
			WriteBuffer[1] = ':';
			WriteBuffer[2] = ',';
		}
		else
		{
			nsNumLen = static_cast<size_t>(std::ceil(std::log10(static_cast<double>(nsPayloadLen) + 1)));
			std::sprintf(reinterpret_cast<char*>(WriteBuffer), "%zu:", nsPayloadLen);
			std::memcpy(WriteBuffer + nsNumLen + 1, nsPayload, nsPayloadLen);
			WriteBuffer[nsNumLen + nsPayloadLen + 1] = ',';
		}

		nsLen = nsNumLen + nsPayloadLen + 2;

		this->producerSocket.Write(WriteBuffer, nsLen);
	}

	void UnixStreamSocket::SendBinary(const uint8_t* nsPayload, size_t nsPayloadLen)
	{
		if (this->producerSocket.IsClosed())
			return;

		size_t nsNumLen;
		size_t nsLen;

		if (nsPayloadLen > NsPayloadMaxLen)
		{
			MS_ERROR_STD("mesage too big");

			return;
		}

		if (nsPayloadLen == 0)
		{
			nsNumLen       = 1;
			WriteBuffer[0] = '0';
			WriteBuffer[1] = ':';
			WriteBuffer[2] = ',';
		}
		else
		{
			nsNumLen = static_cast<size_t>(std::ceil(std::log10(static_cast<double>(nsPayloadLen) + 1)));
			std::sprintf(reinterpret_cast<char*>(WriteBuffer), "%zu:", nsPayloadLen);
			std::memcpy(WriteBuffer + nsNumLen + 1, nsPayload, nsPayloadLen);
			WriteBuffer[nsNumLen + nsPayloadLen + 1] = ',';
		}

		nsLen = nsNumLen + nsPayloadLen + 2;

		this->producerSocket.Write(WriteBuffer, nsLen);
	}

	void UnixStreamSocket::OnConsumerSocketMessage(ConsumerSocket* /*consumerSocket*/, json& jsonMessage)
	{
		try
		{
			auto* request = new Channel::Request(this, jsonMessage);

			// Notify the listener.
			try
			{
				this->listener->OnChannelRequest(this, request);
			}
			catch (const std::exception& error)
			{
				request->TypeError(error.what());
			}


			// Delete the Request.
			delete request;
		}
		catch (const std::exception& error)
		{
			MS_ERROR_STD("discarding wrong Channel request");
		}
	}

	void UnixStreamSocket::OnConsumerSocketClosed(ConsumerSocket* /*consumerSocket*/)
	{
		this->listener->OnChannelClosed(this);
	}

	ConsumerSocket::ConsumerSocket(int fd, size_t bufferSize, Listener* listener)
	  : ::UnixStreamSocket(fd, bufferSize, ::UnixStreamSocket::Role::CONSUMER), listener(listener)
	{
		MS_TRACE_STD();
	}

	void ConsumerSocket::UserOnUnixStreamRead()
	{
		MS_TRACE_STD();

		// Be ready to parse more than a single message in a single TCP chunk.
		while (true)
		{
			if (IsClosed())
				return;

			size_t readLen  = this->bufferDataLen - this->msgStart;
			char* jsonStart = nullptr;
			size_t jsonLen;

			//int static netstring_read(char *buffer, size_t buffer_length,
		  // char **netstring_start, size_t *netstring_length) {

			//int nsRet = netstring_read( nullptr , readLen, &jsonStart, &jsonLen);
			int nsRet = netstring_read1(
			  reinterpret_cast<char*>(this->buffer + this->msgStart), readLen, &jsonStart, &jsonLen);

			if (nsRet != 0)
			{
				switch (nsRet)
				{
					case NETSTRING_ERROR_TOO_SHORT:
					{
						// Check if the buffer is full.
						if (this->bufferDataLen == this->bufferSize)
						{
							// First case: the incomplete message does not begin at position 0 of
							// the buffer, so move the incomplete message to the position 0.
							if (this->msgStart != 0)
							{
								std::memmove(this->buffer, this->buffer + this->msgStart, readLen);
								this->msgStart      = 0;
								this->bufferDataLen = readLen;
							}
							// Second case: the incomplete message begins at position 0 of the buffer.
							// The message is too big, so discard it.
							else
							{
								MS_ERROR_STD(
								  "no more space in the buffer for the unfinished message being parsed, "
								  "discarding it");

								this->msgStart      = 0;
								this->bufferDataLen = 0;
							}
						}

						// Otherwise the buffer is not full, just wait.
						return;
					}

					case NETSTRING_ERROR_TOO_LONG:
					{
						MS_ERROR_STD("NETSTRING_ERROR_TOO_LONG");

						break;
					}

					case NETSTRING_ERROR_NO_COLON:
					{
						MS_ERROR_STD("NETSTRING_ERROR_NO_COLON");

						break;
					}

					case NETSTRING_ERROR_NO_COMMA:
					{
						MS_ERROR_STD("NETSTRING_ERROR_NO_COMMA");

						break;
					}

					case NETSTRING_ERROR_LEADING_ZERO:
					{
						MS_ERROR_STD("NETSTRING_ERROR_LEADING_ZERO");

						break;
					}

					case NETSTRING_ERROR_NO_LENGTH:
					{
						MS_ERROR_STD("NETSTRING_ERROR_NO_LENGTH");

						break;
					}
				}

				// Error, so reset and exit the parsing loop.
				this->msgStart      = 0;
				this->bufferDataLen = 0;

				return;
			}

			// If here it means that jsonStart points to the beginning of a JSON string
			// with jsonLen bytes length, so recalculate readLen.
			readLen =
			  reinterpret_cast<const uint8_t*>(jsonStart) - (this->buffer + this->msgStart) + jsonLen + 1;

			try
			{
				json jsonMessage = json::parse(jsonStart, jsonStart + jsonLen);

				// Notify the listener.
				this->listener->OnConsumerSocketMessage(this, jsonMessage);
			}
			catch (const json::parse_error& error)
			{
				MS_ERROR_STD("JSON parsing error: %s", error.what());
			}

			// If there is no more space available in the buffer and that is because
			// the latest parsed message filled it, then empty the full buffer.
			if ((this->msgStart + readLen) == this->bufferSize)
			{
				this->msgStart      = 0;
				this->bufferDataLen = 0;
			}
			// If there is still space in the buffer, set the beginning of the next
			// parsing to the next position after the parsed message.
			else
			{
				this->msgStart += readLen;
			}

			// If there is more data in the buffer after the parsed message
			// then parse again. Otherwise break here and wait for more data.
			if (this->bufferDataLen > this->msgStart)
			{
				continue;
			}

			break;
		}
	}

	void ConsumerSocket::UserOnUnixStreamSocketClosed()
	{
		MS_TRACE_STD();

		// Notify the listener.
		this->listener->OnConsumerSocketClosed(this);
	}

	ProducerSocket::ProducerSocket(int fd, size_t bufferSize)
	  : ::UnixStreamSocket(fd, bufferSize, ::UnixStreamSocket::Role::PRODUCER)
	{
		MS_TRACE_STD();
	}
} // namespace Channel
