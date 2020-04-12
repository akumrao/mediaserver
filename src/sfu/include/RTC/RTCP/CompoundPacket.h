#ifndef MS_RTC_RTCP_COMPOUND_PACKET_HPP
#define MS_RTC_RTCP_COMPOUND_PACKET_HPP

#include "common.h"
#include "RTC/RTCP/ReceiverReport.h"
#include "RTC/RTCP/Sdes.h"
#include "RTC/RTCP/SenderReport.h"
#include "RTC/RTCP/XrReceiverReferenceTime.h"
#include <vector>

namespace RTC
{
	namespace RTCP
	{
		class CompoundPacket
		{
		public:
			CompoundPacket() = default;

		public:
			const uint8_t* GetData() const;
			size_t GetSize() const;
			size_t GetSenderReportCount() const;
			size_t GetReceiverReportCount() const;
			void Dump();
			void AddSenderReport(SenderReport* report);
			void AddReceiverReport(ReceiverReport* report);
			void AddSdesChunk(SdesChunk* chunk);
			void AddReceiverReferenceTime(ReceiverReferenceTime* report);
			bool HasSenderReport();
			bool HasReceiverReferenceTime();
			void Serialize(uint8_t* data);

		private:
			uint8_t* header{ nullptr };
			size_t size{ 0 };
			SenderReportPacket senderReportPacket;
			ReceiverReportPacket receiverReportPacket;
			SdesPacket sdesPacket;
			ExtendedReportPacket xrPacket;
		};

		/* Inline methods. */

		inline const uint8_t* CompoundPacket::GetData() const
		{
			return this->header;
		}

		inline size_t CompoundPacket::GetSize() const
		{
			return this->size;
		}

		inline size_t CompoundPacket::GetSenderReportCount() const
		{
			return this->senderReportPacket.GetCount();
		}

		inline size_t CompoundPacket::GetReceiverReportCount() const
		{
			return this->receiverReportPacket.GetCount();
		}

		inline void CompoundPacket::AddReceiverReport(ReceiverReport* report)
		{
			this->receiverReportPacket.AddReport(report);
		}

		inline void CompoundPacket::AddSdesChunk(SdesChunk* chunk)
		{
			this->sdesPacket.AddChunk(chunk);
		}

		inline void CompoundPacket::AddReceiverReferenceTime(ReceiverReferenceTime* report)
		{
			this->xrPacket.AddReport(report);
		}

		inline bool CompoundPacket::HasSenderReport()
		{
			return this->senderReportPacket.Begin() != this->senderReportPacket.End();
		}

		inline bool CompoundPacket::HasReceiverReferenceTime()
		{
			return std::any_of(
			  this->xrPacket.Begin(), this->xrPacket.End(), [](const ExtendedReportBlock* report) {
				  return report->GetType() == ExtendedReportBlock::Type::RRT;
			  });
		}
	} // namespace RTCP
} // namespace RTC

#endif
