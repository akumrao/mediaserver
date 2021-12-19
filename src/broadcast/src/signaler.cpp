
#include <iostream>
#include <string>

#include "webrtc/audiopacketmodule.h"
#include "webrtc/multiplexmediacapturer.h"
#include "webrtc/videopacketsource.h"
#include "webrtc/signaler.h"
#include "Settings.h"


using std::endl;
//using namespace base::sockio;
namespace base {
    namespace wrtc {

        Signaler::Signaler() :
        _capturer()
        , _context(_capturer.getAudioModule()) {

        }

        Signaler::~Signaler() {
            
            SInfo << "~Signaler() ";
        }

        void Signaler::startStreaming(const std::string& dir, const std::string& file,  const std::string& type ,  bool looping) {
            // Open the video file
            
             SInfo << "Open  " << dir <<  "/" <<  file;
                     
            _capturer.openFile(dir,file, type, looping);
           //  _capturer.start();  // enable for rapid testing
        }

        void Signaler::sendSDP(wrtc::Peer* conn, const std::string& type,
                const std::string& sdp) {
            assert(type == "offer" || type == "answer");
            //smpl::Message m;
            json desc;
            desc[wrtc::kSessionDescriptionTypeName] = type;
            desc[wrtc::kSessionDescriptionSdpName] = sdp;

            json m;

            m[wrtc::kSessionDescriptionTypeName] = type;
            m["desc"] = desc;
            m["from"] = conn->peerid();
            m["to"]= conn->peerid();
            // smpl::Message m({ type, {
            //     { wrtc::kSessionDescriptionTypeName, type },
            //     { wrtc::kSessionDescriptionSdpName, sdp} }
            // });

            postMessage(m);
        }

        void Signaler::sendCandidate(wrtc::Peer* conn, const std::string& mid,
                int mlineindex, const std::string& sdp) {
            //smpl::Message m;
            json desc;
            desc[wrtc::kCandidateSdpMidName] = mid;
            desc[wrtc::kCandidateSdpMlineIndexName] = mlineindex;
            desc[wrtc::kCandidateSdpName] = sdp;

            json m;
            m[wrtc::kSessionDescriptionTypeName] = "candidate";
            m["candidate"] = desc;
            m["from"] = conn->peerid();
            m["to"]= conn->peerid();

            // smpl::Message m({ "candidate", {
            //     { wrtc::kCandidateSdpMidName, mid },
            //     { wrtc::kCandidateSdpMlineIndexName, mlineindex},
            //     { wrtc::kCandidateSdpName, sdp} }
            // });

            LTrace( "send candidate ",  cnfg::stringify(m))
            postMessage(m);
        }

        void Signaler::onPeerConnected(std::string& peerID ,  std::string &cam , std::string &room ) {

            LDebug("Peer connected: ", peerID)

            if (wrtc::PeerManager::exists(peerID)) {
                LDebug("Peer already has session: ", peerID)
                return;
            }

            // Create the Peer Peer
            auto conn = new wrtc::Peer(this, &_context, cam, room, peerID, "", wrtc::Peer::Offer);
           // conn->constraints().SetMandatoryReceiveAudio(false);
           // conn->constraints().SetMandatoryReceiveVideo(false);
           // conn->constraints().SetAllowDtlsSctpDataChannels();

            // Create the media stream and attach decoder
            // output to the peer connection
            conn->createConnection();
            _capturer.addMediaTracks(_context.factory, conn->_peerConnection , conn);

            // Send the Offer SDP
            
            conn->createOffer();

            wrtc::PeerManager::add(peerID, conn);
        }

        void Signaler::onPeerMessage(std::string &name , json const& m) {

            if (std::string("got user media") == m) {
                return;
            }

            std::string from;
            std::string type;
            std::string room;
            std::string to;
            std::string user;
            
            std::string camT;
	 
            if (m.find("to") != m.end()) {
                to = m["to"].get<std::string>();
            }

            if (m.find("from") != m.end()) {
                from = m["from"].get<std::string>();
            }
            else
            {
                SError << " On Peer message is missing participant id ";
                return;
            }
            
            if (m.find("type") != m.end()) {
                type = m["type"].get<std::string>();
            }else
            {
                SError << " On Peer message is missing SDP type";
            }
            
            if (m.find("room") != m.end()) {
                room = m["room"].get<std::string>();
            }
            else
            {
                SError << " On Peer message is missing room id ";
                return;
            }
            
//             if (m.find("user") != m.end()) {
//                user = m["user"].get<std::string>();
//            }
//            else
//            {
//                SWarn << " On Peer message is missing user name ";
//            }
            
            if (m.find("cam") != m.end())
            {
               camT = m["cam"].get<std::string>();
               // cam = std::stoi(camT);
                if( Settings::configuration.rtsp.find(camT) == Settings::configuration.rtsp.end()  )
                {
                    if(camT == "0"  )                    
                    camT = Settings::configuration.rtsp.begin().key();
                    else
                    {
                        postAppMessage("Camera not available, check with Json API Cam: " + camT, from , room  );
                        return;
                    }
                }
            }

            LInfo("Peer message: ", from, " ", type )

            if (std::string("offer") == type) {

                onPeerConnected(from, camT, room);
                
            } else if (std::string("answer") == type) {
                recvSDP(from, m["desc"]);
            } else if (std::string("candidate") == type) {
                recvCandidate(from, m["candidate"]);
            } else if (std::string("mute") == type) {

                LInfo("Peer message: ", from, " ", m.dump(4) )
                        
                auto conn = wrtc::PeerManager::get(from);
                if (conn) {
                    conn->mute(m["desc"]  );
                }
                

            } else if (std::string("bye") == type) {
               // onPeerDiconnected(from);
            }
            

        }

