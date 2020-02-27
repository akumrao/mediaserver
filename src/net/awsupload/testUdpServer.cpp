#include "base/base.h"
#include "base/logger.h"
#include "base/application.h"
#include "net/UdpSocket.h"
#include "base/test.h"
#include "base/time.h"


using std::endl;
using namespace base;
using namespace net;
using namespace base::test;




class testUdpServer : public UdpServer::Listener {
public:

    testUdpServer(std::string IP, int port):IP(IP), port(port) {
    }

    void start() {
        // socket.send("Arvind", "127.0.0.1", 7331);
        udpServer = new UdpServer( this, IP, port);
        udpServer->bind();
    }

    void send( std::string txt, std::string ip, int port )
    {
         udpServer->send( (char*) txt.c_str(), txt.length() , ip , port);
    }

    void shutdown() {

        delete udpServer;
        udpServer = nullptr;

    }


    void OnUdpSocketPacketReceived(UdpServer* socket, const char* data, size_t len,  struct sockaddr* remoteAddr) {

        int family;
        
        std::string peerIp;
        uint16_t peerPort;

        IP::GetAddressInfo(
                    remoteAddr, family, peerIp, peerPort);
            
        std::cout  <<  "Received from " << peerIp << ":" << peerPort << " size:"<< len << std::endl;
        
    }

    UdpServer *udpServer;

    std::string IP;
    int port;

};


int main(int argc, char** argv) {
    Logger::instance().add(new ConsoleChannel("debug", Level::Error));

 
        Application app;
        
        int port = 51038;
        if(argc > 1)
        {
            port = atoi(argv[1]);
        }
        
        std::cout << "Udp listening at port " << port << std::endl << std::flush;
        
        testUdpServer socket("172.31.31.71", port);
        socket.start();
        
        //socket.send("arvind", "127.0.0.1" , 6000);

        app.waitForShutdown([&](void*) {

            socket.shutdown();

        });



    return 0;
}
