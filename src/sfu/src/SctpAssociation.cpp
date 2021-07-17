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
//		  "data chunk received [length:%zu, streamId:%d, SSN:%d, TSN:%" PRIu32
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



#define DATA_CHANNEL_CLOSED     0
#define DATA_CHANNEL_CONNECTING 1
#define DATA_CHANNEL_OPEN       2
#define DATA_CHANNEL_CLOSING    3


//#define DATA_CHANNEL_OPEN_REQUEST  0
//#define DATA_CHANNEL_OPEN_RESPONSE 1
//#define DATA_CHANNEL_ACK           2

#define DC_TYPE_OPEN 0x03
#define DC_TYPE_ACK 0x02
//
#define DATA_CHANNEL_PPID_CONTROL   50
#define DATA_CHANNEL_PPID_DOMSTRING 51
#define DATA_CHANNEL_PPID_BINARY    52

#define DATA_CHANNEL_RELIABLE                0
#define DATA_CHANNEL_RELIABLE_STREAM         1
#define DATA_CHANNEL_UNRELIABLE              2
#define DATA_CHANNEL_PARTIAL_RELIABLE_REXMIT 3
#define DATA_CHANNEL_PARTIAL_RELIABLE_TIMED  4
//
#define DATA_CHANNEL_FLAG_OUT_OF_ORDER_ALLOWED 0x0001

#ifndef _WIN32
#define SCTP_PACKED __attribute__((packed))
#else
#pragma pack (push, 1)
#define SCTP_PACKED
#endif

#if defined(_WIN32) && !defined(__MINGW32__)
#pragma warning( push )
#pragma warning( disable : 4200 )
#endif /* defined(_WIN32) && !defined(__MINGW32__) */
struct rtcweb_datachannel_open_request {
	uint8_t msg_type; /* DATA_CHANNEL_OPEN_REQUEST */
	uint8_t channel_type;
	uint16_t flags;
	uint16_t reliability_params;
	int16_t priority;
	char label[];
} SCTP_PACKED;
#if defined(_WIN32) && !defined(__MINGW32__)
#pragma warning( pop )
#endif /* defined(_WIN32) && !defined(__MINGW32__) */

struct rtcweb_datachannel_open_response {
	uint8_t  msg_type; /* DATA_CHANNEL_OPEN_RESPONSE */
	uint8_t  error;
	uint16_t flags;
	uint16_t reverse_stream;
} SCTP_PACKED;

struct rtcweb_datachannel_ack {
	uint8_t  msg_type; /* DATA_CHANNEL_ACK */
} SCTP_PACKED;

#ifdef _WIN32
#pragma pack(pop)
#endif


struct channel {
  uint8_t msg_type;
  uint8_t chan_type;
  uint16_t priority;
  uint32_t reliability;
  uint16_t label_len;
  uint16_t protocol_len;
  char *label;
  char *protocol;
};


#undef SCTP_PACKED


static std::map< RTC::SctpAssociation*, struct channel > stpMap;




static void print_status( RTC::SctpAssociation *pc, struct channel *channel )
{
	struct sctp_status status;
	socklen_t len;
	uint32_t i;
        
        struct socket* sock= pc->socket;
        
	len = (socklen_t)sizeof(struct sctp_status);
	if (usrsctp_getsockopt(sock, IPPROTO_SCTP, SCTP_STATUS, &status, &len) < 0) {
		perror("getsockopt");
		return;
	}
	LInfo("Association state: ");
	switch (status.sstat_state) {
	case SCTP_CLOSED:
		LInfo("CLOSED");
		break;
	case SCTP_BOUND:
		LInfo("BOUND");
		break;
	case SCTP_LISTEN:
		LInfo("LISTEN");
		break;
	case SCTP_COOKIE_WAIT:
		LInfo("COOKIE_WAIT");
		break;
	case SCTP_COOKIE_ECHOED:
		LInfo("COOKIE_ECHOED");
		break;
	case SCTP_ESTABLISHED:
		LInfo("ESTABLISHED");
		break;
	case SCTP_SHUTDOWN_PENDING:
		LInfo("SHUTDOWN_PENDING");
		break;
	case SCTP_SHUTDOWN_SENT:
		LInfo("SHUTDOWN_SENT");
		break;
	case SCTP_SHUTDOWN_RECEIVED:
		LInfo("SHUTDOWN_RECEIVED");
		break;
	case SCTP_SHUTDOWN_ACK_SENT:
		LInfo("SHUTDOWN_ACK_SENT");
		break;
	default:
		LInfo("UNKNOWN");
		break;
	}
	//LInfo("Number of streams (i/o) = (%u/%u)",
	//       status.sstat_instrms, status.sstat_outstrms);
	//for (i = 0; i < NUMBER_OF_CHANNELS; i++) 
	{
		//channel = &(pc->channels[i]);
		

		
	}
}




