
#include <iostream>
#include <string>
#include <uv.h>

#include "base/logger.h"

#include "Signaler.h"

#ifndef SHAREDLIB
#include "../examples/unityplugin/unity_plugin_apis.h"
#endif


using std::endl;

namespace base {
namespace SdpParse {

        Signaler::Signaler(const std::string ip, const uint16_t port, const std::string roomid ) : m_IP(ip), m_port(port),shuttingDown(false)
        {
            room = roomid;
           // Logger::instance().add(new ConsoleChannel("debug", Level::Trace));
            Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

        }

        Signaler::~Signaler() {
            LTrace("~Signaler()");

            if(client) {
                delete client;
                client = nullptr;
            }

        }

        static void async_cb_stop(uv_async_t* handle) {

            LTrace(" Upload::async_cb_stop")

            Signaler *p = ( Signaler *) handle->data;

            p->client->close(0, "terminate");

            uv_close((uv_handle_t*)&p->async, nullptr);

            p->stop();

            SInfo << "Upload::async_cb_stop over" ;

        }
        
        void Signaler::shutdown() {
    
            std::lock_guard<std::mutex> guard(g_shutdown_mutex);
    
            if(!shuttingDown )
            {
                shuttingDown =true;
                
                if(async.loop)
                int  r = uv_async_send(&async);

            }

        

            STrace << "shutdown over";
        }


        
        void Signaler::run() {

            SInfo << "run";

            Application app;

            connect(m_IP, m_port);

            async.data = this;
            int r = uv_async_init(app.uvGetLoop(), &async, async_cb_stop);

            app.run();
          
            SInfo << "run over";

        }

//        void Signaler::postAppMessage(const json& m) {
//
//            LTrace("postAppMessage", cnfg::stringify(m));
//            socket->emit("postAppMessage", m);
//        }
        ////////////////////////////////////////////////////////////////////

        void Signaler::sendSDP(const std::string& type,  const std::string& sdp, const std::string & remotePeerID  ) {
            assert(type == "offer" || type == "answer");
            //smpl::Message m;
            json desc;
            desc["type"] = type;
            desc["sdp"] = sdp;

            json m;

            m["type"] = type;
            m["desc"] = desc;
            if(!remotePeerID.empty())
            m["to"] = remotePeerID;
            m["room"] = room;

            postMessage(m);
        }

        void Signaler::sendICE( const std::string& candidate, const int sdpMLineIndex, const std::string& sdpMid, const std::string & remotePeerID, const std::string & from  ) {

           // STrace << "About to send  ICE";

            json desc;
            desc["candidate"] = candidate;
            desc["sdpMLineIndex"] = sdpMLineIndex;
            desc["sdpMid"] = sdpMid;
            desc["type"] = "candidate";
            desc["to"] = remotePeerID;
            desc["room"] = room;

            postMessage(desc);
        }

