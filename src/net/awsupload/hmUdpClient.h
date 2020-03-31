#ifndef HM_UDP_CLIENT
#define HM_UDP_CLIENT

//#include <mutex>
//#include <condition_variable>

#include "base/base.h"
#include "net/netInterface.h"
#include "net/TcpConnection.h"
#include "base/thread.h"
#include "udpUpload.h"
#include <semaphore.h>
#include <time.h>

#include <mutex>

//using std::endl;
using namespace base;
using namespace net;


class hmUdpClient: public TcpConnection, public base::Thread {
public:

    hmUdpClient(std::string IP, int port) ;
    ~hmUdpClient();
    
    void run() ;

   // void send(char* data, unsigned int lent);

    void shutdown() ;

   bool upload( std::string fileName, std::string driverId, std::string metaData);

   void sendPacket(uint8_t type, uint32_t payloadNo,  uint32_t payloadsize, char *payload) ;
       
    //char *clinetstorage [clientCount];

    char *storage;

    std::atomic<uint32_t> restartPacketNo;
    std::atomic<uint32_t> rem;
    //uint32_t rem;
    std::atomic<uint32_t> uploadedPacketNO;
    
    //std::atomic<bool>restUpload;
 
    size_t size;
    int fd;

    char* storage_row(unsigned int n) ;

    //uint32_t lastPacketLen;
    std::atomic<uint32_t> lastPacketNo;
    
    
     void on_writes();

    //std::atomic<bool> sendheader;
    ///std::atomic<bool> sendfile;
        
    void sendHeader(const std::string fileName) ;
    void sendFile() ;

    void restartUPload(uint32_t uploadedPacket);
    
    void on_connect();

    uv_async_t async;

private:

    std::string IP;
    int port;
   
    int size_of_packet;
    char *send_buffer;
      
    std::string m_fileName;
    std::string m_driverId;
    std::string m_metaData;


    std::mutex udp_client_mutex;

    struct timespec tm;

    //sem_t sem;
    
    ///////////////////////////////////////////////////
    
    void on_close( );

    void on_read( const char* data, size_t len);
    
    /////////////////////////////////////////////////

    //hmTcpClient *tcpClient;
};

#endif  //HM_UDP_CLIENT


#if 0
int main(int argc, char** argv) {
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

    int64_t start_time;
    int64_t end_time;
    
    LTrace("./runhmUdpClient test.mp4 127.0.0.1")
    LTrace("./runhmUdpClient test.mp4")
    Application app;

    int port = 51038;
    std::string ip = "18.228.58.178";
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
    
    hmUdpClient socket(ip, port);
    socket.start();

    
    app.run();



    return 0;
}
#endif