static void
handle_open_request_message(RTC::SctpAssociation *pc,
                             uint8_t* raw_msg,
                            size_t length,
                            uint16_t i_stream)
{
	struct channel *channel =  &stpMap[pc];
	

        channel->chan_type = raw_msg[1];
        channel->priority = (raw_msg[2] << 8) + raw_msg[3];
        channel->reliability = (raw_msg[4] << 24) + (raw_msg[5] << 16) + (raw_msg[6] << 8) + raw_msg[7];
        channel->label_len = (raw_msg[8] << 8) + raw_msg[9];
        channel->protocol_len = (raw_msg[10] << 8) + raw_msg[11];

        std::string label(reinterpret_cast<char *>(raw_msg + 12), channel->label_len);
        std::string protocol(reinterpret_cast<char *>(raw_msg + 12 + channel->label_len), channel->protocol_len);

        SInfo << "Creating channel with stream id:" <<  i_stream << " channel type: " <<   channel->chan_type << " label:"  <<  label <<  " protocol: " << protocol;
  
              
        bool unordered;
	
	switch ( channel->chan_type) {
	case DATA_CHANNEL_RELIABLE:
		//pr_policy = SCTP_PR_SCTP_NONE;
		break;
	/* XXX Doesn't make sense */
	case DATA_CHANNEL_RELIABLE_STREAM:
		//pr_policy = SCTP_PR_SCTP_NONE;
		break;
	/* XXX Doesn't make sense */
	case DATA_CHANNEL_UNRELIABLE:
		//pr_policy = SCTP_PR_SCTP_TTL;
		break;
	case DATA_CHANNEL_PARTIAL_RELIABLE_REXMIT:
		//pr_policy = SCTP_PR_SCTP_RTX;
		break;
	case DATA_CHANNEL_PARTIAL_RELIABLE_TIMED:
		//pr_policy = SCTP_PR_SCTP_TTL;
		break;
	default:
		//pr_policy = SCTP_PR_SCTP_NONE;
		/* XXX error handling */
		break;
	}
	//uint32_t pr_value = ntohs(channel->reliability);
	if (ntohs(channel->priority) & DATA_CHANNEL_FLAG_OUT_OF_ORDER_ALLOWED) {
		unordered = 1;
	} else {
		unordered = 0;
	}
 
        
//        pc->listener->OnSctpAssociationOpen(pc, i_stream,  label, protocol,  !unordered);
        
	
        print_status(  pc, channel );
}



