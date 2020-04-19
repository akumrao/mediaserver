

#include "RTC/AudioLevelObserver.h"
#include "LoggerTag.h"
#include "base/error.h"
#include "Utils.h"
#include "Channel/Notifier.h"
#include "RTC/RtpDictionaries.h"
#include <cmath> // std::lround()
#include <map>

namespace RTC
{
	/* Instance methods. */

	AudioLevelObserver::AudioLevelObserver(const std::string& id, json& data) : RTC::RtpObserver(id)
	{
		MS_TRACE();

		auto jsonMaxEntriesIt = data.find("maxEntries");

		
		if (
			jsonMaxEntriesIt == data.end() ||
			!Utils::Json::IsPositiveInteger(*jsonMaxEntriesIt)
		)
		
		{
			base::uv::throwError("missing maxEntries");
		}

		this->maxEntries = jsonMaxEntriesIt->get<uint16_t>();

		if (this->maxEntries < 1)
			base::uv::throwError("invalid maxEntries value %" PRIu16, this->maxEntries);

		auto jsonThresholdIt = data.find("threshold");

		if (jsonThresholdIt == data.end() || !jsonThresholdIt->is_number())
			base::uv::throwError("missing threshold");

		this->threshold = jsonThresholdIt->get<int8_t>();

		if (this->threshold < -127 || this->threshold > 0)
			base::uv::throwError("invalid threshold value %" PRIi8, this->threshold);

		auto jsonIntervalIt = data.find("interval");

		if (jsonIntervalIt == data.end() || !jsonIntervalIt->is_number())
			base::uv::throwError("missing interval");

		this->interval = jsonIntervalIt->get<uint16_t>();

		if (this->interval < 250)
			this->interval = 250;
		else if (this->interval > 5000)
			this->interval = 5000;

		this->periodicTimer = new Timer(this);

		this->periodicTimer->Start(this->interval, this->interval);
	}

	AudioLevelObserver::~AudioLevelObserver()
	{
		MS_TRACE();

		delete this->periodicTimer;
	}

	void AudioLevelObserver::AddProducer(RTC::Producer* producer)
	{
		MS_TRACE();

		if (producer->GetKind() != RTC::Media::Kind::AUDIO)
			base::uv::throwError("not an audio Producer");

		// Insert into the map.
		this->mapProducerDBovs[producer];
	}

	void AudioLevelObserver::RemoveProducer(RTC::Producer* producer)
	{
		MS_TRACE();

		// Remove from the map.
		this->mapProducerDBovs.erase(producer);
	}

	void AudioLevelObserver::ReceiveRtpPacket(RTC::Producer* producer, RTC::RtpPacket* packet)
	{
		MS_TRACE();

		if (IsPaused())
			return;

		uint8_t volume;
		bool voice;

		if (!packet->ReadSsrcAudioLevel(volume, voice))
			return;

		auto& dBovs = this->mapProducerDBovs.at(producer);

		dBovs.totalSum += volume;
		dBovs.count++;
	}

	void AudioLevelObserver::ProducerPaused(RTC::Producer* producer)
	{
		// Remove from the map.
		this->mapProducerDBovs.erase(producer);
	}

	void AudioLevelObserver::ProducerResumed(RTC::Producer* producer)
	{
		// Insert into the map.
		this->mapProducerDBovs[producer];
	}

	void AudioLevelObserver::Paused()
	{
		MS_TRACE();

		this->periodicTimer->Stop();

		ResetMapProducerDBovs();

		if (!this->silence)
		{
			this->silence = true;

			Channel::Notifier::Emit(this->id, "silence");
		}
	}

	void AudioLevelObserver::Resumed()
	{
		MS_TRACE();

		this->periodicTimer->Restart();
	}

	void AudioLevelObserver::Update()
	{
		MS_TRACE();

		std::map<int8_t, RTC::Producer*> mapDBovsProducer;

		for (auto& kv : this->mapProducerDBovs)
		{
			auto* producer = kv.first;
			auto& dBovs    = kv.second;

			if (dBovs.count < 10)
				continue;

			auto avgDBov = -1 * static_cast<int8_t>(std::lround(dBovs.totalSum / dBovs.count));

			if (avgDBov >= this->threshold)
				mapDBovsProducer[avgDBov] = producer;
		}

		// Clear the map.
		ResetMapProducerDBovs();

		if (!mapDBovsProducer.empty())
		{
			this->silence = false;

			uint16_t idx{ 0 };
			auto rit  = mapDBovsProducer.crbegin();
			json data = json::array();

			for (; idx < this->maxEntries && rit != mapDBovsProducer.crend(); ++idx, ++rit)
			{
				data.emplace_back(json::value_t::object);

				auto& jsonEntry = data[idx];

				jsonEntry["producerId"] = rit->second->id;
				jsonEntry["volume"]     = rit->first;
			}

			Channel::Notifier::Emit(this->id, "volumes", data);
		}
		else if (!this->silence)
		{
			this->silence = true;

			Channel::Notifier::Emit(this->id, "silence");
		}
	}

	void AudioLevelObserver::ResetMapProducerDBovs()
	{
		MS_TRACE();

		for (auto& kv : this->mapProducerDBovs)
		{
			auto& dBovs = kv.second;

			dBovs.totalSum = 0;
			dBovs.count    = 0;
		}
	}

	inline void AudioLevelObserver::OnTimer(Timer* /*timer*/)
	{
		MS_TRACE();

		Update();
	}
} // namespace RTC
