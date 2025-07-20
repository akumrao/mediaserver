
#pragma once

#include "media/engine/internal_encoder_factory.h"
#include "api/video_codecs/builtin_video_encoder_factory.h"
#include "api/video_codecs/video_encoder_factory.h"
#include "api/video_codecs/video_encoder.h"

namespace base {
namespace web_rtc {


class EncoderFactory : public webrtc::VideoEncoderFactory
{
public:
	//explicit EncoderFactory(FHWEncoderDetails& HWEncoderDetails);
        
       EncoderFactory();
	/**
	* This is used from the FPlayerSession::OnSucess to let the factory know
	* what session the next created encoder should belong to.
	* It allows us to get the right FPlayerSession <-> NV_X264Encoder relationship
	*/
	//void AddSession(FPlayerSession& PlayerSession);

	//
	// webrtc::VideoEncoderFactory implementation
	//
	std::vector<webrtc::SdpVideoFormat> GetSupportedFormats() const override;
	CodecInfo QueryVideoEncoder(const webrtc::SdpVideoFormat& Format) const override;
	std::unique_ptr<webrtc::VideoEncoder> CreateVideoEncoder(const webrtc::SdpVideoFormat& Format) override;

private:
	//FHWEncoderDetails& HWEncoderDetails;
	//TQueue<FPlayerSession*> PendingPlayerSessions;
    
    
    	std::vector<webrtc::SdpVideoFormat> supported_codecs;
};


}//ns webrtc
}//base
