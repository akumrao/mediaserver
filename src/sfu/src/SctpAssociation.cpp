#define MS_CLASS "RTC::SctpAssociation"
// #define MS_LOG_DEV_LEVEL 3

#include "RTC/SctpAssociation.h"
#include "DepUsrSCTP.h"
#include "LoggerTag.h"
#include "base/error.h"
#include <cstdlib> // std::malloc(), std::free()
#include <cstring> // std::memset(), std::memcpy()
#include <string>

/* SCTP events to which we are subscribing. */


uint16_t EventTypes[] =
{
	SCTP_ADAPTATION_INDICATION,
	SCTP_ASSOC_CHANGE,
	SCTP_ASSOC_RESET_EVENT,
	SCTP_REMOTE_ERROR,
	SCTP_SHUTDOWN_EVENT,
	SCTP_SEND_FAILED_EVENT,
	SCTP_STREAM_RESET_EVENT,
	SCTP_STREAM_CHANGE_EVENT
};

/* Static methods for usrsctp callbacks. */

inline static int onRecvSctpData(
  struct socket* /*sock*/,
  union sctp_sockstore /*addr*/,
  void* data,
  size_t len,
  struct sctp_rcvinfo rcv,
  int flags,
  void* ulpInfo)
{
	auto* sctpAssociation = static_cast<RTC::SctpAssociation*>(ulpInfo);

	if (sctpAssociation == nullptr)
	{
		std::free(data);

		return 0;
	}

	if (flags & MSG_NOTIFICATION)
	{
		sctpAssociation->OnUsrSctpReceiveSctpNotification(
		  static_cast<union sctp_notification*>(data), len);
	}
	else
	{
		uint16_t streamId = rcv.rcv_sid;
		uint32_t ppid     = ntohl(rcv.rcv_ppid);
		uint16_t ssn      = rcv.rcv_ssn;

//		MS_DEBUG_TAG(
//		  sctp,
//		  "data chunk received [length:%zu, streamId:%" PRIu16 ", SSN:%" PRIu16 ", TSN:%" PRIu32
//		  ", PPID:%" PRIu32 ", context:%" PRIu32 ", flags:%d]",
//		  len,
//		  rcv.rcv_sid,
//		  rcv.rcv_ssn,
//		  rcv.rcv_tsn,
//		  ntohl(rcv.rcv_ppid),
//		  rcv.rcv_context,
//		  flags);

		sctpAssociation->OnUsrSctpReceiveSctpData(
		  streamId, ssn, ppid, flags, static_cast<uint8_t*>(data), len);
	}

	std::free(data);

	return 1;
}

namespace RTC
{
	/* Static. */

	static constexpr size_t SctpMtu{ 1200 };
	static constexpr uint16_t MaxSctpStreams{ 65535 };

	/* Instance methods. */

