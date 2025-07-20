
#include <iostream>
#include <string>

#include "webrtc/audiopacketmodule.h"
#include "webrtc/multiplexmediacapturer.h"
#include "webrtc/videopacketsource.h"
#include "webrtc/signaler.h"
#include "Settings.h"


using std::endl;
// using namespace base::sockio;
namespace base
{
namespace web_rtc
{

Signaler::Signaler() : _context(_capturer.getAudioModule()) {}

Signaler::~Signaler()
{
    SInfo << "~Signaler() ";
}

void Signaler::startStreaming(
    const std::string &dir, const std::string &file, const std::string &type, bool looping)
{
    // Open the video file

    //SInfo << "Open  " << dir << "/" << file;

    _capturer.openFile(dir, file, type, looping);
    //  _capturer.start();  // enable for rapid testing
}

void Signaler::sendSDP(web_rtc::Peer *conn, const std::string &type, const std::string &sdp)
{
    assert(type == "offer" || type == "answer");
    // smpl::Message m;
    json desc;
    desc[web_rtc::kSessionDescriptionTypeName] = type;
    desc[web_rtc::kSessionDescriptionSdpName] = sdp;

    json m;

    m[web_rtc::kSessionDescriptionTypeName] = type;
    m["desc"] = desc;
    m["from"] = conn->peerid();
    m["to"] = conn->peerid();
    // smpl::Message m({ type, {
    //     { web_rtc::kSessionDescriptionTypeName, type },
    //     { web_rtc::kSessionDescriptionSdpName, sdp} }
    // });

    postMessage(m);
}

void Signaler::sendCandidate(
    web_rtc::Peer *conn, const std::string &mid, int mlineindex, const std::string &sdp)
{
    // smpl::Message m;
    json desc;
    desc[web_rtc::kCandidateSdpMidName] = mid;
    desc[web_rtc::kCandidateSdpMlineIndexName] = mlineindex;
    desc[web_rtc::kCandidateSdpName] = sdp;

    json m;
    m[web_rtc::kSessionDescriptionTypeName] = "candidate";
    m["candidate"] = desc;
    m["from"] = conn->peerid();
    m["to"] = conn->peerid();

    // smpl::Message m({ "candidate", {
    //     { web_rtc::kCandidateSdpMidName, mid },
    //     { web_rtc::kCandidateSdpMlineIndexName, mlineindex},
    //     { web_rtc::kCandidateSdpName, sdp} }
    // });

    LTrace("send candidate ", cnfg::stringify(m));
    postMessage(m);
}

void Signaler::onPeerOffer(std::string &peerID, st_track &trackInfo, std::string &room)
{
    if (!web_rtc::PeerManager::exists(peerID))
    {
        auto conn = new web_rtc::Peer(this, &_context, room, peerID, web_rtc::Peer::Offer);
        conn->createConnection();
        web_rtc::PeerManager::add(peerID, conn);

        _capturer.addMediaTracks(_context.factory, conn->_peerConnection, conn, trackInfo, room);

        conn->createOffer(true, false);
    }

    else
    {
        auto conn = web_rtc::PeerManager::get(peerID);
        if (conn)
        {
            _capturer.addMediaTracks(_context.factory, conn->_peerConnection, conn, trackInfo, room);

            conn->createOffer(true, false);
        }
    }
}


//        void Signaler::onPeerConnected(std::string& peerID ,  st_track &trackInfo ,  std::string &room ) {
//
//            LDebug("Peer connected: ", peerID)
//
//            if (web_rtc::PeerManager::exists(peerID)) {
//                LDebug("Peer already has session: ", peerID)
//                return;
//            }
//
//            // Create the Peer Peer
//            auto conn = new web_rtc::Peer(this, &_context, cam, room, peerID, "", web_rtc::Peer::Offer);
//           // conn->constraints().SetMandatoryReceiveAudio(false);
//           // conn->constraints().SetMandatoryReceiveVideo(false);
//           // conn->constraints().SetAllowDtlsSctpDataChannels();
//
//            // Create the media stream and attach decoder
//            // output to the peer connection
//            conn->createConnection();
//            _capturer.addMediaTracks(_context.factory, conn->_peerConnection , conn);
//
//            // Send the Offer SDP
//
//            conn->createOffer();
//
//            web_rtc::PeerManager::add(peerID, conn);
//        }

void Signaler::onPeerMessage(std::string &name, json const &m)
{
    if (std::string("got user media") == m) { return; }

    std::string from;
    std::string type;
    std::string room;
    std::string to;
    std::string user;

    st_track camT;
    camT.signaler = this;
    if (m.find("to") != m.end()) { to = m["to"].get<std::string>(); }

    if (m.find("from") != m.end())
    {
        from = m["from"].get<std::string>();
        camT.from = from;
    }
    else
    {
        SError << " On Peer message is missing participant id ";
        return;
    }

    if (m.find("type") != m.end()) { type = m["type"].get<std::string>(); }
    else
    {
        SError << " On Peer message is missing SDP type";
    }

    if (m.find("room") != m.end())
    {
        room = m["room"].get<std::string>();
        camT.room = room;
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
        camT.camid = m["cam"].get<std::string>();
        std::string add;


        if( !Settings::getNodeState(camT.camid, "rtsp" , add ))
        {
             {
                 postAppMessage("Camera not available.", from , room  );
                 return;
             }
        }
    }


    if (m.find("start") != m.end())
    {
        camT.start = m["start"].get<std::string>();

        //                std::string add;
        //               if( !Settings::getNodeState(camT, "rtsp" , add ))
        //               {
        //                    {
        //                        postAppMessage("Camera not available.", from , room  );
        //                        return;
        //                    }
        //                }
    }


    if (m.find("end") != m.end()) { camT.end = m["end"].get<std::string>(); }

    if (m.find("width") != m.end()) { camT.width = std::stoi(m["width"].get<std::string>()); }

    if (m.find("height") != m.end()) { camT.height = std::stoi(m["height"].get<std::string>()); }

    if (m.find("scale") != m.end()) { camT.scale = std::stoi(m["scale"].get<std::string>()); }

    if (m.find("speed") != m.end()) { camT.speed = std::stoi(m["speed"].get<std::string>()); }

    if (m.find("ai") != m.end()) { camT.ai = m["ai"].get<bool>(); }
    if (m.find("encoder") != m.end())
    {
        Settings::encSetting.nvidiaEnc = 0;
        Settings::encSetting.vp9Enc = 0;
        Settings::encSetting.native = 0;
        Settings::encSetting.x264Enc = 0;
        Settings::encSetting.quicksyncEnc = 0;
        Settings::encSetting.VAAPIEnc = 0;
        camT.encoder = m["encoder"].get<std::string>();

       // if (!camT.isLive() && camT.encoder == "NATIVE") { camT.encoder = "X264"; }


        if (camT.encoder == "VP9")
        {
            camT.encType = EN_VP9;
            Settings::encSetting.vp9Enc = Settings::configuration.vp9Enc;
        }
        else if (camT.encoder == "X264")
        {
            camT.encType = EN_X264;
            Settings::encSetting.x264Enc = Settings::configuration.x264Enc;
        }
        else if (camT.encoder == "NVIDIA")
        {
            camT.encType = EN_NVIDIA;
            Settings::encSetting.nvidiaEnc = Settings::configuration.nvidiaEnc;
        }
        else if (camT.encoder == "NATIVE")
        {
            camT.encType = EN_NATIVE;
            Settings::encSetting.native = Settings::configuration.native;
        }
        else if (camT.encoder == "QUICKSYNC")
        {
            camT.encType = EN_QUICKSYNC;
            if (Settings::configuration.haswell)
            {
                Settings::encSetting.VAAPIEnc = Settings::configuration.VAAPIEnc;
            }
            else
            {
                Settings::encSetting.quicksyncEnc = Settings::configuration.quicksyncEnc;
            }
        }
    }

    LInfo("Peer message: ", from, " ", type)

        if (std::string("offer") == type)
    {
        onPeerOffer(from, camT, room);
    }
    else if (std::string("answer") == type) { recvSDP(from, m["desc"]); }
    else if (std::string("candidate") == type) { recvCandidate(from, m["candidate"]); }

    else if (std::string("bye") == type) { onPeerDiconnected(from); }
    else if (std::string("command") == type)
    {
        if (m.find("desc") != m.end())
        {
            std::string cmd = m["desc"].get<std::string>();

            // std::string trackid = m["para"].get<std::string>();

            onPeerCommand(from, camT, cmd, m["trackids"], m["act"]);
        }
    }
}

void Signaler::onPeerDiconnected(std::string &peerID)
{
    SInfo << "onPeerDiconnected " << peerID;

    auto conn = web_rtc::PeerManager::remove(peerID);
    if (conn)
    {
        // web_rtc::PeerManager::onClosed(conn);

        _capturer.remove(conn);
        LInfo("Deleting peer connection: ", peerID)

            //  conn->Release();
            // async delete not essential, but to be safe
            delete conn;
    }
}

void Signaler::onPeerCommand(
    std::string &peerID, st_track &camT, std::string &cmd, const json &trackids, const json &action)
{
    SInfo << "onPeerDiconnected on done yet " << peerID;

    auto conn = web_rtc::PeerManager::get(peerID);
    if (conn)
    {
        bool act = action.get<bool>();

        for (auto &trac : trackids)
        {
            std::string tc = trac.get<std::string>();
            std::cout << tc << '\n';

            if (cmd == "close") {/* _capturer.remove(conn, tc);*/ }
            else if (cmd == "mute")
            {
                _capturer.mute(conn, tc, cmd, act);
            }
            else if (cmd == "muteaudio")
            {
                _capturer.mute(conn, tc, cmd, act);
            }
            else if (cmd == "apply")
            {
                conn->mapcam[tc].width = camT.width;
                conn->mapcam[tc].height = camT.height;
                conn->mapcam[tc].speed = camT.speed;
                conn->mapcam[tc].scale = camT.scale;

                _capturer.oncommand(conn, tc, cmd, 0, 0);
            }
        }

        // web_rtc::PeerManager::onClosed(conn);

        //_capturer.remove(conn);
        // LInfo("Deleting peer connection: ", peerID)

        //  conn->Release();
        // async delete not essential, but to be safe
        //  delete conn;
    }
}


void Signaler::onAddRemoteStream(web_rtc::Peer *conn, webrtc::MediaStreamInterface *stream)
{
    // assert(0 && "not required");
}

void Signaler::onRemoveRemoteStream(web_rtc::Peer *conn, webrtc::MediaStreamInterface *stream)
{
    assert(0 && "not required");
}

void Signaler::onStable(web_rtc::Peer *conn)
{
    // SInfo << "Start Capture cam "  <<  conn->getCam();
    //_capturer.start( conn->getCam());
}

void Signaler::onClosed(web_rtc::Peer *conn)
{
    SInfo << "Stop Capture cam ";
    _capturer.remove(conn);
    web_rtc::PeerManager::onClosed(conn);
}



