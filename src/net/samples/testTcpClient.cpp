#include "base/base.h"
#include "base/logger.h"
#include "base/application.h"

#include "net/TcpConnection.h""
#include "base/test.h"
#include "base/time.h"


using std::endl;
using namespace base;
using namespace net;
using namespace base::test;


class tesTcpClient : public TcpConnection::Listener {
public:

    tesTcpClient() {
    }

    void start() {

        // socket.Send("Arvind", "127.0.0.1", 7331);
        tcpClient = new TcpConnection(this);

        tcpClient->Connect("0.0.0.0", 7000);
        const unsigned char snd[6] = "12345";
        std::cout << "TCP Client send data: " << snd << "len: " << strlen((const char*) snd) << std::endl << std::flush;

        tcpClient->Send(snd, 5);

    }

    void shutdown() {
        // socket.Send("Arvind", "127.0.0.1", 7331);
        delete tcpClient;
        tcpClient = nullptr;

    }

    void OnTcpConnectionClosed(TcpConnection* connection) {

        std::cout << " Close Con LocalIP" << connection->GetLocalIp() << " PeerIP" << connection->GetPeerIp() << std::endl << std::flush;

    }

    void OnTcpConnectionPacketReceived(TcpConnection* connection, const uint8_t* data, size_t len) {
        std::cout << "data: " << data << "len: " << len << std::endl << std::flush;
        std::string send = "12345";
        connection->Send((const uint8_t*) send.c_str(), 5);

    }
    TcpConnection *tcpClient;

};

int main(int argc, char** argv) {
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));


        Application app;

        tesTcpClient socket;
        socket.start();


        app.waitForShutdown([&](void*) {
            socket.shutdown();

        });




    return 0;
}
