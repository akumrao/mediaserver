#define MS_CLASS "RTC::RTCP::FeedbackPsPli"
// #define MS_LOG_DEV_LEVEL 3

#include "RTC/RTCP/FeedbackPsPli.h"
#include "LoggerTag.h"

namespace RTC
{
	namespace RTCP
	{
		/* Class methods. */

		FeedbackPsPliPacket* FeedbackPsPliPacket::Parse(const uint8_t* data, size_t len)
		{
			MS_TRACE();

			if (len < sizeof(CommonHeader) + sizeof(FeedbackPacket::Header))
			{
				MS_WARN_TAG(rtcp, "not enough space for Feedback packet, discarded");

				return nullptr;
			}

			auto* commonHeader = const_cast<CommonHeader*>(reinterpret_cast<const CommonHeader*>(data));

			std::unique_ptr<FeedbackPsPliPacket> packet(new FeedbackPsPliPacket(commonHeader));

			return packet.release();
		}

		void FeedbackPsPliPacket::Dump() const
		{
			MS_TRACE();

			MS_DUMP("<FeedbackPsPliPacket>");
			FeedbackPsPacket::Dump();
			MS_DUMP("</FeedbackPsPliPacket>");
		}
	} // namespace RTCP
} // namespace RTC
