
#include "webrtc/peer.h"
#include "webrtc/peermanager.h"
#include "webrtc/peerfactorycontext.h"
#include "base/logger.h"
#include "p2p/base/transport_info.h"
#include "pc/media_session.h"
#include "pc/peer_connection_wrapper.h"
#include "pc/sdp_utils.h"


using std::endl;

namespace base {
namespace wrtc {

 

cricket::Candidate CreateLocalUdpCandidate(
	const rtc::SocketAddress& address) {
	cricket::Candidate candidate;
	candidate.set_component(cricket::ICE_CANDIDATE_COMPONENT_DEFAULT);
	candidate.set_protocol(cricket::UDP_PROTOCOL_NAME);
	candidate.set_address(address);
	candidate.set_type(cricket::LOCAL_PORT_TYPE);
	return candidate;
}

cricket::Candidate CreateLocalTcpCandidate(
	const rtc::SocketAddress& address) {
	cricket::Candidate candidate;
	candidate.set_component(cricket::ICE_CANDIDATE_COMPONENT_DEFAULT);
	candidate.set_protocol(cricket::TCP_PROTOCOL_NAME);
	candidate.set_address(address);
	candidate.set_type(cricket::LOCAL_PORT_TYPE);
	candidate.set_tcptype(cricket::TCPTYPE_PASSIVE_STR);
	return candidate;
}



bool AddCandidateToFirstTransport(cricket::Candidate* candidate,
	webrtc::SessionDescriptionInterface* sdesc) {
	auto* desc = sdesc->description();
	//RTC_DCHECK(desc->contents().size() > 0);
	const auto& first_content = desc->contents()[0];
	candidate->set_transport_name(first_content.name);
	std::unique_ptr<webrtc::IceCandidateInterface> jsep_candidate =
		webrtc::CreateIceCandidate(first_content.name, 0, *candidate);
	return sdesc->AddCandidate(jsep_candidate.get());
}

    

Peer::Peer(PeerManager* manager,
           PeerFactoryContext* context,
           std::string &cam, 
           const std::string& peerid,
           const std::string& token,
           Mode mode)
    : cam(cam),_manager(manager)
    , _context(context)
    , _peerid(peerid)
    , _token(token)
    , _mode(mode)
    //, _context->factory(manager->factory())
    , _peerConnection(nullptr)
{
      webrtc::PeerConnectionInterface::IceServer stun;
     // stun.uri = kGoogleStunServerUri;
     //_config.servers.push_back(stun);

    stun.uri = "stun:stun.l.google.com:19302";
    _config.servers.push_back(stun);
  //  config_.sdp_semantics = sdp_semantics;

    // _constraints.SetMandatoryReceiveAudio(true);
    // _constraints.SetMandatoryReceiveVideo(true);
    // _constraints.SetAllowDtlsSctpDataChannels();
    
    //_config.servers.clear();
    //_config.servers.empty();
    _config.enable_rtp_data_channel = false;
    _config.enable_dtls_srtp = true;
    _config.sdp_semantics = webrtc::SdpSemantics::kUnifiedPlan;
    _config.rtcp_mux_policy =  webrtc::PeerConnectionInterface::kRtcpMuxPolicyRequire;
    _config.bundle_policy  =  webrtc::PeerConnectionInterface::kBundlePolicyMaxBundle;
    _config.type = webrtc::PeerConnectionInterface::kAll;
    _config.candidate_network_policy = webrtc::PeerConnectionInterface::kCandidateNetworkPolicyLowCost;
   // _config.min_port =80000;
    //_config.max_port =100000;
   // _config.enable_ice_renomination = true;
    //_config.ice_candidate_pool_size=1;
    
    
}


Peer::~Peer()
{
    LInfo(_peerid, ": Destroying")
    // closeConnection();

    if (_peerConnection) {
        _peerConnection->Close();
    }
}


//rtc::scoped_refptr<webrtc::MediaStreamInterface> Peer::createMediaStream()
//{
//   // assert(_mode == Offer);
//    //assert(_context->factory);
//    assert(!_stream);
//    _stream = _context->factory->CreateLocalMediaStream(kStreamLabel);
//    return _stream;
//}


// void Peer::setPortRange(int minPort, int maxPort)
// {
//     assert(!_peerConnection);

//     if (!_context->networkManager) {
//         throw std::runtime_error("Must initialize custom network manager to set port range");
//     }

//     if (!_context->socketFactory) {
//         throw std::runtime_error("Must initialize custom socket factory to set port range");
//     }

//     if (!_portAllocator)
//         _portAllocator.reset(new cricket::BasicPortAllocator(
//             _context->networkManager.get(),
//             _context->socketFactory.get()));
//     _portAllocator->SetPortRange(minPort, maxPort);
// }


void Peer::createConnection()
{
    assert(_context->factory);
    _peerConnection = _context->factory->CreatePeerConnection(_config, nullptr, nullptr, this);

//    if (_stream) {
//        if (!_peerConnection->AddStream(_stream)) {
//            throw std::runtime_error("Adding stream to Peer failed");
//        }
//    }
}


void Peer::closeConnection()
{
    LInfo(_peerid, ": Closing")

    if (_peerConnection) {
        _peerConnection->Close();
    }
    else {
        // Call onClosed if no connection has been
        // made so callbacks are always run.
        _manager->onClosed(this);
    }
}


void Peer::createOffer()
{
    //assert(_mode == Offer);
    //assert(_peerConnection);

     LInfo(_peerid, ": Create Offer")
             
     webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;

    options.offer_to_receive_audio = false;
    options.offer_to_receive_video = true;


    _peerConnection->CreateOffer(this,options);
}


void Peer::recvSDP(const std::string& type, const std::string& sdp)
{
    LInfo(_peerid, ": Received answer ", type, ": ", sdp)

    webrtc::SdpParseError error;
    webrtc::SessionDescriptionInterface* desc(
        webrtc::CreateSessionDescription(type, sdp, &error));
    if (!desc) {
        throw std::runtime_error("Can't parse remote SDP: " + error.description);
    }
    _peerConnection->SetRemoteDescription(
        DummySetSessionDescriptionObserver::Create(), desc);


    webrtc::PeerConnectionInterface::RTCOfferAnswerOptions options;
    options.offer_to_receive_audio = true;
    options.offer_to_receive_video = true;
 
    if (type == "offer") {
       // assert(_mode == Answer);
         LError(_peerid, ": wrong state Received ", type, ": ", sdp)
        _peerConnection->CreateAnswer(this, options);
    } else {
      
    }
}


void Peer::recvCandidate(const std::string& mid, int mlineindex,
                                   const std::string& sdp)
{
    webrtc::SdpParseError error;
    std::unique_ptr<webrtc::IceCandidateInterface> candidate(
        webrtc::CreateIceCandidate(mid, mlineindex, sdp, &error));
    if (!candidate) {
        throw std::runtime_error("Can't parse remote candidate: " + error.description);
    }
    _peerConnection->AddIceCandidate(candidate.get());
}


void Peer::OnSignalingChange(webrtc::PeerConnectionInterface::SignalingState new_state)
{
    LInfo(_peerid, ": On signaling state change: ", new_state)

    switch (new_state) {
        case webrtc::PeerConnectionInterface::kStable:
            _manager->onStable(this);
            break;
        case webrtc::PeerConnectionInterface::kClosed:
            _manager->onClosed(this);
            break;
        case webrtc::PeerConnectionInterface::kHaveLocalOffer:
        case webrtc::PeerConnectionInterface::kHaveRemoteOffer:
        case webrtc::PeerConnectionInterface::kHaveLocalPrAnswer:
        case webrtc::PeerConnectionInterface::kHaveRemotePrAnswer:
            break;
    }
}


void Peer::OnIceConnectionChange(webrtc::PeerConnectionInterface::IceConnectionState new_state)
{
    LInfo(_peerid, ": On ICE connection change: ", new_state)
}


void Peer::OnIceGatheringChange(webrtc::PeerConnectionInterface::IceGatheringState new_state)
{
    LInfo(_peerid, ": On ICE gathering change: ", new_state)
    
    // if( new_state == webrtc::PeerConnectionInterface::kIceGatheringComplete)
    // {
    //     createOffer();
    //     hasIceLiteOffer=true;
    // }
}


void Peer::OnRenegotiationNeeded()
{
    LInfo(_peerid, ": On renegotiation needed")
}


void Peer::OnAddStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
{
    LInfo(_peerid, ": OnAddStream")
    // proxy to deprecated OnAddStream method
    OnAddStream(stream.get());
}

