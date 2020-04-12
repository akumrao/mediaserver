#define MS_CLASS "RTC::RTCP::FeedbackRtp"
// #define MS_LOG_DEV_LEVEL 3

#include "RTC/RTCP/FeedbackRtp.h"
#include "LoggerTag.h"
#include "RTC/RTCP/FeedbackRtpEcn.h"
#include "RTC/RTCP/FeedbackRtpNack.h"
#include "RTC/RTCP/FeedbackRtpTllei.h"
#include "RTC/RTCP/FeedbackRtpTmmb.h"

namespace RTC
{
	namespace RTCP
	{
		/* Class methods. */

		template<typename Item>
		FeedbackRtpItemsPacket<Item>* FeedbackRtpItemsPacket<Item>::Parse(const uint8_t* data, size_t len)
		{
			MS_TRACE();

			if (len < sizeof(CommonHeader) + sizeof(FeedbackPacket::Header))
			{
				MS_WARN_TAG(rtcp, "not enough space for Feedback packet, discarded");

				return nullptr;
			}

			auto* commonHeader = const_cast<CommonHeader*>(reinterpret_cast<const CommonHeader*>(data));

			std::unique_ptr<FeedbackRtpItemsPacket<Item>> packet(
			  new FeedbackRtpItemsPacket<Item>(commonHeader));

			size_t offset = sizeof(CommonHeader) + sizeof(FeedbackPacket::Header);

			while (len > offset)
			{
				auto* item = FeedbackItem::Parse<Item>(data + offset, len - offset);

				if (item)
				{
					packet->AddItem(item);
					offset += item->GetSize();
				}
				else
				{
					break;
				}
			}

			return packet.release();
		}

		/* Instance methods. */

		template<typename Item>
		size_t FeedbackRtpItemsPacket<Item>::Serialize(uint8_t* buffer)
		{
			MS_TRACE();

			size_t offset = FeedbackPacket::Serialize(buffer);

			for (auto* item : this->items)
			{
				offset += item->Serialize(buffer + offset);
			}

			return offset;
		}

		template<typename Item>
		void FeedbackRtpItemsPacket<Item>::Dump() const
		{
			MS_TRACE();

			MS_DUMP("<%s>", FeedbackRtpPacket::MessageType2String(Item::messageType).c_str());
			FeedbackRtpPacket::Dump();
			for (auto* item : this->items)
			{
				item->Dump();
			}
			MS_DUMP("</%s>", FeedbackRtpPacket::MessageType2String(Item::messageType).c_str());
		}

		// Explicit instantiation to have all FeedbackRtpPacket definitions in this file.
		template class FeedbackRtpItemsPacket<FeedbackRtpNackItem>;
		template class FeedbackRtpItemsPacket<FeedbackRtpTmmbrItem>;
		template class FeedbackRtpItemsPacket<FeedbackRtpTmmbnItem>;
		template class FeedbackRtpItemsPacket<FeedbackRtpTlleiItem>;
		template class FeedbackRtpItemsPacket<FeedbackRtpEcnItem>;
	} // namespace RTCP
} // namespace RTC
