

#include "RTC/RtpObserver.h"
#include "LoggerTag.h"

namespace RTC
{
	/* Instance methods. */

	RtpObserver::RtpObserver(const std::string& id) : id(id)
	{
		MS_TRACE();
	}

	RtpObserver::~RtpObserver()
	{
		MS_TRACE();
	}

	void RtpObserver::Pause()
	{
		MS_TRACE();

		if (this->paused)
			return;

		this->paused = true;

		Paused();
	}

	void RtpObserver::Resume()
	{
		MS_TRACE();

		if (!this->paused)
			return;

		this->paused = false;

		Resumed();
	}
} // namespace RTC
