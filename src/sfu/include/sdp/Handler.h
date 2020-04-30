#ifndef SDP_HANDLER_H
#define SDP_HANDLER_H


#include "signaler.h"
#include "sdp/RemoteSdp.h"
#include <json.hpp>
#include <map>
#include <string>

namespace SdpParse {

    class Producer
    {
       
        public:
        
        Producer(Device * device, std::string &peerID);

        Sdp::RemoteSdp *remoteSdp{nullptr};


        void createSdp(const nlohmann::json& iceParameters, const nlohmann::json& iceCandidates, const nlohmann::json& dtlsParameters);

        void _setupTransport(const nlohmann::json & sdpObject, const std::string localDtlsRole);

        void runit(base::wrtc::Signaler *signal);
        std::string answer;
        
        nlohmann::json producer;
        
    private:
        std::string GetAnswer();
        nlohmann::json sendingRtpParameters;
        Device * device;
        std::string peerID;
        std::string transportId;
        nlohmann::json dtlsParameters;
        
    };

    class Consumer {
        
    public:
        Consumer(Device * device, std::string &peerID);
        std::string transportId;
        Sdp::RemoteSdp *remoteSdp{nullptr};


        void createSdp(const nlohmann::json& iceParameters, const nlohmann::json& iceCandidates, const nlohmann::json& dtlsParameters);
        void _setupTransport(const nlohmann::json & sdpObject, const std::string localDtlsRole);
        void runit(base::wrtc::Signaler *signal,  nlohmann::json & producer);

        std::string GetOffer(const std::string& id, const std::string& kind, const nlohmann::json & rtpParameters);

        void loadAnswer(base::wrtc::Signaler *signal, std::string sdp, json & producer);
        void resume(base::wrtc::Signaler *signal , json & producer);
        

        std::string offer;
    private:
        int mid{0};
        Device * device;
        std::string peerID;
        nlohmann::json dtlsParameters;
        json consumer;
        
    };
    
    
} // namespace SdpParse

#endif
