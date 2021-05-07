
#include "webrtc/peerfactorycontext.h"
#include "webrtc/fakeaudiodevicemodule.h"
//#include "pc/test/fakeaudiocapturemodule.h"
#include "base/logger.h"

//#include "api/peerconnectionfactoryproxy.h"
//#include "api/peerconnectionproxy.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/audio_codecs/builtin_audio_encoder_factory.h"
//#include "p2p/base/basicpacketsocketfactory.h"
//#include "p2p/client/basicportallocator.h"
//#include "pc/peerconnection.h"
#include "api/audio_codecs/audio_format.h"
#include "api/audio_codecs/audio_decoder_factory_template.h"
#include "api/audio_codecs/audio_encoder_factory_template.h"
#include "api/audio_codecs/opus/audio_decoder_opus.h"
#include "api/audio_codecs/opus/audio_encoder_opus.h"


using std::endl;


namespace base {
namespace wrtc {


PeerFactoryContext::PeerFactoryContext(
    webrtc::AudioDeviceModule* default_adm)
{
    LInfo("PeerFactoryContext")
            
    // Setup threads
    networkThread = rtc::Thread::CreateWithSocketServer();
    workerThread = rtc::Thread::Create();
    if (!networkThread->Start() || !workerThread->Start())
        throw std::runtime_error("Failed to start WebRTC threads");


      g_signaling_thread = rtc::Thread::Create();
      g_signaling_thread->Start();


    // Init required builtin factories if not provided
    // if (!audio_encoder_factory)
    //     audio_encoder_factory = webrtc::CreateBuiltinAudioEncoderFactory();
    // if (!audio_decoder_factory)
    //     audio_decoder_factory = webrtc::CreateBuiltinAudioDecoderFactory();

   // Create the factory
    // factory = webrtc::CreatePeerConnectionFactory(
    //     networkThread.get(), workerThread.get(), rtc::Thread::Current(),
    //     default_adm, audio_encoder_factory, audio_decoder_factory,
    //     video_encoder_factory, video_decoder_factory,nullptr,nullptr);

    // if (audio_encoder_factory || audio_decoder_factory) {
    //     factory = webrtc::CreatePeerConnectionFactory(
    //         networkThread.get(), workerThread.get(), rtc::Thread::Current(),
    //         default_adm, audio_encoder_factory, audio_decoder_factory,
    //         video_encoder_factory, video_decoder_factory);
    // }
    // else {
    //     factory = webrtc::CreatePeerConnectionFactory(
    //         networkThread.get(), workerThread.get(), rtc::Thread::Current(),
    //         default_adm, video_encoder_factory, video_decoder_factory);
    // }



    factory = webrtc::CreatePeerConnectionFactory(
              networkThread.get(), workerThread.get(), g_signaling_thread.get(),
              default_adm, 

              webrtc::CreateAudioEncoderFactory<webrtc::AudioEncoderOpus>(),
              webrtc::CreateAudioDecoderFactory<webrtc::AudioDecoderOpus>(),

              webrtc::CreateBuiltinVideoEncoderFactory(),

              webrtc::CreateBuiltinVideoDecoderFactory(),

              nullptr, nullptr);

    if (!factory)
        throw std::runtime_error("Failed to create WebRTC factory");
}


void PeerFactoryContext::initCustomNetworkManager()
{
   // networkManager.reset(new rtc::BasicNetworkManager());
    //socketFactory.reset(new rtc::BasicPacketSocketFactory(networkThread.get()));
}


} } // namespace wrtc


