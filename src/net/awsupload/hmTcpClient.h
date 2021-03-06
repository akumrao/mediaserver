/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


#ifndef HM_TCP_CLIENT
#define HM_TCP_CLIENT

#include "base/base.h"
#include "net/netInterface.h"
#include "net/TcpConnection.h"
#include "hmUdpClient.h"
#include "base/application.h"
#include <functional>
#include <mutex>
#include "base/Timer.h"

using namespace base;
using namespace net;



class hmTcpClient : public TcpConnection, public Thread{
public:

    hmTcpClient(const std::string ip, int port);

    ~hmTcpClient();
    void run() ;
    
    void sendPacket(uint8_t type, uint32_t payload);
    
    void timeout_pong();

    void on_connect() ;

    void shutdown() ;

    void on_close();

    void on_read(Listener* connection, const char* data, size_t len) ;
   // void on_read_int( const char* data, size_t len);
    
    void upload(std::string fileName, std::string driverId, std::string metaData);
    
    std::atomic<bool> shuttingDown;
    
    char *send_buffer;
    
    std::mutex g_shutdown_mutex;
private:
    hmUdpClient *udpsocket;
    std::string m_IP;

    int m_port;
    
    std::string m_fileName;
    std::string m_driverId;
    std::string m_metaData;
    


  
public:
   uv_async_t async;
   Timer *m_ping_timeout_timer;
  
   std::function<void(const std::string& , const int&) > fnUpdateProgess; 
   std::function<void(const std::string& , const std::string&) > fnSuccess;
   std::function<void(const std::string& , const std::string&, const int&) > fnFailure;

    enum  { Init,Connected,Progess, Done } en_state;
};


#endif  //HM_UDP_CLIENT
