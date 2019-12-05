#include "base/base.h"
#include "base/logger.h"
#include "base/application.h"
#include "net/netInterface.h"
#include "net/SslConnection.h"
#include "base/test.h"
#include "base/time.h"

#include "net/sslmanager.h"

using std::endl;
using namespace base;
using namespace net;
using namespace base::test;


class tesTcpClient : public Listener {
public:

    tesTcpClient() {
    }

    void start() {

        // socket.send("Arvind", "127.0.0.1", 7331);
        tcpClient = new SslConnection(this);
        tcpClient->Connect("0.0.0.0", 5001);
        const char snd[6] = "12345";
        std::cout << "TCP Client send data: " << snd << "len: " << strlen((const char*) snd) << std::endl << std::flush;

       // tcpClient->send(snd, 5);

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
        
      std::cout << " on_read " << connection->GetLocalIp() << " PeerIP " << connection->GetPeerIp() << std::endl << std::flush;

        std::cout << "data: " << data << " len: " << len << std::endl << std::flush;
        std::string send = "12345";
       // connection->send((const char*) send.c_str(), 5);

    }
    
    void on_connect(Listener* connection) {
        
       std::cout << " on_read " << connection->GetLocalIp() << " PeerIP " << connection->GetPeerIp() << std::endl << std::flush;

     
        std::string send = "12345";
        connection->send((const char*) send.c_str(), 5);

    }
    
    SslConnection *tcpClient;

};

int main(int argc, char** argv) {
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

      // net::SSLManager::initNoVerifyClient();
       
        Application app;

        tesTcpClient socket;
        socket.start();

        app.run();

        




    return 0;
}
