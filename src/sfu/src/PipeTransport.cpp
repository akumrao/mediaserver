#define MS_CLASS "RTC::PipeTransport"
// #define MS_LOG_DEV_LEVEL 3

#include "RTC/PipeTransport.h"
#include "LoggerTag.h"
#include "base/error.h"
#include "net/IP.h"
#include "Utils.h"

namespace RTC
{
	/* Instance methods. */

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
	PipeTransport::PipeTransport(const std::string& id, RTC::Transport::Listener* listener, json& data)
	  : RTC::Transport::Transport(id, listener, data)
	{
		MS_TRACE();

		auto jsonListenIpIt = data.find("listenIp");

		if (jsonListenIpIt == data.end())
			base::uv::throwError("missing listenIp");
		else if (!jsonListenIpIt->is_object())
			base::uv::throwError("wrong listenIp (not an object)");

		auto jsonIpIt = jsonListenIpIt->find("ip");

		if (jsonIpIt == jsonListenIpIt->end())
			base::uv::throwError("missing listenIp.ip");
		else if (!jsonIpIt->is_string())
			base::uv::throwError("wrong listenIp.ip (not an string)");

		this->listenIp.ip.assign(jsonIpIt->get<std::string>());

		// This may throw.
		base::net::IP::NormalizeIp(this->listenIp.ip);

		auto jsonAnnouncedIpIt = jsonListenIpIt->find("announcedIp");

		if (jsonAnnouncedIpIt != jsonListenIpIt->end())
		{
			if (!jsonAnnouncedIpIt->is_string())
				base::uv::throwError("wrong listenIp.announcedIp (not an string");

			this->listenIp.announcedIp.assign(jsonAnnouncedIpIt->get<std::string>());
		}

		try
		{
			// This may throw.
			this->udpSocket = new RTC::UdpSocket(this, this->listenIp.ip);
		}
		catch (const std::exception& error)
		{
			// Must delete everything since the destructor won't be called.

			delete this->udpSocket;
			this->udpSocket = nullptr;

			throw;
		}
	}

	PipeTransport::~PipeTransport()
	{
		MS_TRACE();

		delete this->udpSocket;
		this->udpSocket = nullptr;

		delete this->tuple;
		this->tuple = nullptr;
	}

	void PipeTransport::FillJson(json& jsonObject) const
	{
		MS_TRACE();

		// Call the parent method.
		RTC::Transport::FillJson(jsonObject);

		// Add tuple.
		if (this->tuple != nullptr)
		{
			this->tuple->FillJson(jsonObject["tuple"]);
		}
		else
		{
			jsonObject["tuple"] = json::object();
			auto jsonTupleIt    = jsonObject.find("tuple");

			if (this->listenIp.announcedIp.empty())
				(*jsonTupleIt)["localIp"] = this->udpSocket->GetLocalIp();
			else
				(*jsonTupleIt)["localIp"] = this->listenIp.announcedIp;

			(*jsonTupleIt)["localPort"] = this->udpSocket->GetLocalPort();
			(*jsonTupleIt)["protocol"]  = "udp";
		}
	}

	void PipeTransport::FillJsonStats(json& jsonArray)
	{
		MS_TRACE();

		// Call the parent method.
		RTC::Transport::FillJsonStats(jsonArray);

		auto& jsonObject = jsonArray[0];

		// Add type.
		jsonObject["type"] = "pipe-transport";

		if (this->tuple != nullptr)
		{
			this->tuple->FillJson(jsonObject["tuple"]);
		}
		else
		{
			// Add tuple.
			jsonObject["tuple"] = json::object();
			auto jsonTupleIt    = jsonObject.find("tuple");

			if (this->listenIp.announcedIp.empty())
				(*jsonTupleIt)["localIp"] = this->udpSocket->GetLocalIp();
			else
				(*jsonTupleIt)["localIp"] = this->listenIp.announcedIp;

			(*jsonTupleIt)["localPort"] = this->udpSocket->GetLocalPort();
			(*jsonTupleIt)["protocol"]  = "udp";
		}
	}

