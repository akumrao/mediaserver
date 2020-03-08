#include "base/base.h"
#include "base/logger.h"
#include "net/netInterface.h"
#include "base/application.h"
#include "net/UdpSocket.h"
#include "base/test.h"
#include "base/time.h"
#include "awsUdpServer.h"
#include "tcpUpload.h"
#include "awsS3upload.h"

using std::endl;
using namespace base;
using namespace net;


    
    void awsUdpServer::start() {

       udpServer = new UdpServer(this, IP, port);
       udpServer->bind();
    }

//    void awsUdpServer::send(std::string txt, std::string ip, int port) {
//        send((char*) txt.c_str(), txt.length(), ip, port);
//    }
    
    
    void awsUdpServer::sendTcpPacket(TcpConnection* tcpConn, uint8_t type, uint16_t payload) {

        STrace << "sendTcpPacket  " << (int) type << " payload " << payload;

        if(type == 2 )
        {
             STrace << "sendTcpPacket for retransmission request " << (int) type << " payload " << payload;
        }
        TcpPacket tcpPacket;
        int size_of_packet = sizeof (struct TcpPacket);
        tcpPacket.type = type;
        tcpPacket.sequence_number = payload;
        
        char *send_buffer = (char*)malloc(size_of_packet);
        memset(send_buffer, 0, size_of_packet);
        memcpy(send_buffer, (char*) &tcpPacket, size_of_packet);
        tcpConn->send(send_buffer, size_of_packet);
        free(send_buffer);
    }

    void awsUdpServer::shutdown() {

        delete udpServer;
        udpServer = nullptr;

    }

    void awsUdpServer::OnUdpSocketPacketReceived(UdpServer* socket, const char* data, size_t len, struct sockaddr* remoteAddr) {

        int family;

        std::string peerIp;
        uint16_t peerPort;

        IP::GetAddressInfo(
                remoteAddr, family, peerIp, peerPort);

        if (len != sizeof (struct Packet)) {
            LError("Fatal error: Some part of packet lost. ")
            return;
        }

        Packet packet;
        memcpy(&packet, (void*) data, len);

        switch (packet.type) {
            case 0:
            {
                LTrace("First Packet")
                STrace << "Received from " << peerIp << ":" << peerPort <<  " Payload Size:" << packet.payloadlen << " Last Packet NO:" << packet.payload_number -1;

                LTrace(packet.payload)
                fileName =   packet.payload;      
                if (curPtr) {
                    LError("Fatal error: Two Udp streams are not possible on one port. ")
                }

                curPtr = 0;
                lastPacketNo = packet.payload_number -1;
                
                break;
            }
            case 1:
            {

                STrace << "Received from " << peerIp << ":" << peerPort << " size:" << packet.payloadlen << " sequence:" << packet.payload_number;
                //LTrace(packet.payload)
                if (packet.payload_number == curPtr)
                    memcpy(&serverstorage[curPtr++], packet.payload, packet.payloadlen);
                else {
                    while (packet.payload_number > curPtr) {
                        sendTcpPacket(tcpConn, 2, curPtr);
                        SError << "Packet lost. Sequence No: " << curPtr++;
                    }
                    if (packet.payload_number < curPtr) {
                        STrace << "Lost Packet found. Sequence No: " << packet.payload_number;
                         memcpy(&serverstorage[ packet.payload_number], packet.payload, packet.payloadlen);
                        break;
                    }

                    memcpy(&serverstorage[ packet.payload_number], packet.payload, packet.payloadlen);
                    ++curPtr;
                }
                
                if(curPtr > lastPacketNo )
                {
                    LTrace("Saving S3 file ", fileName )
                    const Aws::String object_name = "test.mp4";
                    put_s3_object_async(object_name, fileName );   
                }
                break;
            }

            default:
            {
                LError("Fatal UPD: Pakcets are dropped, check the internet connection")
            }


        };

    }

    
