#define MS_CLASS "RTC::PlainRtpTransport"
// #define MS_LOG_DEV_LEVEL 3

#include "RTC/PlainRtpTransport.h"
#include "LoggerTag.h"
#include "base/error.h"
#include "Utils.h"

namespace RTC
{
	/* Instance methods. */

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
	PlainRtpTransport::PlainRtpTransport(
	  const std::string& id, RTC::Transport::Listener* listener, json& data)
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
		Utils::IP::NormalizeIp(this->listenIp.ip);

		auto jsonAnnouncedIpIt = jsonListenIpIt->find("announcedIp");

		if (jsonAnnouncedIpIt != jsonListenIpIt->end())
		{
			if (!jsonAnnouncedIpIt->is_string())
				base::uv::throwError("wrong listenIp.announcedIp (not an string");

			this->listenIp.announcedIp.assign(jsonAnnouncedIpIt->get<std::string>());
		}

		auto jsonRtcpMuxIt = data.find("rtcpMux");

		if (jsonRtcpMuxIt != data.end())
		{
			if (!jsonRtcpMuxIt->is_boolean())
				base::uv::throwError("wrong rtcpMux (not a boolean)");

			this->rtcpMux = jsonRtcpMuxIt->get<bool>();
		}

		auto jsonComediaIt = data.find("comedia");

		if (jsonComediaIt != data.end())
		{
			if (!jsonComediaIt->is_boolean())
				base::uv::throwError("wrong comedia (not a boolean)");

			this->comedia = jsonComediaIt->get<bool>();
		}

		auto jsonMultiSourceIt = data.find("multiSource");

		if (jsonMultiSourceIt != data.end())
		{
			if (!jsonMultiSourceIt->is_boolean())
				base::uv::throwError("wrong multiSource (not a boolean)");

			this->multiSource = jsonMultiSourceIt->get<bool>();

			// If multiSource is set disable RTCP-mux and comedia mode.
			if (this->multiSource)
			{
				this->rtcpMux = false;
				this->comedia = false;
			}
		}

