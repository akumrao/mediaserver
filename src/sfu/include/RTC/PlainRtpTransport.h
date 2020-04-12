#ifndef MS_RTC_PLAIN_RTP_TRANSPORT_HPP
#define MS_RTC_PLAIN_RTP_TRANSPORT_HPP

#include "RTC/Transport.h"
#include "RTC/TransportTuple.h"
#include "RTC/UdpSocket.h"

namespace RTC
{
	class PlainRtpTransport : public RTC::Transport, public RTC::UdpSocket::Listener
	{
	private:
		struct ListenIp
		{
			std::string ip;
			std::string announcedIp;
		};

	public:
		PlainRtpTransport(const std::string& id, RTC::Transport::Listener* listener, json& data);
		~PlainRtpTransport() override;

	public:
		void FillJson(json& jsonObject) const override;
		void FillJsonStats(json& jsonArray) override;
		void HandleRequest(Channel::Request* request) override;

	private:
		bool IsConnected() const override;
		void SendRtpPacket(RTC::RtpPacket* packet, RTC::Transport::onSendCallback* cb = nullptr) override;
		void SendRtcpPacket(RTC::RTCP::Packet* packet) override;
		void SendRtcpCompoundPacket(RTC::RTCP::CompoundPacket* packet) override;
		void SendSctpData(const uint8_t* data, size_t len) override;
		void OnPacketReceived(RTC::TransportTuple* tuple, const uint8_t* data, size_t len);
		void OnRtpDataReceived(RTC::TransportTuple* tuple, const uint8_t* data, size_t len);
		void OnRtcpDataReceived(RTC::TransportTuple* tuple, const uint8_t* data, size_t len);
		void OnSctpDataReceived(RTC::TransportTuple* tuple, const uint8_t* data, size_t len);

		/* Pure virtual methods inherited from RTC::UdpSocket::Listener. */
	public:
		void OnUdpSocketPacketReceived(
		  RTC::UdpSocket* socket, const uint8_t* data, size_t len, const struct sockaddr* remoteAddr) override;

	private:
		// Allocated by this.
		RTC::UdpSocket* udpSocket{ nullptr };
		RTC::UdpSocket* rtcpUdpSocket{ nullptr };
		RTC::TransportTuple* tuple{ nullptr };
		RTC::TransportTuple* rtcpTuple{ nullptr };
		// Others.
		ListenIp listenIp;
		bool rtcpMux{ true };
		bool comedia{ false };
		bool multiSource{ false };
		struct sockaddr_storage remoteAddrStorage;
		struct sockaddr_storage rtcpRemoteAddrStorage;
	};
} // namespace RTC

#endif
