

#ifndef WebRTC_PeerFactoryContext_H
#define WebRTC_PeerFactoryContext_H


#include "webrtc/webrtc.h"
#include "rtc_base/checks.h"
#include "api/media_stream_interface.h"
#include "api/peer_connection_interface.h"
#include "absl/memory/memory.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
#include "api/create_peerconnection_factory.h"
#include "media/engine/internal_decoder_factory.h"
#include "media/engine/internal_encoder_factory.h"

#include <api/video_codecs/builtin_video_encoder_factory.h>
#include <api/video_codecs/builtin_video_decoder_factory.h>


#include "modules/audio_device/include/audio_device.h"
#include "modules/audio_processing/include/audio_processing.h"


namespace base {
namespace wrtc {


class PeerFactoryContext
{
public:
    PeerFactoryContext(
        webrtc::AudioDeviceModule* default_adm = nullptr
     );

private:
    void initCustomNetworkManager();

    std::unique_ptr<rtc::Thread> networkThread;
    std::unique_ptr<rtc::Thread> workerThread;
    std::unique_ptr<rtc::Thread> g_signaling_thread;

    
 public:
    //std::unique_ptr<rtc::NetworkManager> networkManager;
    //std::unique_ptr<rtc::PacketSocketFactory> socketFactory;
    rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory;
    // rtc::scoped_refptr<webrtc::AudioDeviceModule> audioDeviceManager;
};


} } // namespace wrtc


#endif

