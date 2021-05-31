#define MS_CLASS "DepUsrSCTP"
// #define MS_LOG_DEV_LEVEL 3

#include "DepUsrSCTP.h"
#include "base/application.h"
#include "LoggerTag.h"
#include "RTC/SctpAssociation.h"

//#define SCTP_DEBUG 1
#include <usrsctp.h>



/* Static. */

static constexpr size_t CheckerInterval{ 10u }; // In ms.

/* Static methods for usrsctp global callbacks. */

inline static int onSendSctpData(void* addr, void* data, size_t len, uint8_t /*tos*/, uint8_t /*setDf*/)
{
	auto* sctpAssociation = static_cast<RTC::SctpAssociation*>(addr);

	if (sctpAssociation == nullptr)
		return -1;

	sctpAssociation->OnUsrSctpSendSctpData(data, len);

	// NOTE: Must not free data, usrsctp lib does it.

	return 0;
}

// Static method for printing usrsctp debug.
inline static void sctpDebug(const char* format, ...)
{
	char buffer[10000];
	va_list ap;

	va_start(ap, format);
	vsprintf(buffer, format, ap);

	// Remove the artificial carriage return set by usrsctp.
	buffer[std::strlen(buffer) - 1] = '\0';

	MS_DEBUG_TAG(sctp, buffer);

	va_end(ap);
}

/* Static variables. */

DepUsrSCTP::Checker* DepUsrSCTP::checker{ nullptr };
uint64_t DepUsrSCTP::numSctpAssociations{ 0u };

/* Static methods. */

void DepUsrSCTP::ClassInit()
{
	
	LTrace("ClassInit()");

	usrsctp_init_nothreads(0, onSendSctpData, sctpDebug);

	// Disable explicit congestion notifications (ecn).
	usrsctp_sysctl_set_sctp_ecn_enable(0);

#ifdef SCTP_DEBUG
	usrsctp_sysctl_set_sctp_debug_on(SCTP_DEBUG_ALL);
#endif

	DepUsrSCTP::checker = new DepUsrSCTP::Checker();
}

void DepUsrSCTP::ClassDestroy()
{
	

	usrsctp_finish();

	delete DepUsrSCTP::checker;
}

void DepUsrSCTP::IncreaseSctpAssociations()
{
	

	if (++DepUsrSCTP::numSctpAssociations == 1u)
		DepUsrSCTP::checker->Start();
}

void DepUsrSCTP::DecreaseSctpAssociations()
{
	

	assertm(DepUsrSCTP::numSctpAssociations > 0u, "numSctpAssociations was not higher than 0");

	if (--DepUsrSCTP::numSctpAssociations == 0u)
		DepUsrSCTP::checker->Stop();
}

/* DepUsrSCTP::Checker instance methods. */

DepUsrSCTP::Checker::Checker()
{
	

	this->timer = new Timer(this);
}

DepUsrSCTP::Checker::~Checker()
{
	

	delete this->timer;
}

void DepUsrSCTP::Checker::Start()
{
	

	MS_DEBUG_TAG(sctp, "usrsctp periodic check started");

	this->lastCalledAtMs = 0u;

	this->timer->Start(CheckerInterval, CheckerInterval);
}

void DepUsrSCTP::Checker::Stop()
{
	

	MS_DEBUG_TAG(sctp, "usrsctp periodic check stopped");

	this->lastCalledAtMs = 0u;

	this->timer->Stop();
}

void DepUsrSCTP::Checker::OnTimer(Timer* /*timer*/)
{
	

	auto nowMs = base::Application::GetTimeMs();
	int delta  = this->lastCalledAtMs ? static_cast<int>(nowMs - this->lastCalledAtMs) : 0;

	usrsctp_handle_timers(delta);

	this->lastCalledAtMs = nowMs;
}
