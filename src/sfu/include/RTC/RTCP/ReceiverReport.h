#ifndef MS_RTC_RTCP_RECEIVER_REPORT_HPP
#define MS_RTC_RTCP_RECEIVER_REPORT_HPP

#include "common.h"
#include "Utils.h"
#include "RTC/RTCP/Packet.h"
#include <vector>

namespace RTC
{
	namespace RTCP
	{
		class ReceiverReport
		{
		public:
			/* Struct for RTCP receiver report. */
			struct Header
			{
				uint32_t ssrc;
				uint32_t fractionLost : 8;
				uint32_t totalLost : 24;
				uint32_t lastSeq;
				uint32_t jitter;
				uint32_t lsr;
				uint32_t dlsr;
			};

		public:
			static ReceiverReport* Parse(const uint8_t* data, size_t len);

		public:
			// Parsed Report. Points to an external data.
			explicit ReceiverReport(Header* header);
			explicit ReceiverReport(ReceiverReport* report);
			// Locally generated Report. Holds the data internally.
			ReceiverReport();

			void Dump() const;
			size_t Serialize(uint8_t* buffer);
			size_t GetSize() const;
			uint32_t GetSsrc() const;
			void SetSsrc(uint32_t ssrc);
			uint8_t GetFractionLost() const;
			void SetFractionLost(uint8_t fractionLost);
			int32_t GetTotalLost() const;
			void SetTotalLost(int32_t totalLost);
			uint32_t GetLastSeq() const;
			void SetLastSeq(uint32_t lastSeq);
			uint32_t GetJitter() const;
			void SetJitter(uint32_t jitter);
			uint32_t GetLastSenderReport() const;
			void SetLastSenderReport(uint32_t lsr);
			uint32_t GetDelaySinceLastSenderReport() const;
			void SetDelaySinceLastSenderReport(uint32_t dlsr);

		private:
			Header* header{ nullptr };
			uint8_t raw[sizeof(Header)]{ 0u };
		};

		class ReceiverReportPacket : public Packet
		{
		public:
			using Iterator = std::vector<ReceiverReport*>::iterator;

		public:
			static ReceiverReportPacket* Parse(const uint8_t* data, size_t len, size_t offset = 0);

		public:
			ReceiverReportPacket();
			explicit ReceiverReportPacket(CommonHeader* commonHeader);
			~ReceiverReportPacket() override;

			uint32_t GetSsrc() const;
			void SetSsrc(uint32_t ssrc);
			void AddReport(ReceiverReport* report);
			Iterator Begin();
			Iterator End();

			/* Pure virtual methods inherited from Packet. */
		public:
			void Dump() const override;
			size_t Serialize(uint8_t* buffer) override;
			Type GetType() const override;
			size_t GetCount() const override;
			size_t GetSize() const override;

		private:
			// SSRC of packet sender.
			uint32_t ssrc{ 0u };
			std::vector<ReceiverReport*> reports;
		};

		/* Inline instance methods. */

		inline ReceiverReport::ReceiverReport()
		{
			this->header = reinterpret_cast<Header*>(this->raw);
		}

		inline ReceiverReport::ReceiverReport(Header* header) : header(header)
		{
		}

		inline ReceiverReport::ReceiverReport(ReceiverReport* report) : header(report->header)
		{
		}

		inline size_t ReceiverReport::GetSize() const
		{
			return sizeof(Header);
		}

		inline uint32_t ReceiverReport::GetSsrc() const
		{
			return uint32_t{ ntohl(this->header->ssrc) };
		}

		inline void ReceiverReport::SetSsrc(uint32_t ssrc)
		{
			this->header->ssrc = uint32_t{ htonl(ssrc) };
		}

		inline uint8_t ReceiverReport::GetFractionLost() const
		{
			return uint8_t{ Utils::Byte::Get1Byte((uint8_t*)this->header, 4) };
		}

		inline void ReceiverReport::SetFractionLost(uint8_t fractionLost)
		{
			Utils::Byte::Set1Byte((uint8_t*)this->header, 4, fractionLost);
		}