	SctpAssociation::SctpAssociation(
	  Listener* listener, uint16_t os, uint16_t mis, size_t maxSctpMessageSize, bool isDataChannel)
	  : listener(listener), os(os), mis(mis), maxSctpMessageSize(maxSctpMessageSize),
	    isDataChannel(isDataChannel)
	{
		

		// Register ourselves in usrsctp.
		usrsctp_register_address(static_cast<void*>(this));

		int ret;

		this->socket = usrsctp_socket(
		  AF_CONN, SOCK_STREAM, IPPROTO_SCTP, onRecvSctpData, nullptr, 0, static_cast<void*>(this));

		if (this->socket == nullptr)
			base::uv::throwError("usrsctp_socket() failed: ", errno);

		usrsctp_set_ulpinfo(this->socket, static_cast<void*>(this));

		// Make the socket non-blocking.
		ret = usrsctp_set_non_blocking(this->socket, 1);

		if (ret < 0)
			base::uv::throwError("usrsctp_set_non_blocking() failed: ", errno);

		// Set SO_LINGER.
		// This ensures that the usrsctp close call deletes the association. This
		// prevents usrsctp from calling the global send callback with references to
		// this class as the address.
		struct linger lingerOpt; // NOLINT(cppcoreguidelines-pro-type-member-init)

		lingerOpt.l_onoff  = 1;
		lingerOpt.l_linger = 0;

		ret = usrsctp_setsockopt(this->socket, SOL_SOCKET, SO_LINGER, &lingerOpt, sizeof(lingerOpt));

		if (ret < 0)
			base::uv::throwError("usrsctp_setsockopt(SO_LINGER) failed: ", errno);

		// Set SCTP_ENABLE_STREAM_RESET.
		struct sctp_assoc_value av; // NOLINT(cppcoreguidelines-pro-type-member-init)

		av.assoc_value =
		  SCTP_ENABLE_RESET_STREAM_REQ | SCTP_ENABLE_RESET_ASSOC_REQ | SCTP_ENABLE_CHANGE_ASSOC_REQ;

		ret = usrsctp_setsockopt(this->socket, IPPROTO_SCTP, SCTP_ENABLE_STREAM_RESET, &av, sizeof(av));

		if (ret < 0)
		{
			base::uv::throwError("usrsctp_setsockopt(SCTP_ENABLE_STREAM_RESET) failed: ", errno);
		}

		// Set SCTP_NODELAY.
		uint32_t noDelay = 1;

		ret = usrsctp_setsockopt(this->socket, IPPROTO_SCTP, SCTP_NODELAY, &noDelay, sizeof(noDelay));

		if (ret < 0)
			base::uv::throwError("usrsctp_setsockopt(SCTP_NODELAY) failed: ", errno);

		// Enable events.
		struct sctp_event event; // NOLINT(cppcoreguidelines-pro-type-member-init)

		std::memset(&event, 0, sizeof(event));
		event.se_on = 1;

		for (size_t i{ 0 }; i < sizeof(EventTypes) / sizeof(uint16_t); ++i)
		{
			event.se_type = EventTypes[i];

			ret = usrsctp_setsockopt(this->socket, IPPROTO_SCTP, SCTP_EVENT, &event, sizeof(event));

			if (ret < 0)
				base::uv::throwError("usrsctp_setsockopt(SCTP_EVENT) failed: ", errno);
		}

		// Init message.
		struct sctp_initmsg initmsg; // NOLINT(cppcoreguidelines-pro-type-member-init)

		std::memset(&initmsg, 0, sizeof(initmsg));
		initmsg.sinit_num_ostreams  = this->os;
		initmsg.sinit_max_instreams = this->mis;

		ret = usrsctp_setsockopt(this->socket, IPPROTO_SCTP, SCTP_INITMSG, &initmsg, sizeof(initmsg));

		if (ret < 0)
			base::uv::throwError("usrsctp_setsockopt(SCTP_INITMSG) failed: ", errno);

		// Server side.
		struct sockaddr_conn sconn; // NOLINT(cppcoreguidelines-pro-type-member-init)

		std::memset(&sconn, 0, sizeof(sconn));
		sconn.sconn_family = AF_CONN;
		sconn.sconn_port   = htons(5000);
		sconn.sconn_addr   = static_cast<void*>(this);
#ifdef HAVE_SCONN_LEN
		rconn.sconn_len = sizeof(sconn);
#endif

		ret = usrsctp_bind(this->socket, reinterpret_cast<struct sockaddr*>(&sconn), sizeof(sconn));

		if (ret < 0)
			base::uv::throwError("usrsctp_bind() failed:", errno);

		DepUsrSCTP::IncreaseSctpAssociations();
	}

	SctpAssociation::~SctpAssociation()
	{
		

		usrsctp_set_ulpinfo(this->socket, nullptr);
		usrsctp_close(this->socket);

		// Deregister ourselves from usrsctp.
		usrsctp_deregister_address(static_cast<void*>(this));

		DepUsrSCTP::DecreaseSctpAssociations();

		delete[] this->messageBuffer;
	}

