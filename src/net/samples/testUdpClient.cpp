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

class testUdpClient{
public:

    testUdpClient(std::string IP, int port):IP(IP), port(port) {
    }

    void start() {
        // socket.Send("Arvind", "127.0.0.1", 7331);
        udpClient = new UdpClient(IP, port );

    }

    void send( std::string txt  )
    {
         udpClient->Send(txt);
    }

    void shutdown() {
        delete udpClient;
        udpClient = nullptr;

    }


    

    UdpClient *udpClient;

    std::string IP;
    int port;

};


int main(int argc, char** argv) {
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

 

        Application app;

        testUdpClient socket("127.0. 0.1" , 6000);
        socket.start();
        socket.send("arvind testing");

       app.run();



    return 0;
}
