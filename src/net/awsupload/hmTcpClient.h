
#ifndef HM_TCP_CLIENT
#define HM_TCP_CLIENT

#include "base/base.h"
#include "net/netInterface.h"
#include "net/TcpConnection.h"
#include "hmUdpClient.h"
#include <functional>

using namespace base;
using namespace net;



class hmTcpClient : public TcpConnection, public Thread{
public:

    hmTcpClient(const std::string ip, int port) : m_IP(ip), m_port(port), udpsocket(nullptr),
    TcpConnection(this) {
    }

    void run() ;
    
    void sendPacket(uint8_t type, uint16_t payload);

    void on_connect() ;

    void shutdown() ;

    void on_close(Listener* connection);

    void on_read(Listener* connection, const char* data, size_t len) ;
    
    void upload(std::string fileName, std::string driverId, std::string metaData);


private:
    hmUdpClient *udpsocket;
    std::string m_IP;

    int m_port;
    
    std::string m_fileName;
    std::string m_driverId;
    std::string m_metaData;
  
public:
   std::function<void(const std::string& , const int&) > fnUpdateProgess; ///< Signals when raw data is received

};


#endif  //HM_UDP_CLIENT