	void SctpAssociation::TransportConnected()
	{
		

		// Just run the SCTP stack if our state is 'new'.
		if (this->state != SctpState::NEW)
			return;

		try
		{
			int ret;
			struct sockaddr_conn rconn; // NOLINT(cppcoreguidelines-pro-type-member-init)

			std::memset(&rconn, 0, sizeof(rconn));
			rconn.sconn_family = AF_CONN;
			rconn.sconn_port   = htons(5000);
			rconn.sconn_addr   = static_cast<void*>(this);
#ifdef HAVE_SCONN_LEN
			rconn.sconn_len = sizeof(rconn);
#endif

			ret = usrsctp_connect(this->socket, reinterpret_cast<struct sockaddr*>(&rconn), sizeof(rconn));

			if (ret < 0 && errno != EINPROGRESS)
				base::uv::throwError("usrsctp_connect() failed: ", errno);

			// Disable MTU discovery.
			sctp_paddrparams peerAddrParams; // NOLINT(cppcoreguidelines-pro-type-member-init)

			std::memset(&peerAddrParams, 0, sizeof(peerAddrParams));
			std::memcpy(&peerAddrParams.spp_address, &rconn, sizeof(rconn));
			peerAddrParams.spp_flags = SPP_PMTUD_DISABLE;

			// The MTU value provided specifies the space available for chunks in the
			// packet, so let's subtract the SCTP header size.
			peerAddrParams.spp_pathmtu = SctpMtu - sizeof(struct sctp_common_header);

			ret = usrsctp_setsockopt(
			  this->socket, IPPROTO_SCTP, SCTP_PEER_ADDR_PARAMS, &peerAddrParams, sizeof(peerAddrParams));

			if (ret < 0)
				base::uv::throwError("usrsctp_setsockopt(SCTP_PEER_ADDR_PARAMS) failed: ", errno);

			// Announce connecting state.
			this->state = SctpState::CONNECTING;
			this->listener->OnSctpAssociationConnecting(this);
		}
		catch (const std::exception& /*error*/)
		{
			this->state = SctpState::FAILED;
			this->listener->OnSctpAssociationFailed(this);
		}
	}

	void SctpAssociation::FillJson(json& jsonObject) const
	{
		

		// Add port (always 5000).
		jsonObject["port"] = 5000;

		// Add OS.
		jsonObject["OS"] = this->os;

		// Add MIS.
		jsonObject["MIS"] = this->mis;

		// Add maxMessageSize.
		jsonObject["maxMessageSize"] = this->maxSctpMessageSize;

		// Add isDataChannel.
		jsonObject["isDataChannel"] = this->isDataChannel;
	}

	void SctpAssociation::ProcessSctpData(const uint8_t* data, size_t len)
	{
		

//#if MS_LOG_DEV_LEVEL == 3
		MS_DUMP_DATA(data, len);
//#endif

		usrsctp_conninput(static_cast<void*>(this), data, len, 0);
	}

