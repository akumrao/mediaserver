#include "base/base.h"
#include "base/logger.h"
#include "base/application.h"
#include "net/UdpSocket.h"
#include "net/TcpServer.h"
#include "base/test.h"
#include "base/time.h"


using std::endl;
using namespace base;
using namespace net;
using namespace base::test;

class testUdpServer : public UdpServer::Listener {
public:

    testUdpServer() : socket(this, "127.0.0.1", 7331) {
        socket.send("Arvind", "127.0.0.1", 7331);
    }

    void OnUdpSocketPacketReceived(UdpServer* socket, const char* data, size_t len, const struct sockaddr* remoteAddr) {

        std::cout << data << std::endl << std::flush;
    }

    UdpServer socket;

};

class tesTcpServer : Listener {
public:

    tesTcpServer() {
    }

    void start() {
        // socket.send("Arvind", "127.0.0.1", 7331);
        tcpServer = new TcpServer(this, "0.0.0.0", 7000);

    }

    void shutdown() {
        // socket.send("Arvind", "127.0.0.1", 7331);
        delete tcpServer;
        tcpServer = nullptr;

    }

    void OnTcpConnectionClosed(TcpServer* /*tcpServer*/, TcpConnection* connection) {

        std::cout << "TCP server closing, LocalIP" << connection->GetLocalIp() << " PeerIP" << connection->GetPeerIp() << std::endl << std::flush;

    }

    void OnTcpConnectionPacketReceived(TcpConnection* connection, const char* data, size_t len) {
        std::cout << "TCP server send data: " << data << "len: " << len << std::endl << std::flush;
        std::string send = "12345";
        connection->send((const char*) send.c_str(), 5);

    }
    TcpServer *tcpServer;

};

class tesTcpClient : public Listener {
public:

    tesTcpClient() {
    }

    void start() {

        // socket.send("Arvind", "127.0.0.1", 7331);
        tcpClient = new TcpConnection(this);

        tcpClient->Connect("0.0.0.0", 7000);
        const char snd[6] = "12345";
        std::cout << "TCP Client send data: " << snd << "len: " << strlen((const char*) snd) << std::endl << std::flush;

        tcpClient->send(snd, 5);

    }

    void shutdown() {
        // socket.send("Arvind", "127.0.0.1", 7331);
        delete tcpClient;
        tcpClient = nullptr;

    }

    void OnTcpConnectionClosed(TcpConnection* connection) {

        std::cout << " Close Con LocalIP" << connection->GetLocalIp() << " PeerIP" << connection->GetPeerIp() << std::endl << std::flush;

    }

    void OnTcpConnectionPacketReceived(TcpConnection* connection, const char* data, size_t len) {
        std::cout << "data: " << data << "len: " << len << std::endl << std::flush;
        std::string send = "12345";
        connection->send((const char*) send.c_str(), 5);

    }
    TcpConnection *tcpClient;

};

int main(int argc, char** argv) {
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

    describe("server", []() {

        Application app;

        tesTcpServer socket;
        socket.start();

        app.waitForShutdown([&](void*) {

            socket.shutdown();

        });


    });

    /**/
    describe("client", []() {


        Application app;

        tesTcpClient socket;
        socket.start();


        app.waitForShutdown([&](void*) {
            socket.shutdown();

        });


    });

    test::runAll();

    return test::finalize();


    return 0;
}
