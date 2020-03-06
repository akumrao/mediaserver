#include "base/base.h"
#include "base/logger.h"
#include "base/application.h"
#include "net/UdpSocket.h"
#include "base/test.h"
#include "base/time.h"
#include "upload.h"

using std::endl;
using namespace base;
using namespace net;
using namespace base::test;


class testUdpServer : public UdpServer::Listener {
public:

    testUdpServer(std::string IP, int port):IP(IP), port(port) {
    }

    void start() {
        // socket.send("Arvind", "127.0.0.1", 7331);
        udpServer = new UdpServer( this, IP, port);
        udpServer->bind();
    }

    void send( std::string txt, std::string ip, int port )
    {
         udpServer->send( (char*) txt.c_str(), txt.length() , ip , port);
    }

    void shutdown() {

        delete udpServer;
        udpServer = nullptr;

    }


    void OnUdpSocketPacketReceived(UdpServer* socket, const char* data, size_t len,  struct sockaddr* remoteAddr) {

        int family;
        
        std::string peerIp;
        uint16_t peerPort;

        IP::GetAddressInfo(
                    remoteAddr, family, peerIp, peerPort);
            
      
        
        
      Packet packet;
      memcpy(&packet,(void*)data,len);
      
      switch(packet.type)
      {
          case 0:
          {   STrace <<  "Received from " << peerIp << ":" << peerPort << " size:"<< packet.len  << " sequence:"<< packet.sequence_number ;
              LTrace("First Packet")
              break;
          }   
          case 1:
          {
              STrace <<  "Received from " << peerIp << ":" << peerPort << " size:"<< packet.len  << " sequence:"<< packet.sequence_number ;
             LTrace(packet.data)
              break;
          }

          case 2:
          {
               STrace <<  "Received from " << peerIp << ":" << peerPort << " size:"<< packet.len  << " sequence:"<< packet.sequence_number ;
              
               LTrace(packet.data)
                       
               LTrace("Last Packet")
              break;
          }
          default:
          {
              LError("fatal UPD pakcets are dropped, check the internet connection")
          }
              
      
      };
    
        
    }

    UdpServer *udpServer;

    std::string IP;
    int port;

};


int main(int argc, char** argv) {
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

 
        Application app;
        
        int port = 51038;
        
        std::string ip = "0.0.0.0";
        
        if (argc > 1) {
            ip = argv[1];
        }
        
        if(argc > 2)
        {
            port = atoi(argv[2]);
        }
        
        STrace << "Udp listening at port " << port << std::endl << std::flush;
        
        testUdpServer socket(ip, port);
        socket.start();
        
        //socket.send("arvind", "127.0.0.1" , 6000);

        app.waitForShutdown([&](void*) {

            socket.shutdown();

        });



    return 0;
}
