#ifndef MS_RTC_RTCP_FEEDBACK_PS_PLI_HPP
#define MS_RTC_RTCP_FEEDBACK_PS_PLI_HPP

#include "common.h"
#include "RTC/RTCP/Feedback.h"

namespace RTC
{
	namespace RTCP
	{
		class FeedbackPsPliPacket : public FeedbackPsPacket
		{
		public:
			static FeedbackPsPliPacket* Parse(const uint8_t* data, size_t len);

		public:
			// Parsed Report. Points to an external data.
			explicit FeedbackPsPliPacket(CommonHeader* commonHeader);
			FeedbackPsPliPacket(uint32_t senderSsrc, uint32_t mediaSsrc);
			~FeedbackPsPliPacket() override = default;

		public:
			void Dump() const override;
		};

		/* Inline instance methods. */

		inline FeedbackPsPliPacket::FeedbackPsPliPacket(CommonHeader* commonHeader)
		  : FeedbackPsPacket(commonHeader)
		{
		}

		inline FeedbackPsPliPacket::FeedbackPsPliPacket(uint32_t senderSsrc, uint32_t mediaSsrc)
		  : FeedbackPsPacket(FeedbackPs::MessageType::PLI, senderSsrc, mediaSsrc)
		{
		}
	} // namespace RTCP
} // namespace RTC

#endif
