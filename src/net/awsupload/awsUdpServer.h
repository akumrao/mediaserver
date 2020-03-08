#ifndef AWS_UDP_SERVER_H
#define AWS_UDP_SERVER_H


#include "base/base.h"
#include "net/UdpSocket.h"
#include "udpUpload.h"


using namespace base;
using namespace net;



class awsUdpServer : public UdpServer::Listener {
public:

    awsUdpServer(std::string IP, int port) : IP(IP), port(port), curPtr(0) {

        for (int x = 0; x < serverCount; ++x) {
            serverstorage[x] = new char[UdpDataSize];
        }
    }

    void start();

    void send(std::string txt, std::string ip, int port);

    void shutdown();

    void OnUdpSocketPacketReceived(UdpServer* socket, const char* data, size_t len, struct sockaddr* remoteAddr) ;

    UdpServer *udpServer;

    std::string IP;
    int port;

    unsigned int curPtr;
    char *serverstorage[serverCount];
};



#endif  //AWS_UDP_SERVER_H