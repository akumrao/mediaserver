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
#include "net/TcpServer.h"
#include "base/test.h"
#include "base/time.h"


///ssl
//#include "net/sslmanager.h"

using std::endl;
using namespace base;
using namespace net;
using namespace base::test;



class tesTcpServer : public Listener {
public:

    tesTcpServer() {
    }

    void start() {
        // socket.send("Arvind", "127.0.0.1", 7331);
        tcpServer = new TcpServer(this, "0.0.0.0", 5001, true);

    }

    void shutdown() {
        // socket.send("Arvind", "127.0.0.1", 7331);
        delete tcpServer;
        tcpServer = nullptr;

    }

    void on_close( Listener* connection) {

        std::cout << "TCP server closing, LocalIP" << connection->GetLocalIp() << " PeerIP" << connection->GetPeerIp() << std::endl << std::flush;

    }

    void on_read(Listener* connection, const char* data, size_t len) {
        std::cout << "TCP server send data: " << data << "len: " << len << std::endl << std::flush;
        std::string send = "12345";
        connection->send((const char*) send.c_str(), 5);

    }
    TcpServer *tcpServer;

};


int main(int argc, char** argv) {
    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

 
     //  net::SSLManager::initNoVerifyServer();

        Application app;

        tesTcpServer socket;
        socket.start();

        app.waitForShutdown([&](void*) {

            socket.shutdown();

        });



    return 0;
}
