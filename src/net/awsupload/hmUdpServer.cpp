#include "base/base.h"
#include "base/logger.h"
#include "net/netInterface.h"
#include "base/application.h"
#include "net/UdpSocket.h"
#include "base/test.h"
#include "base/time.h"
#include "hmUdpServer.h"
#include "tcpUpload.h"
#include "awsS3upload.h"
#include "awsDynamodb.h"
#include <sys/mman.h>


using std::endl;
using namespace base;
using namespace net;

#define UPDServerUPloadTimeout 90000

void hmUdpServer::run() {

    Application app;
    m_ping_timeout_timer = new Timer (nullptr);
    m_ping_timeout_timer->cb_timeout = std::bind(&hmUdpServer::resetUdpServer, this);
     
    udpServer = new TcpServer(this, m_ip, m_port);
    app.run();
    // udpServer->bind();
}

hmUdpServer::~hmUdpServer() {
    
    shutdown();
}

char *hmUdpServer::storage_row(unsigned int n) {
    return serverstorage + (n * UdpDataSize);
}

hmUdpServer::hmUdpServer(std::string IP, int port) : m_ip(IP), m_port(port), curPtr(0), freePort(true), serverstorage(nullptr) {

    serverstorage = (char*) mmap(0, serverCount * UdpDataSize, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    //        for (int x = 0; x < ; ++x) {
    //            serverstorage[x] = new char[UdpDataSize];
    //            
    //        }

    curPtr = -1;
    waitingPtr = false;
}


//    void hmUdpServer::send(std::string txt, std::string ip, int port) {
//        send((char*) txt.c_str(), txt.length(), ip, port);
//    }

void hmUdpServer::resetUdpServer() {
    SInfo << "Reset and free udp port";
    m_ping_timeout_timer->Stop();

    if (!freePort && lastPacketNo && waitingPtr) {
        waitingPtr = false;
        uint32_t totalPacket = curPtr / UdpDataSize;
       
        SInfo << "Last Packet " << lastPacketNo << " totalPacket " << totalPacket;
       
        sendTcpPacket(tcpConn, 3, 100);
        savetoS3();
        savetoDB();
        curPtr = -1;
        lastPacketNo = 0;
       
    }
   freePort = true;

}

void hmUdpServer::sendTcpPacket(TcpConnection* tcpConn, uint8_t type, uint32_t payload) {

    STrace << "sendTcpPacket  " << (int) type << " payload " << payload;

    if (type == 2) {
        STrace << "sendTcpPacket for retransmission request " << (int) type << " payload " << payload;
    }
    TcpPacket tcpPacket;
    int size_of_packet = sizeof (struct TcpPacket);
    tcpPacket.type = type;
    tcpPacket.sequence_number = payload;

    char *send_buffer = (char*) malloc(size_of_packet);
    memset(send_buffer, 0, size_of_packet);
    memcpy(send_buffer, (char*) &tcpPacket, size_of_packet);
    tcpConn->send(send_buffer, size_of_packet);
    free(send_buffer);
}

void hmUdpServer::shutdown() {

    if (udpServer) {
        delete udpServer;
        udpServer = nullptr;
        
        m_ping_timeout_timer->Stop();
        m_ping_timeout_timer->Close();
        delete m_ping_timeout_timer;
    }
}

void hmUdpServer::on_close(Listener* connection) {
    LError(" hmUdpServer:: on_close. ")
}

void hmUdpServer::on_read(Listener* connection, const char* data, size_t len) {

  
    if (curPtr >-1 && lastPacketNo > 0) {
        memcpy(serverstorage + curPtr, data, len);
        curPtr = curPtr + len;


        uint32_t totalPacket = curPtr / UdpDataSize;

        Packet lpacket;
        lpacket.type = 1;
        lpacket.payload_number = totalPacket;
        on_fill(lpacket);

        if (totalPacket == lastPacketNo) {
            return;
        }
        return;
    } else {
        if (len == sizeof (struct Packet)) {
            Packet packet;
            memcpy(&packet, (void*) data, len);
            on_fill(packet);
        } else {

            SError << "Header is two big:" << len << " struct Packet " << sizeof (struct Packet);

            bool foundHeader = false;
            if (len > sizeof (struct Packet)) {

                for (int i = 0; i < len; i = i + sizeof (struct Packet)) {

                    Packet lpacket;
                    lpacket.type = 9;

                    if (!foundHeader) {
                        memcpy(&lpacket, (void*) (data + i), sizeof (struct Packet));
                    }

                    if (lpacket.type == 0) {
                        foundHeader = true;
                        on_fill(lpacket);

                    } else {
                        if (foundHeader == true) {
                            memcpy(serverstorage + curPtr, (data + i), len - sizeof (struct Packet));
                            curPtr = curPtr + len - sizeof (struct Packet);
                            break;
                        }
                        continue;
                    }
                }
                if (!foundHeader) {
                    SError << "Could not find header";
                }

            } else {

                SInfo << "Failure TCP len " << len << " TCP Pakcet " << sizeof (struct Packet);

            }
        }

    }
}

void hmUdpServer::on_fill(Packet & packet) {

    switch (packet.type) {
        case 0:
        {
            //LTrace("First Packet")
            SInfo << "Received from " << " Payload Size:" << packet.payloadlen << " Last Packet NO:" << packet.payload_number - 1;

            //LTrace(packet.payload)

            std::vector<std::string> tmp = base::util::split(packet.payload, ';', 1);
            if (tmp.size() > 1) {
                driverId = tmp[0];
                metadata = tmp[1];
                sharedS3File = driverId + "_" + std::to_string(Application::GetTime());
            }


            SInfo << " S3 shared file " << sharedS3File << ".mp4";
            SInfo << "metadata " << metadata;
            SInfo << "driverid " << driverId;

            //            if (curPtr) {
            //                LError("Fatal error: Two Udp streams are not possible on one port. ")
            //            }

            curPtr = 0;
            waitingPtr = true;
            lastPacketNo = packet.payload_number ;

            m_ping_timeout_timer->Start(UPDServerUPloadTimeout, UPDServerUPloadTimeout);

            break;
        }
        case 1:
        {

            //SInfo << "Received from " << " size:" << packet.payloadlen << " sequence:" << packet.payload_number;
            // memcpy(serverstorage[curPtr++], packet.payload, packet.payloadlen);
            // memcpy(storage_row(packet.payload_number), packet.payload, packet.payloadlen);
            //   ++curPtr;
            
            freePort = false;

            // LInfo( curPtr % ((lastPacketNo+1)/10 ))

            if (packet.payload_number + 1 >= lastPacketNo) {
                SInfo << "percentage uploaded 100";
                sendTcpPacket(tcpConn, 3, packet.payload_number);
                lastPacketLen = packet.payloadlen;
                resetUdpServer();
                m_ping_timeout_timer->Stop();

            } else if (!((packet.payload_number + 1) % ((lastPacketNo ) / 10))) {
                int per = 10 * (packet.payload_number / ((lastPacketNo ) / 10));
                if (per != 100) {
                    SInfo << "percentage1 uploaded " << per;
                }
                m_ping_timeout_timer->Reset();
            } else if (packet.payload_number > 1001 && !(packet.payload_number % 1001)) {
                int per = 10 * (packet.payload_number / ((lastPacketNo ) / 10));
                if (per != 100) {
                    SInfo << "percentage2 uploaded " << per;
                    //sendTcpPacket(tcpConn, 3, packet.payload_number);
                }
                m_ping_timeout_timer->Reset();
            }

            break;
        }

        default:
        {
            LError("Fatal UPD: Pakcets are dropped, check the internet connection")
        }

    };

}

void hmUdpServer::savetoS3() {

    std::string driverIdTmp = sharedS3File + ".mp4";

    LInfo("Saving S3 file ", "https://uberproject.s3.amazonaws.com/", sharedS3File, ".mp4")
            const Aws::String object_name = driverIdTmp.c_str();
    put_s3_object_async(object_name, serverstorage, curPtr, lastPacketLen);
}

void hmUdpServer::savetoDB() {
    PutItem(driverId, sharedS3File, metadata);
}

