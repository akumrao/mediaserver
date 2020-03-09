#include "base/base.h"
#include "base/logger.h"
#include "base/application.h"
#include "net/TcpServer.h"
#include "base/time.h"
#include "net/netInterface.h"
#include "tcpUpload.h"
#include "awsUdpServer.h"
#include "awsS3upload.h"

using std::endl;
using namespace base;
using namespace net;


class awsTcpServer : public Listener {
public:

    awsTcpServer() {
    }

    void start(std::string ip, int port) {
        // socket.send("Arvind", "127.0.0.1", 7331);
        tcpServer = new TcpServer(this, ip, port);
        m_ip = ip;
    }

    void shutdown() {
        // socket.send("Arvind", "127.0.0.1", 7331);
        delete tcpServer;
        tcpServer = nullptr;
    }

    void on_close(Listener* connection) {
        std::cout << "TCP server closing, LocalIP" << connection->GetLocalIp() << " PeerIP" << connection->GetPeerIp() << std::endl << std::flush;
    }

    
    void on_read(Listener* connection, const char* data, size_t len) {
       // STrace << "TCP server on_read: " << data << "len: " << len;
        //connection->send((const char*) send.c_str(), 5);
        
        if (len != sizeof (struct TcpPacket)) {
            LError("Fatal error: Some part of packet lost. ")
            return;
        }

        TcpPacket packet;
        memcpy(&packet, (void*) data, len);

        switch (packet.type) {
            case 0:
            {
               
                STrace << "TCP Packet type " <<  (int) packet.type << " Request for UDP port allocation: " ;

                uint16_t port = 46001;

                int portmangersize = udpPortManager.size();
                if (portmangersize) {
                    port = (--udpPortManager.end())->first + 1;
                    // todo circular port allocation
                }
                
                awsUdpServer  *socket  = new awsUdpServer((TcpConnection*)connection, m_ip, port);
                socket->start();
                udpPortManager[port]= socket;
               
                socket->sendTcpPacket( (TcpConnection*)connection, 1, port);
              
                break;
            }
//            case 2:
//            {
//                
//                LTrace("First TCP Packet received")
//                STrace << "TCP Received type " << (int) packet.type << " payload:" << packet.sequence_number;
//                break;
//            }
//
//            case 3:
//            {
//                LTrace("First TCP Packet received")
//                STrace << "TCP Received type " << (int) packet.type << " payload:" << packet.sequence_number;
//                break;
//                break;
//            }
            default:
            {
                LError("Fatal TCP: Not a valid state")
            }

        };


    }
    TcpServer *tcpServer;

    std::map<uint16_t, awsUdpServer* > udpPortManager;
    std::string m_ip;

};

int main(int argc, char** argv) {
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

    awsInit();
    //TCP PORT RANGE 47000-47999
    //UDP PORT RANGE 46000-46999

    int port = 47001;

    std::string ip = "0.0.0.0";

    if (argc > 1) {
        ip = argv[1];
    }

    if (argc > 2) {
        port = atoi(argv[2]);
    }


    Application app;

    awsTcpServer socket;
    socket.start(ip, port);



    app.waitForShutdown([&](void*) {

        socket.shutdown();
        awsExit();


    });




    return 0;
}
