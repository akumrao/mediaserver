#define MS_CLASS "TcpConnection"
// #define MS_LOG_DEV_LEVEL 3

#include "handles/TcpConnection.h"
#include "base/application.h"
#include "LoggerTag.h"
#include "base/error.h"
#include "Utils.h"
#include <cstring> // std::memcpy()

/* Static methods for UV callbacks. */

inline static void onAlloc(uv_handle_t* handle, size_t suggestedSize, uv_buf_t* buf)
{
	auto* connection = static_cast<TcpConnection*>(handle->data);

	if (connection)
		connection->OnUvReadAlloc(suggestedSize, buf);
}

inline static void onRead(uv_stream_t* handle, ssize_t nread, const uv_buf_t* buf)
{
	auto* connection = static_cast<TcpConnection*>(handle->data);

	if (connection)
		connection->OnUvRead(nread, buf);
}

inline static void onWrite(uv_write_t* req, int status)
{
	auto* writeData  = static_cast<TcpConnection::UvWriteData*>(req->data);
	auto* handle     = req->handle;
	auto* connection = static_cast<TcpConnection*>(handle->data);
	auto* cb         = writeData->cb;

	if (connection)
		connection->OnUvWrite(status, cb);

	// Delete the UvWriteData struct and the cb.
	delete writeData;
}

inline static void onClose(uv_handle_t* handle)
{
	delete handle;
}

inline static void onShutdown(uv_shutdown_t* req, int /*status*/)
{
	auto* handle = req->handle;

	delete req;

	// Now do close the handle.
	uv_close(reinterpret_cast<uv_handle_t*>(handle), static_cast<uv_close_cb>(onClose));
}

/* Instance methods. */

// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
TcpConnection::TcpConnection(size_t bufferSize) : bufferSize(bufferSize)
{
	MS_TRACE();

	this->uvHandle       = new uv_tcp_t;
	this->uvHandle->data = static_cast<void*>(this);

	// NOTE: Don't allocate the buffer here. Instead wait for the first uv_alloc_cb().
}

TcpConnection::~TcpConnection()
{
	MS_TRACE();

	if (!this->closed)
		Close();

	delete[] this->buffer;
}

void TcpConnection::Close()
{
	MS_TRACE();

	if (this->closed)
		return;

	int err;

	this->closed = true;

	// Tell the UV handle that the TcpConnection has been closed.
	this->uvHandle->data = nullptr;

	// Don't read more.
	err = uv_read_stop(reinterpret_cast<uv_stream_t*>(this->uvHandle));

	if (err != 0)
		MS_ABORT("uv_read_stop() failed: %s", uv_strerror(err));

	// If there is no error and the peer didn't close its connection side then close gracefully.
	if (!this->hasError && !this->isClosedByPeer)
	{
		// Use uv_shutdown() so pending data to be written will be sent to the peer
		// before closing.
		auto req  = new uv_shutdown_t;
		req->data = static_cast<void*>(this);
		err       = uv_shutdown(
      req, reinterpret_cast<uv_stream_t*>(this->uvHandle), static_cast<uv_shutdown_cb>(onShutdown));

		if (err != 0)
			MS_ABORT("uv_shutdown() failed: %s", uv_strerror(err));
	}
	// Otherwise directly close the socket.
	else
	{
		uv_close(reinterpret_cast<uv_handle_t*>(this->uvHandle), static_cast<uv_close_cb>(onClose));
	}
}

void TcpConnection::Dump() const
{
	MS_DUMP("<TcpConnection>");
	MS_DUMP("  localIp    : %s", this->localIp.c_str());
	MS_DUMP("  localPort  : %" PRIu16, static_cast<uint16_t>(this->localPort));
	MS_DUMP("  remoteIp   : %s", this->peerIp.c_str());
	MS_DUMP("  remotePort : %" PRIu16, static_cast<uint16_t>(this->peerPort));
	MS_DUMP("  closed     : %s", !this->closed ? "open" : "closed");
	MS_DUMP("</TcpConnection>");
}

void TcpConnection::Setup(
  Listener* listener, struct sockaddr_storage* localAddr, const std::string& localIp, uint16_t localPort)
{
	MS_TRACE();

	// Set the UV handle.
	int err = uv_tcp_init(base::Application::uvGetLoop(), this->uvHandle);

	if (err != 0)
	{
		delete this->uvHandle;
		this->uvHandle = nullptr;

		base::uv::throwError("uv_tcp_init() failed:", err);
	}

	// Set the listener.
	this->listener = listener;

	// Set the local address.
	this->localAddr = localAddr;
	this->localIp   = localIp;
	this->localPort = localPort;
}

