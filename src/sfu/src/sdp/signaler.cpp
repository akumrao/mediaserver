
#include <iostream>
#include <string>

#include "sdp/signaler.h"
#include "Settings.h"
#include "sdp/Room.h"

using std::endl;

namespace SdpParse {

        Signaler::Signaler() {
            Logger::instance().add(new ConsoleChannel("debug", Level::Trace));
            
           rooms =  new Rooms(this);

        }

        Signaler::~Signaler() {
            if(worker)
            delete worker;
            
            delete rooms;
        }

        void Signaler::postAppMessage(const json& m) {

            LTrace("postAppMessage", cnfg::stringify(m));
            socket->emit("postAppMessage", m);
        }
        ////////////////////////////////////////////////////////////////////

        void Signaler::sendSDP(const std::string& type,  const std::string& sdp, std::string & remotePeerID) {
            assert(type == "offer" || type == "answer");
            //smpl::Message m;
            json desc;
            desc["type"] = type;
            desc["sdp"] = sdp;

            json m;

            m["type"] = type;
            m["desc"] = desc;
            m["from"] = sfuID;
            m["to"] = remotePeerID;


            postMessage(m);
        }

        void Signaler::postMessage(const json& m) {

            LTrace("postMessage", cnfg::stringify(m));
            socket->emit("message", m);
        }

        
        void Signaler::request(string const& name, json const& data, bool isAck, json & ack_resp) {

            SInfo << name << ":" << data[0].get<std::string>()  << " for  " << data[1].get<std::string>();

            json jsonRequest = data[2];
            jsonRequest["id"] = reqId++;
            LInfo("arvind ", cnfg::stringify(jsonRequest))
            Channel::Request req(jsonRequest);
            worker->OnChannelRequest(&req);
            if (isAck) {
                ack_resp = json::array();
                ack_resp.push_back(req.jsonResponse);
               // ack_resp = arr;
                SInfo<< "ack" << ack_resp.dump(4); //
            }

        }



        void Signaler::onPeerMessage(std::string &name , json const& m) {


            std::string from;
            std::string type;
            std::string room;

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
            

            SInfo << "On Peer message for room:" << room << " from: " << from << " type: " << type ;
            
  

            if (std::string("offer") == type) {
                //assert(0 && "offer not supported");

                //std::string remotePeerID = from;
                //onffer(room, from, m["desc"]);
                rooms->on_producer_offer( room,  from, m["desc"] );

            } else if (std::string("answer") == type) {
               // recvSDP(from, m["desc"]);
                rooms->on_consumer_answer( room,  from, m["desc"] );
            } else if (std::string("candidate") == type) {
                recvCandidate(from, m["candidate"]);
            } else if (std::string("bye") == type) {
                rooms->onDisconnect( room, from);
            } else if (std::string("subscribe") == type) {
                json peerIds;
                if(m.find("desc") != m.end())
                {
                  peerIds =m["desc"];
                }
                SInfo << " PeerIds " << peerIds.dump(4); 
                rooms->onSubscribe(room, from, peerIds);
            } else if (std::string("producer_getStats") == type) {
                json stats = json::array();
                rooms->producer_getStats(room, from, stats );
                json m;
                m["type"] = "prodstats";
                m["desc"] = stats;
                m["from"] = sfuID;
                m["to"] = from;
                postAppMessage(m);
            } else if (std::string("rtpObserver_addProducer") == type) {
                rooms->rtpObserver_addProducer(room, from , true);
            } else if (std::string("rtpObserver_removeProducer") == type) {
                rooms->rtpObserver_addProducer(room, from , false);
            } else if (std::string("consumer_getStats") == type) {
                json stats = json::array();
                rooms->consumer_getStats(room, from, stats );
                json m;
                m["type"] = "constats";
                m["desc"] = stats;
                m["from"] = sfuID;
                m["to"] = from;
                postAppMessage(m);
                
            } 
            else if (std::string("setPreferredLayers") == type) {
                json layer = json::array();
                rooms->setPreferredLayers(room, from, layer );
            } 
            else if (std::string("subscribe-resume") == type) {
              // rooms->resume(this , producer->producer,false);
            }
            else if (std::string("subscribe-pause") == type) {
              //consumer->resume(this , producer->producer, true);
            }

        }



        void Signaler::recvCandidate(const std::string& token, const json& data) {
            SDebug << "recvCandidate " << token << "  " << data;

        }


        //////////////////////////////////////////////////////////////////////////////////////

        void Signaler::connect(const std::string& host, const uint16_t port) {

            worker = new Worker();

           // room = rm;

            LTrace("Tests signalling Begin. Please run signalling server at webrtc folder")

            client = new SocketioClient(host, port, true);
            client->connect();

            socket = client->io();

            socket->on("connection", Socket::event_listener_aux([ & ](string const& name, json const& data, bool isAck, json & ack_resp) {

                    socket->on("created", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp){
                    
                    sfuID = data[1].get<std::string>();   
                    
                    if(data.size() > 2) // for ORTC
                    {
                        SInfo << "Created room " << cnfg::stringify(data[0]) << " - my client ID is " << cnfg::stringify(data[1]);
                        request(name, data, isAck, ack_resp);
                    }
                    else // webrtc
                    {
                        rooms->createRoom(name, data, isAck, ack_resp);
                    }
                     isInitiator = true;
                }));

      /// for ORTC messages    
                socket->on("rest", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {

                    //SInfo << "room " << cnfg::stringify(data[0]) << " - my client ID is " << cnfg::stringify(data[1]);
                    request(name, data, isAck, ack_resp);

                }));



                socket->on("full", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                    LTrace(cnfg::stringify(data));
                    //LTrace("Room " + room + " is full.")

                }));


                socket->on("join", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                    LTrace(cnfg::stringify(data));
                   // LTrace("Another peer made a request to join room " + room)
                    //LTrace("This peer is the initiator of room " + room + "!")
                    isChannelReady = true;

                }));

   /// for webrtc messages
                socket->on("message", Socket::event_listener_aux([&](string const& name, json const& m, bool isAck, json & ack_resp) {
                    //  LTrace(cnfg::stringify(m));
                    // LTrace('SocketioClient received message:', cnfg::stringify(m));

                    onPeerMessage((string &)name, m);
                    // signalingMessageCallback(message);


                }));



                // Leaving rooms and disconnecting from peers.
                socket->on("disconnectClient", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                    
                    
                    std::string roomName = data[0].get<std::string>();
                    std::string participant = data[1].get<std::string>();
                    rooms->onDisconnect( roomName, participant);
                    
                }));


                socket->on("bye", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                    LTrace(cnfg::stringify(data));
                 //  LTrace("Peer leaving room", room);

                }));

                    socket->emit("CreateSFU");
            }));

        }


} // namespace SdpParse 