 void Peer::OnTrack( rtc::scoped_refptr<webrtc::RtpTransceiverInterface> transceiver) {

     LInfo(_peerid, ": OnTrack")
    //_manager->onAddRemoteTrack(this, transceiver.get());
     
//    const char * pMid  = transceiver->mid()->c_str();
//    int iMid = atoi(pMid);
//    RTC_LOG(INFO)  << "OnAddTrack " <<  " mid "  << pMid;
//    if(  transceiver->current_direction() !=  webrtc::RtpTransceiverDirection::kInactive &&    transceiver->direction() !=  webrtc::RtpTransceiverDirection::kInactive  )
//    {
//
//        rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track =
//                transceiver->receiver()->track();
//        RTC_LOG(INFO)  << "OnAddTrack " << track->id() <<  " kind " << track->kind() ;
//
//        if (track && remote_video_observer_[0] &&
//            track->kind() == webrtc::MediaStreamTrackInterface::kVideoKind) {
//            static_cast<webrtc::VideoTrackInterface*>(track.get())
//                    ->AddOrUpdateSink(remote_video_observer_[0].get(), rtc::VideoSinkWants());
//            RTC_LOG(LS_INFO) << "Remote video sink set up: " << track;
//
//        }
//
//    }

 }
 
void Peer::OnRemoveTrack(rtc::scoped_refptr<webrtc::RtpReceiverInterface> receiver)
{
    LInfo(_peerid, ": OnRemoveTrack")
}

void Peer::OnRemoveStream(rtc::scoped_refptr<webrtc::MediaStreamInterface> stream)
{
    LInfo(_peerid, ": OnRemoveStream")
    OnRemoveStream(stream.get());
}


void Peer::OnDataChannel(rtc::scoped_refptr<webrtc::DataChannelInterface> stream)
{
    LInfo(_peerid, ": OnDataChannel")
    assert(0 && "virtual");
}


void Peer::OnAddStream(webrtc::MediaStreamInterface* stream)
{
    //assert(_mode == Answer);

    LInfo(_peerid, ": On add stream")
    _manager->onAddRemoteStream(this, stream);
}


void Peer::OnRemoveStream(webrtc::MediaStreamInterface* stream)
{
    //assert(_mode == Answer);

    LInfo(_peerid, ": On remove stream")
    _manager->onRemoveRemoteStream(this, stream);
}


void Peer::OnIceCandidate(const webrtc::IceCandidateInterface* candidate)
{
 



 /*  std::string mid = candidate->sdp_mid() ;
   
   int line = candidate->sdp_mline_index();
  // Only for use internally.
   cricket::Candidate& can =  (cricket::Candidate& ) candidate->candidate();
  
   
   rtc::SocketAddress& add =  ( rtc::SocketAddress& ) can.address();
   
   bool x  =add.IsPrivateIP();
   if(x)
   add.SetIP("44.226.10.202");
   
  */
   
   std::string sdp;
    if (!candidate->ToString(&sdp)) {
        LError(_peerid, ": Failed to serialize candidate")
        assert(0);
        return;
    }
      
    LInfo(_peerid, sdp);
    _manager->sendCandidate(this, candidate->sdp_mid(),
                           candidate->sdp_mline_index(), sdp);

 
}

void Peer::pushFrame( fmp4::BasicFrame *f)
{
    lock_.lock();
    
    bframe.push(f);
    
    lock_.unlock();
}

fmp4::BasicFrame* Peer::popFrame( )
{
    lock_.lock();
     
    
    fmp4::BasicFrame *f  = bframe.front();
     
    bframe.pop();
    
    lock_.unlock();
    
    return f;
}

 
void Peer::OnSuccess(webrtc::SessionDescriptionInterface* desc)
{
    LInfo(_peerid, ": Set local description")
            
    cricket::SessionDescription* desc1 = desc->description();
    
 
    /*for (const auto& content : desc1->contents()) {
       auto* transport_info = desc1->GetTransportInfoByName(content.name);
       transport_info->description.ice_mode = cricket::IceMode::ICEMODE_LITE;
      // transport_info->description.connection_role =  cricket::CONNECTIONROLE_ACTIVE;
       transport_info->description.transport_options.clear();
        transport_info->description.transport_options.push_back("renomination");
       
     }*/
    
    /*
    const rtc::SocketAddress kCallerAddress1("192.168.0.17", 1111);
    cricket::Candidate candidate1 = CreateLocalUdpCandidate(kCallerAddress1);
    AddCandidateToFirstTransport(&candidate1, SDP);
    const rtc::SocketAddress kCallerAddress2("192.168.0.17", 1001);
    cricket::Candidate candidate2 = CreateLocalTcpCandidate(kCallerAddress2);
    AddCandidateToFirstTransport(&candidate2, SDP);
    */
    
    _peerConnection->SetLocalDescription(
        DummySetSessionDescriptionObserver::Create(), desc);

    // Send an SDP offer to the peer

    //if(hasIceLiteOffer)
    {   
         std::string sdp;
        if (!desc->ToString(&sdp)) {
            LError(_peerid, ": Failed to serialize local sdp")
            assert(0);
            return;
        }
        _manager->sendSDP(this, desc->type(), sdp);
    }

    
}


void Peer::OnFailure(const std::string& error)
{
    LError(_peerid, ": On failure: ", error)

    _manager->onFailure(this, error);
}


void Peer::setPeerFactory(rtc::scoped_refptr<webrtc::PeerConnectionFactoryInterface> factory)
{
    assert(!_context->factory); // should not be already set via PeerManager
    _context->factory = factory;
}


std::string Peer::peerid() const
{
    return _peerid;
}


std::string Peer::token() const
{
    return _token;
}