        void Signaler::postMessage(const json& m) {

            SInfo <<  cnfg::stringify(m);
            socket->emit("sfu-message", m);
        }

        
          void Signaler::onPeerMessage(std::string &name , json const& m) {

             
            SInfo <<  "onMessage "  <<  cnfg::stringify(m);
             

            std::string from;
            std::string type;
            std::string room;
            std::string to;
            
            
            if (m.find("to") != m.end()) {
                to = m["to"].get<std::string>();
            }
            

            if (m.find("from") != m.end()) {
                from = m["from"].get<std::string>();
            }
            else
            {
                //SError << " On Peer message is missing participant id ";
               // return;
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
//            else
//            {
//                SError << " On Peer message is missing room id ";
//                return;
//            }
            


            if (std::string("offer") == type) {




               std::string sSdp  =  m["desc"]["sdp"].get<std::string>();

              // SInfo <<  "onoffer "  << sSdp ;

              //  CreatePeerConnection(m_IP.c_str(),m_port,room.c_str(), nullptr, 0,
              //                                         nullptr,nullptr );

                SInfo <<  "onoffer  "  ;
                OnUnityMessage("log","onoffer");

                //RegisterOnRemoteI420FrameReady( 2, nullptr);

                #ifndef SHAREDLIB
                SetRemoteDescription(2, "offer", sSdp.c_str());

                CreateAnswer_cb(2,  [ to, from, this ]( string type, string sdp ) {

                    SInfo <<  "Sending " << type    << " remote " <<  from  << " local  " << to ;

                    sendSDP( type, sdp,   from  );

                    }

                );

                #else

                 if (OnLocalSdpReady)
                 {
                    OnUnityMessage("log",sSdp.c_str());
                    OnLocalSdpReady("offer", sSdp.c_str());
                  }

                #endif

             //  ++rounRobClientID;


//                OnIce(1,
//                [ to, from , this]( const std::string& candidate, const int sdp_mline_index, const std::string& sdp_mid ) {
//
//                    //SInfo <<  candidate  << " " <<  sdp_mline_index << " " << sdp_mid << " remote " <<  from  << " local  " << to ;
//
//                    sendICE( candidate, sdp_mline_index, sdp_mid,  from,  to  );
//
//                }
//                );

            } else if (std::string("answer") == type) {

                SInfo <<  "onanswer " ;

                OnUnityMessage("log", "onanswer");

               // recvSDP(from, m["desc"]);
                //rooms->on_consumer_answer( room, from, to, m["desc"] );
                std::string sSdp  =  m["desc"]["sdp"].get<std::string>();

                #ifndef SHAREDLIB
                SetRemoteDescription(1, "answer", sSdp.c_str());
                
                #else

                 if (OnLocalSdpReady)
                 {
                     SInfo <<  "onanswer1 " <<  sSdp;

                     OnUnityMessage("log",sSdp.c_str());

                    OnLocalSdpReady("answer", sSdp.c_str());
                 }

                #endif

                subscribe();
//                OnIce(1,
//                        [ to, from , this]( const std::string& candidate, const int sdp_mline_index, const std::string& sdp_mid ) {
//
//                            //SInfo <<  candidate  << " " <<  sdp_mline_index << " " << sdp_mid << " remote " <<  from  << " local  " << to ;
//
//                            sendICE( candidate, sdp_mline_index, sdp_mid,  from,  to  );
//
//                        }
//                );

            } else if (std::string("candidate") == type) {
               // recvCandidate(from, m);
            } else if (std::string("bye") == type) {
               // rooms->onDisconnect( from);
            } else if (type == std::string("soundlevel")) {
                 OnUnityMessage(type,cnfg::stringify(m["desc"]));
            } else if (type == std::string("score")) {
                OnUnityMessage(type,cnfg::stringify(m["desc"]));
            } else if (type == std::string("bwe")) {
                OnUnityMessage(type,cnfg::stringify(m["desc"]));
            }



        }

        void Signaler::OnUnityMessage(std::string type, std::string msg)
        {
           if( OnMessage)
                OnMessage(type.c_str(), msg.c_str());
        }   

        void Signaler::subscribe() {
             json desc;
             desc["type"] = "subscribe";
             desc["room"] = room;

             postMessage(desc);
        }

//        void Signaler::recvCandidate(const std::string& from, const json& data) {
//
//            SInfo << "recvCandidate " << from << "  " << data["candidate"].get<std::string>() <<  " " << data["sdpMLineIndex"].get<int>()  << " " << data["sdpMid"].get<std::string>();
//
//            std::string candidate = data["candidate"].get<std::string>();
//            const int sdpMLineIndex= data["sdpMLineIndex"].get<int>();
//            const char* sdpMid= data["sdpMid"].get<std::string>().c_str();
//
//            AddIceCandidate( 1, candidate.c_str(), sdpMLineIndex, sdpMid);
//
//        }

        void Signaler::RegisterOnLocalSdpReadytoSend(LOCALSDPREADYTOSEND_CALLBACK callback) {
            SInfo <<  "RegisterOnLocalSdpReadytoSend" ;
            OnLocalSdpReady  = callback;
      
        }

        void Signaler::RegisterOnMessage(MESSAGE_CALLBACK callback) {
          OnMessage = callback;
        }


        //////////////////////////////////////////////////////////////////////////////////////

        void Signaler::connect(const std::string& host, const uint16_t port) {

   
            client = new SocketioClient(host, port, true);
            client->connect();

            socket = client->io();

            socket->on("connection", Socket::event_listener_aux([ & ](string const& name, json const& data, bool isAck, json & ack_resp) {

                    socket->on("created", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp){
                    

                    if(data.size() > 2) // for ORTC
                    {
                        SInfo << "Created room " << cnfg::stringify(data[0]) << " - my client ID is " << cnfg::stringify(data[1]);
     
                    }
                    else // webrtc
                    {
                        //rooms->createRoom(name, data, isAck, ack_resp);
                    }

                }));




                socket->on("full", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                    LTrace(cnfg::stringify(data));
                    //LTrace("Room " + room + " is full.")
                        SInfo <<  "Room "  << room  << " is full";
                }));


                socket->on("join", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                    LTrace(cnfg::stringify(data));
                     LTrace("Another peer made a request to join room " + room)
                    LTrace("This peer is the initiator of room " + room + "!")
                    isChannelReady = true;
                    
                    OnUnityMessage("numClients",cnfg::stringify(data[2]));


                }));
                
                socket->on("joined", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                    LTrace(cnfg::stringify(data));
                    LTrace("joined: " + room);
                    isChannelReady = true;
                    isInitiator = true;
                    OnUnityMessage("numClients",cnfg::stringify(data[2]));
                   // maybeStart();
                    subscribe();

                }));

                 socket->on("leave", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                    LTrace(cnfg::stringify(data));
                    LTrace("leave: " + room);
                    
                    int numClients = data[2].get<int>();

                    if(  numClients == -1 )
                    {
                        OnUnityMessage("log","SFU server is down.");
                    }
                    else
                    {
                        OnUnityMessage("numClients",cnfg::stringify(data[2]));
                    }


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
                    std::string participant = data;
                   // rooms->onDisconnect( participant);
                    
                }));


                socket->on("bye", Socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                    LTrace(cnfg::stringify(data));
                 //  LTrace("Peer leaving room", room);

                }));
                   
                    socket->emit("create or join", room);
            }));

        }


} // namespace SdpParse 
}
