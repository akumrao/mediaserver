#ifndef MS_UDP_SOCKET_HPP
#define MS_UDP_SOCKET_HPP

#include "common.h"
#include <uv.h>
#include <string>

class UdpSocket
{
protected:
	using onSendCallback = const std::function<void(bool sent)>;

public:
	/* Struct for the data field of uv_req_t when sending a datagram. */
	struct UvSendData
	{
		explicit UvSendData(size_t storeSize)
		{
			this->store = new uint8_t[storeSize];
		}

		// Disable copy constructor because of the dynamically allocated data (store).
		UvSendData(const UvSendData&) = delete;

		~UvSendData()
		{
			delete[] this->store;
			delete this->cb;
		}

		uv_udp_send_t req;
		uint8_t* store{ nullptr };
		UdpSocket::onSendCallback* cb{ nullptr };
	};

public:
	/**
	 * uvHandle must be an already initialized and binded uv_udp_t pointer.
	 */
	explicit UdpSocket(uv_udp_t* uvHandle);
	UdpSocket& operator=(const UdpSocket&) = delete;
	UdpSocket(const UdpSocket&)            = delete;
	virtual ~UdpSocket();

public:
	void Close();
	virtual void Dump() const;
	void Send(const uint8_t* data, size_t len, const struct sockaddr* addr, UdpSocket::onSendCallback* cb);
	const struct sockaddr* GetLocalAddress() const;
	int GetLocalFamily() const;
	const std::string& GetLocalIp() const;
	uint16_t GetLocalPort() const;
	size_t GetRecvBytes() const;
	size_t GetSentBytes() const;

private:
	bool SetLocalAddress();

	/* Callbacks fired by UV events. */
public:
	void OnUvRecvAlloc(size_t suggestedSize, uv_buf_t* buf);
	void OnUvRecv(ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned int flags);
	void OnUvSend(int status, UdpSocket::onSendCallback* cb);

	/* Pure virtual methods that must be implemented by the subclass. */
protected:
	virtual void UserOnUdpDatagramReceived(
	  const uint8_t* data, size_t len, const struct sockaddr* addr) = 0;

protected:
	struct sockaddr_storage localAddr;
	std::string localIp;
	uint16_t localPort{ 0 };

private:
	// Allocated by this (may be passed by argument).
	uv_udp_t* uvHandle{ nullptr };
	// Others.
	bool closed{ false };
	size_t recvBytes{ 0 };
	size_t sentBytes{ 0 };
};

/* Inline methods. */

inline const struct sockaddr* UdpSocket::GetLocalAddress() const
{
	return reinterpret_cast<const struct sockaddr*>(&this->localAddr);
}

inline int UdpSocket::GetLocalFamily() const
{
	return reinterpret_cast<const struct sockaddr*>(&this->localAddr)->sa_family;
}

inline const std::string& UdpSocket::GetLocalIp() const
{
	return this->localIp;
}

inline uint16_t UdpSocket::GetLocalPort() const
{
	return this->localPort;
}

inline size_t UdpSocket::GetRecvBytes() const
{
	return this->recvBytes;
}

inline size_t UdpSocket::GetSentBytes() const
{
	return this->sentBytes;
}

#endif
