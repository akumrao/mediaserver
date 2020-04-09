#define MS_CLASS "RTC::Codecs"
// #define MS_LOG_DEV_LEVEL 3

#include "RTC/Codecs/Codecs.h"
#include "LoggerTag.h"
#include "RTC/Codecs/VP8.h"

namespace RTC
{
	namespace Codecs
	{
		void ProcessRtpPacket(RTC::RtpPacket* packet, const RTC::RtpCodecMimeType& mimeType)
		{
			MS_TRACE();

			switch (mimeType.type)
			{
				case RTC::RtpCodecMimeType::Type::VIDEO:
				{
					switch (mimeType.subtype)
					{
						case RTC::RtpCodecMimeType::Subtype::VP8:
						{
							RTC::Codecs::VP8::ProcessRtpPacket(packet);

							break;
						}

						case RTC::RtpCodecMimeType::Subtype::VP9:
						{
							RTC::Codecs::VP9::ProcessRtpPacket(packet);

							break;
						}

						case RTC::RtpCodecMimeType::Subtype::H264:
						{
							RTC::Codecs::H264::ProcessRtpPacket(packet);

							break;
						}

						default:;
					}
				}

				default:;
			}
		}
	} // namespace Codecs
} // namespace RTC
