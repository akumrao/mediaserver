#include "base/base.h"
#include "base/logger.h"
#include "base/application.h"
#include "net/UdpSocket.h"
#include "base/test.h"
#include "base/time.h"
#include "awsUdpServer.h"

using std::endl;
using namespace base;
using namespace net;



    void awsUdpServer::start() {
        // socket.send("Arvind", "127.0.0.1", 7331);
        udpServer = new UdpServer(this, IP, port);
        udpServer->bind();
    }

    void awsUdpServer::send(std::string txt, std::string ip, int port) {
        udpServer->send((char*) txt.c_str(), txt.length(), ip, port);
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
                STrace << "Received from " << peerIp << ":" << peerPort << " size:" << packet.len << " sequence:" << packet.sequence_number;

                LTrace(packet.data)

                if (curPtr) {
                    LError("Fatal error: Two Udp streams are not possible on one port. ")
                }

                curPtr = 0;
                break;
            }
            case 1:
            {
                LTrace("Next Packet")
                STrace << "Received from " << peerIp << ":" << peerPort << " size:" << packet.len << " sequence:" << packet.sequence_number;
                //LTrace(packet.data)
                if (packet.sequence_number == curPtr)
                    memcpy(&serverstorage[curPtr++], packet.data, packet.len);
                else {
                    while (packet.sequence_number > curPtr) {
                        SError << "Packet lost. Sequence No: " << curPtr++;
                    }
                    if (packet.sequence_number < curPtr) {
                        SError << "Lost Packet found. Sequence No: " << ++curPtr;
                    }

                    memcpy(&serverstorage[ packet.sequence_number], packet.data, packet.len);
                    ++curPtr;
                }
                break;
            }

            case 2:
            {
                LTrace("Last Packet")
                STrace << "Received from " << peerIp << ":" << peerPort << " size:" << packet.len << " sequence:" << packet.sequence_number;

                if (packet.sequence_number == curPtr)
                    memcpy(&serverstorage[curPtr++], packet.data, packet.len);
                else {
                    while (packet.sequence_number > curPtr) {
                        SError << "Packet lost. Sequence No: " << curPtr++;
                    }
                    if (packet.sequence_number < curPtr) {
                        SError << "Lost Packet found. Sequence No: " << ++curPtr;
                    }

                    memcpy(&serverstorage[ packet.sequence_number], packet.data, packet.len);
                    ++curPtr;
                }
                curPtr = 0;
                break;
            }
            default:
            {
                LError("Fatal UPD: Pakcets are dropped, check the internet connection")
            }


        };

    }

    
