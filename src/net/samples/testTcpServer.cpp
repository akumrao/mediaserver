#include "base/base.h"
#include "base/logger.h"
#include "base/application.h"
#include "net/TcpServer.h"
#include "base/test.h"
#include "base/time.h"
#include "net/netInterface.h"

using std::endl;
using namespace base;
using namespace net;
using namespace base::test;



class tesTcpServer : public Listener {
public:

    tesTcpServer() {
    }

    void start(std::string ip, int port) {
        // socket.send("Arvind", "127.0.0.1", 7331);
        tcpServer = new TcpServer(this, ip, port);

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
        std::cout << "TCP server send data: " << data << "len: " << len << std::endl << std::flush;
        std::string send = "12345";
        connection->send((const char*) send.c_str(), 5);

    }
    TcpServer *tcpServer;

};


int main(int argc, char** argv) {
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

        int port = 51038;
        
        std::string ip = "0.0.0.0";
        
        if (argc > 1) {
            ip = argv[1];
        }
        
        if(argc > 2)
        {
            port = atoi(argv[2]);
        }

        Application app;

        tesTcpServer socket;
        socket.start(ip, port);

        app.waitForShutdown([&](void*) {

            socket.shutdown();

        });



    return 0;
}
