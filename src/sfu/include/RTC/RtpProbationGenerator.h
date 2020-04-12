#ifndef MS_RTC_RTP_PROBATION_GENERATOR_HPP
#define MS_RTC_RTP_PROBATION_GENERATOR_HPP

#include "common.h"
#include "RTC/RtpPacket.h"

namespace RTC
{
	// SSRC of the probation RTP stream.
	constexpr uint32_t RtpProbationSsrc{ 1234u };

	class RtpProbationGenerator
	{
	public:
		explicit RtpProbationGenerator();
		virtual ~RtpProbationGenerator();

	public:
		RTC::RtpPacket* GetNextPacket(size_t size);

	private:
		// Allocated by this.
		uint8_t* probationPacketBuffer{ nullptr };
		RTC::RtpPacket* probationPacket{ nullptr };
	}; // namespace RTC

} // namespace RTC

#endif
