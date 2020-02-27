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

    void send(char* data, unsigned int lent) {
        std::cout << "sending data " << lent << std::endl;
        udpClient->send(data, lent);
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


    testUdpClient socket(ip, port);
    socket.start();

    std::ifstream infile;
    infile.open(filename, std::ios::binary | std::ios::in);

    char* buffer = new char[32768];

    if (infile.is_open()) {

        while (infile.read(buffer, 32768)) {
            socket.send(buffer, 32768);
        }

        // if the bytes of the block are less than 65536,
        // use fin.gcount() calculate the number, put the va
        // into var s
        socket.send(buffer, infile.gcount());

        infile.close();
    } else {
        std::cerr << "Cannot open file:" << filename << endl;
    }
    delete[] buffer;



    app.run();



    return 0;
}
