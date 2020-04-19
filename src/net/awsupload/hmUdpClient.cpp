/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include "hmUdpClient.h"
#include "base/logger.h"
#include "base/application.h"
#include "base/platform.h"
#include "net/TcpConnection.h"
#include <math.h>       /* ceil */


////////////////////
#include <stdio.h>
#include <sys/stat.h>

#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
//////////////////

//using std::endl;
using namespace base;
using namespace net;

hmUdpClient::hmUdpClient(std::string IP, int port) : IP(IP), port(port),restartPacketNo(0), uploadedPacketNO(0),TcpConnection(this) {

//    for (int x = 0; x < clientCount; ++x) {
//        clinetstorage[x] = new char[UdpDataSize];
//    }


    
    storage = nullptr;

    size_of_packet = sizeof (struct Packet);
    send_buffer = new char[size_of_packet];
    
    //sendheader = true;
   // sendfile= true;

    //uv_sem_init(&sem, 0);
   
//   if (sem_init(&sem, 0, 0) == -1)
 //      LTrace("handle_error sem_init")  ;
    
    rem=0;

}

hmUdpClient::~hmUdpClient() {

//    join();
    
    delete []send_buffer;
     
    if(fd  > 0 )
    {
      munmap(storage, size);
      close(fd);
    }  
    
   // delete udpClient;
   // udpClient = nullptr;

  //  sem_destroy(&sem);


    LTrace("~hmUdpClient()" )
}



static void async_cb_upload(uv_async_t* handle) {

    LTrace(" Upload::async_cb_upload")


    hmUdpClient *p = ( hmUdpClient *) handle->data;

    uv_close((uv_handle_t*)&p->async, nullptr);

    p->Close();

    SInfo << "Upload::async_cb_upload over" ;

}


void hmUdpClient::on_connect() {
    STrace << "hmUdpClient::on_connect() ";

      while( !stopped()  && rem < lastPacketNo  ) {
        //
        //
        if (!rem) {
            //udpClient->connect();
            sendHeader(m_fileName);
        }

        if (rem < lastPacketNo)
            sendFile();

        ++rem;

//        udp_client_mutex.lock();
//        if (restartPacketNo) {
//            rem = 0;
//            restartPacketNo = 0;
//            STrace << "restartPacketNo Frame " << rem << " Uploaded " << uploadedPacketNO << " Lastpacketno " <<  lastPacketNo ;
//
//        }
//        udp_client_mutex.unlock();

       // STrace << "Read Packet Frame " << rem;

//        if( ( (uploadedPacketNO < lastPacketNo) && (rem >  lastPacketNo)  ))
//        {
//          STrace << "Read Packet Frame " << rem << " Uploaded " << uploadedPacketNO << " Lastpacketno " <<  lastPacketNo ;
//         // ; /* should block */
//           //udpClient->connect();
//        } else {

          //  clock_gettime(CLOCK_REALTIME, &tm);
            //tm.tv_nsec += 1000000000L;
           // tm.tv_sec += 1;
          // int x = sem_timedwait(&sem, &tm);
          // SInfo << x;
        //}
           usleep(100); //2900

         //  sem_wait(&sem);
    }
    
   // Close();
}

void hmUdpClient::restartUPload(uint32_t uploaded)
{
   // udp_client_mutex.lock();
 //   restUpload = true;

    udp_client_mutex.lock();
    restartPacketNo = uploaded;
    udp_client_mutex.unlock();

    //sem_post(&sem);

    //udp_client_mutex.unlock();

}


void hmUdpClient::on_close() {


    SInfo<< "hmUdpClient::on_close" ;//<< connection->GetLocalIp() << " PeerIP" << connection->GetPeerIp() << std::endl << std::flush;

}

void hmUdpClient::on_read( const char* data, size_t len) {
   // std::cout << "data: " << data << "len: " << len << std::endl << std::flush;
   // std::string send = "12345";
//    connection->send((const char*) send.c_str(), 5);

}
    

void hmUdpClient::on_writes()
{
  LTrace("on_write")
    
  //  sem_post(&sem);
}
void hmUdpClient::run() {

    Application app;
    Connect(IP, port);
  
    LTrace("run start")
    SInfo << "Send File start";


    async.data = this;
    int r = uv_async_init(app.uvGetLoop(), &async, async_cb_upload);
    app.run();


    LTrace("hmUdpClient::run() over")
}

//void hmUdpClient::send(char* data, unsigned int lent) {
//    std::cout << "sending data " << lent << std::endl;
//    udpClient->send(data, lent);
//}

void hmUdpClient::shutdown() {
    
    LInfo("hmUdpClient::shutdown()::stop");
    
    stop();
    int  r = uv_async_send(&async);
    assert(r == 0);
    //restartUPload(lastPacketNo);
    join();
    
//    if(udpClient)
//    {
//
//        udpClient->Close();
//        
//      //  base::sleep(500);
//        
//     
//         LInfo("hmUdpClient::shutdown()::udpClient");
//
//    }
    
    LInfo("hmUdpClient::shutdown()::over");
}

bool hmUdpClient::upload( std::string fileName, std::string driverId, std::string metaData)
{
    struct stat st;
    fd = open(fileName.c_str(), O_RDONLY,1);

    int rc = fstat(fd, &st);

    size=st.st_size;
    m_fileName = fileName;
    m_driverId  = driverId;
    m_metaData = metaData;

    if(fd > 0)
    {
        
        
        lastPacketNo = ceil((float)size / (float) (UdpDataSize));
        SInfo << "fileSize: "  <<  size ;
        SInfo << "Last packet no: "  <<  lastPacketNo ;

        storage = (char *)mmap(0, size, PROT_READ ,MAP_SHARED , fd, 0);
        return true;
    }
    else {
        SError << "Cannot open file: " << fileName ;
        return false;
    }
}

void hmUdpClient::sendPacket(uint8_t type, uint32_t payloadNo, uint32_t payloadsize, char *payload) {

  
   // SInfo << "Sending "  <<  (int) type <<  " payloadNo " << payloadNo  << " payloadsize " << payloadsize;
    if(type == 0)
    {
        Packet packet;
        packet.type = type;
        packet.payload_number = payloadNo;
        packet.payloadlen = payloadsize;

        memcpy(packet.payload, payload, payloadsize);
        memset(send_buffer, 0, size_of_packet);
        memcpy(send_buffer, (char*) &packet, size_of_packet);
        send(send_buffer, size_of_packet);
    }
    else
    send(payload, payloadsize);

}

char *hmUdpClient::storage_row(unsigned int n) {
    return storage + (n * UdpDataSize);
}




void hmUdpClient::sendHeader(const std::string fileName) {

    SInfo << "Send Header";
            
    if (fd> 0 ) {

         if (!stopped())
         {

            std::string mtTmp = m_driverId  +";" + m_metaData;
            sendPacket(0, lastPacketNo, mtTmp.length()+1, (char*)mtTmp.c_str());
         }
    

    }
}

void hmUdpClient::sendFile() {


    if (rem  < lastPacketNo-1) {
         // char *output = str2md5(data_packet.data, data_size);
        //char *output1 = str2md5(buffer[send_count], data_size);
        sendPacket(1, rem, UdpDataSize , storage_row(rem));
        
    }

    else if( rem  < lastPacketNo) {
        uint32_t lastPacketLen = size - rem*UdpDataSize;
        sendPacket(1, rem, lastPacketLen, storage_row(rem));
    }
}

   // end_time = base::Application::GetTime();

   // STrace << "time_s " << double(end_time - start_time) / 1000.00 ;