		try
		{
			// This may throw.
			this->udpSocket = new RTC::UdpSocket(this, this->listenIp.ip);

			if (!this->rtcpMux)
			{
				// This may throw.
				this->rtcpUdpSocket = new RTC::UdpSocket(this, this->listenIp.ip);
			}
		}
		catch (const std::exception& error)
		{
			delete this->udpSocket;
			this->udpSocket = nullptr;

			delete this->rtcpUdpSocket;
			this->rtcpUdpSocket = nullptr;

			throw;
		}
	}

	PlainRtpTransport::~PlainRtpTransport()
	{
		MS_TRACE();

		delete this->udpSocket;
		this->udpSocket = nullptr;

		delete this->rtcpUdpSocket;
		this->rtcpUdpSocket = nullptr;

		delete this->tuple;
		this->tuple = nullptr;

		delete this->rtcpTuple;
		this->rtcpTuple = nullptr;
	}

	void PlainRtpTransport::FillJson(json& jsonObject) const
	{
		MS_TRACE();

		// Call the parent method.
		RTC::Transport::FillJson(jsonObject);

		// Add rtcpMux.
		jsonObject["rtcpMux"] = this->rtcpMux;

		// Add comedia.
		jsonObject["comedia"] = this->comedia;

		// Add multiSource.
		jsonObject["multiSource"] = this->multiSource;

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

		// Add rtcpTuple.
		if (!this->rtcpMux)
		{
			if (this->rtcpTuple != nullptr)
			{
				this->rtcpTuple->FillJson(jsonObject["rtcpTuple"]);
			}
			else
			{
				jsonObject["rtcpTuple"] = json::object();
				auto jsonRtcpTupleIt    = jsonObject.find("rtcpTuple");

				if (this->listenIp.announcedIp.empty())
					(*jsonRtcpTupleIt)["localIp"] = this->rtcpUdpSocket->GetLocalIp();
				else
					(*jsonRtcpTupleIt)["localIp"] = this->listenIp.announcedIp;

				(*jsonRtcpTupleIt)["localPort"] = this->rtcpUdpSocket->GetLocalPort();
				(*jsonRtcpTupleIt)["protocol"]  = "udp";
			}
		}
	}

	void PlainRtpTransport::FillJsonStats(json& jsonArray)
	{
		MS_TRACE();

		// Call the parent method.
		RTC::Transport::FillJsonStats(jsonArray);

		auto& jsonObject = jsonArray[0];

		// Add type.
		jsonObject["type"] = "plain-rtp-transport";

		// Add rtcpMux.
		jsonObject["rtcpMux"] = this->rtcpMux;

		// Add comedia.
		jsonObject["comedia"] = this->comedia;

		// Add multiSource.
		jsonObject["multiSource"] = this->multiSource;

		if (this->tuple != nullptr)
		{
			// Add tuple.
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

		// Add rtcpTuple.
		if (!this->rtcpMux && this->rtcpTuple != nullptr)
			this->rtcpTuple->FillJson(jsonObject["rtcpTuple"]);
	}

	void PlainRtpTransport::HandleRequest(Channel::Request* request)
	{
		MS_TRACE();

		switch (request->methodId)
		{
			case Channel::Request::MethodId::TRANSPORT_CONNECT:
			{
				// Reject if comedia mode or multiSource is set.
				if (this->comedia)
					base::uv::throwError("cannot call connect() when comedia mode is set");
				else if (this->multiSource)
					base::uv::throwError("cannot call connect() when multiSource is set");

				// Ensure this method is not called twice.
				if (this->tuple != nullptr)
					base::uv::throwError("connect() already called");

				try
				{
					std::string ip;
					uint16_t port{ 0u };
					uint16_t rtcpPort{ 0u };

					auto jsonIpIt = request->data.find("ip");

					if (jsonIpIt == request->data.end() || !jsonIpIt->is_string())
						base::uv::throwError("missing ip");

					ip = jsonIpIt->get<std::string>();

					// This may throw.
					Utils::IP::NormalizeIp(ip);

					auto jsonPortIt = request->data.find("port");

					
					if (
						jsonPortIt == request->data.end() ||
						!Utils::Json::IsPositiveInteger(*jsonPortIt)
					)
					
					{
						base::uv::throwError("missing port");
					}

					port = jsonPortIt->get<uint16_t>();

					auto jsonRtcpPortIt = request->data.find("rtcpPort");

					
					if (
						jsonRtcpPortIt != request->data.end() &&
						Utils::Json::IsPositiveInteger(*jsonRtcpPortIt)
					)
					
					{
						if (this->rtcpMux)
							base::uv::throwError("cannot set rtcpPort with rtcpMux enabled");

						rtcpPort = jsonRtcpPortIt->get<uint16_t>();
					}
					else
					{
						if (!this->rtcpMux)
							base::uv::throwError("missing rtcpPort (required with rtcpMux disabled)");
					}

					int err;

					switch (Utils::IP::GetFamily(ip))
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
							base::uv::throwError("invalid IP " + ip );
						}
					}

					// Create the tuple.
					this->tuple = new RTC::TransportTuple(
					  this->udpSocket, reinterpret_cast<struct sockaddr*>(&this->remoteAddrStorage));

					if (!this->listenIp.announcedIp.empty())
						this->tuple->SetLocalAnnouncedIp(this->listenIp.announcedIp);

					if (!this->rtcpMux)
					{
						switch (Utils::IP::GetFamily(ip))
						{
							case AF_INET:
							{
								err = uv_ip4_addr(
								  ip.c_str(),
								  static_cast<int>(rtcpPort),
								  reinterpret_cast<struct sockaddr_in*>(&this->rtcpRemoteAddrStorage));

								if (err != 0)
									MS_ABORT("uv_ip4_addr() failed: %s", uv_strerror(err));

								break;
							}

							case AF_INET6:
							{
								err = uv_ip6_addr(
								  ip.c_str(),
								  static_cast<int>(rtcpPort),
								  reinterpret_cast<struct sockaddr_in6*>(&this->rtcpRemoteAddrStorage));

								if (err != 0)
									MS_ABORT("uv_ip6_addr() failed: %s", uv_strerror(err));

								break;
							}

							default:
							{
								base::uv::throwError("invalid IP " + ip );
							}
						}

						// Create the tuple.
						this->rtcpTuple = new RTC::TransportTuple(
						  this->rtcpUdpSocket, reinterpret_cast<struct sockaddr*>(&this->rtcpRemoteAddrStorage));

						if (!this->listenIp.announcedIp.empty())
							this->rtcpTuple->SetLocalAnnouncedIp(this->listenIp.announcedIp);
					}
				}
				catch (const std::exception& error)
				{
					if (this->tuple != nullptr)
					{
						delete this->tuple;
						this->tuple = nullptr;
					}

					if (this->rtcpTuple != nullptr)
					{
						delete this->rtcpTuple;
						this->rtcpTuple = nullptr;
					}

					throw;
				}

				// Tell the caller about the selected local DTLS role.
				json data = json::object();

				this->tuple->FillJson(data["tuple"]);

				if (!this->rtcpMux)
					this->rtcpTuple->FillJson(data["rtcpTuple"]);

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

	inline bool PlainRtpTransport::IsConnected() const
	{
		return this->tuple != nullptr;
	}

	void PlainRtpTransport::SendRtpPacket(RTC::RtpPacket* packet, RTC::Transport::onSendCallback* cb)
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

	void PlainRtpTransport::SendRtcpPacket(RTC::RTCP::Packet* packet)
	{
		MS_TRACE();

		if (!IsConnected())
			return;

		const uint8_t* data = packet->GetData();
		size_t len          = packet->GetSize();

		if (this->rtcpMux)
			this->tuple->Send(data, len);
		else if (this->rtcpTuple)
			this->rtcpTuple->Send(data, len);

		// Increase send transmission.
		RTC::Transport::DataSent(len);
	}

	void PlainRtpTransport::SendRtcpCompoundPacket(RTC::RTCP::CompoundPacket* packet)
	{
		MS_TRACE();

		if (!IsConnected())
			return;

		const uint8_t* data = packet->GetData();
		size_t len          = packet->GetSize();

		if (this->rtcpMux)
			this->tuple->Send(data, len);
		else if (this->rtcpTuple)
			this->rtcpTuple->Send(data, len);

		// Increase send transmission.
		RTC::Transport::DataSent(len);
	}

	void PlainRtpTransport::SendSctpData(const uint8_t* data, size_t len)
	{
		MS_TRACE();

		if (!IsConnected())
			return;

		this->tuple->Send(data, len);

		// Increase send transmission.
		RTC::Transport::DataSent(len);
	}

	inline void PlainRtpTransport::OnPacketReceived(
	  RTC::TransportTuple* tuple, const uint8_t* data, size_t len)
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

	inline void PlainRtpTransport::OnRtpDataReceived(
	  RTC::TransportTuple* tuple, const uint8_t* data, size_t len)
	{
		MS_TRACE();

		// If multiSource is set allow it without any checking.
		if (this->multiSource)
		{
			// Do nothing.
		}
		// Otherwise, if we don't have a RTP tuple yet, check whether comedia mode
		// is set,
		else if (!this->tuple)
		{
			if (!this->comedia)
			{
				MS_DEBUG_TAG(rtp, "ignoring RTP packet while not connected");

				return;
			}

			MS_DEBUG_TAG(rtp, "setting RTP tuple (comedia mode enabled)");

			auto wasConnected = IsConnected();

			this->tuple = new RTC::TransportTuple(tuple);

			if (!this->listenIp.announcedIp.empty())
				this->tuple->SetLocalAnnouncedIp(this->listenIp.announcedIp);

			// If not yet connected do it now.
			if (!wasConnected)
				RTC::Transport::Connected();
		}
		// Otherwise, if RTP tuple is set, verify that it matches the origin
		// of the packet.
		else if (!this->tuple->Compare(tuple))
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

	inline void PlainRtpTransport::OnRtcpDataReceived(
	  RTC::TransportTuple* tuple, const uint8_t* data, size_t len)
	{
		MS_TRACE();

		// If multiSource is set allow it without any checking.
		if (this->multiSource)
		{
			// Just allow it.
		}
		// Otherwise, if we don't have a RTP tuple yet, check whether RTCP-mux
		// and comedia mode are set.
		else if (this->rtcpMux && !this->tuple)
		{
			if (!this->comedia)
			{
				MS_DEBUG_TAG(rtcp, "ignoring RTCP packet while not connected");

				return;
			}

			MS_DEBUG_TAG(rtp, "setting RTP tuple (comedia mode enabled)");

			auto wasConnected = IsConnected();

			this->tuple = new RTC::TransportTuple(tuple);

			if (!this->listenIp.announcedIp.empty())
				this->tuple->SetLocalAnnouncedIp(this->listenIp.announcedIp);

			// If not yet connected do it now.
			if (!wasConnected)
				RTC::Transport::Connected();
		}
		// Otherwise, if RTCP-mux is unset and RTCP tuple is unset, set it if we
		// are in comedia mode.
		else if (!this->rtcpMux && !this->rtcpTuple)
		{
			if (!this->comedia)
			{
				MS_DEBUG_TAG(rtcp, "ignoring RTCP packet while not connected");

				return;
			}

			MS_DEBUG_TAG(rtcp, "setting RTCP tuple (comedia mode enabled)");

			this->rtcpTuple = new RTC::TransportTuple(tuple);

			if (!this->listenIp.announcedIp.empty())
				this->rtcpTuple->SetLocalAnnouncedIp(this->listenIp.announcedIp);
		}
		// If RTCP-mux verify that the packet's tuple matches our RTP tuple.
		else if (this->rtcpMux && !this->tuple->Compare(tuple))
		{
			MS_DEBUG_TAG(rtcp, "ignoring RTCP packet from unknown IP:port");

			return;
		}
		// If no RTCP-mux verify that the packet's tuple matches our RTCP tuple.
		else if (!this->rtcpMux && !this->rtcpTuple->Compare(tuple))
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

	inline void PlainRtpTransport::OnSctpDataReceived(
	  RTC::TransportTuple* tuple, const uint8_t* data, size_t len)
	{
		MS_TRACE();

		// If multiSource reject it.
		if (this->multiSource)
		{
			MS_DEBUG_TAG(sctp, "ignoring SCTP packet in multiSource mode");

			return;
		}
		// Otherwise, if we don't have a RTP tuple yet, check whether comedia mode
		// is set,
		else if (!this->tuple)
		{
			if (!this->comedia)
			{
				MS_DEBUG_TAG(sctp, "ignoring SCTP packet while not connected");

				return;
			}

			MS_DEBUG_TAG(sctp, "setting RTP/SCTP tuple (comedia mode enabled)");

			auto wasConnected = IsConnected();

			this->tuple = new RTC::TransportTuple(tuple);

			if (!this->listenIp.announcedIp.empty())
				this->tuple->SetLocalAnnouncedIp(this->listenIp.announcedIp);

			// If not yet connected do it now.
			if (!wasConnected)
				RTC::Transport::Connected();
		}
		// Otherwise, if RTP tuple is set, verify that it matches the origin
		// of the packet.
		if (!this->tuple->Compare(tuple))
		{
			MS_DEBUG_TAG(sctp, "ignoring SCTP packet from unknown IP:port");

			return;
		}

		// Pass it to the parent transport.
		RTC::Transport::ReceiveSctpData(data, len);
	}

	inline void PlainRtpTransport::OnUdpSocketPacketReceived(
	  RTC::UdpSocket* socket, const uint8_t* data, size_t len, const struct sockaddr* remoteAddr)
	{
		MS_TRACE();

		RTC::TransportTuple tuple(socket, remoteAddr);

		OnPacketReceived(&tuple, data, len);
	}
} // namespace RTC
