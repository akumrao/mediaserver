#include "base/base.h"
#include "base/logger.h"
#include "base/application.h"
#include "net/TcpServer.h"
#include "base/time.h"
#include "net/netInterface.h"
#include "tcpUpload.h"
#include "hmUdpServer.h"
#include "awsS3upload.h"
#include "awsDynamodb.h"

using std::endl;
using namespace base;
using namespace net;


class hmTcpServer : public Listener {
public:

    hmTcpServer() {
    }

    void start(std::string ip, int port) {
        // socket.send("Arvind", "127.0.0.1", 7331);
        tcpServer = new TcpServer(this, ip, port);
        SInfo << "Tcp Port listening at " <<  port;
        m_ip = ip;
        
        for(  uint32_t iport =46001 ;  iport < 46038 ;  ++iport  )  
        {
            hmUdpServer  *socket  = new hmUdpServer(m_ip, iport);
            socket->start();
            udpPortManager[iport]= socket;
        }
        
      //  m_ping_timeout_timer.cb_timeout = std::bind(&hmTcpServer::reset, this);
       // m_ping_timeout_timer.Start(9000);
        
    }

    void shutdown() {
        // socket.send("Arvind", "127.0.0.1", 7331);
        delete tcpServer;
        tcpServer = nullptr;
    }

    void on_close(Listener* connection) {
      //  std::cout << "TCP server closing, LocalIP" << connection->GetLocalIp() << " PeerIP" << connection->GetPeerIp() << std::endl << std::flush;
       SInfo << "TCP server closing, LocalIP" << connection->GetLocalIp() << " PeerIP" << connection->GetPeerIp();
       
       auto search = udpConManager.find((TcpConnection*)connection);
        if (search != udpConManager.end()) {
            SInfo << "Freeing UDP Port "  << search->second ;
            udpPortManager[search->second]->resetUdpServer();
            udpConManager.erase (search, udpConManager.end());

           
        } else {
            SError << "Orphan Tcp Connection";
        }
       
        
    }

    void reset() {
        
       // m_ping_timeout_timer.Reset();
        SInfo << "reset Timer"; 
    }
    
    void on_read(Listener* connection, const char* data, size_t len) {
       // STrace << "TCP server on_read: " << data << "len: " << len;
        //connection->send((const char*) send.c_str(), 5);
        
        if (len != sizeof (struct TcpPacket)) {
            LError("Fatal error: Some part of packet lost. ")
            return;
        }

        TcpPacket packet;
        memcpy(&packet, (void*) data, len);

        switch (packet.type) {
            case 0:
            {
               
                SInfo << "TCP Packet type " <<  (int) packet.type << " Request for UDP port allocation: " ;

                
                bool foundFeePort= false;
                {
                    std::lock_guard<std::mutex> guard(g_port_mutex);
                    for ( std::map<uint32_t, hmUdpServer* >::iterator it=udpPortManager.begin(); it!=udpPortManager.end(); ++it)
                    {

                         if( it->second->freePort)
                         {
                             it->second->freePort = false;

                             it->second->tcpConn = (TcpConnection*)connection;
                             it->second->sendTcpPacket( (TcpConnection*)connection, 1, it->first);
                             udpConManager[(TcpConnection*)connection] = it->first;

                             foundFeePort= true;
                             SInfo << "UDP port allocated: " <<  it->first  ;
                             break;
                         }
                    }

                    if(!foundFeePort)
                    {
                        SInfo << "No free port available right now";
                    }
              
                }
              
                break;
            }

            default:
            {
                LError("Fatal TCP: Not a valid state")
            }

        };


    }
    TcpServer *tcpServer;

    std::map<uint32_t, hmUdpServer* > udpPortManager;
    
    std::map<TcpConnection*, uint32_t> udpConManager;
    
    std::mutex g_port_mutex;
    
    std::string m_ip;
    
    
    //Timer m_ping_timeout_timer{nullptr};

};

int main(int argc, char** argv) {
    
    Logger::instance().add(new ConsoleChannel("mediaserver", Level::Debug));
   // Logger::instance().add(new FileChannel("mediaserver","/var/log/mediaserver", Level::Debug));

    Logger::instance().setWriter(new AsyncLogWriter);
    
    // std::string jsonArray = "{filename:driver-1234-1232323.mp4, gps-latitude:28.674109, gps-longitude:77.438009, timestamp:20200309194530, uploadmode:normal}";
    // std::cout << jsonArray << std::endl << std::flush;
    // PutItem("driver12345", jsonArray);
    
   // PutItem( driverId , metadata);
    
    //TCP PORT RANGE 47000-47999
    //UDP PORT RANGE 46000-46999

    int port = 47001;

    std::string ip = "0.0.0.0";

    if (argc > 1) {
        ip = argv[1];
    }

    if (argc > 2) {
        port = atoi(argv[2]);
    }

     awsInit();


    Application app;

    hmTcpServer socket;
    socket.start(ip, port);



    app.waitForShutdown([&](void*) {
        socket.shutdown();
        awsExit();
    });




    return 0;
}