	void SctpAssociation::SendSctpMessage(
	  RTC::DataConsumer* dataConsumer, uint32_t ppid, const uint8_t* msg, size_t len)
	{
                

		// This must be controlled by the DataConsumer.
		assertm(
		  len <= this->maxSctpMessageSize,
		  "given message exceeds max allowed message size" );

		auto& parameters = dataConsumer->GetSctpStreamParameters();

		// Fill stcp_sendv_spa.
		struct sctp_sendv_spa spa; // NOLINT(cppcoreguidelines-pro-type-member-init)

		std::memset(&spa, 0, sizeof(spa));
		spa.sendv_flags             = SCTP_SEND_SNDINFO_VALID;
		spa.sendv_sndinfo.snd_sid   = parameters.streamId;
		spa.sendv_sndinfo.snd_ppid  = htonl(ppid);
		spa.sendv_sndinfo.snd_flags = SCTP_EOR;

		// If ordered it must be reliable.
		if (parameters.ordered)
		{
			spa.sendv_prinfo.pr_policy = SCTP_PR_SCTP_NONE;
			spa.sendv_prinfo.pr_value  = 0;
		}
		// Configure reliability: https://tools.ietf.org/html/rfc3758
		else
		{
			spa.sendv_flags |= SCTP_SEND_PRINFO_VALID;
			spa.sendv_sndinfo.snd_flags |= SCTP_UNORDERED;

			if (parameters.maxPacketLifeTime != 0)
			{
				spa.sendv_prinfo.pr_policy = SCTP_PR_SCTP_TTL;
				spa.sendv_prinfo.pr_value  = parameters.maxPacketLifeTime;
			}
			else if (parameters.maxRetransmits != 0)
			{
				spa.sendv_prinfo.pr_policy = SCTP_PR_SCTP_RTX;
				spa.sendv_prinfo.pr_value  = parameters.maxRetransmits;
			}
		}

		int ret = usrsctp_sendv(
		  this->socket, msg, len, nullptr, 0, &spa, static_cast<socklen_t>(sizeof(spa)), SCTP_SENDV_SPA, 0);

		if (ret < 0)
		{
			MS_WARN_TAG(
			  sctp,
			  "error sending SCTP message [sid:%" PRIu16 ", ppid:%" PRIu32 ", message size:%zu]: %s",
			  parameters.streamId,
			  ppid,
			  len,
			  std::strerror(errno));
		}
	}

	void SctpAssociation::HandleDataConsumer(RTC::DataConsumer* dataConsumer)
	{
		

		auto streamId = dataConsumer->GetSctpStreamParameters().streamId;

		// We need more OS.
		if (streamId > this->os - 1)
			AddOutgoingStreams(/*force*/ false);
	}

	void SctpAssociation::DataProducerClosed(RTC::DataProducer* dataProducer)
	{
		

		auto streamId = dataProducer->GetSctpStreamParameters().streamId;

		// Send SCTP_RESET_STREAMS to the remote.
		// https://tools.ietf.org/html/draft-ietf-rtcweb-data-channel-13#section-6.7
		if (this->isDataChannel)
			ResetSctpStream(streamId, StreamDirection::OUTGOING);
		else
			ResetSctpStream(streamId, StreamDirection::INCOMING);
	}

	void SctpAssociation::DataConsumerClosed(RTC::DataConsumer* dataConsumer)
	{
		

		auto streamId = dataConsumer->GetSctpStreamParameters().streamId;

		// Send SCTP_RESET_STREAMS to the remote.
		ResetSctpStream(streamId, StreamDirection::OUTGOING);
	}

	void SctpAssociation::ResetSctpStream(uint16_t streamId, StreamDirection direction)
	{
		

		// Do nothing if an outgoing stream that could not be allocated by us.
		if (direction == StreamDirection::OUTGOING && streamId > this->os - 1)
			return;

		int ret;
		struct sctp_assoc_value av; // NOLINT(cppcoreguidelines-pro-type-member-init)
		socklen_t len = sizeof(av);

		ret = usrsctp_getsockopt(this->socket, IPPROTO_SCTP, SCTP_RECONFIG_SUPPORTED, &av, &len);

		if (ret == 0)
		{
			if (av.assoc_value != 1)
			{
				MS_DEBUG_TAG(sctp, "stream reconfiguration not negotiated");

				return;
			}
		}
		else
		{
			MS_WARN_TAG(
			  sctp,
			  "could not retrieve whether stream reconfiguration has been negotiated: %s\n",
			  std::strerror(errno));

			return;
		}

		// As per spec: https://tools.ietf.org/html/rfc6525#section-4.1
		len = sizeof(sctp_assoc_t) + (2 + 1) * sizeof(uint16_t);

		auto* srs = static_cast<struct sctp_reset_streams*>(std::malloc(len));

		switch (direction)
		{
			case StreamDirection::INCOMING:
				srs->srs_flags = SCTP_STREAM_RESET_INCOMING;
				break;

			case StreamDirection::OUTGOING:
				srs->srs_flags = SCTP_STREAM_RESET_OUTGOING;
				break;
		}

		srs->srs_number_streams = 1;
		srs->srs_stream_list[0] = streamId; // No need for htonl().

		ret = usrsctp_setsockopt(this->socket, IPPROTO_SCTP, SCTP_RESET_STREAMS, srs, len);

		if (ret == 0)
		{
			MS_DEBUG_TAG(sctp, "SCTP_RESET_STREAMS sent [streamId:%" PRIu16 "]", streamId);
		}
		else
		{
			MS_WARN_TAG(sctp, "usrsctp_setsockopt(SCTP_RESET_STREAMS) failed: %s", std::strerror(errno));
		}

		std::free(srs);
	}

