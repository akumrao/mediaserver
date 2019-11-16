#include "base/base.h"
#include "base/logger.h"
#include "base/application.h"
#include "net/TcpServer.h"
#include "base/test.h"
#include "base/time.h"


using std::endl;
using namespace base;
using namespace net;
using namespace base::test;



class tesTcpServer : public TcpServer::Listener, public TcpConnection::Listener {
public:

    tesTcpServer() {
    }

    void start() {
        // socket.Send("Arvind", "127.0.0.1", 7331);
        tcpServer = new TcpServer(this, this, "0.0.0.0", 7000);

    }

    void shutdown() {
        // socket.Send("Arvind", "127.0.0.1", 7331);
        delete tcpServer;
        tcpServer = nullptr;

    }

    void OnTcpConnectionClosed(TcpServer* /*tcpServer*/, TcpConnection* connection) {

        std::cout << "TCP server closing, LocalIP" << connection->GetLocalIp() << " PeerIP" << connection->GetPeerIp() << std::endl << std::flush;

    }

    void OnTcpConnectionPacketReceived(TcpConnection* connection, const uint8_t* data, size_t len) {
        std::cout << "TCP server send data: " << data << "len: " << len << std::endl << std::flush;
        std::string send = "12345";
        connection->Send((const uint8_t*) send.c_str(), 5);

    }
    TcpServer *tcpServer;

};


int main(int argc, char** argv) {
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

 

        Application app;

        tesTcpServer socket;
        socket.start();

        app.waitForShutdown([&](void*) {

            socket.shutdown();

        });



    return 0;
}