void TcpConnection::Start()
{
	MS_TRACE();

	if (this->closed)
		return;

	int err = uv_read_start(
	  reinterpret_cast<uv_stream_t*>(this->uvHandle),
	  static_cast<uv_alloc_cb>(onAlloc),
	  static_cast<uv_read_cb>(onRead));

	if (err != 0)
		base::uv::throwError("uv_read_start() failed: ", err);

	// Get the peer address.
	if (!SetPeerAddress())
		base::uv::throwError("error setting peer IP and port");
}

void TcpConnection::Write(const uint8_t* data, size_t len, TcpConnection::onSendCallback* cb)
{
	MS_TRACE();

	if (this->closed)
	{
		if (cb)
		{
			(*cb)(false);

			delete cb;
		}

		return;
	}

	if (len == 0)
	{
		if (cb)
		{
			(*cb)(false);

			delete cb;
		}

		return;
	}

	// First try uv_try_write(). In case it can not directly write all the given
	// data then build a uv_req_t and use uv_write().

	uv_buf_t buffer = uv_buf_init(reinterpret_cast<char*>(const_cast<uint8_t*>(data)), len);
	int written     = uv_try_write(reinterpret_cast<uv_stream_t*>(this->uvHandle), &buffer, 1);

	// All the data was written. Done.
	if (written == static_cast<int>(len))
	{
		// Update sent bytes.
		this->sentBytes += written;

		if (cb)
		{
			(*cb)(true);

			delete cb;
		}

		return;
	}
	// Cannot write any data at first time. Use uv_write().
	else if (written == UV_EAGAIN || written == UV_ENOSYS)
	{
		// Set written to 0 so pendingLen can be properly calculated.
		written = 0;
	}
	// Any other error.
	else if (written < 0)
	{
		MS_WARN_DEV("uv_try_write() failed, trying uv_write(): %s", uv_strerror(written));

		// Set written to 0 so pendingLen can be properly calculated.
		written = 0;
	}

	size_t pendingLen = len - written;
	auto* writeData   = new UvWriteData(pendingLen);

	writeData->req.data = static_cast<void*>(writeData);
	std::memcpy(writeData->store, data + written, pendingLen);
	writeData->cb = cb;

	buffer = uv_buf_init(reinterpret_cast<char*>(writeData->store), pendingLen);

	int err = uv_write(
	  &writeData->req,
	  reinterpret_cast<uv_stream_t*>(this->uvHandle),
	  &buffer,
	  1,
	  static_cast<uv_write_cb>(onWrite));

	if (err != 0)
	{
		MS_WARN_DEV("uv_write() failed: %s", uv_strerror(err));

		if (cb)
			(*cb)(false);

		// Delete the UvWriteData struct (it will delete the store and cb too).
		delete writeData;
	}
	else
	{
		// Update sent bytes.
		this->sentBytes += pendingLen;
	}
}

void TcpConnection::Write(
  const uint8_t* data1, size_t len1, const uint8_t* data2, size_t len2, TcpConnection::onSendCallback* cb)
{
	MS_TRACE();

	if (this->closed)
	{
		if (cb)
		{
			(*cb)(false);

			delete cb;
		}

		return;
	}

	if (len1 == 0 && len2 == 0)
	{
		if (cb)
		{
			(*cb)(false);

			delete cb;
		}

		return;
	}

	size_t totalLen = len1 + len2;
	uv_buf_t buffers[2];
	int written{ 0 };
	int err;

	// First try uv_try_write(). In case it can not directly write all the given
	// data then build a uv_req_t and use uv_write().

	buffers[0] = uv_buf_init(reinterpret_cast<char*>(const_cast<uint8_t*>(data1)), len1);
	buffers[1] = uv_buf_init(reinterpret_cast<char*>(const_cast<uint8_t*>(data2)), len2);
	written    = uv_try_write(reinterpret_cast<uv_stream_t*>(this->uvHandle), buffers, 2);

	// All the data was written. Done.
	if (written == static_cast<int>(totalLen))
	{
		// Update sent bytes.
		this->sentBytes += written;

		if (cb)
		{
			(*cb)(true);

			delete cb;
		}

		return;
	}
	// Cannot write any data at first time. Use uv_write().
	else if (written == UV_EAGAIN || written == UV_ENOSYS)
	{
		// Set written to 0 so pendingLen can be properly calculated.
		written = 0;
	}
	// Any other error.
	else if (written < 0)
	{
		MS_WARN_DEV("uv_try_write() failed, trying uv_write(): %s", uv_strerror(written));

		// Set written to 0 so pendingLen can be properly calculated.
		written = 0;
	}

	size_t pendingLen = totalLen - written;
	auto* writeData   = new UvWriteData(pendingLen);

	writeData->req.data = static_cast<void*>(writeData);

	// If the first buffer was not entirely written then splice it.
	if (static_cast<size_t>(written) < len1)
	{
		std::memcpy(
		  writeData->store, data1 + static_cast<size_t>(written), len1 - static_cast<size_t>(written));
		std::memcpy(writeData->store + (len1 - static_cast<size_t>(written)), data2, len2);
	}
	// Otherwise just take the pending data in the second buffer.
	else
	{
		std::memcpy(
		  writeData->store,
		  data2 + (static_cast<size_t>(written) - len1),
		  len2 - (static_cast<size_t>(written) - len1));
	}

	writeData->cb = cb;

	uv_buf_t buffer = uv_buf_init(reinterpret_cast<char*>(writeData->store), pendingLen);

	err = uv_write(
	  &writeData->req,
	  reinterpret_cast<uv_stream_t*>(this->uvHandle),
	  &buffer,
	  1,
	  static_cast<uv_write_cb>(onWrite));

	if (err != 0)
	{
		MS_WARN_DEV("uv_write() failed: %s", uv_strerror(err));

		if (cb)
			(*cb)(false);

		// Delete the UvWriteData struct (it will delete the store and cb too).
		delete writeData;
	}
	else
	{
		// Update sent bytes.
		this->sentBytes += pendingLen;
	}
}

