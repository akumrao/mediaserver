/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include "http/HTTPResponder.h"
#include "base/test.h"
#include "base/logger.h"
#include "base/application.h"

using namespace base;
using namespace base::net;
using namespace base::test;



class testwebscoket: public net::HttpsServer 
{
public:
    
     testwebscoket( std::string ip, int port, ServerConnectionFactory *factory = nullptr,  bool multithreaded =false) : net::HttpsServer(  ip, port,  factory, multithreaded)
     {
         
     }
    
    void on_read(Listener* connection, const char* msg, size_t len) {
      
        //connection->send("arvind", 6 );
        SInfo << "msg " << std::string(msg,len);
        WebSocketConnection *con = (WebSocketConnection*)connection;
        
        //con->send( msg, len );
        
        sendAll( msg, len );
         
    }
    
    void sendAll(const char* msg, size_t len) {
      
        
        SInfo << "No of Connectons " << this->GetNumConnections();
        
        for (auto* connection :  this->GetConnections())
        {
             WebSocketConnection *con = ((HttpsConnection*)connection)->getWebSocketCon();
             if(con)
             con->send(msg ,len );
//             else
//             {
//                WebSocketConnection *con = ((HttpsConnection*)connection)->getWebSocketCon();
//                if(con)
//                con->send(msg ,len );
//             }
        }
         
    }
    
};

int main(int argc, char** argv) {

    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));
    //test::init();
  
    Application app;
    testwebscoket socket("0.0.0.0", 8000, new StreamingResponderFactory(), true );
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
