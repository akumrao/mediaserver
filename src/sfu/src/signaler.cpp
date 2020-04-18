
#include <iostream>
#include <string>

#include "signaler.h"

using std::endl;

namespace base {
    namespace wrtc {

        Signaler::Signaler() {
            Logger::instance().add(new ConsoleChannel("debug", Level::Trace));
            worker = new Worker(nullptr);

        }

        Signaler::~Signaler() {
        }

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


                socket->on("created", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {


                    SInfo << "Created room " << cnfg::stringify(data[0]) << " - my client ID is " << cnfg::stringify(data[1]);

                    json jsonRequest = data[2];
                    LTrace("arvind ", cnfg::stringify(jsonRequest))
                    Channel::Request req(nullptr, jsonRequest);
                    worker->OnChannelRequest(nullptr, &req);
                    if (isAck) {
                        json arr = json::array();
                                arr.push_back(req.jsonResponse);
                                ack_resp = arr;
                    }


                    isInitiator = true;
                    //grabWebCamVideo();
                }));


                socket->on("createWebRtcTransport", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {

                    SInfo << "room " << cnfg::stringify(data[0]) << " - my client ID is " << cnfg::stringify(data[1]);

                    json jsonRequest = data[2]; //json::parse("{\"id\":1,\"method\":\"worker.createRouter\",\"internal\":{\"routerId\":\"2e32062d-f04a-4c2d-a656-b586e50498ef\"}}");//_json;
                    LTrace("arvind ", cnfg::stringify(jsonRequest))
                    Channel::Request req(nullptr, jsonRequest);
                    worker->OnChannelRequest(nullptr, &req);
                    if (isAck) {
                        json arr = json::array();
                                arr.push_back(req.jsonResponse);
                                ack_resp = arr;
                    }


                }));




                socket->on("rest", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {

                    SInfo << "room " << cnfg::stringify(data[0]) << " - my client ID is " << cnfg::stringify(data[1]);

                    json jsonRequest = data[2]; //json::parse("{\"id\":1,\"method\":\"worker.createRouter\",\"internal\":{\"routerId\":\"2e32062d-f04a-4c2d-a656-b586e50498ef\"}}");//_json;
                    LTrace("arvind ", cnfg::stringify(jsonRequest))
                    Channel::Request req(nullptr, jsonRequest);
                    worker->OnChannelRequest(nullptr, &req);
                    if (isAck) {
                        json arr = json::array();
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


//
//                socket->on("ready", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
//                    LTrace(cnfg::stringify(data))
//                            // LTrace('Socket is ready');
//                            // createPeerConnection(isInitiator, configuration);
//                }));
//
//                socket->on("log", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
//                    // LTrace(cnfg::stringify(data))
//                    LTrace(cnfg::stringify(data))
//                }));
//
//                socket->on("message", Socket::event_listener_aux([&](string const& name, json const& m, bool isAck, json & ack_resp) {
//                    LTrace(cnfg::stringify(m));
//                    LTrace('SocketioClient received message:', cnfg::stringify(m));
//
//                    //  onPeerMessage(m);
//                    // signalingMessageCallback(message);
//
//
//
//                }));



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