	void PipeTransport::HandleRequest(Channel::Request* request)
	{
		MS_TRACE();

		switch (request->methodId)
		{
			case Channel::Request::MethodId::TRANSPORT_CONNECT:
			{
				// Ensure this method is not called twice.
				if (this->tuple != nullptr)
					base::uv::throwError("connect() already called");

				try
				{
					std::string ip;
					uint16_t port{ 0u };

					auto jsonIpIt = request->data.find("ip");

					if (jsonIpIt == request->data.end() || !jsonIpIt->is_string())
						base::uv::throwError("missing ip");

					ip = jsonIpIt->get<std::string>();

					// This may throw.
					base::net::IP::NormalizeIp(ip);

					auto jsonPortIt = request->data.find("port");

					
					if (
						jsonPortIt == request->data.end() ||
						!Utils::Json::IsPositiveInteger(*jsonPortIt)
					)
					
					{
						base::uv::throwError("missing port");
					}

					port = jsonPortIt->get<uint16_t>();

					int err;

					switch (base::net::IP::GetFamily(ip))
					{
						case AF_INET:
						{
							err = uv_ip4_addr(
							  ip.c_str(),
							  static_cast<int>(port),
							  reinterpret_cast<struct sockaddr_in*>(&this->remoteAddrStorage));

							if (err != 0)
								MS_ABORT("uv_ip4_addr() failed: %s", uv_strerror(err));

							break;
						}

						case AF_INET6:
						{
							err = uv_ip6_addr(
							  ip.c_str(),
							  static_cast<int>(port),
							  reinterpret_cast<struct sockaddr_in6*>(&this->remoteAddrStorage));

							if (err != 0)
								MS_ABORT("uv_ip6_addr() failed: %s", uv_strerror(err));

							break;
						}

						default:
						{
							base::uv::throwError("invalid IP " + ip);
						}
					}

					// Create the tuple.
					this->tuple = new RTC::TransportTuple(
					  this->udpSocket, reinterpret_cast<struct sockaddr*>(&this->remoteAddrStorage));

					if (!this->listenIp.announcedIp.empty())
						this->tuple->SetLocalAnnouncedIp(this->listenIp.announcedIp);
				}
				catch (const std::exception& error)
				{
					if (this->tuple != nullptr)
					{
						delete this->tuple;
						this->tuple = nullptr;
					}

					throw;
				}

				// Tell the caller about the selected local DTLS role.
				json data = json::object();

				this->tuple->FillJson(data["tuple"]);

				request->Accept(data);

				// Assume we are connected (there is no much more we can do to know it)
				// and tell the parent class.
				RTC::Transport::Connected();

				break;
			}

			default:
			{
				// Pass it to the parent class.
				RTC::Transport::HandleRequest(request);
			}
		}
	}

	inline bool PipeTransport::IsConnected() const
	{
		return this->tuple != nullptr;
	}

	void PipeTransport::SendRtpPacket(RTC::RtpPacket* packet, RTC::Transport::onSendCallback* cb)
	{
		MS_TRACE();

		if (!IsConnected())
		{
			if (cb)
			{
				(*cb)(false);

				delete cb;
			}

			return;
		}

		const uint8_t* data = packet->GetData();
		size_t len          = packet->GetSize();

		this->tuple->Send(data, len, cb);

		// Increase send transmission.
		RTC::Transport::DataSent(len);
	}

	void PipeTransport::SendRtcpPacket(RTC::RTCP::Packet* packet)
	{
		MS_TRACE();

		if (!IsConnected())
			return;

		const uint8_t* data = packet->GetData();
		size_t len          = packet->GetSize();

		this->tuple->Send(data, len);

		// Increase send transmission.
		RTC::Transport::DataSent(len);
	}

