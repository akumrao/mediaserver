
#include "base/filesystem.h"
//#include "socketio/transaction.h"
//#include "net/sslmanager.h"
#include "base/application.h"
#include "base/util.h"
#include "base/idler.h"
#include "base/logger.h"
#include "rtc_base/ssladapter.h"
//#include "rtc_base/thread.h"


//#include "webrtc/peermanager.h"
//#include "webrtc/multiplexmediacapturer.h"


using namespace std;
using namespace base;
//using namespace base::wrtc;
//using namespace base::util;


/*
// Detect Memory Leaks
#ifdef _DEBUG
#include "MemLeakDetect/MemLeakDetect.h"
#include "MemLeakDetect/MemLeakDetect.cpp"
CMemLeakDetect memLeakDetect;
#endif
 */

#define SERVER_HOST "arvindubuntu"
#define SERVER_PORT 8080 //443
#define USE_SSL     0 //1
#define JOIN_ROOM  "foo"        
        
#include "webrtc/signaler.h"


  std::string sampleDataDir(const std::string& file)
        {
            std::string dir;
            fs::addnode(dir, base_SOURCE_DIR);
            fs::addnode(dir, "ffmpeg");
            fs::addnode(dir, "samples");
            fs::addnode(dir, "data");
            if (!file.empty())
                fs::addnode(dir, file);
            return dir;
        }
  /*
namespace base {
    namespace sockio {



      

        class Tests: public wrtc::PeerManager {
        

        public:

            Tests(): _capturer()
           , _context(_capturer.getAudioModule())
            {
                  std::string sourceFile(sampleDataDir("test.mp4"));
                 _capturer.openFile(sourceFile, true);
            }

            void run() {
                
                   LTrace("Tests signalling Begin. Please run signalling server at webrtc folder")

                client = new SocketioClient(SERVER_HOST, 8080);
                client->connect();

                socket* socket = client->io();

                socket->on("connection", socket::event_listener_aux([=](string const& name, json const& data, bool isAck, json & ack_resp) {

                    socket->on("ipaddr", socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                        LTrace(cnfg::stringify(data))

                        LTrace("Server IP address is: ", data)
                                // updateRoomURL(ipaddr);
                    }));

                    socket->on("created", socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {

                        LTrace(cnfg::stringify(data))
                        LTrace("Created room", data[0], "- my client ID is", data[1])
                        isInitiator = true;
                        //grabWebCamVideo();
                    }));

                    socket->on("full", socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                        LTrace(cnfg::stringify(data));
                        LTrace("Room " + room + " is full.")
                                // window.location.hash = '';
                                // window.location.reload();
                    }));


                    socket->on("join", socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                        LTrace(cnfg::stringify(data));
                        LTrace("Another peer made a request to join room " + room)
                        LTrace("This peer is the initiator of room " + room + "!")
                        isChannelReady = true;
                    }));

                    socket->on("joined", socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                        
                        LTrace(cnfg::stringify(data))
                        LTrace("joined: " + room)
                                
                         ////////////////////////     
                                
                        std::string peerid = data[1];
                         LTrace("Peer connected: ", peerid );

                        if (wrtc::PeerManager::exists(peerid)) {
                            LDebug("Peer already has session: ",peerid)
                            return;
                        }

                        // Create the Peer Peer
                        auto conn = new wrtc::Peer(this, &_context, peerid, "", wrtc::Peer::Offer);
                        conn->constraints().SetMandatoryReceiveAudio(false);
                        conn->constraints().SetMandatoryReceiveVideo(false);
                        conn->constraints().SetAllowDtlsSctpDataChannels();

                        // Create the media stream and attach decoder
                        // output to the peer connection
                        _capturer.addMediaTracks(_context.factory, conn->createMediaStream());

                        // Send the Offer SDP
                        conn->createConnection();
                        conn->createOffer();

                        wrtc::PeerManager::add(peerid, conn);       
                                            isChannelReady = true;
                                            
                        LTrace("offer SDP for peer : ", peerid );                  
                                       
                    }));

                                        

                    socket->on("ready", socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                        LTrace(cnfg::stringify(data))
                                // LTrace('Socket is ready');
                                // createPeerConnection(isInitiator, configuration);
                    }));

                    socket->on("log", socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                        // LTrace(cnfg::stringify(data))
                        LTrace(cnfg::stringify(data))
                    }));

                    socket->on("message", socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                        LTrace(cnfg::stringify(data));
                         LTrace('SocketioClient received message:', cnfg::stringify(data));
                        // signalingMessageCallback(message);
                        
                        
//                         if (m.find("offer") != m.end()) {
//                            assert(0 && "offer not supported");
//                        } else if (m.find("answer") != m.end()) {
//                            recvSDP(m.from().id, m["answer"]);
//                        } else if (m.find("candidate") != m.end()) {
//                            recvCandidate(m.from().id, m["candidate"]);
//                        }
                        
                        
                    }));



                    // Leaving rooms and disconnecting from peers.
                    socket->on("disconnect", socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                        LTrace(cnfg::stringify(data));
                        //LTrace(`Disconnected: ${reason}.`);
                        // sendBtn.disabled = true;
                        // snapAndSendBtn.disabled = true;
                    }));


                    socket->on("bye", socket::event_listener_aux([&](string const& name, json const& data, bool isAck, json & ack_resp) {
                        LTrace(cnfg::stringify(data));
                        //  LTrace("Peer leaving room {" "room" }.`);
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


          
            //var localStream;
            //var pc;
            // var remoteStream;
            // var turnReady;

            //var pcConfig = {
            //  'iceServers': [{
            //    'urls': 'stun:stun.l.google.com:19302'
            //  }]
            //};
            wrtc::MultiplexMediaCapturer _capturer;
            wrtc::PeerFactoryContext _context;
        };


    }
} // namespace sockio
 * 
 * */

int main(int argc, char** argv) {
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));


    // Setup WebRTC environment
    rtc::LogMessage::LogToDebug(rtc::LS_VERBOSE); // LS_VERBOSE, LS_INFO, LERROR
    // rtc::LogMessage::LogTimestamps();
    // rtc::LogMessage::LogThreads();

    rtc::InitializeSSL();

      
   Application app;
   
   std::string sourceFile(sampleDataDir("test.mp4"));

  base::wrtc::Signaler sig;
  sig.startStreaming(sourceFile, true);
  
  sig.connect(SERVER_HOST,SERVER_PORT,JOIN_ROOM );

  // test._capturer.start();
   
    auto rtcthread = rtc::Thread::Current();
    Idler rtc([rtcthread]() {
        rtcthread->ProcessMessages(3);
        LTrace(" rtcthread->ProcessMessages")
        base::sleep(1000);        
    });

    LTrace("app.run() run start")
    app.run();
    LTrace("app.run() is over")
    rtc::CleanupSSL();
    Logger::destroy();

    return 0;
}