	void SctpAssociation::AddOutgoingStreams(bool force)
	{
		

		uint16_t additionalOs{ 0 };

		if (MaxSctpStreams - this->os >= 32)
			additionalOs = 32;
		else
			additionalOs = MaxSctpStreams - this->os;

		if (additionalOs == 0)
		{
			MS_WARN_TAG(sctp, "cannot add more outgoing streams [OS:%" PRIu16 "]", this->os);

			return;
		}

		auto nextDesiredOs = this->os + additionalOs;

		// Already in progress, ignore (unless forced).
		if (!force && nextDesiredOs == this->desiredOs)
			return;

		// Update desired value.
		this->desiredOs = nextDesiredOs;

		// If not connected, defer it.
		if (this->state != SctpState::CONNECTED)
		{
			MS_DEBUG_TAG(sctp, "SCTP not connected, deferring OS increase");

			return;
		}

		struct sctp_add_streams sas; // NOLINT(cppcoreguidelines-pro-type-member-init)

		std::memset(&sas, 0, sizeof(sas));
		sas.sas_instrms  = 0;
		sas.sas_outstrms = additionalOs;

		MS_DEBUG_TAG(sctp, "adding %" PRIu16 " outgoing streams", additionalOs);

		int ret = usrsctp_setsockopt(
		  this->socket, IPPROTO_SCTP, SCTP_ADD_STREAMS, &sas, static_cast<socklen_t>(sizeof(sas)));

		if (ret < 0)
			MS_WARN_TAG(sctp, "usrsctp_setsockopt(SCTP_ADD_STREAMS) failed: %s", std::strerror(errno));
	}

	void SctpAssociation::OnUsrSctpSendSctpData(void* buffer, size_t len)
	{
		

		const uint8_t* data = static_cast<uint8_t*>(buffer);

#if MS_LOG_DEV_LEVEL == 3
		MS_DUMP_DATA(data, len);
#endif

		this->listener->OnSctpAssociationSendData(this, data, len);
	}

