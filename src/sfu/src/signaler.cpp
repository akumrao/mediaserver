
#include <iostream>
#include <string>

#include "signaler.h"

using std::endl;

namespace base {
    namespace wrtc {

        Signaler::Signaler() 
        {
            Logger::instance().add(new ConsoleChannel("debug", Level::Trace));
            worker = new Worker(nullptr);

        }

        Signaler::~Signaler() {
        }

//        void Signaler::startStreaming(const std::string& file, bool looping) {
//
//        }
//
//        void Signaler::sendSDP(wrtc::Peer* conn, const std::string& type,
//                const std::string& sdp) {
//            assert(type == "offer" || type == "answer");
//            //smpl::Message m;
//            json desc;
//            desc[wrtc::kSessionDescriptionTypeName] = type;
//            desc[wrtc::kSessionDescriptionSdpName] = sdp;
//
//            json m;
//
//            m[wrtc::kSessionDescriptionTypeName] = type;
//            m["desc"] = desc;
//            m["from"] = peerID;
//            m["to"]=remotePeerID;
//            // smpl::Message m({ type, {
//            //     { wrtc::kSessionDescriptionTypeName, type },
//            //     { wrtc::kSessionDescriptionSdpName, sdp} }
//            // });
//
//            postMessage(m);
//        }
//
//        void Signaler::sendCandidate(wrtc::Peer* conn, const std::string& mid,
//                int mlineindex, const std::string& sdp) {
//            //smpl::Message m;
//            json desc;
//            desc[wrtc::kCandidateSdpMidName] = mid;
//            desc[wrtc::kCandidateSdpMlineIndexName] = mlineindex;
//            desc[wrtc::kCandidateSdpName] = sdp;
//
//            json m;
//            m[wrtc::kSessionDescriptionTypeName] = "candidate";
//            m["candidate"] = desc;
//            m["from"] = peerID;
//            m["to"]=remotePeerID;
//
//            // smpl::Message m({ "candidate", {
//            //     { wrtc::kCandidateSdpMidName, mid },
//            //     { wrtc::kCandidateSdpMlineIndexName, mlineindex},
//            //     { wrtc::kCandidateSdpName, sdp} }
//            // });
//
//            LTrace( "send candidate ",  cnfg::stringify(m))
//            postMessage(m);
//        }
//
//        void Signaler::onPeerConnected(std::string& peerID) {
//
//            LDebug("Peer connected: ", peerID)
//
//            if (wrtc::PeerManager::exists(peerID)) {
//                LDebug("Peer already has session: ", peerID)
//                return;
//            }
//
//            // Create the Peer Peer
//            auto conn = new wrtc::Peer(this, &_context, peerID, "", wrtc::Peer::Offer);
//            conn->constraints().SetMandatoryReceiveAudio(false);
//            conn->constraints().SetMandatoryReceiveVideo(false);
//            conn->constraints().SetAllowDtlsSctpDataChannels();
//
//            // Create the media stream and attach decoder
//            // output to the peer connection
//            _capturer.addMediaTracks(_context.factory, conn->createMediaStream());
//
//            // Send the Offer SDP
//            conn->createConnection();
//            conn->createOffer();
//
//            wrtc::PeerManager::add(peerID, conn);
//        }
//
//        void Signaler::onPeerMessage(json const& m) {
//
//            if (std::string("got user media") == m) {
//                return;
//            }
//
//            std::string from;
//            std::string type;
//
//            if (m.find("from") != m.end()) {
//                from = m["from"].get<std::string>();
//            }
//            if (m.find("type") != m.end()) {
//                type = m["type"].get<std::string>();
//            }
//
//            LDebug("Peer message: ", from)
//
//            if (std::string("offer") == type) {
//                //assert(0 && "offer not supported");
//                remotePeerID = from;
//                onPeerConnected(from);
//                
//            } else if (std::string("answer") == type) {
//                recvSDP(from, m["desc"]);
//            } else if (std::string("candidate") == type) {
//                recvCandidate(from, m["candidate"]);
//            } else if (std::string("bye") == type) {
//                onPeerDiconnected(from);
//            }
//
//        }
//
//        void Signaler::onPeerDiconnected(std::string& peerID) {
//            LDebug("Peer disconnected")
//
//                    auto conn = wrtc::PeerManager::remove(peerID);
//            if (conn) {
//                LDebug("Deleting peer connection: ", peerID)
//                        // async delete not essential, but to be safe
//                        delete conn;
//                //deleteLater<wrtc::Peer>(conn);
//            }
//        }
//
//        void Signaler::onAddRemoteStream(wrtc::Peer* conn, webrtc::MediaStreamInterface* stream) {
//            assert(0 && "not required");
//        }
//
//        void Signaler::onRemoveRemoteStream(wrtc::Peer* conn, webrtc::MediaStreamInterface* stream) {
//            assert(0 && "not required");
//        }
//
//        void Signaler::onStable(wrtc::Peer* conn) {
//            LTrace("Start FFMPEG Capture")
//            _capturer.start();
//        }
//
//        void Signaler::onClosed(wrtc::Peer* conn) {
//            LTrace("stop FFMPEG Capture")
//            _capturer.stop();
//            wrtc::PeerManager::onClosed(conn);
//        }
//
//        void Signaler::onFailure(wrtc::Peer* conn, const std::string& error) {
//            LTrace("onFailure stop FFMPEG Capture")
//            _capturer.stop();
//            wrtc::PeerManager::onFailure(conn, error);
//        }

