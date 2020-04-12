#ifndef MS_RTC_SIMPLE_CONSUMER_HPP
#define MS_RTC_SIMPLE_CONSUMER_HPP

#include "RTC/Consumer.h"
#include "RTC/RtpStreamSend.h"
#include "RTC/SeqManager.h"

namespace RTC
{
	class SimpleConsumer : public RTC::Consumer, public RTC::RtpStreamSend::Listener
	{
	public:
		SimpleConsumer(const std::string& id, RTC::Consumer::Listener* listener, json& data);
		~SimpleConsumer() override;

	public:
		void FillJson(json& jsonObject) const override;
		void FillJsonStats(json& jsonArray) const override;
		void FillJsonScore(json& jsonObject) const override;
		void HandleRequest(Channel::Request* request) override;
		bool IsActive() const override;
		void ProducerRtpStream(RTC::RtpStream* rtpStream, uint32_t mappedSsrc) override;
		void ProducerNewRtpStream(RTC::RtpStream* rtpStream, uint32_t mappedSsrc) override;
		void ProducerRtpStreamScore(RTC::RtpStream* rtpStream, uint8_t score, uint8_t previousScore) override;
		void ProducerRtcpSenderReport(RTC::RtpStream* rtpStream, bool first) override;
		uint8_t GetBitratePriority() const override;
		uint32_t IncreaseLayer(uint32_t bitrate, bool considerLoss) override;
		void ApplyLayers() override;
		uint32_t GetDesiredBitrate() const override;
		void SendRtpPacket(RTC::RtpPacket* packet) override;
		std::vector<RTC::RtpStreamSend*> GetRtpStreams() override;
		void GetRtcp(RTC::RTCP::CompoundPacket* packet, RTC::RtpStreamSend* rtpStream, uint64_t nowMs) override;
		void NeedWorstRemoteFractionLost(uint32_t mappedSsrc, uint8_t& worstRemoteFractionLost) override;
		void ReceiveNack(RTC::RTCP::FeedbackRtpNackPacket* nackPacket) override;
		void ReceiveKeyFrameRequest(RTC::RTCP::FeedbackPs::MessageType messageType, uint32_t ssrc) override;
		void ReceiveRtcpReceiverReport(RTC::RTCP::ReceiverReport* report) override;
		uint32_t GetTransmissionRate(uint64_t nowMs) override;
		float GetRtt() const override;

	private:
		void UserOnTransportConnected() override;
		void UserOnTransportDisconnected() override;
		void UserOnPaused() override;
		void UserOnResumed() override;
		void CreateRtpStream();
		void RequestKeyFrame();
		void EmitScore() const;

		/* Pure virtual methods inherited from RtpStreamSend::Listener. */
	public:
		void OnRtpStreamScore(RTC::RtpStream* rtpStream, uint8_t score, uint8_t previousScore) override;
		void OnRtpStreamRetransmitRtpPacket(RTC::RtpStreamSend* rtpStream, RTC::RtpPacket* packet) override;

	private:
		// Allocated by this.
		RTC::RtpStreamSend* rtpStream{ nullptr };
		// Others.
		std::vector<RTC::RtpStreamSend*> rtpStreams;
		RTC::RtpStream* producerRtpStream{ nullptr };
		bool keyFrameSupported{ false };
		bool syncRequired{ false };
		RTC::SeqManager<uint16_t> rtpSeqManager;
		bool managingBitrate{ false };
	};

	/* Inline methods. */

	inline bool SimpleConsumer::IsActive() const
	{
		
		return (
			RTC::Consumer::IsActive() &&
			this->producerRtpStream &&
			this->producerRtpStream->GetScore() > 0u
		);
		
	}

	inline std::vector<RTC::RtpStreamSend*> SimpleConsumer::GetRtpStreams()
	{
		return this->rtpStreams;
	}
} // namespace RTC

#endif
