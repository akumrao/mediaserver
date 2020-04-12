#define MS_CLASS "RTC::RTCP::CompoundPacket"
// #define MS_LOG_DEV_LEVEL 3

#include "RTC/RTCP/CompoundPacket.h"
#include "LoggerTag.h"

namespace RTC
{
	namespace RTCP
	{
		/* Instance methods. */

		void CompoundPacket::Serialize(uint8_t* data)
		{
			MS_TRACE();

			this->header = data;

			// Calculate the total required size for the entire message.
			if (HasSenderReport())
			{
				this->size = this->senderReportPacket.GetSize();

				if (this->receiverReportPacket.GetCount() != 0u)
				{
					this->size += sizeof(ReceiverReport::Header) * this->receiverReportPacket.GetCount();
				}
			}
			// If no sender nor receiver reports are present send an empty Receiver Report
			// packet as the head of the compound packet.
			else
			{
				this->size = this->receiverReportPacket.GetSize();
			}

			if (this->sdesPacket.GetCount() != 0u)
				this->size += this->sdesPacket.GetSize();

			if (this->xrPacket.Begin() != this->xrPacket.End())
				this->size += this->xrPacket.GetSize();

			// Fill it.
			size_t offset{ 0 };

			if (HasSenderReport())
			{
				this->senderReportPacket.Serialize(this->header);
				offset = this->senderReportPacket.GetSize();

				// Fix header count field.
				auto* header = reinterpret_cast<Packet::CommonHeader*>(this->header);

				header->count = 0;

				if (this->receiverReportPacket.GetCount() != 0u)
				{
					// Fix header length field.
					size_t length =
					  ((sizeof(SenderReport::Header) +
					    (sizeof(ReceiverReport::Header) * this->receiverReportPacket.GetCount())) /
					   4);

					header->length = uint16_t{ htons(length) };

					// Fix header count field.
					header->count = this->receiverReportPacket.GetCount();

					auto it = this->receiverReportPacket.Begin();

					for (; it != this->receiverReportPacket.End(); ++it)
					{
						ReceiverReport* report = (*it);

						report->Serialize(this->header + offset);
						offset += sizeof(ReceiverReport::Header);
					}
				}
			}
			else
			{
				this->receiverReportPacket.Serialize(this->header);
				offset = this->receiverReportPacket.GetSize();
			}

			if (this->sdesPacket.GetCount() != 0u)
				offset += this->sdesPacket.Serialize(this->header + offset);

			if (this->xrPacket.Begin() != this->xrPacket.End())
				this->xrPacket.Serialize(this->header + offset);
		}

		void CompoundPacket::Dump()
		{
			MS_TRACE();

			MS_DUMP("<CompoundPacket>");

			if (HasSenderReport())
			{
				this->senderReportPacket.Dump();

				if (this->receiverReportPacket.GetCount() != 0u)
					this->receiverReportPacket.Dump();
			}
			else
			{
				this->receiverReportPacket.Dump();
			}

			if (this->sdesPacket.GetCount() != 0u)
				this->sdesPacket.Dump();

			if (this->xrPacket.Begin() != this->xrPacket.End())
				this->xrPacket.Dump();

			MS_DUMP("</CompoundPacket>");
		}

		void CompoundPacket::AddSenderReport(SenderReport* report)
		{
			assertm(!HasSenderReport(), "a Sender Report is already present");

			this->senderReportPacket.AddReport(report);
		}
	} // namespace RTCP
} // namespace RTC
