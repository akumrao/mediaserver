#ifndef MS_RTC_RTCP_FEEDBACK_REMB_HPP
#define MS_RTC_RTCP_FEEDBACK_REMB_HPP

#include "common.h"
#include "RTC/RTCP/FeedbackPsAfb.h"
#include <vector>

/* draft-alvestrand-rmcat-remb-03
 * RTCP message for Receiver Estimated Maximum Bitrate (REMB)

   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |V=2|P| FMT=15  |   PT=206      |             length            |
  +=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+=+
  |                  SSRC of packet sender                        |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |                  SSRC of media source                         |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |  Unique identifier 'R' 'E' 'M' 'B'                            |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |  Num SSRC     | BR Exp    |  BR Mantissa                      |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |   SSRC feedback                                               |
  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
  |  ...                                                          |
 */

namespace RTC
{
	namespace RTCP
	{
		class FeedbackPsRembPacket : public FeedbackPsAfbPacket
		{
		public:
			// 'R' 'E' 'M' 'B'.
			static uint32_t uniqueIdentifier;

		public:
			static FeedbackPsRembPacket* Parse(const uint8_t* data, size_t len);

		public:
			// Parsed Report. Points to an external data.
			FeedbackPsRembPacket(uint32_t senderSsrc, uint32_t mediaSsrc);
			FeedbackPsRembPacket(CommonHeader* commonHeader, size_t availableLen);
			~FeedbackPsRembPacket() override = default;

			bool IsCorrect();
			void SetBitrate(uint64_t bitrate);
			void SetSsrcs(const std::vector<uint32_t>& ssrcs);
			uint64_t GetBitrate();
			const std::vector<uint32_t>& GetSsrcs();

			/* Pure virtual methods inherited from Packet. */
		public:
			void Dump() const override;
			size_t Serialize(uint8_t* buffer) override;
			size_t GetSize() const override;

		private:
			std::vector<uint32_t> ssrcs;
			// Bitrate represented in bps.
			uint64_t bitrate{ 0 };
			bool isCorrect{ true };
		};

		/* Inline instance methods. */

		inline FeedbackPsRembPacket::FeedbackPsRembPacket(uint32_t senderSsrc, uint32_t mediaSsrc)
		  : FeedbackPsAfbPacket(senderSsrc, mediaSsrc, FeedbackPsAfbPacket::Application::REMB)
		{
		}

		inline bool FeedbackPsRembPacket::IsCorrect()
		{
			return this->isCorrect;
		}

		inline void FeedbackPsRembPacket::SetBitrate(uint64_t bitrate)
		{
			this->bitrate = bitrate;
		}

		inline void FeedbackPsRembPacket::SetSsrcs(const std::vector<uint32_t>& ssrcs)
		{
			this->ssrcs = ssrcs;
		}

		inline uint64_t FeedbackPsRembPacket::GetBitrate()
		{
			return this->bitrate;
		}

		inline const std::vector<uint32_t>& FeedbackPsRembPacket::GetSsrcs()
		{
			return this->ssrcs;
		}

		inline size_t FeedbackPsRembPacket::GetSize() const
		{
			return FeedbackPsPacket::GetSize() + 8 + (4u * this->ssrcs.size());
		}
	} // namespace RTCP
} // namespace RTC

#endif
