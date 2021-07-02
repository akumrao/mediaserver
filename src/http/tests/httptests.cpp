/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include "http/HttpServer.h"
#include "base/test.h"
#include "base/logger.h"
#include "base/application.h"

using namespace base;
using namespace base::net;
using namespace base::test;

class BasicResponder : public net::ServerResponder
/// Basic server responder (make echo?)
{
public:

    BasicResponder(net::HttpBase* conn) :
    ServerResponder(conn) {
        STrace << "BasicResponder" << std::endl;
    }

    virtual void onClose() {
        ;
        LDebug("On close")

    }

    void onRequest(net::Request& request, net::Response& response) {
        STrace << "On complete" << std::endl;

        response.setContentLength(14); // headers will be auto flushed

        connection()->send((const char *) "hello universe", 14);
        connection()->Close();
    }
};

class StreamingResponderFactory : public ServerConnectionFactory {
public:

    ServerResponder* createResponder(HttpBase* conn) {
        
         auto& request = conn->_request;

        // Log incoming requests
        STrace << "Incoming connection from " << ": URI:\n" << request.getURI() << ": Request:\n" << request << std::endl;

        // Handle websocket connections
        if (request.getURI().find("/upload") == 0 ) {   // || request.has("Sec-WebSocket-Key")) {
            return new BasicResponder(conn);
        }
        else
        {
            return new BasicResponder(conn);
        }

        
        
        
    }
};


class testwebscoket: public net::HttpServer 
{
public:
    
     testwebscoket( std::string ip, int port, ServerConnectionFactory *factory = nullptr): net::HttpServer(  ip, port,  nullptr)
     {
         
     }
    
    void on_read(Listener* connection, const char* BODY, size_t len) {
      
        //connection->send("arvind", 6 );
        
        WebSocketConnection *con = (WebSocketConnection*)connection;
        
        con->send("arvind", 6 );
         
    }
    
};


int main(int argc, char** argv) {

    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));
    //test::init();
  
        Application app;
        testwebscoket socket("0.0.0.0", 8000 );
        socket.start();

        app.waitForShutdown([&](void*) {

            socket.shutdown();

        });

    
    /*
     
  
    /// for websocket we do not need responder
        Application app;
        net::HttpServer websocket("0.0.0.0", 8000  );
        websocket.start();

        app.waitForShutdown([&](void*) {

            websocket.shutdown();

        });
     */



    return 0;

    //  test::runAll();

    // return test::finalize();
}
