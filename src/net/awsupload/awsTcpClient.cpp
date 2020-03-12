#include "base/base.h"
#include "base/logger.h"
#include "base/application.h"
#include "net/netInterface.h"
#include "net/TcpConnection.h"
#include "awsUdpClient.h"
#include "base/time.h"
#include "tcpUpload.h"

using std::endl;
using namespace base;
using namespace net;

class awsTcpClient : public TcpConnection {
public:

    awsTcpClient(const std::string ip, int port) : m_IP(ip), m_port(port), udpsocket(nullptr),
    TcpConnection(this) {
    }

    void start() {
        Connect(m_IP, m_port);
    }
    
    void upload( std::string fileName, std::string driverId, std::string metaData)
    {
       m_fileName = fileName;
       m_driverId  = driverId;
       m_metaData = metaData;
    }

    void sendPacket(uint8_t type, uint16_t payload) {

        STrace << "Send Type" << (int) type << " payload " << payload;

        TcpPacket tcpPacket;
        int size_of_packet = sizeof (struct TcpPacket);
        tcpPacket.type = type;
        tcpPacket.sequence_number = payload;

        char *send_buffer = (char*) malloc(size_of_packet);
        memset(send_buffer, 0, size_of_packet);
        memcpy(send_buffer, (char*) &tcpPacket, size_of_packet);
        send(send_buffer, size_of_packet);
        free(send_buffer);
    }

    void on_connect() {
        STrace << "on_connect Send Init: ";
        sendPacket(0, 0);
    }

    void shutdown() {
        Close();
    }

    void on_close(Listener* connection) {
        std::cout << " Close Con LocalIP" << connection->GetLocalIp() << " PeerIP" << connection->GetPeerIp() << std::endl << std::flush;
    }

    void on_read(Listener* connection, const char* data, size_t len) {
        STrace << "TCP on_read " << "len: " << len;
        // connection->send((const char*) send.c_str(), 5);

        if (len != sizeof (struct TcpPacket)) {
            LTrace(data)
            LError("Fatal error: Some part of packet lost. ")
            return;
        }

        TcpPacket packet;
        memcpy(&packet, (void*) data, len);

        switch (packet.type) {
            case 1:
            {
                //LTrace("First TCP Packet received. ")
                STrace << "UDP Client connect at " << " Port: " << packet.sequence_number;

                udpsocket = new awsUdpClient(m_IP, packet.sequence_number);
                udpsocket->upload( m_fileName, m_driverId, m_metaData); 

                udpsocket->start();

                break;
            }
            case 2:
            {
                SInfo << "TCP Received type " << (int) packet.type << " Retransmission: " << packet.sequence_number;
            
                uint16_t payloadsize = UdpDataSize;
                
                if(packet.sequence_number == udpsocket->lastPacketNo )
                {
                    payloadsize =udpsocket->lastPacketLen;
                    SInfo << "Retransmission of lastpacket: " << packet.sequence_number << " size " <<  payloadsize;
                }
                        
                        
                int rem = packet.sequence_number % clientCount;
                udpsocket->sendPacket(1, packet.sequence_number, payloadsize, udpsocket->clinetstorage[rem]);
                break;
            }

            case 3:
            {
                STrace << "TCP Received type " << packet.type << " payload:" << packet.sequence_number;
                SInfo << "Percentage uploaded " << packet.sequence_number;
            
                break;
            }
            default:
            {
                LError("Fatal TCP: Not a valid state")
            }
        };

    }

    // TcpConnection *tcpClient;

private:
    awsUdpClient *udpsocket;
    std::string m_IP;

    int m_port;
    
    std::string m_fileName;
    std::string m_driverId;
    std::string m_metaData;

};

int main(int argc, char** argv) {
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

    Application app;


    int port = 47001;
    std::string ip = "52.14.171.173";
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
    awsTcpClient socket(ip, port);
    
    std::string metadata = "{filename:driver-1234-1232323.mp4, gps-latitude:28.674109, gps-longitude:77.438009, timestamp:20200309194530, uploadmode:normal}";

    socket.upload( filename, "driver-1234", metadata);
            
    socket.start();

    app.waitForShutdown([&](void*) {
        socket.shutdown();

    });


    return 0;
}