 void Peer::mute( const json& m)
 {
     bool val = m.get<bool>();
     
     SInfo << _peerid <<  ": On mute: " <<  val ;
     
    std::vector<rtc::scoped_refptr<webrtc::RtpSenderInterface>> senders =  _peerConnection->GetSenders();
     
    for (const auto& sender : senders) {
     rtc::scoped_refptr<webrtc::MediaStreamTrackInterface> track = sender->track();
     if( track && track->kind() =="audio")
     {

         track->set_enabled(!val); 

     }
       
    }
     
 }
 

// webrtc::FakeConstraints& Peer::constraints()
// {
//     return _constraints;
// }


webrtc::PeerConnectionFactoryInterface* Peer::factory() const
{
    return _context->factory.get();
}


rtc::scoped_refptr<webrtc::PeerConnectionInterface> Peer::peerConnection() const
{
    return _peerConnection;
}


//rtc::scoped_refptr<webrtc::MediaStreamInterface> Peer::stream() const
//{
//    return _stream;
//}


//
// Dummy Set Peer Description Observer
//


void DummySetSessionDescriptionObserver::OnSuccess()
{
    LDebug("On SDP parse success")
}


void DummySetSessionDescriptionObserver::OnFailure(const std::string& error)
{
    LError("On SDP parse error: ", error)
    assert(0);
}


} } // namespace wrtc

