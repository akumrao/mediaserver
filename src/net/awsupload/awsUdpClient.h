#ifndef AWS_UDP_CLIENT
#define AWS_UDP_CLIENT


#include "base/base.h"
#include "net/UdpSocket.h"
#include "udpUpload.h"
//using std::endl;
using namespace base;
using namespace net;


class awsUdpClient {
public:

    awsUdpClient(std::string IP, int port) ;
    ~awsUdpClient();
    
    void start() ;

   // void send(char* data, unsigned int lent);

    void shutdown() ;
    
    void sendFile(const std::string);
    void sendPacket(uint8_t type, uint16_t payloadNo,  uint16_t payloadsize, char *payload) ;
       
    char *clinetstorage[clientCount];
    uint16_t lastPacketLen;
    uint16_t lastPacketNo;
private:
    UdpSocket *udpClient;
    std::string IP;
    int port;

    
    int size_of_packet;
    char *send_buffer;
      
 
};

#endif  //AWS_UDP_CLIENT


#if 0
int main(int argc, char** argv) {
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

    int64_t start_time;
    int64_t end_time;
    
    LTrace("./runawsUdpClient test.mp4 127.0.0.1")
    LTrace("./runawsUdpClient test.mp4")
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


    for( int x=0; x < clientCount; ++x )
    {
        clinetstorage[x] = new char[UdpDataSize];
    }
    
    awsUdpClient socket(ip, port);
    socket.start();

    
    app.run();



    return 0;
}
#endif