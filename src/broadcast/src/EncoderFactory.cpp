/*
 * https://www.beikejiedeliulangmao.top/webrtc/use-in-java/port-limit-and-encode/
 * Restrict connection ports
Looking back at the previous completion process of port restriction, when creating PeerConnectionFactory , we
instantiated a SocketFactory and a default NetworkManager, and then when creating PeerConnection , we created a
PortAllocator through these two instances, and set the PortAllocator Inject into PeerConnection. In the whole
process, the code that actually performs port restriction is in SocketFactory, of course, the API of
PortAllocator is also used. Here you may have doubts, isn't there an interface in PortAllocator that can limit
the port range, why do you need SocketFactory?

std::unique_ptr<cricket::PortAllocator> port_allocator(
new cricket::BasicPortAllocator(network_manager.get(), socket_factory.get()));
port_allocator->SetPortRange(this->min_port, this->max_port); // Port allocator
I only set the port through this API at the time, but I found that it would still apply for ports beyond the
limit to do other things, so in the end I directly rewritten SocketFactory and banned all illegal port
applications. In addition, because there are still some subnet IPs that cannot be used on our server, I also
processed them in SocketFactory. My implementation is as follows:

rtc::AsyncPacketSocket *
rtc::SocketFactoryWrapper::CreateUdpSocket(const rtc::SocketAddress &local_address, uint16_t min_port,
                                            uint16_t max_port) {

    if (min_port < this->min_port || max_port > this->max_port) {
        WEBRTC_LOG("Create udp socket cancelled, port out of range, expect port range is:" +
                    std::to_string(this->min_port) + "->" + std::to_string(this->max_port)
                    + "parameter port range is: " + std::to_string(min_port) + "->" + std::to_string(max_port),
                    LogLevel::INFO);
        return nullptr;
    }
    //
    if (!local_address.IsPrivateIP() || local_address.HostAsURIString().find(this->white_private_ip_prefix) ==
0) { rtc::AsyncPacketSocket *result = BasicPacketSocketFactory::CreateUdpSocket(local_address, min_port,
max_port); const auto *address = static_cast<const void *>(result); std::stringstream ss; ss << address;
        WEBRTC_LOG("Create udp socket, min port is:" + std::to_string(min_port) + ", max port is: " +
                    std::to_string(max_port) + ", result is: " + result->GetLocalAddress().ToString() + "->" +
                    result->GetRemoteAddress().ToString() + ", new socket address is: " + ss.str(),
LogLevel::INFO);

        return result;
    } else {
        WEBRTC_LOG("Create udp socket cancelled, this ip is not in while list:" +
local_address.HostAsURIString(), LogLevel::INFO); return nullptr;
    }
}
*/

#include "EncoderFactory.h"
#include "VAAPIEncoder.h"
#include "QSVEncoder.h"
#include "NULLEncoder.h"
#include "NVEncoder.h"
#include "X264Encoder.h"

#include "base/logger.h"
#include "absl/strings/match.h"
#include "Settings.h"
//#include "api/video_codecs/builtin_video_encoder_factory.h"


