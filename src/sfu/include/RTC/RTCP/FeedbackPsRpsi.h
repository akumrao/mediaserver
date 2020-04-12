#ifndef MS_RTC_RTCP_FEEDBACK_PS_RPSI_HPP
#define MS_RTC_RTCP_FEEDBACK_PS_RPSI_HPP

#include "common.h"
#include "RTC/RTCP/FeedbackPs.h"

/* RFC 4585
 * Reference Picture Selection Indication (RPSI)
 *

   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |      PB       |
                  |0| Payload Type|
                                  |    Native RPSI bit string     |
  |   defined per codec          ...                | Padding (0) |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 */

namespace RTC
{
	namespace RTCP
	{
		class FeedbackPsRpsiItem : public FeedbackItem
		{
			const static size_t maxBitStringSize{ 6 };
			const static size_t bitStringOffset{ 2 };

		public:
			struct Header
			{
				uint8_t paddingBits;
				uint8_t zero : 1;
				uint8_t payloadType : 7;
				uint8_t bitString[maxBitStringSize];
			};

		public:
			static const FeedbackPs::MessageType messageType{ FeedbackPs::MessageType::RPSI };

		public:
			explicit FeedbackPsRpsiItem(Header* header);
			explicit FeedbackPsRpsiItem(FeedbackPsRpsiItem* item);
			FeedbackPsRpsiItem(uint8_t payloadType, uint8_t* bitString, size_t length);
			~FeedbackPsRpsiItem() override = default;

			bool IsCorrect() const;
			uint8_t GetPayloadType() const;
			uint8_t* GetBitString() const;
			size_t GetLength() const;

			/* Virtual methods inherited from FeedbackItem. */
		public:
			void Dump() const override;
			size_t Serialize(uint8_t* buffer) override;
			size_t GetSize() const override;

		private:
			Header* header{ nullptr };
			size_t length{ 0 };
		};

		// Rpsi packet declaration.
		using FeedbackPsRpsiPacket = FeedbackPsItemsPacket<FeedbackPsRpsiItem>;

		/* Inline instance methods. */

		inline FeedbackPsRpsiItem::FeedbackPsRpsiItem(FeedbackPsRpsiItem* item) : header(item->header)
		{
		}

		inline size_t FeedbackPsRpsiItem::GetSize() const
		{
			return sizeof(Header);
		}

		inline uint8_t FeedbackPsRpsiItem::GetPayloadType() const
		{
			return this->header->payloadType;
		}

		inline uint8_t* FeedbackPsRpsiItem::GetBitString() const
		{
			return this->header->bitString;
		}

		inline size_t FeedbackPsRpsiItem::GetLength() const
		{
			return this->length;
		}

		inline bool FeedbackPsRpsiItem::IsCorrect() const
		{
			return this->isCorrect;
		}
	} // namespace RTCP
} // namespace RTC

#endif
