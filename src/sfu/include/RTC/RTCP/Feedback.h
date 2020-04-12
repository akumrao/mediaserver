#ifndef MS_RTC_RTCP_FEEDBACK_HPP
#define MS_RTC_RTCP_FEEDBACK_HPP

#include "common.h"
#include "RTC/RTCP/FeedbackItem.h"
#include "RTC/RTCP/Packet.h"

namespace RTC
{
	namespace RTCP
	{
		template<typename T>
		class FeedbackPacket : public Packet
		{
		public:
			/* Struct for RTP Feedback message. */
			struct Header
			{
				uint32_t senderSsrc;
				uint32_t mediaSsrc;
			};

		public:
			static RTCP::Type rtcpType;
			static FeedbackPacket<T>* Parse(const uint8_t* data, size_t len);
			static const std::string& MessageType2String(typename T::MessageType type);

		private:
			static std::map<typename T::MessageType, std::string> type2String;

		public:
			typename T::MessageType GetMessageType() const;
			uint32_t GetSenderSsrc() const;
			void SetSenderSsrc(uint32_t ssrc);
			uint32_t GetMediaSsrc() const;
			void SetMediaSsrc(uint32_t ssrc);

			/* Pure virtual methods inherited from Packet. */
		public:
			void Dump() const override;
			size_t Serialize(uint8_t* buffer) override;
			size_t GetCount() const override;
			size_t GetSize() const override;

		protected:
			explicit FeedbackPacket(CommonHeader* commonHeader);
			FeedbackPacket(typename T::MessageType messageType, uint32_t senderSsrc, uint32_t mediaSsrc);
			~FeedbackPacket() override;

		private:
			Header* header{ nullptr };
			uint8_t* raw{ nullptr };
			typename T::MessageType messageType;
		};

		class FeedbackPs
		{
		public:
			enum class MessageType : uint8_t
			{
				PLI   = 1,
				SLI   = 2,
				RPSI  = 3,
				FIR   = 4,
				TSTR  = 5,
				TSTN  = 6,
				VBCM  = 7,
				PSLEI = 8,
				ROI   = 9,
				AFB   = 15,
				EXT   = 31
			};
		};

		class FeedbackRtp
		{
		public:
			enum class MessageType : uint8_t
			{
				NACK   = 1,
				TMMBR  = 3,
				TMMBN  = 4,
				SR_REQ = 5,
				RAMS   = 6,
				TLLEI  = 7,
				ECN    = 8,
				PS     = 9,
				TCC    = 15,
				EXT    = 31
			};
		};

		using FeedbackPsPacket  = FeedbackPacket<RTC::RTCP::FeedbackPs>;
		using FeedbackRtpPacket = FeedbackPacket<RTC::RTCP::FeedbackRtp>;

		/* Inline instance methods. */

		template<typename T>
		inline typename T::MessageType FeedbackPacket<T>::GetMessageType() const
		{
			return this->messageType;
		}

		template<typename T>
		inline size_t FeedbackPacket<T>::GetCount() const
		{
			return static_cast<size_t>(this->GetMessageType());
		}

		template<typename T>
		inline size_t FeedbackPacket<T>::GetSize() const
		{
			return sizeof(CommonHeader) + sizeof(Header);
		}

		template<typename T>
		inline uint32_t FeedbackPacket<T>::GetSenderSsrc() const
		{
			return uint32_t{ ntohl(this->header->senderSsrc) };
		}

		template<typename T>
		inline void FeedbackPacket<T>::SetSenderSsrc(uint32_t ssrc)
		{
			this->header->senderSsrc = uint32_t{ htonl(ssrc) };
		}

		template<typename T>
		inline uint32_t FeedbackPacket<T>::GetMediaSsrc() const
		{
			return uint32_t{ ntohl(this->header->mediaSsrc) };
		}

		template<typename T>
		inline void FeedbackPacket<T>::SetMediaSsrc(uint32_t ssrc)
		{
			this->header->mediaSsrc = uint32_t{ htonl(ssrc) };
		}
	} // namespace RTCP
} // namespace RTC

#endif
