#ifndef MS_RTC_WEBRTC_TRANSPORT_HPP
#define MS_RTC_WEBRTC_TRANSPORT_HPP

#include "RTC/DtlsTransport.h"
#include "RTC/IceCandidate.h"
#include "RTC/IceServer.h"
#include "RTC/SrtpSession.h"
#include "RTC/StunPacket.h"
#include "net/netInterface.h"
#include "RTC/TcpConnection.h"
#include "RTC/TcpServer.h"
#include "RTC/Transport.h"
#include "RTC/TransportTuple.h"
#include "RTC/UdpSocket.h"
#include <vector>

namespace RTC
{
	class WebRtcTransport : public RTC::Transport,
	                        public RTC::UdpSocket::Listener,
	                       // public RTC::TcpServer::Listener,
	                       // public RTC::TcpConnection::Listener,
                                public base::net::Listener,
	                        public RTC::IceServer::Listener,
	                        public RTC::DtlsTransport::Listener
	{
	private:
		struct ListenIp
		{
			std::string ip;
			std::string announcedIp;
		};

	public:
		WebRtcTransport(const std::string& id, RTC::Transport::Listener* listener, json& data);
		~WebRtcTransport() override;

	public:
		void FillJson(json& jsonObject) const override;
		void FillJsonStats(json& jsonArray) override;
		void HandleRequest(Channel::Request* request) override;

	private:
		bool IsConnected() const override;
		void MayRunDtlsTransport();
		void SendRtpPacket(RTC::RtpPacket* packet, RTC::Transport::onSendCallback cb = nullptr) override;
		void SendRtcpPacket(RTC::RTCP::Packet* packet) override;
		void SendRtcpCompoundPacket(RTC::RTCP::CompoundPacket* packet) override;
		void SendSctpData(const uint8_t* data, size_t len) override;
		void OnPacketReceived(RTC::TransportTuple* tuple, const uint8_t* data, size_t len);
		void OnStunDataReceived(RTC::TransportTuple* tuple, const uint8_t* data, size_t len);
		void OnDtlsDataReceived(const RTC::TransportTuple* tuple, const uint8_t* data, size_t len);
		void OnRtpDataReceived(RTC::TransportTuple* tuple, const uint8_t* data, size_t len);
		void OnRtcpDataReceived(RTC::TransportTuple* tuple, const uint8_t* data, size_t len);

		/* Pure virtual methods inherited from RTC::UdpSocket::Listener. */
	public:
		void OnUdpSocketPacketReceived(
		  RTC::UdpSocket* socket, const uint8_t* data, size_t len, const struct sockaddr* remoteAddr) override;

		/* Pure virtual methods inherited from RTC::TcpServer::Listener. */
	public:
		//void OnRtcTcpConnectionClosed(RTC::TcpServer* tcpServer, RTC::TcpConnection* connection) override;
		void on_close( base::net::Listener* conn);
		/* Pure virtual methods inherited from RTC::TcpConnection::Listener. */
	public:
//		void OnTcpConnectionPacketReceived(
//		  RTC::TcpConnection* connection, const uint8_t* data, size_t len) override;  //
                 void on_read( base::net::Listener* conn, const char* data, size_t len);
		/* Pure virtual methods inherited from RTC::IceServer::Listener. */
	public:
		void OnIceServerSendStunPacket(
		  const RTC::IceServer* iceServer,
		  const RTC::StunPacket* packet,
		  RTC::TransportTuple* tuple) override;
		void OnIceServerSelectedTuple(const RTC::IceServer* iceServer, RTC::TransportTuple* tuple) override;
		void OnIceServerConnected(const RTC::IceServer* iceServer) override;
		void OnIceServerCompleted(const RTC::IceServer* iceServer) override;
		void OnIceServerDisconnected(const RTC::IceServer* iceServer) override;

		/* Pure virtual methods inherited from RTC::DtlsTransport::Listener. */
	public:
		void OnDtlsTransportConnecting(const RTC::DtlsTransport* dtlsTransport) override;
		void OnDtlsTransportConnected(
		  const RTC::DtlsTransport* dtlsTransport,
		  RTC::SrtpSession::Profile srtpProfile,
		  uint8_t* srtpLocalKey,
		  size_t srtpLocalKeyLen,
		  uint8_t* srtpRemoteKey,
		  size_t srtpRemoteKeyLen,
		  std::string& remoteCert) override;
		void OnDtlsTransportFailed(const RTC::DtlsTransport* dtlsTransport) override;
		void OnDtlsTransportClosed(const RTC::DtlsTransport* dtlsTransport) override;
		void OnDtlsTransportSendData(
		  const RTC::DtlsTransport* dtlsTransport, const uint8_t* data, size_t len) override;
		void OnDtlsTransportApplicationDataReceived(
		  const RTC::DtlsTransport* dtlsTransport, const uint8_t* data, size_t len) override;

	private:
		// Allocated by this.
		RTC::IceServer* iceServer{ nullptr };
		// Map of UdpSocket/TcpServer and local announced IP (if any).
		std::unordered_map<RTC::UdpSocket*, std::string> udpSockets;
		std::unordered_map<RTC::TcpServer*, std::string> tcpServers;
		RTC::DtlsTransport* dtlsTransport{ nullptr };
		RTC::SrtpSession* srtpRecvSession{ nullptr };
		RTC::SrtpSession* srtpSendSession{ nullptr };
		// Others.
		bool connectCalled{ false }; // Whether connect() was succesfully called.
		std::vector<RTC::IceCandidate> iceCandidates;
		RTC::DtlsTransport::Role dtlsRole{ RTC::DtlsTransport::Role::AUTO };
	};
} // namespace RTC

#endif