namespace base
{
namespace web_rtc
{


EncoderFactory::EncoderFactory() : supported_codecs(webrtc::SupportedH264Codecs()) {}


std::vector<webrtc::SdpVideoFormat> EncoderFactory::GetSupportedFormats() const
{
    std::vector<webrtc::SdpVideoFormat> supported_codecs;


    if (NVEncoder::nvhwinstance < Settings::encSetting.nvidiaEnc)
    {
        const absl::optional<std::string> profile_string = webrtc::H264::ProfileLevelIdToString(
            webrtc::H264::ProfileLevelId(webrtc::H264::kProfileMain, webrtc::H264::kLevel3_1));

        supported_codecs.push_back(webrtc::SdpVideoFormat(

            cricket::kH264CodecName,
            {{cricket::kH264FmtpProfileLevelId, *profile_string},
             {cricket::kH264FmtpLevelAsymmetryAllowed, "1"},
             {cricket::kH264FmtpPacketizationMode, "1"},
             {"Enc", "NVIDIA"},
             {"MaxEnc", std::to_string(Settings::configuration.nvidiaEnc)},
             {"PresentEncIns", std::to_string(NVEncoder::nvhwinstance)}

            }

            ));
    }


    if (!Settings::configuration.haswell && QSVEncoder::hwinstance < Settings::encSetting.quicksyncEnc)
    {
        const absl::optional<std::string> profile_string = webrtc::H264::ProfileLevelIdToString(
            webrtc::H264::ProfileLevelId(webrtc::H264::kProfileMain, webrtc::H264::kLevel3_1));

        supported_codecs.push_back(webrtc::SdpVideoFormat(

            cricket::kH264CodecName,
            {{cricket::kH264FmtpProfileLevelId, *profile_string},
             {cricket::kH264FmtpLevelAsymmetryAllowed, "1"},
             {cricket::kH264FmtpPacketizationMode, "1"},
             {"Enc", "QUICKSYNC"},
             {"MaxEnc", std::to_string(Settings::configuration.quicksyncEnc)},
             {"PresentEncIns", std::to_string(QSVEncoder::hwinstance)}

            }

            ));
    }


    if (Settings::configuration.haswell && VAAPIEncoder::hwinstance < Settings::encSetting.VAAPIEnc)
    {
        const absl::optional<std::string> profile_string = webrtc::H264::ProfileLevelIdToString(
            webrtc::H264::ProfileLevelId(webrtc::H264::kProfileMain, webrtc::H264::kLevel3_1));

        supported_codecs.push_back(webrtc::SdpVideoFormat(

            cricket::kH264CodecName,
            {{cricket::kH264FmtpProfileLevelId, *profile_string},
             {cricket::kH264FmtpLevelAsymmetryAllowed, "1"},
             {cricket::kH264FmtpPacketizationMode, "1"},
             {"Enc", "QUICKSYNC"},
             {"MaxEnc", std::to_string(Settings::configuration.VAAPIEnc)},
             {"PresentEncIns", std::to_string(VAAPIEncoder::hwinstance)}

            }

            ));
    }

    if (Vp9Encoder::vp9instance < Settings::encSetting.vp9Enc)
    {
        SInfo << Vp9Encoder::vp9instance;

        supported_codecs.push_back(webrtc::SdpVideoFormat(
            cricket::kVp9CodecName,
            {{"MaxEnc", std::to_string(Settings::configuration.vp9Enc)},
             {"PresentEncIns", std::to_string(Vp9Encoder::vp9instance)}}));
    }


    if (X264Encoder::x264hwinstance < Settings::encSetting.x264Enc)
    {
        const absl::optional<std::string> profile_string = webrtc::H264::ProfileLevelIdToString(
            webrtc::H264::ProfileLevelId(webrtc::H264::kProfileMain, webrtc::H264::kLevel3_1));

        supported_codecs.push_back(webrtc::SdpVideoFormat(

            cricket::kH264CodecName,
            {{cricket::kH264FmtpProfileLevelId, *profile_string},
             {cricket::kH264FmtpLevelAsymmetryAllowed, "1"},
             {cricket::kH264FmtpPacketizationMode, "1"},
             {"Enc", "X264"},
             {"MaxEnc", std::to_string(Settings::configuration.x264Enc)},
             {"PresentEncIns", std::to_string(X264Encoder::x264hwinstance)}

            }

            ));
    }


    if (NULLEncoder::nativeInstance < Settings::encSetting.native)
    {
        SInfo << NULLEncoder::nativeInstance;

        const absl::optional<std::string> profile_string = webrtc::H264::ProfileLevelIdToString(
            webrtc::H264::ProfileLevelId(webrtc::H264::kProfileMain, webrtc::H264::kLevel3_1));

        supported_codecs.push_back(webrtc::SdpVideoFormat(

            cricket::kH264CodecName,
            {{cricket::kH264FmtpProfileLevelId, *profile_string},
             {cricket::kH264FmtpLevelAsymmetryAllowed, "1"},
             {cricket::kH264FmtpPacketizationMode, "1"},
             {"Enc", "NATIVE"},
             {"MaxEnc", std::to_string(Settings::configuration.native)},
             {"PresentEncIns", std::to_string(NULLEncoder::nativeInstance)}

            }

            ));
    }

    return supported_codecs;
}

webrtc::VideoEncoderFactory::CodecInfo
    EncoderFactory::QueryVideoEncoder(const webrtc::SdpVideoFormat &format) const
{
    CodecInfo Info;

    if (absl::EqualsIgnoreCase(format.name, cricket::kVp9CodecName))
    {
        Info.is_hardware_accelerated = true;
        Info.has_internal_source = false;
    }
    else
    {
        Info.is_hardware_accelerated = true;
        Info.has_internal_source = false;
    }
    return Info;
}


std::unique_ptr<webrtc::VideoEncoder> EncoderFactory::CreateVideoEncoder(const webrtc::SdpVideoFormat &format)
{
    if (absl::EqualsIgnoreCase(format.name, cricket::kVp9CodecName))
    {
        auto VideoEncoder = std::make_unique<Vp9Encoder>();  // webrtc::VP8Encoder::Create();
        return VideoEncoder;
    }
    else
    {
        std::map<std::string, std::string> parameters = format.parameters;
        if (parameters["Enc"] == "NATIVE")
        {
            auto VideoEncoder = std::make_unique<NULLEncoder>();  // for cam encoders
            return VideoEncoder;
        }
        else if (parameters["Enc"] == "NVIDIA")
        {
            auto VideoEncoder = std::make_unique<NVEncoder>(EN_NVIDIA);
            return VideoEncoder;
        }
        else if (parameters["Enc"] == "QUICKSYNC")
        {
            if (Settings::configuration.haswell)
            {
                auto VideoEncoder = std::make_unique<VAAPIEncoder>(EN_QUICKSYNC);
                return VideoEncoder;
            }
            else
            {
                auto VideoEncoder = std::make_unique<QSVEncoder>(EN_QUICKSYNC);
                return VideoEncoder;
            }
        }
        else if (parameters["Enc"] == "X264")
        {
            auto VideoEncoder = std::make_unique<X264Encoder>(EN_X264);
            return VideoEncoder;
        }
    }

    return nullptr;
}


}  // namespace web_rtc
}  // namespace base
