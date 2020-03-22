
#ifndef HM_TCP_CLIENT
#define HM_TCP_CLIENT

#include "base/base.h"
#include "net/netInterface.h"
#include "net/TcpConnection.h"
#include "hmUdpClient.h"
#include "base/application.h"
#include <functional>

using namespace base;
using namespace net;



class hmTcpClient : public TcpConnection, public Thread{
public:

    hmTcpClient(const std::string ip, int port) : m_IP(ip), m_port(port), udpsocket(nullptr),
     TcpConnection(this) {
    }

    ~hmTcpClient();
    void run() ;
    
    void sendPacket(uint8_t type, uint32_t payload);

    void on_connect() ;

    void shutdown() ;

    void on_close();

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
   uv_async_t async;
   Application app;
   std::function<void(const std::string& , const int&) > fnUpdateProgess; 
   std::function<void(const std::string& , const std::string&) > fnSuccess;
   std::function<void(const std::string& , const std::string&, const int&) > fnFailure;

    enum  { Init,Connected,Progess, Done } en_state;
};


#endif  //HM_UDP_CLIENT