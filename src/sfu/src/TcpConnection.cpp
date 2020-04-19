
#include "RTC/TcpConnection.h"
#include "LoggerTag.h"
#include "Utils.h"
#include <cstring> // std::memmove(), std::memcpy()

namespace RTC
{
	/* Static. */

	static constexpr size_t ReadBufferSize{ 65536 };
	static uint8_t ReadBuffer[ReadBufferSize];

	/* Instance methods. */

	TcpConnection::TcpConnection(Listener* listener)
	  : TcpConnectionBase(false), listener(listener)
	{
		
	}

	TcpConnection::~TcpConnection()
	{
		
	}

	void TcpConnection::on_read(const char*, size_t len)
	{

                SDebug << "data received local: " <<  GetLocalIp() << ":" << GetLocalPort() << "  remote: " <<  GetPeerIp() << ":" << GetPeerPort();
                
		/*
		 * Framing RFC 4571
		 *
		 *     0                   1                   2                   3
		 *     0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
		 *     ---------------------------------------------------------------
		 *     |             LENGTH            |  STUN / DTLS / RTP / RTCP   |
		 *     ---------------------------------------------------------------
		 *
		 * A 16-bit unsigned integer LENGTH field, coded in network byte order
		 * (big-endian), begins the frame.  If LENGTH is non-zero, an RTP or
		 * RTCP packet follows the LENGTH field.  The value coded in the LENGTH
		 * field MUST equal the number of octets in the RTP or RTCP packet.
		 * Zero is a valid value for LENGTH, and it codes the null packet.
		 */

               bufferDataLen += len;
		// Be ready to parse more than a single frame in a single TCP chunk.
		while (true)
		{
			// We may receive multiple packets in the same TCP chunk. If one of them is
			// a DTLS Close Alert this would be closed (Close() called) so we cannot call
			// our listeners anymore.
			if (IsClosed())
				return;

			size_t dataLen = bufferDataLen - frameStart;
			size_t packetLen;

			if (dataLen >= 2)
				packetLen = size_t{ Utils::Byte::Get2Bytes((const uint8_t*)buffer + frameStart, 0) };

			// We have packetLen bytes.
			if (dataLen >= 2 && dataLen >= 2 + packetLen)
			{
				const char* packet = buffer + frameStart + 2;

				// Update received bytes and notify the listener.
				if (packetLen != 0)
				{
					// Copy the received packet into the static buffer so it can be expanded
					// later.
					std::memcpy(ReadBuffer, packet, packetLen);

					listener->on_read(this, (const char*) ReadBuffer, packetLen);//arvind
				}

				// If there is no more space available in the buffer and that is because
				// the latest parsed frame filled it, then empty the full buffer.
				if ((frameStart + 2 + packetLen) == bufferSize)
				{
					MS_DEBUG_DEV("no more space in the buffer, emptying the buffer data");

					frameStart    = 0;
					bufferDataLen = 0;
				}
				// If there is still space in the buffer, set the beginning of the next
				// frame to the next position after the parsed frame.
				else
				{
					frameStart += 2 + packetLen;
				}

				// If there is more data in the buffer after the parsed frame then
				// parse again. Otherwise break here and wait for more data.
				if (bufferDataLen > frameStart)
				{
					MS_DEBUG_DEV("there is more data after the parsed frame, continue parsing");

					continue;
				}

				break;
			}

			// Incomplete packet.

			// Check if the buffer is full.
			if (bufferDataLen == bufferSize)
			{
				// First case: the incomplete frame does not begin at position 0 of
				// the buffer, so move the frame to the position 0.
				if (frameStart != 0)
				{
					MS_DEBUG_DEV(
					  "no more space in the buffer, moving parsed bytes to the beginning of "
					  "the buffer and wait for more data");

					std::memmove(
					  buffer, buffer + frameStart, bufferSize - frameStart);
					bufferDataLen = bufferSize - frameStart;
					frameStart    = 0;
				}
				// Second case: the incomplete frame begins at position 0 of the buffer.
				// The frame is too big, so close the connection.
				else
				{
					MS_WARN_DEV(
					  "no more space in the buffer for the unfinished frame being parsed, closing the "
					  "connection");

					// Close the socket.
					ErrorReceiving();

					// And exit fast since we are supposed to be deallocated.
					return;
				}
			}
			// The buffer is not full.
			else
			{
				MS_DEBUG_DEV("frame not finished yet, waiting for more data");
			}

			// Exit the parsing loop.
			break;
		}
	}

	void TcpConnection::Send(const uint8_t* data, size_t len, onSendCallback* cb)
	{
		 SDebug << len <<  " len send: " <<  GetLocalIp() << ":" << GetLocalPort() << "  remote: " <<  GetPeerIp() << ":" << GetPeerPort();
                 
//                if (cb)
//                {
//                   m_cb =cb;
//                }
                 
		// Write according to Framing RFC 4571.

		uint8_t frameLen[2];

		Utils::Byte::Set2Bytes(frameLen, 0, len);
		int r = base::net::TcpConnectionBase::Write((const char*)frameLen, 2, (const char*) data, len); //arvind
                if(r==len && cb)
                {
                    (*cb)(true);
                }else if (cb)
                    (*cb)(false);

	}
        
         void TcpConnection::on_close() {

            listener->on_close(this);
        }
        
        
} // namespace RTC
