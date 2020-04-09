#define MS_CLASS "RTC::Media"
// #define MS_LOG_DEV_LEVEL 3

#include "LoggerTag.h"
#include "base/error.h"
#include "Utils.h"
#include "RTC/RtpDictionaries.h"

namespace RTC
{
	/* Class variables. */

	
	std::unordered_map<std::string, Media::Kind> Media::string2Kind =
	{
		{ "",      Media::Kind::ALL   },
		{ "audio", Media::Kind::AUDIO },
		{ "video", Media::Kind::VIDEO }
	};
	std::map<Media::Kind, std::string> Media::kind2String =
	{
		{ Media::Kind::ALL,   ""      },
		{ Media::Kind::AUDIO, "audio" },
		{ Media::Kind::VIDEO, "video" }
	};
	

	/* Class methods. */

	Media::Kind Media::GetKind(std::string& str)
	{
		MS_TRACE();

		// Force lowcase kind.
		Utils::String::ToLowerCase(str);

		auto it = Media::string2Kind.find(str);

		if (it == Media::string2Kind.end())
			base::uv::throwError("invalid media kind [kind:] " +  str);

		return it->second;
	}

	Media::Kind Media::GetKind(std::string&& str)
	{
		MS_TRACE();

		// Force lowcase kind.
		Utils::String::ToLowerCase(str);

		auto it = Media::string2Kind.find(str);

		if (it == Media::string2Kind.end())
			base::uv::throwError("invalid media kind [kind:%s] " +  str );

		return it->second;
	}

	const std::string& Media::GetString(Media::Kind kind)
	{
		MS_TRACE();

		return Media::kind2String.at(kind);
	}
} // namespace RTC