void TcpConnection::ErrorReceiving()
{
	MS_TRACE();

	Close();

	this->listener->OnTcpConnectionClosed(this);
}

bool TcpConnection::SetPeerAddress()
{
	MS_TRACE();

	int err;
	int len = sizeof(this->peerAddr);

	err = uv_tcp_getpeername(this->uvHandle, reinterpret_cast<struct sockaddr*>(&this->peerAddr), &len);

	if (err != 0)
	{
		MS_ERROR("uv_tcp_getpeername() failed: %s", uv_strerror(err));

		return false;
	}

	int family;

	Utils::IP::GetAddressInfo(
	  reinterpret_cast<const struct sockaddr*>(&this->peerAddr), family, this->peerIp, this->peerPort);

	return true;
}

inline void TcpConnection::OnUvReadAlloc(size_t /*suggestedSize*/, uv_buf_t* buf)
{
	MS_TRACE();

	// If this is the first call to onUvReadAlloc() then allocate the receiving buffer now.
	if (this->buffer == nullptr)
		this->buffer = new uint8_t[this->bufferSize];

	// Tell UV to write after the last data byte in the buffer.
	buf->base = reinterpret_cast<char*>(this->buffer + this->bufferDataLen);

	// Give UV all the remaining space in the buffer.
	if (this->bufferSize > this->bufferDataLen)
	{
		buf->len = this->bufferSize - this->bufferDataLen;
	}
	else
	{
		buf->len = 0;

		MS_WARN_DEV("no available space in the buffer");
	}
}

inline void TcpConnection::OnUvRead(ssize_t nread, const uv_buf_t* /*buf*/)
{
	MS_TRACE();

	if (nread == 0)
		return;

	// Data received.
	if (nread > 0)
	{
		// Update received bytes.
		this->recvBytes += nread;

		// Update the buffer data length.
		this->bufferDataLen += static_cast<size_t>(nread);

		// Notify the subclass.
		UserOnTcpConnectionRead();
	}
	// Client disconnected.
	else if (nread == UV_EOF || nread == UV_ECONNRESET)
	{
		MS_DEBUG_DEV("connection closed by peer, closing server side");

		this->isClosedByPeer = true;

		// Close server side of the connection.
		Close();

		// Notify the listener.
		this->listener->OnTcpConnectionClosed(this);
	}
	// Some error.
	else
	{
		MS_WARN_DEV("read error, closing the connection: %s", uv_strerror(nread));

		this->hasError = true;

		// Close server side of the connection.
		Close();

		// Notify the listener.
		this->listener->OnTcpConnectionClosed(this);
	}
}

inline void TcpConnection::OnUvWrite(int status, TcpConnection::onSendCallback* cb)
{
	MS_TRACE();

	if (status == 0)
	{
		if (cb)
			(*cb)(true);
	}
	else
	{
		if (status != UV_EPIPE && status != UV_ENOTCONN)
			this->hasError = true;

		MS_WARN_DEV("write error, closing the connection: %s", uv_strerror(status));

		if (cb)
			(*cb)(false);

		Close();

		this->listener->OnTcpConnectionClosed(this);
	}
}