	void SctpAssociation::OnUsrSctpReceiveSctpData(
	  uint16_t streamId, uint16_t ssn, uint32_t ppid, int flags, const uint8_t* data, size_t len)
	{
                SInfo << "streamId: " << streamId << " ssn: "  << ssn << " ppid: " << ppid << " data: " << data;
                    
                    
		// Ignore WebRTC DataChannel Control DATA chunks.
		if (ppid == 50)
		{
			MS_WARN_TAG(sctp, "ignoring SCTP data with ppid:50 (WebRTC DataChannel Control)");

			return;
		}

		if (this->messageBufferLen != 0 && ssn != this->lastSsnReceived)
		{
			MS_WARN_TAG(
			  sctp,
			  "message chunk received with different SSN while buffer not empty, buffer discarded [ssn:%" PRIu16
			  ", last ssn received:%" PRIu16 "]",
			  ssn,
			  this->lastSsnReceived);

			this->messageBufferLen = 0;
		}

		// Update last SSN received.
		this->lastSsnReceived = ssn;

		auto eor = static_cast<bool>(flags & MSG_EOR);

		if (this->messageBufferLen + len > this->maxSctpMessageSize)
		{
			MS_WARN_TAG(
			  sctp,
			  "ongoing received message exceeds max allowed message size [message size:%zu, max message size:%zu, eor:%u]",
			  this->messageBufferLen + len,
			  this->maxSctpMessageSize,
			  eor ? 1 : 0);

			this->lastSsnReceived = 0;

			return;
		}

		// If end of message and there is no buffered data, notify it directly.
		if (eor && this->messageBufferLen == 0)
		{
			///MS_DEBUG_DEV("directly notifying listener [eor:1, buffer len:0]");

			this->listener->OnSctpAssociationMessageReceived(this, streamId, ppid, data, len);
		}
		// If end of message and there is buffered data, append data and notify buffer.
		else if (eor && this->messageBufferLen != 0)
		{
			std::memcpy(this->messageBuffer + this->messageBufferLen, data, len);
			this->messageBufferLen += len;

			MS_DEBUG_DEV("notifying listener [eor:1, buffer len:%zu]", this->messageBufferLen);

			this->listener->OnSctpAssociationMessageReceived(
			  this, streamId, ppid, this->messageBuffer, this->messageBufferLen);

			this->messageBufferLen = 0;
		}
		// If non end of message, append data to the buffer.
		else if (!eor)
		{
			// Allocate the buffer if not already done.
			if (!this->messageBuffer)
				this->messageBuffer = new uint8_t[this->maxSctpMessageSize];

			std::memcpy(this->messageBuffer + this->messageBufferLen, data, len);
			this->messageBufferLen += len;

			MS_DEBUG_DEV("data buffered [eor:0, buffer len:%zu]", this->messageBufferLen);
		}
	}