		inline int32_t ReceiverReport::GetTotalLost() const
		{
			auto value = uint32_t{ Utils::Byte::Get3Bytes((uint8_t*)this->header, 5) };

			// Possitive value.
			if (((value >> 23) & 1) == 0)
				return value;

			// Negative value.
			if (value != 0x0800000)
				value &= ~(1 << 23);

			return -value;
		}

		inline void ReceiverReport::SetTotalLost(int32_t totalLost)
		{
			// Get the limit value for possitive and negative totalLost.
			int32_t clamp = (totalLost >= 0) ? totalLost > 0x07FFFFF ? 0x07FFFFF : totalLost
			                                 : -totalLost > 0x0800000 ? 0x0800000 : -totalLost;

			uint32_t value = (totalLost >= 0) ? (clamp & 0x07FFFFF) : (clamp | 0x0800000);

			Utils::Byte::Set3Bytes(reinterpret_cast<uint8_t*>(this->header), 5, value);
		}

		inline uint32_t ReceiverReport::GetLastSeq() const
		{
			return uint32_t{ ntohl(this->header->lastSeq) };
		}

		inline void ReceiverReport::SetLastSeq(uint32_t lastSeq)
		{
			this->header->lastSeq = uint32_t{ htonl(lastSeq) };
		}

		inline uint32_t ReceiverReport::GetJitter() const
		{
			return uint32_t{ ntohl(this->header->jitter) };
		}

		inline void ReceiverReport::SetJitter(uint32_t jitter)
		{
			this->header->jitter = uint32_t{ htonl(jitter) };
		}

		inline uint32_t ReceiverReport::GetLastSenderReport() const
		{
			return uint32_t{ ntohl(this->header->lsr) };
		}

		inline void ReceiverReport::SetLastSenderReport(uint32_t lsr)
		{
			this->header->lsr = uint32_t{ htonl(lsr) };
		}

		inline uint32_t ReceiverReport::GetDelaySinceLastSenderReport() const
		{
			return uint32_t{ ntohl(this->header->dlsr) };
		}

		inline void ReceiverReport::SetDelaySinceLastSenderReport(uint32_t dlsr)
		{
			this->header->dlsr = uint32_t{ htonl(dlsr) };
		}

		inline ReceiverReportPacket::ReceiverReportPacket() : Packet(Type::RR)
		{
		}

		inline ReceiverReportPacket::ReceiverReportPacket(CommonHeader* commonHeader)
		  : Packet(commonHeader)
		{
		}

		inline ReceiverReportPacket::~ReceiverReportPacket()
		{
			for (auto* report : this->reports)
			{
				delete report;
			}
		}

		// NOTE: We need to force this since when we parse a SenderReportPacket that
		// contains receive report blocks we also generate a second ReceiverReportPacket
		// from same data and len, so parent Packet::GetType() would return
		// this->type which would be SR instead of RR.
		inline Type ReceiverReportPacket::GetType() const
		{
			return Type::RR;
		}

		inline size_t ReceiverReportPacket::GetCount() const
		{
			return this->reports.size();
		}

		inline size_t ReceiverReportPacket::GetSize() const
		{
			size_t size = sizeof(Packet::CommonHeader) + 4u /* this->ssrc */;

			for (auto* report : reports)
			{
				size += report->GetSize();
			}

			return size;
		}

		inline uint32_t ReceiverReportPacket::GetSsrc() const
		{
			return this->ssrc;
		}

		inline void ReceiverReportPacket::SetSsrc(uint32_t ssrc)
		{
			this->ssrc = ssrc;
		}

		inline void ReceiverReportPacket::AddReport(ReceiverReport* report)
		{
			this->reports.push_back(report);
		}

		inline ReceiverReportPacket::Iterator ReceiverReportPacket::Begin()
		{
			return this->reports.begin();
		}

		inline ReceiverReportPacket::Iterator ReceiverReportPacket::End()
		{
			return this->reports.end();
		}
	} // namespace RTCP
} // namespace RTC

#endif
