
#ifndef WebRTC_Peer_H
#define WebRTC_Peer_H


#include "webrtc/peerfactorycontext.h"

#include "api/jsep.h"
#include <json/json.hpp>
using json = nlohmann::json;

namespace base {
namespace wrtc {


class PeerManager;


class Peer : public webrtc::PeerConnectionObserver,
             public webrtc::CreateSessionDescriptionObserver
{
public:
    enum Mode
    {
        Offer, ///< Operating as offerer
        Answer ///< Operating as answerer
    };

    Peer(PeerManager* manager,
         PeerFactoryContext* context,
         const std::string& peerid,
         const std::string& token,
         Mode mode);
    virtual ~Peer();

    /// Create the local media stream.
    /// Only necessary when we are creating the offer.
  //  virtual rtc::scoped_refptr<webrtc::MediaStreamInterface> createMediaStream();

    /// Create the peer connection once configuration, constraints and
    /// streams have been created.
    virtual void createConnection();

    /// Close the peer connection.
    virtual void closeConnection();

    /// Create the offer SDP tos end to the peer.
    /// No offer should be received after creating the offer.
    /// A call to `recvSDP` with answer is expected in order to initiate the session.
    virtual void createOffer();

    /// Receive a remote offer or answer.
    virtual void recvSDP(const std::string& type, const std::string& sdp);

    /// Receive a remote candidate.
    virtual void recvCandidate(const std::string& mid, int mlineindex, const std::string& sdp);

    /// Set a custom PeerFactory object.
    /// Must be set before any streams are initiated.
    void setPeerFactory(rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory);

    /// Set the port range for WebRTC connections.
    /// Must be done before the connection is initiated.
    void setPortRange(int minPort, int maxPort);

    std::string peerid() const;
    std::string token() const;
    
    void mute( const json& message);
     
    //webrtc::FakeConstraints& constraints();
    webrtc::PeerConnectionFactoryInterface* factory() const;
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> peerConnection() const;
    rtc::scoped_refptr<webrtc::MediaStreamInterface> stream() const;

protected:
    /// inherited from PeerConnectionObserver
    virtual void OnAddStream(webrtc::MediaStreamInterface* stream); ///< @deprecated
    virtual void OnRemoveStream(webrtc::MediaStreamInterface* stream); ///< @deprecated
    virtual void OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override; ///< since 7f0676
    virtual void OnTrack( rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) override;
    virtual void OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver)  override;
    virtual void OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream) override; ///< since 7f0676
    virtual void OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> stream) override; ///< since 7f0676
    virtual void OnIceCandidate(const webrtc::IceCandidateInterface* candidate) override;
    virtual void OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state) override;
    virtual void OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state) override;
    virtual void OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state) override;
    virtual void OnRenegotiationNeeded();

    /// inherited from CreateSessionDescriptionObserver
    virtual void OnSuccess(webrtc::SessionDescriptionInterface* desc) override;
    virtual void OnFailure(const std::string& error) override;

    virtual void AddRef() const override { return; }
    virtual rtc::RefCountReleaseStatus Release() const override { return rtc::RefCountReleaseStatus::kDroppedLastRef; }

protected:
    PeerManager* _manager;
    PeerFactoryContext* _context;
    std::string _peerid;
    std::string _token;
    Mode _mode;
    webrtc::PeerConnectionInterface::RTCConfiguration _config;
   // webrtc::FakeConstraints _constraints;
public:
    rtc::scoped_refptr<webrtc::PeerConnectionInterface> _peerConnection;

    bool hasIceLiteOffer{false};
    //rtc::scoped_refptr<webrtc::MediaStreamInterface> _stream;
    //std::unique_ptr<cricket::BasicPortAllocator> _portAllocator;
};


class DummySetSessionDescriptionObserver
    : public webrtc::SetSessionDescriptionObserver
{
public:
    static DummySetSessionDescriptionObserver* Create()
    {
        return new rtc::RefCountedObject<DummySetSessionDescriptionObserver>();
    }

    virtual void OnSuccess() override;
    virtual void OnFailure(const std::string& error) override;

protected:
    DummySetSessionDescriptionObserver() = default;
    ~DummySetSessionDescriptionObserver() = default;
};


} } // namespace wrtc


#endif