static void
handle_open_response_message(RTC::SctpAssociation *pc,
                             struct rtcweb_datachannel_open_response *rsp,
                             size_t length, uint16_t i_stream)
{
	uint16_t o_stream;


	o_stream = ntohs(rsp->reverse_stream);
        
        struct channel *channel =  &stpMap[pc];
        
//	channel = find_channel_by_o_stream(pc, o_stream);
//	if (channel == NULL) {
//		/* XXX: improve error handling */
//		printf("handle_open_response_message: Can't find channel for outgoing steam %d.", o_stream);
//		return;
//	}
//	if (channel->state != DATA_CHANNEL_CONNECTING) {
//		/* XXX: improve error handling */
//		printf("handle_open_response_message: Channel with id %u for outgoing steam %u is in state %u.", channel->id, o_stream, channel->state);
//		return;
//	}
//	if (find_channel_by_i_stream(pc, i_stream)) {
//		/* XXX: improve error handling */
//		printf("handle_open_response_message: Channel collision for channel with id %u and streams (in/out) = (%u/%u).", channel->id, i_stream, o_stream);
//		return;
//	}
	//channel->i_stream = i_stream;
	//channel->state = DATA_CHANNEL_OPEN;
	//pc->i_stream_channel[i_stream] = channel;
//	if (send_open_ack_message(pc->sock, o_stream)) {
//		channel->flags = 0;
//	} else {
//		channel->flags |= DATA_CHANNEL_FLAGS_SEND_ACK;
//	}
         print_status(  pc, channel );
	return;
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
                
                
                SInfo << "SctpAssociation with socket "  << this->socket  << " this " << this ;

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
		

#if MS_LOG_DEV_LEVEL == 3
		MS_DUMP_DATA(data, len);
#endif

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
                parameters.streamId = m_streamId;
                
                SInfo << "Send Sct pMessage to dataConsumer " <<  dataConsumer  <<  " to stream id " <<  parameters.streamId;; 

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
			  "error sending SCTP message [sid:%d, ppid:%" PRIu32 ", message size:%zu]: %s",
			  parameters.streamId,
			  ppid,
			  len,
			  std::strerror(errno));
		}
                
                  SInfo << "usrsctp_sendv with this "  << this <<  " socket " << this->socket  << " msg " <<  std::string((const char*) msg, len) ;
	}

	void SctpAssociation::HandleDataConsumer(RTC::DataConsumer* dataConsumer)
	{
//             if( m_streamId < 0  )
//             {
//                 SError << "SCTP is not initialzed yet. You can try to supply correct value from config file";
//                 return ;
//             }
//            else
//            dataConsumer->GetSctpStreamParameters().streamId = m_streamId; 
		
            
            auto streamId = dataConsumer->GetSctpStreamParameters().streamId;
		// We need more than 1024 OS. 
            if (m_streamId > this->os - 1)   // tjos cpde will never get executed
                    AddOutgoingStreams(/*force*/ false);
	}

	void SctpAssociation::DataProducerClosed(RTC::DataProducer* dataProducer)
	{
		
                if( m_streamId < 0  )
                {
                   SError << "SCTP is not initialzed yet. You can try to supply correct value from config file";
                   return ;
                }
                
		dataProducer->GetSctpStreamParameters().streamId = m_streamId;

		// Send SCTP_RESET_STREAMS to the remote.
		// https://tools.ietf.org/html/draft-ietf-rtcweb-data-channel-13#section-6.7
		if (this->isDataChannel)
			ResetSctpStream(m_streamId, StreamDirection::OUTGOING);
		else
			ResetSctpStream(m_streamId, StreamDirection::INCOMING);
	}

	void SctpAssociation::DataConsumerClosed(RTC::DataConsumer* dataConsumer)
	{
		
                if( m_streamId < 0  )
                {
                   SError << "SCTP is not initialzed yet. You can try to supply correct value from config file";
                   return ;
                }
                  
		dataConsumer->GetSctpStreamParameters().streamId = m_streamId;
		// Send SCTP_RESET_STREAMS to the remote.
		ResetSctpStream(m_streamId, StreamDirection::OUTGOING);
                
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
			MS_DEBUG_TAG(sctp, "SCTP_RESET_STREAMS sent [streamId:%d]", streamId);
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
			MS_WARN_TAG(sctp, "cannot add more outgoing streams [OS:%d]", this->os);

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

		MS_DEBUG_TAG(sctp, "adding %d outgoing streams", additionalOs);

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
              //  SInfo << "OnUsrSctpSendSctpData "  <<  this <<  " msg "  <<  data;
		this->listener->OnSctpAssociationSendData(this, data, len);
	}

	void SctpAssociation::OnUsrSctpReceiveSctpData(
	  uint16_t i_stream, uint16_t ssn, uint32_t ppid, int flags, const uint8_t* data, size_t len)
	{
              
         ///////////////////////////////
        struct rtcweb_datachannel_open_request *req;
	struct rtcweb_datachannel_open_response *rsp;
	struct rtcweb_datachannel_ack *ack, *msg;   
        //int length=0;
        switch (ppid) 
        {
            case DATA_CHANNEL_PPID_CONTROL:
                if (len < sizeof (struct rtcweb_datachannel_ack)) {
                    return;
                }
                msg = (struct rtcweb_datachannel_ack *) data;
                switch (msg->msg_type) {
                    case DC_TYPE_OPEN:
                        if (len < sizeof (struct rtcweb_datachannel_open_request)) {
                            /* XXX: error handling? */
                            return;
                        }
                        m_streamId = i_stream;
                        SInfo << " channel open request streamid " <<  i_stream << " this " << this;
                        
                       // req = (struct rtcweb_datachannel_open_request *) data;
                        handle_open_request_message(this, ( uint8_t*)data, len, i_stream);
                        break;
                   /* case 1:
                        if (len < sizeof (struct rtcweb_datachannel_open_response)) {
                           
                            return;
                        }
                         SInfo << " channel open response streamid " <<  i_stream ;
                        rsp = (struct rtcweb_datachannel_open_response *) data;
                        handle_open_response_message(this, rsp, len, i_stream);
                        break;
		   */
                    case DC_TYPE_ACK:
                        if (len < sizeof (struct rtcweb_datachannel_ack)) {
                            /* XXX: error handling? */
                            return;
                        }
                        
                         SInfo << " channel ack streamid " <<  i_stream ;
                         
                        ack = (struct rtcweb_datachannel_ack *) data;
                       // handle_open_ack_message(this, ack, len, i_stream);
                        break;
                    default:
                        //handle_unknown_message(buffer, length, i_stream);
                        SError <<  "Unknown state ";
                        break;
                }
                break;
            case DATA_CHANNEL_PPID_DOMSTRING:
            case DATA_CHANNEL_PPID_BINARY:
              //  handle_data_message(pc, buffer, length, i_stream);
                break;
            default:
              //  LInfo("Message of length %zu, PPID %u on stream %u received.\n",
                   //     length, ppid, i_stream);
                break;
        };


   
            
            
            
            
            
            //////////////////////////////////////////
            
            
            
                
		// Ignore WebRTC DataChannel Control DATA chunks.
		if (ppid == 50)
		{
			MS_WARN_TAG(sctp, "ignoring SCTP data with ppid:50 (WebRTC DataChannel Control)");

			return;
		}
                SInfo << "streamId: " << i_stream << " this "  << this  <<  " ssn: "  << ssn << " ppid: " << ppid << " data: " << data;
		if (this->messageBufferLen != 0 && ssn != this->lastSsnReceived)
		{
			MS_WARN_TAG(
			  sctp,
			  "message chunk received with different SSN while buffer not empty, buffer discarded [ssn:%" PRIu16
			  ", last ssn received:%d]",
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

			this->listener->OnSctpAssociationMessageReceived(this, i_stream, ppid, data, len);
		}
		// If end of message and there is buffered data, append data and notify buffer.
		else if (eor && this->messageBufferLen != 0)
		{
			std::memcpy(this->messageBuffer + this->messageBufferLen, data, len);
			this->messageBufferLen += len;

			MS_DEBUG_DEV("notifying listener [eor:1, buffer len:%zu]", this->messageBufferLen);

			this->listener->OnSctpAssociationMessageReceived(
			  this, i_stream, ppid, this->messageBuffer, this->messageBufferLen);

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

				SInfo <<  "SCTP adaptation indication " <<   notification->sn_adaptation_event.sai_adaptation_ind ;

				break;
			}

			case SCTP_ASSOC_CHANGE:
			{
				switch (notification->sn_assoc_change.sac_state)
				{
					case SCTP_COMM_UP:
					{
						  SInfo << "SCTP association connected, streams  out:"  << 	  notification->sn_assoc_change.sac_outbound_streams << " in:" <<  notification->sn_assoc_change.sac_inbound_streams;

						// Update our OS.
						this->os = notification->sn_assoc_change.sac_outbound_streams;

						// Increase if requested before connected.
						if (this->desiredOs > this->os)
							AddOutgoingStreams(/*force*/ true);

						if (this->state != SctpState::CONNECTED)
						{
                                                        SInfo << "OnSctpAssociationConnected "  ;
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
						  "SCTP remote association restarted, streams [out:%d, int:%d]",
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
				  "SCTP message sent failure [streamId:%d, ppid:%" PRIu32
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
					  "SCTP stream reset event [flags:%x, i|o:%s|%s, num streams:%d, stream ids:%s]",
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
					  "SCTP stream changed, streams [out:%d, in:%d, flags:%x]",
					  notification->sn_strchange_event.strchange_outstrms,
					  notification->sn_strchange_event.strchange_instrms,
					  notification->sn_strchange_event.strchange_flags);
				}
				else if (notification->sn_strchange_event.strchange_flags & SCTP_STREAM_RESET_DENIED)
				{
					MS_WARN_TAG(
					  sctp,
					  "SCTP stream change denied, streams [out:%d, in:%d, flags:%x]",
					  notification->sn_strchange_event.strchange_outstrms,
					  notification->sn_strchange_event.strchange_instrms,
					  notification->sn_strchange_event.strchange_flags);

					break;
				}
				else if (notification->sn_strchange_event.strchange_flags & SCTP_STREAM_RESET_FAILED)
				{
					MS_WARN_TAG(
					  sctp,
					  "SCTP stream change failed, streams [out:%d, in:%d, flags:%x]",
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
				  sctp, "unhandled SCTP event received [type:%d]", notification->sn_header.sn_type);
			}
		}
	}
} // namespace RTC