    void Signaler::postcloseCamera(std::string &cam ,  std::string  reason )
    {
        SInfo << "close Camera "  << cam ;
                 
        std::set< std::string>  peeerids;

         _capturer.stop(cam , peeerids);
         //_capturer.getPeerids(cam , peeerids);// when you do not want to close the websocket. For example
        // keyboard event, metadata etc
         std::string room(JOIN_ROOM); // Arvind: hard coded room, soon we will
        // remove it 
        for( std::string from : peeerids   )
        {
            
            json m;
            m["type"] = "error";
            // m["type"] = "ai"; // when you do not want to close the websocket. For example keyboard event, metadata
            // etc
            m["desc"] = reason;
            m["to"] = from;
            m["room"] = room;
            m["cam"] = cam;
            SInfo << "postcloseCamera " << cnfg::stringify(m);
            socket->emit("postAppMessage", m);
        }
    }


//       void Signaler::closeCamera(std::string &cam ,  std::string  reason )
//       {
//            SInfo << "Remove cam "  << cam;
//            std::set< std::string>  peeerids;
//
//            _capturer.stop(cam , peeerids);
//
//            std::string room("foo"); // Arvind: hard coded room, soon we will remove it
//            for( std::string from : peeerids   )
//            {
//                postAppMessage(reason, from, room );
//                onPeerDiconnected( from);
//            }
//
//       }

void Signaler::onFailure(web_rtc::Peer *conn, const std::string &error)
{
    SInfo << "onFailure stop FFMPEG Capture";
    _capturer.remove(conn);
    web_rtc::PeerManager::onFailure(conn, error);
}

void Signaler::postMessage(const json &m)
{
    SDebug << "postMessage " << cnfg::stringify(m);

    socket->emit("message", m);
}

void Signaler::postAppMessage(std::string message, std::string from, std::string &room)
{
    // LTrace("postAppMessage", cnfg::stringify(m));

    json m;
    m["type"] = "error";
    // m["type"] = "ai"; // when you do not want to close the websocket. For example keyboard event, metadata
    // etc
    m["desc"] = message;
    m["to"] = from;
    m["room"] = room;

    SInfo << "postMessage " << cnfg::stringify(m);

    socket->emit("postAppMessage", m);
}


void Signaler::connect(const std::string &host, const uint16_t port, const std::string rm)
{
    LTrace("Tests signalling Begin. Please run signalling server at webrtc folder")

        client
        = new sockio::SocketioClient(host, port, true);
    client->connect();

    socket = client->io();

    socket->on(
        "connection",
        sockio::Socket::event_listener_aux(
            [=](string const &name, json const &data, bool isAck, json &ack_resp)
            {
                socket->on(
                    "ipaddr",
                    sockio::Socket::event_listener_aux(
                        [&](string const &name, json const &data, bool isAck, json &ack_resp)
                        {
                            STrace << cnfg::stringify(data);

                            STrace << "Server IP address is: " << data;
                            // updateRoomURL(ipaddr);
                        }));

                socket->on(
                    "created",
                    sockio::Socket::event_listener_aux(
                        [&](string const &name, json const &data, bool isAck, json &ack_resp)
                        {
                            STrace << cnfg::stringify(data);
                            STrace << "Created room " << data[0] << "- my client ID is " << data[1];
                            isInitiator = true;
                            // grabWebCamVideo();
                        }));

                socket->on(
                    "full",
                    sockio::Socket::event_listener_aux(
                        [&](string const &name, json const &data, bool isAck, json &ack_resp)
                        {
                            STrace << cnfg::stringify(data);
                            // LTrace("Room " + room + " is full.")
                        }));


                socket->on(
                    "join",
                    sockio::Socket::event_listener_aux(
                        [&](string const &name, json const &data, bool isAck, json &ack_resp)
                        {
                            STrace << cnfg::stringify(data);
                            // LTrace("Another peer made a request to join room " + room)
                            // LTrace("This peer is the initiator of room " + room + "!")
                            isChannelReady = true;
                        }));

                /// for webrtc messages
                socket->on(
                    "message",
                    sockio::Socket::event_listener_aux(
                        [&](string const &name, json const &m, bool isAck, json &ack_resp)
                        {
                            //  LTrace(cnfg::stringify(m));
                            // LTrace('SocketioClient received message:', cnfg::stringify(m));

                            onPeerMessage((string &) name, m);
                            // signalingMessageCallback(message);
                        }));


                // Leaving rooms and disconnecting from peers.
                socket->on(
                    "disconnectClient",
                    sockio::Socket::event_listener_aux(
                        [&](string const &name, json const &data, bool isAck, json &ack_resp)
                        {
                            std::string from = data.get<std::string>();
                            // SInfo << "disconnectClient " <<  from;
                            // LInfo(cnfg::stringify(data));
                            onPeerDiconnected(from);
                        }));


                socket->on(
                    "bye",
                    sockio::Socket::event_listener_aux(
                        [&](string const &name, json const &data, bool isAck, json &ack_resp)
                        {
                            STrace << cnfg::stringify(data);
                            // LTrace("Peer leaving room", room);
                        }));

                socket->emit("WebrtcSocket");
            }));
}


}  // namespace web_rtc
}  // namespace base
