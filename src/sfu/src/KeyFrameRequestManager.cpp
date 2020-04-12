#define MS_CLASS "KeyFrameRequestManager"
// #define MS_LOG_DEV_LEVEL 3

#include "RTC/KeyFrameRequestManager.h"
#include "LoggerTag.h"

static uint16_t KeyFrameWaitTime{ 1000 };

/* PendingKeyFrameInfo methods */

RTC::PendingKeyFrameInfo::PendingKeyFrameInfo(PendingKeyFrameInfo::Listener* listener, uint32_t ssrc)
  : listener(listener), ssrc(ssrc)
{
	MS_TRACE();

	this->timer = new Timer(this);
	this->timer->Start(KeyFrameWaitTime);
}

RTC::PendingKeyFrameInfo::~PendingKeyFrameInfo()
{
	MS_TRACE();

	this->timer->Stop();
	delete this->timer;
}

inline void RTC::PendingKeyFrameInfo::OnTimer(Timer* timer)
{
	MS_TRACE();

	if (timer == this->timer)
		this->listener->OnKeyFrameRequestTimeout(this);
}

/* KeyFrameRequestManager methods */

RTC::KeyFrameRequestManager::KeyFrameRequestManager(KeyFrameRequestManager::Listener* listener)
  : listener(listener)
{
	MS_TRACE();
}

RTC::KeyFrameRequestManager::~KeyFrameRequestManager()
{
	MS_TRACE();

	for (auto& kv : this->mapSsrcPendingKeyFrameInfo)
	{
		auto* pendingKeyFrameInfo = kv.second;

		delete pendingKeyFrameInfo;
	}

	this->mapSsrcPendingKeyFrameInfo.clear();
}

void RTC::KeyFrameRequestManager::KeyFrameNeeded(uint32_t ssrc)
{
	MS_TRACE();

	auto it = this->mapSsrcPendingKeyFrameInfo.find(ssrc);

	// There is a pending key frame for the given ssrc.
	if (it != this->mapSsrcPendingKeyFrameInfo.end())
	{
		auto* pendingKeyFrameInfo = it->second;

		// Re-request the key frame if not received on time.
		pendingKeyFrameInfo->SetRetryOnTimeout(true);

		return;
	}

	this->mapSsrcPendingKeyFrameInfo[ssrc] = new PendingKeyFrameInfo(this, ssrc);

	this->listener->OnKeyFrameNeeded(this, ssrc);
}

void RTC::KeyFrameRequestManager::ForceKeyFrameNeeded(uint32_t ssrc)
{
	MS_TRACE();

	auto it = this->mapSsrcPendingKeyFrameInfo.find(ssrc);

	// There is a pending key frame for the given ssrc.
	if (it != this->mapSsrcPendingKeyFrameInfo.end())
	{
		auto* pendingKeyFrameInfo = it->second;

		pendingKeyFrameInfo->SetRetryOnTimeout(true);
		pendingKeyFrameInfo->Restart();
	}
	else
	{
		this->mapSsrcPendingKeyFrameInfo[ssrc] = new PendingKeyFrameInfo(this, ssrc);
	}

	this->listener->OnKeyFrameNeeded(this, ssrc);
}

void RTC::KeyFrameRequestManager::KeyFrameReceived(uint32_t ssrc)
{
	MS_TRACE();

	auto it = this->mapSsrcPendingKeyFrameInfo.find(ssrc);

	// There is no pending key frame for the given ssrc.
	if (it == this->mapSsrcPendingKeyFrameInfo.end())
		return;

	auto* pendingKeyFrameInfo = it->second;

	delete pendingKeyFrameInfo;

	this->mapSsrcPendingKeyFrameInfo.erase(it);
}

inline void RTC::KeyFrameRequestManager::OnKeyFrameRequestTimeout(PendingKeyFrameInfo* pendingKeyFrameInfo)
{
	MS_TRACE();

	auto it = this->mapSsrcPendingKeyFrameInfo.find(pendingKeyFrameInfo->GetSsrc());

	assertm(
	  it != this->mapSsrcPendingKeyFrameInfo.end(), "PendingKeyFrameInfo not present in the map");

	if (!pendingKeyFrameInfo->GetRetryOnTimeout())
	{
		auto* pendingKeyFrameInfo = it->second;

		delete pendingKeyFrameInfo;

		this->mapSsrcPendingKeyFrameInfo.erase(it);

		return;
	}

	// Best effort in case the PLI/FIR was lost. Do not retry on timeout.
	pendingKeyFrameInfo->SetRetryOnTimeout(false);
	pendingKeyFrameInfo->Restart();

	MS_DEBUG_DEV("requesting key frame on timeout");

	this->listener->OnKeyFrameNeeded(this, pendingKeyFrameInfo->GetSsrc());
}