        void Signaler::onPeerDiconnected(std::string& peerID) {
            LDebug("Peer disconnected")

            auto conn = wrtc::PeerManager::remove(peerID);
            if (conn) {
                LInfo("Deleting peer connection: ", peerID)
                        // async delete not essential, but to be safe
                        delete conn;
                //deleteLater<wrtc::Peer>(conn);
            }
        }

        void Signaler::onAddRemoteStream(wrtc::Peer* conn, webrtc::MediaStreamInterface* stream) {
            //assert(0 && "not required");
        }

        void Signaler::onRemoveRemoteStream(wrtc::Peer* conn, webrtc::MediaStreamInterface* stream) {
            assert(0 && "not required");
        }

        void Signaler::onStable(wrtc::Peer* conn) {
             SInfo << "Start Capture cam "  <<  conn->getCam();
            _capturer.start( conn->getCam());
        }

        void Signaler::onClosed(wrtc::Peer* conn) {
              SInfo << "Stop Capture cam "  <<  conn->getCam();
            _capturer.stop(conn->getCam());
            wrtc::PeerManager::onClosed(conn);
        }

        void Signaler::onFailure(wrtc::Peer* conn, const std::string& error) {
            LInfo("onFailure stop FFMPEG Capture")
            _capturer.stop(conn->getCam());
            wrtc::PeerManager::onFailure(conn, error);
        }

        void Signaler::postMessage(const json& m)
        {

            SDebug << "postMessage " <<  cnfg::stringify(m);
            
            socket->emit("message", m);
        }

       void Signaler::postAppMessage(std::string message , std::string from , std::string &room) 
       {

           // LTrace("postAppMessage", cnfg::stringify(m));
           
            json m;
            m["type"] = "error";
            m["desc"] = message ;
            m["to"] =from;
            m["room"] = room;
            
            socket->emit("postAppMessage", m);
       }


       void Signaler::connect(const std::string& host, const uint16_t port, const std::string rm)
       {

            LTrace("Tests signalling Begin. Please run signalling server at webrtc folder")

            client = new sockio::SocketioClient(host, port, true);
            client->connect();

            socket = client->io();

            socket->on("connection", sockio::Socket::event_listener_aux([ = ](string const& name, json const& data, bool isAck, json & ack_resp){

                socket->on("ipaddr", sockio::Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                    LTrace(cnfg::stringify(data))

                    LTrace("Server IP address is: ", data)
                            // updateRoomURL(ipaddr);
                }));

                socket->on("created", sockio::Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {

                    LTrace(cnfg::stringify(data))
                    LTrace("Created room", data[0], "- my client ID is", data[1])
                    isInitiator = true;
                    //grabWebCamVideo();
                }));

                socket->on("full", sockio::Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                    LTrace(cnfg::stringify(data));
                    //LTrace("Room " + room + " is full.")

                }));


                socket->on("join", sockio::Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                    LTrace(cnfg::stringify(data));
                   // LTrace("Another peer made a request to join room " + room)
                    //LTrace("This peer is the initiator of room " + room + "!")
                    isChannelReady = true;

                }));

   /// for webrtc messages
                socket->on("message", sockio::Socket::event_listener_aux([&](string const& name, json const& m, bool isAck, json & ack_resp) {
                    //  LTrace(cnfg::stringify(m));
                    // LTrace('SocketioClient received message:', cnfg::stringify(m));

                    onPeerMessage((string &)name, m);
                    // signalingMessageCallback(message);


                }));


                // Leaving rooms and disconnecting from peers.
                socket->on("disconnectClient", sockio::Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                    std::string from = data.get<std::string>();
                    SInfo << "disconnectClient " <<  from;
                    //LInfo(cnfg::stringify(data));
                    onPeerDiconnected(from);
                }));


                socket->on("bye", sockio::Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                    LTrace(cnfg::stringify(data));
                    //LTrace("Peer leaving room", room);
   
                }));

                socket->emit("CreateSFU");


            }));


        }



    }
} // namespace base
