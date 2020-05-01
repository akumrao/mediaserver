#ifndef SDP_HANDLER_H
#define SDP_HANDLER_H


#include "signaler.h"
#include "sdp/RemoteSdp.h"
#include <json.hpp>
#include <map>
#include <string>

namespace SdpParse {
    
    
    class Handler
    {
    
    public:
                
        Handler(Device * device, std::string &peerID): device(device), peerID(peerID) 
        {
            
        }
        ~Handler()
        {
            if (remoteSdp) {
                delete remoteSdp;
                remoteSdp = nullptr;
            }
        }
        
        void transportCreate(base::wrtc::Signaler *signal);
        void transportConnect(base::wrtc::Signaler *signal);
       

        Sdp::RemoteSdp *remoteSdp{nullptr};
        void createSdp(const nlohmann::json& iceParameters, const nlohmann::json& iceCandidates, const nlohmann::json& dtlsParameters);
        void _setupTransport(const nlohmann::json & sdpObject, const std::string localDtlsRole);
        
        
    protected:
        Device * device;
        std::string peerID;
        std::string transportId;
        nlohmann::json dtlsParameters;
        bool forceTcp{false};
        static int ids;
        
    };

    class Producer:public Handler
    {
       
        public:
        
        Producer(Device * device, std::string &peerID);

        void runit(base::wrtc::Signaler *signal);
        std::string answer;
        
        nlohmann::json producer;
        
    private:
        std::string GetAnswer();
        nlohmann::json sendingRtpParameters;
    
        
    };

    class Consumer : public Handler
    {
        
    public:
        Consumer(Device * device, std::string &peerID);
      
        void runit(base::wrtc::Signaler *signal,  nlohmann::json & producer);

        std::string GetOffer(const std::string& id, const std::string& kind, const nlohmann::json & rtpParameters);

        void loadAnswer(base::wrtc::Signaler *signal, std::string sdp);
        void resume(base::wrtc::Signaler *signal , json & producer);
        

        std::string offer;
    private:
        int mid{0};
      
        json consumer;

    };
    
    
} // namespace SdpParse

#endif