	void SctpAssociation::OnUsrSctpReceiveSctpNotification(union sctp_notification* notification, size_t len)
	{
		if (notification->sn_header.sn_length != (uint32_t)len)
			return;

		switch (notification->sn_header.sn_type)
		{
			case SCTP_ADAPTATION_INDICATION:
			{
				MS_DEBUG_TAG(
				  sctp,
				  "SCTP adaptation indication [%x]",
				  notification->sn_adaptation_event.sai_adaptation_ind);

				break;
			}

			case SCTP_ASSOC_CHANGE:
			{
				switch (notification->sn_assoc_change.sac_state)
				{
					case SCTP_COMM_UP:
					{
						MS_DEBUG_TAG(
						  sctp,
						  "SCTP association connected, streams [out:%" PRIu16 ", in:%" PRIu16 "]",
						  notification->sn_assoc_change.sac_outbound_streams,
						  notification->sn_assoc_change.sac_inbound_streams);

						// Update our OS.
						this->os = notification->sn_assoc_change.sac_outbound_streams;

						// Increase if requested before connected.
						if (this->desiredOs > this->os)
							AddOutgoingStreams(/*force*/ true);

						if (this->state != SctpState::CONNECTED)
						{
							this->state = SctpState::CONNECTED;
							this->listener->OnSctpAssociationConnected(this);
						}

						break;
					}

					case SCTP_COMM_LOST:
					{
						if (notification->sn_header.sn_length > 0)
						{
							static const size_t BufferSize{ 1024 };
							static char buffer[BufferSize];

							uint32_t len = notification->sn_header.sn_length;

							for (uint32_t i{ 0 }; i < len; ++i)
							{
								std::snprintf(
								  buffer, BufferSize, " 0x%02x", notification->sn_assoc_change.sac_info[i]);
							}

							MS_DEBUG_TAG(sctp, "SCTP communication lost [info:%s]", buffer);
						}
						else
						{
							MS_DEBUG_TAG(sctp, "SCTP communication lost");
						}

						if (this->state != SctpState::CLOSED)
						{
							this->state = SctpState::CLOSED;
							this->listener->OnSctpAssociationClosed(this);
						}

						break;
					}

					case SCTP_RESTART:
					{
						MS_DEBUG_TAG(
						  sctp,
						  "SCTP remote association restarted, streams [out:%" PRIu16 ", int:%" PRIu16 "]",
						  notification->sn_assoc_change.sac_outbound_streams,
						  notification->sn_assoc_change.sac_inbound_streams);

						// Update our OS.
						this->os = notification->sn_assoc_change.sac_outbound_streams;

						// Increase if requested before connected.
						if (this->desiredOs > this->os)
							AddOutgoingStreams(/*force*/ true);

						if (this->state != SctpState::CONNECTED)
						{
							this->state = SctpState::CONNECTED;
							this->listener->OnSctpAssociationConnected(this);
						}

						break;
					}

					case SCTP_SHUTDOWN_COMP:
					{
						MS_DEBUG_TAG(sctp, "SCTP association gracefully closed");

						if (this->state != SctpState::CLOSED)
						{
							this->state = SctpState::CLOSED;
							this->listener->OnSctpAssociationClosed(this);
						}

						break;
					}

					case SCTP_CANT_STR_ASSOC:
					{
						if (notification->sn_header.sn_length > 0)
						{
							static const size_t BufferSize{ 1024 };
							static char buffer[BufferSize];

							uint32_t len = notification->sn_header.sn_length;

							for (uint32_t i{ 0 }; i < len; ++i)
							{
								std::snprintf(
								  buffer, BufferSize, " 0x%02x", notification->sn_assoc_change.sac_info[i]);
							}

							MS_WARN_TAG(sctp, "SCTP setup failed: %s", buffer);
						}

						if (this->state != SctpState::FAILED)
						{
							this->state = SctpState::FAILED;
							this->listener->OnSctpAssociationFailed(this);
						}

						break;
					}

					default:;
				}

				break;
			}

			// https://tools.ietf.org/html/rfc6525#section-6.1.2.
			case SCTP_ASSOC_RESET_EVENT:
			{
				MS_DEBUG_TAG(sctp, "SCTP association reset event received");

				break;
			}

			// An Operation Error is not considered fatal in and of itself, but may be
			// used with an ABORT chunk to report a fatal condition.
			case SCTP_REMOTE_ERROR:
			{
				static const size_t BufferSize{ 1024 };
				static char buffer[BufferSize];

				uint32_t len = notification->sn_remote_error.sre_length - sizeof(struct sctp_remote_error);

				for (uint32_t i{ 0 }; i < len; i++)
				{
					std::snprintf(buffer, BufferSize, "0x%02x", notification->sn_remote_error.sre_data[i]);
				}

				MS_WARN_TAG(
				  sctp,
				  "remote SCTP association error [type:0x%04x, data:%s]",
				  notification->sn_remote_error.sre_error,
				  buffer);

				break;
			}

			// When a peer sends a SHUTDOWN, SCTP delivers this notification to
			// inform the application that it should cease sending data.
			case SCTP_SHUTDOWN_EVENT:
			{
				MS_DEBUG_TAG(sctp, "remote SCTP association shutdown");

				if (this->state != SctpState::CLOSED)
				{
					this->state = SctpState::CLOSED;
					this->listener->OnSctpAssociationClosed(this);
				}

				break;
			}

			case SCTP_SEND_FAILED_EVENT:
			{
				static const size_t BufferSize{ 1024 };
				static char buffer[BufferSize];

				uint32_t len =
				  notification->sn_send_failed_event.ssfe_length - sizeof(struct sctp_send_failed_event);

				for (uint32_t i{ 0 }; i < len; ++i)
				{
					std::snprintf(buffer, BufferSize, "0x%02x", notification->sn_send_failed_event.ssfe_data[i]);
				}

				MS_WARN_TAG(
				  sctp,
				  "SCTP message sent failure [streamId:%" PRIu16 ", ppid:%" PRIu32
				  ", sent:%s, error:0x%08x, info:%s]",
				  notification->sn_send_failed_event.ssfe_info.snd_sid,
				  ntohl(notification->sn_send_failed_event.ssfe_info.snd_ppid),
				  (notification->sn_send_failed_event.ssfe_flags & SCTP_DATA_SENT) ? "yes" : "no",
				  notification->sn_send_failed_event.ssfe_error,
				  buffer);

				break;
			}

			case SCTP_STREAM_RESET_EVENT:
			{
				bool incoming{ false };
				bool outgoing{ false };
				uint16_t numStreams =
				  (notification->sn_strreset_event.strreset_length - sizeof(struct sctp_stream_reset_event)) /
				  sizeof(uint16_t);

				if (notification->sn_strreset_event.strreset_flags & SCTP_STREAM_RESET_INCOMING_SSN)
					incoming = true;

				if (notification->sn_strreset_event.strreset_flags & SCTP_STREAM_RESET_OUTGOING_SSN)
					outgoing = true;

				if (MS_HAS_DEBUG_TAG(sctp))
				{
					std::string streamIds;

					for (uint16_t i{ 0 }; i < numStreams; ++i)
					{
						auto streamId = notification->sn_strreset_event.strreset_stream_list[i];

						// Don't log more than 5 stream ids.
						if (i > 4)
						{
							streamIds.append("...");

							break;
						}

						if (i > 0)
							streamIds.append(",");

						streamIds.append(std::to_string(streamId));
					}

					MS_DEBUG_TAG(
					  sctp,
					  "SCTP stream reset event [flags:%x, i|o:%s|%s, num streams:%" PRIu16 ", stream ids:%s]",
					  notification->sn_strreset_event.strreset_flags,
					  incoming ? "true" : "false",
					  outgoing ? "true" : "false",
					  numStreams,
					  streamIds.c_str());
				}

				// Special case for WebRTC DataChannels in which we must also reset our
				// outgoing SCTP stream.
				if (incoming && !outgoing && this->isDataChannel)
				{
					for (uint16_t i{ 0 }; i < numStreams; ++i)
					{
						auto streamId = notification->sn_strreset_event.strreset_stream_list[i];

						ResetSctpStream(streamId, StreamDirection::OUTGOING);
					}
				}

				break;
			}

			case SCTP_STREAM_CHANGE_EVENT:
			{
				if (notification->sn_strchange_event.strchange_flags == 0)
				{
					MS_DEBUG_TAG(
					  sctp,
					  "SCTP stream changed, streams [out:%" PRIu16 ", in:%" PRIu16 ", flags:%x]",
					  notification->sn_strchange_event.strchange_outstrms,
					  notification->sn_strchange_event.strchange_instrms,
					  notification->sn_strchange_event.strchange_flags);
				}
				else if (notification->sn_strchange_event.strchange_flags & SCTP_STREAM_RESET_DENIED)
				{
					MS_WARN_TAG(
					  sctp,
					  "SCTP stream change denied, streams [out:%" PRIu16 ", in:%" PRIu16 ", flags:%x]",
					  notification->sn_strchange_event.strchange_outstrms,
					  notification->sn_strchange_event.strchange_instrms,
					  notification->sn_strchange_event.strchange_flags);

					break;
				}
				else if (notification->sn_strchange_event.strchange_flags & SCTP_STREAM_RESET_FAILED)
				{
					MS_WARN_TAG(
					  sctp,
					  "SCTP stream change failed, streams [out:%" PRIu16 ", in:%" PRIu16 ", flags:%x]",
					  notification->sn_strchange_event.strchange_outstrms,
					  notification->sn_strchange_event.strchange_instrms,
					  notification->sn_strchange_event.strchange_flags);

					break;
				}

				// Update OS.
				this->os = notification->sn_strchange_event.strchange_outstrms;

				break;
			}

			default:
			{
				MS_WARN_TAG(
				  sctp, "unhandled SCTP event received [type:%" PRIu16 "]", notification->sn_header.sn_type);
			}
		}
	}
} // namespace RTC