        void Signaler::postMessage(const json& m) {

            LTrace("postMessage", cnfg::stringify(m));
            socket->emit("message", m);
        }

        void Signaler::connect(const std::string& host, const uint16_t port, const std::string rm) {

            room = rm;

            LTrace("Tests signalling Begin. Please run signalling server at webrtc folder")

            client = new SocketioClient(host, port, true);
            client->connect();

            socket = client->io();

            socket->on("connection", Socket::event_listener_aux([ = ](string const& name, json const& data, bool isAck, json & ack_resp){

                socket->on("ipaddr", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                    LTrace(cnfg::stringify(data))

                    LTrace("Server IP address is: ", data)
                            // updateRoomURL(ipaddr);
                }));

                
                /*
{\"id\":6,\"method\":\"router.createWebRtcTransport\",\"internal\":{\"routerId\":\"2e32062d-f04a-4c2d-a656-b586e50498ef\",\"transportId\":\"4ca62904-639c-4b5a-86e8-f0bc84bfe776\"},\"data\":{\"listenIps\":[{\"ip\":\"127.0.0.1\"}],\"enableUdp\":true,\"enableTcp\":true,\"preferUdp\":true,\"preferTcp\":false,\"initialAvailableOutgoingBitrate\":1000000,\"enableSctp\":false,\"numSctpStreams\":{\"OS\":1024,\"MIS\":1024},\"maxSctpMessageSize\":262144,\"isDataChannel\":true}}
{\"id\":7,\"method\":\"transport.setMaxIncomingBitrate\",\"internal\":{\"routerId\":\"2e32062d-f04a-4c2d-a656-b586e50498ef\",\"transportId\":\"4ca62904-639c-4b5a-86e8-f0bc84bfe776\"},\"data\":{\"bitrate\":1500000}}
{\"id\":8,\"method\":\"transport.consume\",\"internal\":{\"routerId\":\"2e32062d-f04a-4c2d-a656-b586e50498ef\",\"transportId\":\"4ca62904-639c-4b5a-86e8-f0bc84bfe776\",\"consumerId\":\"c3dbd428-ff6e-46b6-a1e7-ba3891b70f34\",\"producerId\":\"87c81cfc-b307-4270-a213-4f17e4776931\"},\"data\":{\"kind\":\"video\",\"rtpParameters\":{\"codecs\":[{\"mimeType\":\"video/VP8\",\"clockRate\":90000,\"payloadType\":101,\"rtcpFeedback\":[{\"type\":\"transport-cc\"},{\"type\":\"ccm\",\"parameter\":\"fir\"},{\"type\":\"nack\"},{\"type\":\"nack\",\"parameter\":\"pli\"}],\"parameters\":{}},{\"mimeType\":\"video/rtx\",\"clockRate\":90000,\"payloadType\":102,\"rtcpFeedback\":[],\"parameters\":{\"apt\":101}}],\"headerExtensions\":[{\"uri\":\"http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\",\"id\":4},{\"uri\":\"http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\",\"id\":5},{\"uri\":\"http://tools.ietf.org/html/draft-ietf-avtext-framemarking-07\",\"id\":6},{\"uri\":\"urn:3gpp:video-orientation\",\"id\":11},{\"uri\":\"urn:ietf:params:rtp-hdrext:toffset\",\"id\":12}],\"encodings\":[{\"ssrc\":685058887,\"rtx\":{\"ssrc\":463150472}}],\"rtcp\":{\"cname\":\"d1blHuW4nPAoUtQb\",\"reducedSize\":true,\"mux\":true}},\"type\":\"simple\",\"consumableRtpEncodings\":[{\"ssrc\":632915476}],\"paused\":true}}
{\"id\":9,\"method\":\"transport.connect\",\"internal\":{\"routerId\":\"2e32062d-f04a-4c2d-a656-b586e50498ef\",\"transportId\":\"4ca62904-639c-4b5a-86e8-f0bc84bfe776\"},\"data\":{\"dtlsParameters\":{\"role\":\"client\",\"fingerprints\":[{\"algorithm\":\"sha-256\",\"value\":\"D0:75:50:E5:7E:B7:EB:5D:0D:A2:2B:C6:2E:E6:F0:20:66:2E:91:25:3D:3E:DF:F5:F1:0C:62:3A:9E:40:60:C0\"}]}}}
{\"id\":10,\"method\":\"consumer.resume\",\"internal\":{\"routerId\":\"2e32062d-f04a-4c2d-a656-b586e50498ef\",\"transportId\":\"4ca62904-639c-4b5a-86e8-f0bc84bfe776\",\"consumerId\":\"c3dbd428-ff6e-46b6-a1e7-ba3891b70f34\",\"producerId\":\"87c81cfc-b307-4270-a213-4f17e4776931\"}}
 */                   
   
                
                socket->on("created", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                    
               
                SInfo << "Created room " <<  cnfg::stringify( data[0]) <<  " - my client ID is "  <<  cnfg::stringify( data[1]) ;
                              
                json jsonRequest = data[2]; //json::parse("{\"id\":1,\"method\":\"worker.createRouter\",\"internal\":{\"routerId\":\"2e32062d-f04a-4c2d-a656-b586e50498ef\"}}");//_json;
                LTrace("arvind ", cnfg::stringify(jsonRequest))
                Channel::Request req(nullptr, jsonRequest);
                worker->OnChannelRequest(nullptr, &req);
                if (isAck)
                {   
                    json arr  =  json::array();
                    arr.push_back(req.jsonResponse);
                    ack_resp = arr; 
                }
                
//                LTrace("Created room", data[0], "- my client ID is", data[1])
//                {   
//                    json jsonRequest=data[2];//json::parse("{\"id\":1,\"method\":\"worker.createRouter\",\"internal\":{\"routerId\":\"2e32062d-f04a-4c2d-a656-b586e50498ef\"}}");//_json;
//                     LTrace("arvind ", cnfg::stringify(jsonRequest))
//                    Channel::Request req(nullptr, jsonRequest);
//                    worker->OnChannelRequest(nullptr, &req );
//                     if(isAck)
//                      ack_resp =req.jsonResponse; 
//                }
//                 
                
//                {   
//                    json jsonRequest=json::parse("{\"id\":2,\"method\":\"router.createWebRtcTransport\",\"internal\":{\"routerId\":\"2e32062d-f04a-4c2d-a656-b586e50498ef\",\"transportId\":\"e5302612-283c-4532-8acb-8f3cbb87a8a5\"},\"data\":{\"listenIps\":[{\"ip\":\"127.0.0.1\"}],\"enableUdp\":true,\"enableTcp\":true,\"preferUdp\":true,\"preferTcp\":false,\"initialAvailableOutgoingBitrate\":1000000,\"enableSctp\":false,\"numSctpStreams\":{\"OS\":1024,\"MIS\":1024},\"maxSctpMessageSize\":262144,\"isDataChannel\":true}}");//_json;
//                     LTrace("arvind ", cnfg::stringify(jsonRequest))
//                    Channel::Request req(nullptr, jsonRequest);
//                    worker->OnChannelRequest(nullptr, &req );
//                    
//                    
//                    LTrace("arvind return", cnfg::stringify(req.data))
//                    
//                    
//                    int x = 0;
//                    
//                }
//                
//                {   
//                    json jsonRequest=json::parse("{\"id\":3,\"method\":\"transport.setMaxIncomingBitrate\",\"internal\":{\"routerId\":\"2e32062d-f04a-4c2d-a656-b586e50498ef\",\"transportId\":\"e5302612-283c-4532-8acb-8f3cbb87a8a5\"},\"data\":{\"bitrate\":1500000}}");
//                    
//                    //json jsonRequest=json::parse("{\"id\":1,\"method\":\"worker.createRouter\",\"internal\":{\"routerId\":\"58088165-9080-49ce-a377-3a88f858bbd7\"}}");//_json;
//                    Channel::Request req(nullptr, jsonRequest);
//                    LTrace("arvind ", cnfg::stringify(jsonRequest))
//                    worker->OnChannelRequest(nullptr, &req );
//                    
//                    LTrace("arvind return", cnfg::stringify(req.data))
//                             
//                    int x = 1;
//                     
//                }
//                
//                {   
//                    json jsonRequest=json::parse(" {\"id\":4,\"method\":\"transport.connect\",\"internal\":{\"routerId\":\"2e32062d-f04a-4c2d-a656-b586e50498ef\",\"transportId\":\"e5302612-283c-4532-8acb-8f3cbb87a8a5\"},\"data\":{\"dtlsParameters\":{\"role\":\"server\",\"fingerprints\":[{\"algorithm\":\"sha-256\",\"value\":\"30:D3:F2:7C:DB:12:F3:FD:D4:38:31:19:2F:48:B5:ED:85:59:85:99:D2:5C:E8:A5:AE:A2:57:C6:FF:93:57:65\"}]}}}");
//                     LTrace("arvind ", cnfg::stringify(jsonRequest))
//                    //json jsonRequest=json::parse(\\"{\\\"id\\":1,\\"method\\":\\"worker.createRouter\\",\\"internal\\":{\\"routerId\\":\"58088165-9080-49ce-a377-3a88f858bbd7\"}}");//_json;
//                    Channel::Request req(nullptr, jsonRequest);
//                    worker->OnChannelRequest(nullptr, &req );
//                }
//                
//                
//                {   
//                     
//                 json jsonRequest=json::parse("{\"id\":5,\"method\":\"transport.produce\",\"internal\":{\"routerId\":\"2e32062d-f04a-4c2d-a656-b586e50498ef\",\"transportId\":\"e5302612-283c-4532-8acb-8f3cbb87a8a5\",\"producerId\":\"87c81cfc-b307-4270-a213-4f17e4776931\"},\"data\":{\"kind\":\"video\",\"rtpParameters\":{\"mid\":\"0\",\"codecs\":[{\"mimeType\":\"video/VP8\",\"clockRate\":90000,\"payloadType\":96,\"rtcpFeedback\":[{\"type\":\"goog-remb\"},{\"type\":\"transport-cc\"},{\"type\":\"ccm\",\"parameter\":\"fir\"},{\"type\":\"nack\"},{\"type\":\"nack\",\"parameter\":\"pli\"}],\"parameters\":{}},{\"mimeType\":\"video/rtx\",\"clockRate\":90000,\"payloadType\":97,\"rtcpFeedback\":[],\"parameters\":{\"apt\":96}}],\"headerExtensions\":[{\"uri\":\"urn:ietf:params:rtp-hdrext:sdes:mid\",\"id\":4},{\"uri\":\"urn:ietf:params:rtp-hdrext:sdes:rtp-stream-id\",\"id\":5},{\"uri\":\"urn:ietf:params:rtp-hdrext:sdes:repaired-rtp-stream-id\",\"id\":6},{\"uri\":\"http://www.webrtc.org/experiments/rtp-hdrext/abs-send-time\",\"id\":2},{\"uri\":\"http://www.ietf.org/id/draft-holmer-rmcat-transport-wide-cc-extensions-01\",\"id\":3},{\"uri\":\"http://tools.ietf.org/html/draft-ietf-avtext-framemarking-07\",\"id\":8},{\"uri\":\"urn:3gpp:video-orientation\",\"id\":13},{\"uri\":\"urn:ietf:params:rtp-hdrext:toffset\",\"id\":14}],\"encodings\":[{\"ssrc\":6897981,\"rtx\":{\"ssrc\":3637419867}}],\"rtcp\":{\"cname\":\"d1blHuW4nPAoUtQb\"}},\"rtpMapping\":{\"codecs\":[{\"payloadType\":96,\"mappedPayloadType\":101},{\"payloadType\":97,\"mappedPayloadType\":102}],\"encodings\":[{\"mappedSsrc\":632915476,\"ssrc\":6897981}]},\"paused\":false}}");
//                    //json jsonRequest=json::parse(\\"{\\\"id\\":1,\\"method\\":\\"worker.createRouter\\",\\"internal\\":{\\"routerId\\":\"58088165-9080-49ce-a377-3a88f858bbd7\"}}");//_json;
//                    LTrace("arvind ", cnfg::stringify(jsonRequest))
//                    Channel::Request req(nullptr, jsonRequest);
//                    worker->OnChannelRequest(nullptr, &req );
//                }

            
                isInitiator = true;
                    //grabWebCamVideo();
                }));
                
                
                socket->on("createWebRtcTransport", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                    
                SInfo << "room " <<  cnfg::stringify( data[0]) <<  " - my client ID is "  <<  cnfg::stringify( data[1]) ;
                              
                json jsonRequest = data[2]; //json::parse("{\"id\":1,\"method\":\"worker.createRouter\",\"internal\":{\"routerId\":\"2e32062d-f04a-4c2d-a656-b586e50498ef\"}}");//_json;
                LTrace("arvind ", cnfg::stringify(jsonRequest))
                Channel::Request req(nullptr, jsonRequest);
                worker->OnChannelRequest(nullptr, &req);
                if (isAck)
                {   
                    json arr  =  json::array();
                    arr.push_back(req.jsonResponse);
                    ack_resp = arr; 
                }
                
                   
                }));
                
                
                
                
                socket->on("rest", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                    
                SInfo << "room " <<  cnfg::stringify( data[0]) <<  " - my client ID is "  <<  cnfg::stringify( data[1]) ;
                              
                json jsonRequest = data[2]; //json::parse("{\"id\":1,\"method\":\"worker.createRouter\",\"internal\":{\"routerId\":\"2e32062d-f04a-4c2d-a656-b586e50498ef\"}}");//_json;
                LTrace("arvind ", cnfg::stringify(jsonRequest))
                Channel::Request req(nullptr, jsonRequest);
                worker->OnChannelRequest(nullptr, &req);
                if (isAck)
                {   
                    json arr  =  json::array();
                    arr.push_back(req.jsonResponse);
                    ack_resp = arr; 
                }
                
                   
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

                    
               //     arvind request() {"id":2,"method":"router.createWebRtcTransport","internal":{"routerId":"2e32062d-f04a-4c2d-a656-b586e50498ef","transportId":"e5302612-283c-4532-8acb-8f3cbb87a8a5"},"data":{"listenIps":[{"ip":"127.0.0.1"}],"enableUdp":true,"enableTcp":true,"preferUdp":true,"preferTcp":false,"initialAvailableOutgoingBitrate":1000000,"enableSctp":false,"numSctpStreams":{"OS":1024,"MIS":1024},"maxSctpMessageSize":262144,"isDataChannel":true}}

                    {
                        
                    json jsonRequest=json::parse("{\"id\":1,\"method\":\"worker.createRouter\",\"internal\":{\"routerId\":\"2e32062d-f04a-4c2d-a656-b586e50498ef\"}}");//_json;
                
                     Channel::Request req(nullptr, jsonRequest);
                     worker->OnChannelRequest(nullptr, &req );
                    }
                    {
                         
                        json jsonRequest=json::parse("{\"id\":2,\"method\":\"router.createWebRtcTransport\",\"internal\":{\"routerId\":\"2e32062d-f04a-4c2d-a656-b586e50498ef\",\"transportId\":\"e5302612-283c-4532-8acb-8f3cbb87a8a5\"},\"data\":{\"listenIps\":[{\"ip\":\"127.0.0.1\"}],\"enableUdp\":true,\"enableTcp\":true,\"preferUdp\":true,\"preferTcp\":false,\"initialAvailableOutgoingBitrate\":1000000,\"enableSctp\":false,\"numSctpStreams\":{\"OS\":1024,\"MIS\":1024},\"maxSctpMessageSize\":262144,\"isDataChannel\":true}}");//_json;
                        Channel::Request req(nullptr, jsonRequest);
                        worker->OnChannelRequest(nullptr, &req );
                     
                    }
                    
                    
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

                  //  onPeerMessage(m);
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
