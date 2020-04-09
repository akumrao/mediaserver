#define MS_CLASS "RTC::RTCP::FeedbackPsAfb"
// #define MS_LOG_DEV_LEVEL 3

#include "RTC/RTCP/FeedbackPsAfb.h"
#include "LoggerTag.h"
#include "Utils.h"
#include "RTC/RTCP/FeedbackPsRemb.h"
#include <cstring>

namespace RTC
{
	namespace RTCP
	{
		/* Class methods. */

		FeedbackPsAfbPacket* FeedbackPsAfbPacket::Parse(const uint8_t* data, size_t len)
		{
			MS_TRACE();

			if (len < sizeof(CommonHeader) + sizeof(FeedbackPacket::Header))
			{
				MS_WARN_TAG(rtcp, "not enough space for Feedback packet, discarded");

				return nullptr;
			}

			auto* commonHeader = const_cast<CommonHeader*>(reinterpret_cast<const CommonHeader*>(data));

			std::unique_ptr<FeedbackPsAfbPacket> packet;

			constexpr size_t Offset = sizeof(CommonHeader) + sizeof(FeedbackPacket::Header);

			
			if (
				len >= sizeof(CommonHeader) + sizeof(FeedbackPacket::Header) + 4 &&
				Utils::Byte::Get4Bytes(data, Offset) == FeedbackPsRembPacket::uniqueIdentifier
			)
			
			{
				packet.reset(FeedbackPsRembPacket::Parse(data, len));
			}
			else
			{
				packet.reset(new FeedbackPsAfbPacket(commonHeader));
			}

			return packet.release();
		}

		size_t FeedbackPsAfbPacket::Serialize(uint8_t* buffer)
		{
			MS_TRACE();

			size_t offset = FeedbackPsPacket::Serialize(buffer);

			// Copy the content.
			std::memcpy(buffer + offset, this->data, this->size);

			return offset + this->size;
		}

		void FeedbackPsAfbPacket::Dump() const
		{
			MS_TRACE();

			MS_DUMP("<FeedbackPsAfbPacket>");
			FeedbackPsPacket::Dump();
			MS_DUMP("</FeedbackPsAfbPacket>");
		}
	} // namespace RTCP
} // namespace RTC
