#define MS_CLASS "RTC::RtpCodecMimeType"
// #define MS_LOG_DEV_LEVEL 3

#include "LoggerTag.h"
#include "base/error.h"
#include "Utils.h"
#include "RTC/RtpDictionaries.h"

namespace RTC
{
	/* Class variables. */

	
	std::unordered_map<std::string, RtpCodecMimeType::Type> RtpCodecMimeType::string2Type =
	{
		{ "audio", RtpCodecMimeType::Type::AUDIO },
		{ "video", RtpCodecMimeType::Type::VIDEO }
	};
	std::map<RtpCodecMimeType::Type, std::string> RtpCodecMimeType::type2String =
	{
		{ RtpCodecMimeType::Type::AUDIO, "audio" },
		{ RtpCodecMimeType::Type::VIDEO, "video" }
	};
	std::unordered_map<std::string, RtpCodecMimeType::Subtype> RtpCodecMimeType::string2Subtype =
	{
		// Audio codecs:
		{ "opus",            RtpCodecMimeType::Subtype::OPUS            },
		{ "pcma",            RtpCodecMimeType::Subtype::PCMA            },
		{ "pcmu",            RtpCodecMimeType::Subtype::PCMU            },
		{ "isac",            RtpCodecMimeType::Subtype::ISAC            },
		{ "g722",            RtpCodecMimeType::Subtype::G722            },
		{ "ilbc",            RtpCodecMimeType::Subtype::ILBC            },
		{ "silk",            RtpCodecMimeType::Subtype::SILK            },
		// Video codecs:
		{ "vp8",             RtpCodecMimeType::Subtype::VP8             },
		{ "vp9",             RtpCodecMimeType::Subtype::VP9             },
		{ "h264",            RtpCodecMimeType::Subtype::H264            },
		{ "x-h264uc",        RtpCodecMimeType::Subtype::X_H264UC        },
		{ "h265",            RtpCodecMimeType::Subtype::H265            },
                { "multiplex",       RtpCodecMimeType::Subtype::multiplex       },
                
		// Complementary codecs:
		{ "cn",              RtpCodecMimeType::Subtype::CN              },
		{ "telephone-event", RtpCodecMimeType::Subtype::TELEPHONE_EVENT },
		// Feature codecs:
		{ "rtx",             RtpCodecMimeType::Subtype::RTX             },
		{ "ulpfec",          RtpCodecMimeType::Subtype::ULPFEC          },
		{ "flexfec",         RtpCodecMimeType::Subtype::FLEXFEC         },
		{ "x-ulpfecuc",      RtpCodecMimeType::Subtype::X_ULPFECUC      },
		{ "red",             RtpCodecMimeType::Subtype::RED             }
	};
	std::map<RtpCodecMimeType::Subtype, std::string> RtpCodecMimeType::subtype2String =
	{
		// Audio codecs:
		{ RtpCodecMimeType::Subtype::OPUS,            "opus"            },
		{ RtpCodecMimeType::Subtype::PCMA,            "PCMA"            },
		{ RtpCodecMimeType::Subtype::PCMU,            "PCMU"            },
		{ RtpCodecMimeType::Subtype::ISAC,            "ISAC"            },
		{ RtpCodecMimeType::Subtype::G722,            "G722"            },
		{ RtpCodecMimeType::Subtype::ILBC,            "iLBC"            },
		{ RtpCodecMimeType::Subtype::SILK,            "SILK"            },
		// Video codecs:
		{ RtpCodecMimeType::Subtype::VP8,             "VP8"             },
		{ RtpCodecMimeType::Subtype::VP9,             "VP9"             },
		{ RtpCodecMimeType::Subtype::H264,            "H264"            },
		{ RtpCodecMimeType::Subtype::X_H264UC,        "X-H264UC"        },
		{ RtpCodecMimeType::Subtype::H265,            "H265"            },
                { RtpCodecMimeType::Subtype::multiplex,       "multiplex"       },
		// Complementary codecs:
		{ RtpCodecMimeType::Subtype::CN,              "CN"              },
		{ RtpCodecMimeType::Subtype::TELEPHONE_EVENT, "telephone-event" },
		// Feature codecs:
		{ RtpCodecMimeType::Subtype::RTX,             "rtx"             },
		{ RtpCodecMimeType::Subtype::ULPFEC,          "ulpfec"          },
		{ RtpCodecMimeType::Subtype::FLEXFEC,         "flexfec"         },
		{ RtpCodecMimeType::Subtype::X_ULPFECUC,      "x-ulpfecuc"      },
		{ RtpCodecMimeType::Subtype::RED,             "red"             }
	};
	

	/* Instance methods. */

	void RtpCodecMimeType::SetMimeType(const std::string& mimeType)
	{
		MS_TRACE();

		auto slashPos = mimeType.find('/');

		if (slashPos == std::string::npos || slashPos == 0 || slashPos == mimeType.length() - 1)
		{
			base::uv::throwError("wrong codec MIME");
		}

		std::string type    = mimeType.substr(0, slashPos);
		std::string subtype = mimeType.substr(slashPos + 1);

		// Force lowcase names.
		Utils::String::ToLowerCase(type);
		Utils::String::ToLowerCase(subtype);

		// Set MIME type.
		{
			auto it = RtpCodecMimeType::string2Type.find(type);

			if (it == RtpCodecMimeType::string2Type.end())
				base::uv::throwError("unknown codec MIME type " + type);

			this->type = it->second;
		}

		// Set MIME subtype.
		{
			auto it = RtpCodecMimeType::string2Subtype.find(subtype);

			if (it == RtpCodecMimeType::string2Subtype.end())
				base::uv::throwError("unknown codec MIME subtype " + subtype);

			this->subtype = it->second;
		}

		// Set mimeType.
		this->mimeType = RtpCodecMimeType::type2String[this->type] + "/" +
		                 RtpCodecMimeType::subtype2String[this->subtype];
	}

	void RtpCodecMimeType::UpdateMimeType()
	{
		MS_TRACE();

		assertm(this->type != Type::UNSET, "type unset");
		assertm(this->subtype != Subtype::UNSET, "subtype unset");

		// Set mimeType.
		this->mimeType = RtpCodecMimeType::type2String[this->type] + "/" +
		                 RtpCodecMimeType::subtype2String[this->subtype];
	}
} // namespace RTC
