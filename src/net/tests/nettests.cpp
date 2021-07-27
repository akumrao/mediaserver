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
#include "base/application.h"
#include "net/UdpSocket.h"
#include "net/TcpServer.h"
#include "base/test.h"
#include "base/time.h"


using std::endl;
using namespace base;
using namespace net;
using namespace base::test;

class testUdpServer : public UdpServer::Listener {
public:

    testUdpServer() : socket(this, "127.0.0.1", 7331) {
        socket.send("Arvind", "127.0.0.1", 7331);
    }

    void OnUdpSocketPacketReceived(UdpServer* socket, const char* data, size_t len, const struct sockaddr* remoteAddr) {

        std::cout << data << std::endl << std::flush;
    }

    UdpServer socket;

};

class tesTcpServer : public Listener {
public:

    tesTcpServer() {
    }

    void start(std::string ip, int port) {
        // socket.send("Arvind", "127.0.0.1", 7331);
        tcpServer = new TcpServer(this, ip, port, true);

    }

    void shutdown()
    {
        // socket.send("Arvind", "127.0.0.1", 7331);
        delete tcpServer;
        tcpServer = nullptr;

    }

    void on_close(Listener* connection) {

        std::cout << "TCP server closing, LocalIP" << connection->GetLocalIp() << " PeerIP" << connection->GetPeerIp() << std::endl << std::flush;

    }

    void on_read(Listener* connection, const char* data, size_t len) {
        std::cout << "TCP server send data: " << data << "len: " << len << std::endl << std::flush;
        std::string send = "12345";
        connection->send((const char*) send.c_str(), 5);
       /* 
        TcpConnectionBase *obj = (TcpConnectionBase*) connection;
        obj->Close();
	*/

    }
    TcpServer *tcpServer;

};

class tesTcpClient : public Listener {
public:

    tesTcpClient() {
    }

    void start() {

        // socket.send("Arvind", "127.0.0.1", 7331);
        tcpClient = new TcpConnectionBase(this);

        tcpClient->Connect("0.0.0.0", 1111);
        const char snd[6] = "12345";
        std::cout << "TCP Client send data: " << snd << "len: " << strlen((const char*) snd) << std::endl << std::flush;

        tcpClient->send(snd, 5);

    }

    void shutdown() 
    {
        // socket.send("Arvind", "127.0.0.1", 7331);
        delete tcpClient;
        tcpClient = nullptr;

    }

    void on_close(Listener* connection) 
    {

        std::cout << " Close Con LocalIP" << connection->GetLocalIp() << " PeerIP" << connection->GetPeerIp() << std::endl << std::flush;

    }

    void on_read(Listener* connection, const char* data, size_t len)
    { 
        std::cout << "data: " << data << "len: " << len << std::endl << std::flush;
        std::string send = "12345";
        connection->send((const char*) send.c_str(), 5);

    }
    
    TcpConnectionBase *tcpClient;

};

int main(int argc, char** argv) {
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

    describe("server", []() {

        Application app;

        tesTcpServer socket;
        socket.start("0.0.0.0", 1111);

        app.waitForShutdown([&](void*) {

           socket.shutdown();

        });


    });

    /**/
//    describe("client", []() {
//
//
//        Application app;
//
//        tesTcpClient socket;
//        socket.start();
//
//
//        app.waitForShutdown([&](void*) {
//            socket.shutdown();
//
//        });
//
//
//    });

    test::runAll();

    return test::finalize();


    return 0;
}
