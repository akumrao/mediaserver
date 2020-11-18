
#include <iostream>
#include <string>

#include "webrtc/audiopacketmodule.h"
#include "webrtc/multiplexmediacapturer.h"
#include "webrtc/videopacketsource.h"
#include "webrtc/signaler.h"
#include "api/mediastreamtrackproxy.h"


using std::endl;

namespace base {
    namespace wrtc {

        Signaler::Signaler() :
        _capturer()
        , _context(_capturer.getAudioModule()) {

        }

        Signaler::~Signaler() {
        }

        void Signaler::startStreaming(const std::string& file, bool looping) {
            // Open the video file
            _capturer.openFile(file, looping);
            // _capturer.start();
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
            m["from"] = peerID;
            m["to"]=remotePeerID;
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
            m["from"] = peerID;
            m["to"]=remotePeerID;

            // smpl::Message m({ "candidate", {
            //     { wrtc::kCandidateSdpMidName, mid },
            //     { wrtc::kCandidateSdpMlineIndexName, mlineindex},
            //     { wrtc::kCandidateSdpName, sdp} }
            // });

            LTrace( "send candidate ",  cnfg::stringify(m))
            postMessage(m);
        }

        void Signaler::onPeerConnected(std::string& peerID) {

            LDebug("Peer connected: ", peerID)

            if (wrtc::PeerManager::exists(peerID)) {
                LDebug("Peer already has session: ", peerID)
                return;
            }

            // Create the Peer Peer
            auto conn = new wrtc::Peer(this, &_context, peerID, "", wrtc::Peer::Offer);
            conn->constraints().SetMandatoryReceiveAudio(false);
            conn->constraints().SetMandatoryReceiveVideo(false);
            conn->constraints().SetAllowDtlsSctpDataChannels();

            // Create the media stream and attach decoder
            // output to the peer connection
            _capturer.addMediaTracks(_context.factory, conn->createMediaStream());

            // Send the Offer SDP
            conn->createConnection();
            conn->createOffer();

            wrtc::PeerManager::add(peerID, conn);
        }

        void Signaler::onPeerMessage(json const& m) {

            if (std::string("got user media") == m) {
                return;
            }

            std::string from;
            std::string type;

            if (m.find("from") != m.end()) {
                from = m["from"].get<std::string>();
            }
            if (m.find("type") != m.end()) {
                type = m["type"].get<std::string>();
            }

            LDebug("Peer message: ", from)

            if (std::string("offer") == type) {
                //assert(0 && "offer not supported");
                remotePeerID = from;
                onPeerConnected(from);
                
            } else if (std::string("answer") == type) {
                recvSDP(from, m["desc"]);
            } else if (std::string("candidate") == type) {
                recvCandidate(from, m["candidate"]);
            } else if (std::string("bye") == type) {
                onPeerDiconnected(from);
            }

        }

        void Signaler::onPeerDiconnected(std::string& peerID) {
            LDebug("Peer disconnected")

                    auto conn = wrtc::PeerManager::remove(peerID);
            if (conn) {
                LDebug("Deleting peer connection: ", peerID)
                        // async delete not essential, but to be safe
                        delete conn;
                //deleteLater<wrtc::Peer>(conn);
            }
        }

        void Signaler::onAddRemoteStream(wrtc::Peer* conn, webrtc::MediaStreamInterface* stream) {
            assert(0 && "not required");
        }

        void Signaler::onRemoveRemoteStream(wrtc::Peer* conn, webrtc::MediaStreamInterface* stream) {
            assert(0 && "not required");
        }

        void Signaler::onStable(wrtc::Peer* conn) {
            LTrace("Start FFMPEG Capture")
            _capturer.start();
        }

        void Signaler::onClosed(wrtc::Peer* conn) {
            LTrace("stop FFMPEG Capture")
            _capturer.stop();
            wrtc::PeerManager::onClosed(conn);
        }

        void Signaler::onFailure(wrtc::Peer* conn, const std::string& error) {
            LTrace("onFailure stop FFMPEG Capture")
            _capturer.stop();
            wrtc::PeerManager::onFailure(conn, error);
        }

        void Signaler::postMessage(const json& m) {

            LTrace("postMessage", cnfg::stringify(m));
            socket->emit("message", m);
        }

        void Signaler::connect(const std::string& host, const uint16_t port, const std::string rm) {

            room = rm;

            LTrace("Tests signalling Begin. Please run signalling server at webrtc folder")

            client = new SocketioClient(host, port);
            client->connect();

            socket = client->io();

            socket->on("connection", Socket::event_listener_aux([ = ](string const& name, json const& data, bool isAck, json & ack_resp){

                socket->on("ipaddr", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                    LTrace(cnfg::stringify(data))

                    LTrace("Server IP address is: ", data)
                            // updateRoomURL(ipaddr);
                }));

                socket->on("created", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {

                    LTrace(cnfg::stringify(data))
                    LTrace("Created room", data[0], "- my client ID is", data[1])
                    isInitiator = true;
                    //grabWebCamVideo();
                }));

                socket->on("full", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                    LTrace(cnfg::stringify(data));
                    LTrace("Room " + room + " is full.")
                            // window.location.hash = '';
                            // window.location.reload();
                }));


                socket->on("join", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                    LTrace(cnfg::stringify(data));
                    LTrace("Another peer made a request to join room " + room)
                    LTrace("This peer is the initiator of room " + room + "!")
                    isChannelReady = true;



                }));

                socket->on("joined", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {

                    LTrace(cnfg::stringify(data))
                    LTrace("joined: ", data[0])
                    peerID = data[1];
                    ////////////////////////     

                }));



                socket->on("ready", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                    LTrace(cnfg::stringify(data))
                            // LTrace('Socket is ready');
                            // createPeerConnection(isInitiator, configuration);
                }));

                socket->on("log", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                    // LTrace(cnfg::stringify(data))
                    LTrace(cnfg::stringify(data))
                }));

                socket->on("message", Socket::event_listener_aux([&](string const& name, json const& m, bool isAck, json & ack_resp) {
                    LTrace(cnfg::stringify(m));
                    LTrace('SocketioClient received message:', cnfg::stringify(m));

                    onPeerMessage(m);
                    // signalingMessageCallback(message);



                }));



                // Leaving rooms and disconnecting from peers.
                socket->on("disconnect", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                    LTrace(cnfg::stringify(data));
                    //LTrace(`Disconnected: ${reason}.`);
                    // sendBtn.disabled = true;
                    // snapAndSendBtn.disabled = true;
                }));


                socket->on("bye", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                    LTrace(cnfg::stringify(data));
                    LTrace("Peer leaving room", room);
                    // sendBtn.disabled = true;
                    //snapAndSendBtn.disabled = true;
                    // If peer did not create the room, re-enter to be creator.
                    //if (!isInitiator) {
                    // window.location.reload();
                    //}
                }));


                // window.addEventListener('unload', function() {
                //  LTrace(`Unloading window. Notifying peers in ${room}.`);
                // socket->emit('bye', room);
                // });

                if (room != "") {
                    socket->emit("create or join", room);
                            LTrace("Attempted to create or  join room ", room);
                }


                //socket->emit("ipaddr");
            }));


        }



    }
} // namespace base
