#ifndef HM_UDP_SERVER_H
#define HM_UDP_SERVER_H


#include "base/base.h"
#include "net/UdpSocket.h"
#include "udpUpload.h"
#include "net/TcpConnection.h"


using namespace base;
using namespace net;

class hmUdpServer : public UdpServer::Listener, public base::Thread {
public:

    hmUdpServer(std::string IP, int port);

    ~hmUdpServer();

    void run();

    void send(std::string txt, std::string ip, int port);

    void shutdown();

    void OnUdpSocketPacketReceived(UdpServer* socket, const char* data, size_t len, struct sockaddr* remoteAddr);
    void sendTcpPacket(TcpConnection* tcpConn, uint8_t type, uint16_t payload);

    char *serverstorage; //[serverCount];

    void savetoS3();
    void savetoDB();

    bool freePort;
    TcpConnection* tcpConn;

private:

    char *storage_row(unsigned int n);

    UdpServer *udpServer;

    std::string m_ip;
    int m_port;

    unsigned int curPtr;

    int lastPacketNo{0};
    int lastPacketLen;
    std::string driverId;
    std::string metadata;

    std::string sharedS3File;

};


#endif  //hm_UDP_SERVER_H