	void PipeTransport::SendRtcpCompoundPacket(RTC::RTCP::CompoundPacket* packet)
	{
		MS_TRACE();

		if (!IsConnected())
			return;

		const uint8_t* data = packet->GetData();
		size_t len          = packet->GetSize();

		this->tuple->Send(data, len);

		// Increase send transmission.
		RTC::Transport::DataSent(len);
	}

	void PipeTransport::SendSctpData(const uint8_t* data, size_t len)
	{
		MS_TRACE();

		if (!IsConnected())
			return;

		this->tuple->Send(data, len);

		// Increase send transmission.
		RTC::Transport::DataSent(len);
	}

	inline void PipeTransport::OnPacketReceived(RTC::TransportTuple* tuple, const uint8_t* data, size_t len)
	{
		MS_TRACE();

		// Increase receive transmission.
		RTC::Transport::DataReceived(len);

		// Check if it's RTCP.
		if (RTC::RTCP::Packet::IsRtcp(data, len))
		{
			OnRtcpDataReceived(tuple, data, len);
		}
		// Check if it's RTP.
		else if (RTC::RtpPacket::IsRtp(data, len))
		{
			OnRtpDataReceived(tuple, data, len);
		}
		// Check if it's SCTP.
		else if (RTC::SctpAssociation::IsSctp(data, len))
		{
			OnSctpDataReceived(tuple, data, len);
		}
		else
		{
			MS_WARN_DEV("ignoring received packet of unknown type");
		}
	}

	inline void PipeTransport::OnRtpDataReceived(RTC::TransportTuple* tuple, const uint8_t* data, size_t len)
	{
		MS_TRACE();

		if (!IsConnected())
			return;

		// Verify that the packet's tuple matches our tuple.
		if (!this->tuple->Compare(tuple))
		{
			MS_DEBUG_TAG(rtp, "ignoring RTP packet from unknown IP:port");

			return;
		}

		RTC::RtpPacket* packet = RTC::RtpPacket::Parse(data, len);

		if (packet == nullptr)
		{
			MS_WARN_TAG(rtp, "received data is not a valid RTP packet");

			return;
		}

		// Pass the packet to the parent transport.
		RTC::Transport::ReceiveRtpPacket(packet);
	}

	inline void PipeTransport::OnRtcpDataReceived(
	  RTC::TransportTuple* tuple, const uint8_t* data, size_t len)
	{
		MS_TRACE();

		if (!IsConnected())
			return;

		// Verify that the packet's tuple matches our tuple.
		if (!this->tuple->Compare(tuple))
		{
			MS_DEBUG_TAG(rtcp, "ignoring RTCP packet from unknown IP:port");

			return;
		}

		RTC::RTCP::Packet* packet = RTC::RTCP::Packet::Parse(data, len);

		if (packet == nullptr)
		{
			MS_WARN_TAG(rtcp, "received data is not a valid RTCP compound or single packet");

			return;
		}

		// Pass the packet to the parent transport.
		RTC::Transport::ReceiveRtcpPacket(packet);
	}

	inline void PipeTransport::OnSctpDataReceived(
	  RTC::TransportTuple* tuple, const uint8_t* data, size_t len)
	{
		MS_TRACE();

		if (!IsConnected())
			return;

		// Verify that the packet's tuple matches our tuple.
		if (!this->tuple->Compare(tuple))
		{
			MS_DEBUG_TAG(sctp, "ignoring SCTP packet from unknown IP:port");

			return;
		}

		// Pass it to the parent transport.
		RTC::Transport::ReceiveSctpData(data, len);
	}

	inline void PipeTransport::OnUdpSocketPacketReceived(
	  RTC::UdpSocket* socket, const uint8_t* data, size_t len, const struct sockaddr* remoteAddr)
	{
		MS_TRACE();

		RTC::TransportTuple tuple(socket, remoteAddr);

		OnPacketReceived(&tuple, data, len);
	}
} // namespace RTC
