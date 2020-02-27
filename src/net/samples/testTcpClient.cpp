#include "base/base.h"
#include "base/logger.h"
#include "base/application.h"
#include "net/netInterface.h"
#include "net/TcpConnection.h"
#include "base/test.h"
#include "base/time.h"


using std::endl;
using namespace base;
using namespace net;
using namespace base::test;


class tesTcpClient : public TcpConnection{
public:

    tesTcpClient(): TcpConnection(this) {}

    void start(std::string ip, int port) {

        // socket.send("Arvind", "127.0.0.1", 7331);
        tcpClient = new TcpConnection(this);

        tcpClient->Connect(ip, port);
        const char snd[6] = "12345";
        std::cout << "TCP Client send data: " << snd << "len: " << strlen((const char*) snd) << std::endl << std::flush;

        tcpClient->send(snd, 5);

    }

    void shutdown() {
        // socket.send("Arvind", "127.0.0.1", 7331);
        delete tcpClient;
        tcpClient = nullptr;

    }

    void on_close(Listener* connection) {

    
        std::cout << " Close Con LocalIP" << connection->GetLocalIp() << " PeerIP" << connection->GetPeerIp() << std::endl << std::flush;

    }

    void on_read(Listener* connection, const char* data, size_t len) {
        std::cout << "data: " << data << "len: " << len << std::endl << std::flush;
        std::string send = "12345";
        connection->send((const char*) send.c_str(), 5);

    }
    TcpConnection *tcpClient;

};

int main(int argc, char** argv) {
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));


        Application app;

        tesTcpClient socket;


	int port = 51038;
	std::string ip = "18.221.232.217";
	std::string filename;
	    
	if (argc > 1) {
	   filename = argv[1];
	}

	if (argc > 2) {
	   ip = argv[2];
	 }

	 if (argc > 3) {
	    port = atoi(argv[3]);
	 }

        socket.start(ip, port);


        app.waitForShutdown([&](void*) {
            socket.shutdown();

        });




    return 0;
}
