

#ifndef WebRTC_PeerFactoryContext_H
#define WebRTC_PeerFactoryContext_H


#include "webrtc/webrtc.h"

#include "pc/peerconnectionfactory.h"


namespace base {
namespace wrtc {


class PeerFactoryContext
{
public:
    PeerFactoryContext(
        webrtc::AudioDeviceModule* default_adm = nullptr,
        cricket::WebRtcVideoEncoderFactory* video_encoder_factory = nullptr,
        cricket::WebRtcVideoDecoderFactory* video_decoder_factory = nullptr,
        rtc::scoped_refptr<webrtc::AudioEncoderFactory> audio_encoder_factory = nullptr,
        rtc::scoped_refptr<webrtc::AudioDecoderFactory> audio_decoder_factory = nullptr);

    void initCustomNetworkManager();

    std::unique_ptr<rtc::Thread> networkThread;
    std::unique_ptr<rtc::Thread> workerThread;
    std::unique_ptr<rtc::NetworkManager> networkManager;
    std::unique_ptr<rtc::PacketSocketFactory> socketFactory;
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory;
    // rtc::scoped_refptr<webrtc::AudioDeviceModule> audioDeviceManager;
};


} } // namespace wrtc


#endif

