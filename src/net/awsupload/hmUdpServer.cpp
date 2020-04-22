/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

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
       // uint32_t totalPacket = curPtr / UdpDataSize;
       
       // SInfo << "Last Packet " << lastPacketNo << " totalPacket " << totalPacket;
       
       // sendTcpPacket(tcpConn, 3, 100);
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

    if (len != sizeof (struct Packet)) {

        SError << "Fatal error: Some part of packet lost. "  << "Received len " << len << " . But packet size is " <<  sizeof (struct TcpPacket );
        return;
    }

    Packet packet;
    memcpy(&packet, (void*) data, len);
    on_fill(packet);  	
  
   
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
	    m_ping_timeout_timer->Reset();


  	    //SInfo << "Received from " << " size:" << packet.payloadlen << " sequence:" << packet.payload_number;
            //LTrace(packet.payload)
            if (packet.payload_number == curPtr) {
                // memcpy(serverstorage[curPtr++], packet.payload, packet.payloadlen);
                memcpy(storage_row(curPtr), packet.payload, packet.payloadlen);
                ++curPtr;
                waitingPtr = -1;
                freePort = false;
            } else {
                if(waitingPtr > -1)
                {
                    SInfo << "waitingPtr for " << " size:" << packet.payloadlen << " sequence:" << waitingPtr;
                    return;
                }
                if (packet.payload_number < curPtr) {
                    SInfo << "Get Packet already saved : " << packet.payload_number;
                    waitingPtr = curPtr;
                    sendTcpPacket(tcpConn, 3, curPtr);
                    //  memcpy(serverstorage[ packet.payload_number], packet.payload, packet.payloadlen);
                    //curPtr = packet.payload_number;
                    // memcpy(storage_row(packet.payload_number), packet.payload, packet.payloadlen);
                    return;
                }

                if(curPtr == -1 || lastPacketNo ==0)
                {
                    SInfo << "Header not found " << " size:" << packet.payloadlen << " sequence:" << waitingPtr;
                    sendTcpPacket(tcpConn, 4, 0);
                    waitingPtr = 0;
                    return;
                }
                if (packet.payload_number > curPtr) {
                    SInfo << "Packet lost. Sequence No: " << curPtr;
                    waitingPtr = curPtr;
                    sendTcpPacket(tcpConn, 2, curPtr);
                    return;
                    
                }
 
            }



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
		    sendTcpPacket(tcpConn, 3, packet.payload_number);
                }
               // m_ping_timeout_timer->Reset();
            } else if (packet.payload_number > 1001 && !(packet.payload_number % 1001)) {
                int per = 10 * (packet.payload_number / ((lastPacketNo ) / 10));
                if (per != 100) {
                    SInfo << "percentage2 uploaded " << per;
                   sendTcpPacket(tcpConn, 3, packet.payload_number);
                }
                //m_ping_timeout_timer->Reset();
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
    put_s3_object_async(object_name, serverstorage, lastPacketNo-1, lastPacketLen);
}

void hmUdpServer::savetoDB() {
    PutItem(driverId, sharedS3File, metadata);
}

