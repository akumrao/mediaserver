#ifndef MS_RTC_SCTP_ASSOCIATION_HPP
#define MS_RTC_SCTP_ASSOCIATION_HPP

#include "common.h"
#include "Utils.h"
#include "RTC/DataConsumer.h"
#include "RTC/DataProducer.h"
#include <json.hpp>
#include <usrsctp.h>

using json = nlohmann::json;

namespace RTC
{
	class SctpAssociation
	{
	public:
		enum class SctpState
		{
			NEW = 1,
			CONNECTING,
			CONNECTED,
			FAILED,
			CLOSED
		};

	private:
		enum class StreamDirection
		{
			INCOMING = 1,
			OUTGOING
		};

	public:
		class Listener
		{
		public:
			virtual void OnSctpAssociationConnecting(RTC::SctpAssociation* sctpAssociation) = 0;
			virtual void OnSctpAssociationConnected(RTC::SctpAssociation* sctpAssociation)  = 0;
			virtual void OnSctpAssociationFailed(RTC::SctpAssociation* sctpAssociation)     = 0;
			virtual void OnSctpAssociationClosed(RTC::SctpAssociation* sctpAssociation)     = 0;
			virtual void OnSctpAssociationSendData(
			  RTC::SctpAssociation* sctpAssociation, const uint8_t* data, size_t len) = 0;
			virtual void OnSctpAssociationMessageReceived(
			  RTC::SctpAssociation* sctpAssociation,
			  uint16_t streamId,
			  uint32_t ppid,
			  const uint8_t* msg,
			  size_t len) = 0;
		};

	public:
		static bool IsSctp(const uint8_t* data, size_t len);

	public:
		SctpAssociation(
		  Listener* listener, uint16_t os, uint16_t mis, size_t maxSctpMessageSize, bool isDataChannel);
		~SctpAssociation();

	public:
		void FillJson(json& jsonObject) const;
		void TransportConnected();
		size_t GetMaxSctpMessageSize() const;
		SctpState GetState() const;
		void ProcessSctpData(const uint8_t* data, size_t len);
		void SendSctpMessage(RTC::DataConsumer* dataConsumer, uint32_t ppid, const uint8_t* msg, size_t len);
		void HandleDataConsumer(RTC::DataConsumer* dataConsumer);
		void DataProducerClosed(RTC::DataProducer* dataProducer);
		void DataConsumerClosed(RTC::DataConsumer* dataConsumer);

	private:
		void ResetSctpStream(uint16_t streamId, StreamDirection);
		void AddOutgoingStreams(bool force = false);

		/* Callbacks fired by usrsctp events. */
	public:
		void OnUsrSctpSendSctpData(void* buffer, size_t len);
		void OnUsrSctpReceiveSctpData(
		  uint16_t streamId, uint16_t ssn, uint32_t ppid, int flags, const uint8_t* data, size_t len);
		void OnUsrSctpReceiveSctpNotification(union sctp_notification* notification, size_t len);

	private:
		// Passed by argument.
		Listener* listener{ nullptr };
		uint16_t os{ 1024 };
		uint16_t mis{ 1024 };
		size_t maxSctpMessageSize{ 262144 };
		bool isDataChannel{ false };
		// Allocated by this.
		uint8_t* messageBuffer{ nullptr };
		// Others.
		SctpState state{ SctpState::NEW };
		//struct socket* socket{ nullptr };
		uint16_t desiredOs{ 0 };
		size_t messageBufferLen{ 0 };
		uint16_t lastSsnReceived{ 0 }; // Valid for us since no SCTP I-DATA support.
        public:
                struct socket* socket{ nullptr };
	};

	/* Inline static methods. */

	inline bool SctpAssociation::IsSctp(const uint8_t* data, size_t len)
	{
		
		return (
			(len >= 12) &&
			// Must have Source Port Number and Destination Port Number set to 5000 (hack).
			(Utils::Byte::Get2Bytes(data, 0) == 5000) &&
			(Utils::Byte::Get2Bytes(data, 2) == 5000)
		);
		
	}

	/* Inline instance methods. */

	inline size_t SctpAssociation::GetMaxSctpMessageSize() const
	{
		return this->maxSctpMessageSize;
	}

	inline SctpAssociation::SctpState SctpAssociation::GetState() const
	{
		return this->state;
	}
} // namespace RTC

#endif
