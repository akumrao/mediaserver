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
        // socket.Send("Arvind", "127.0.0.1", 7331);
        udpServer = new UdpServer( this, IP, port);

    }

    void send( std::string txt, std::string ip, int port )
    {
         udpServer->Send(txt, ip , port);
    }

    void shutdown() {

        delete udpServer;
        udpServer = nullptr;

    }


    void OnUdpSocketPacketReceived(UdpServer* socket, const uint8_t* data, size_t len, const struct sockaddr* remoteAddr) {

        std::cout << data << std::endl << std::flush;
    }

    UdpServer *udpServer;

    std::string IP;
    int port;

};


int main(int argc, char** argv) {
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

 
        Application app;

        testUdpServer socket("0:0:0:0", 6000);
        socket.start();
        //socket.send("arvind testing");

        app.waitForShutdown([&](void*) {

            socket.shutdown();

        });



    return 0;
}
