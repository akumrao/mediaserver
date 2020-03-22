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
        
        for(  uint16_t iport =46001 ;  iport < 46008 ;  ++iport  )  
        {
            hmUdpServer  *socket  = new hmUdpServer(m_ip, iport);
            socket->start();
            udpPortManager[iport]= socket;
        }
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
            udpPortManager[search->second]->freePort = free;
            udpConManager.erase (search, udpConManager.end());

           
        } else {
            SError << "Orphan Tcp Connection";
        }
       
        
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

//                uint16_t port = 46001;
//
//                int portmangersize = udpPortManager.size();
//                for( int i =0 ; i< portmangersize ++ i  )
//                if (portmangersize) {
//                    port = (--udpPortManager.end())->first + 1;
//                    // todo circular port allocation
//                }

                for ( std::map<uint16_t, hmUdpServer* >::iterator it=udpPortManager.begin(); it!=udpPortManager.end(); ++it)
                {
                     if( it->second->freePort)
                     {
                         it->second->tcpConn = (TcpConnection*)connection;
                         it->second->sendTcpPacket( (TcpConnection*)connection, 1, it->first);
                         udpConManager[(TcpConnection*)connection] = it->first;
                         it->second->freePort = false;
                         
                         SInfo << "UDP port allocated: " <<  it->first  ;
                         break;
                     }
                }
             //  // std::cout << it->first << " => " << it->second << '\n'
                
            ///////////
               
               
                  
              
                break;
            }
//            case 2:
//            {
//                
//                LTrace("First TCP Packet received")
//                STrace << "TCP Received type " << (int) packet.type << " payload:" << packet.sequence_number;
//                break;
//            }
//
//            case 3:
//            {
//                LTrace("First TCP Packet received")
//                STrace << "TCP Received type " << (int) packet.type << " payload:" << packet.sequence_number;
//                break;
//                break;
//            }
            default:
            {
                LError("Fatal TCP: Not a valid state")
            }

        };


    }
    TcpServer *tcpServer;

    std::map<uint16_t, hmUdpServer* > udpPortManager;
    
    std::map<TcpConnection*, uint16_t> udpConManager;
    
    std::string m_ip;

};

int main(int argc, char** argv) {
    Logger::instance().add(new ConsoleChannel("debug", Level::Debug));

    
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
