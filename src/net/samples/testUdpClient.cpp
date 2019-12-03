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

class testUdpClient {
public:

    testUdpClient(std::string IP, int port) : IP(IP), port(port) {
    }

    void start() {
        LTrace("start UDP client")
        udpClient = new UdpSocket(IP, port);
        udpClient->connect();

    }

    void send(std::string txt) {
        udpClient->send((char*) txt.c_str(), txt.length());
    }

    void shutdown() {
        delete udpClient;
        udpClient = nullptr;

    }




    UdpSocket *udpClient;

    std::string IP;
    int port;

};

int main(int argc, char** argv) {
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

    Application app;

    testUdpClient socket("127.0.0.1", 6000);
    socket.start();
    while (true)
    {
    socket.send("arvind testing");
    socket.send("arvind testing1");
    socket.send("arvind testing2");
    socket.send("arvind testing3");
    socket.send("arvind testing4");
    socket.send("arvind testing5");
    }
    app.run();



    return 0;
}
