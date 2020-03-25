#ifndef HM_UDP_SERVER_H
#define HM_UDP_SERVER_H


#include "base/base.h"
#include "net/TcpServer.h"
#include "udpUpload.h"
#include "net/TcpConnection.h"
#include "base/Timer.h"

using namespace base;
using namespace net;

class hmUdpServer : public TcpServer::Listener, public base::Thread {
public:

    hmUdpServer(std::string IP, int port);

    ~hmUdpServer();

    void run();

    void send(std::string txt, std::string ip, int port);

    void shutdown();

    void on_close(Listener* connection);
    void on_read(Listener* connection, const char* data, size_t len);
    
    void sendTcpPacket(TcpConnection* tcpConn, uint8_t type, uint32_t payload);

    char *serverstorage; //[serverCount];

    void savetoS3();
    void savetoDB();

    std::atomic<bool> freePort;

    void resetUdpServer();
    
    TcpConnection* tcpConn;

private:
    
    Timer m_ping_timeout_timer{nullptr};

    char *storage_row(unsigned int n);

    TcpServer *udpServer;

    std::string m_ip;
    int m_port;

    long curPtr;
    int waitingPtr;
    

    uint32_t lastPacketNo{0};
    uint32_t lastPacketLen;
    std::string driverId;
    std::string metadata;

    std::string sharedS3File;

};


#endif  //hm_UDP_SERVER_H
