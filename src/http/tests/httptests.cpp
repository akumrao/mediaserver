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

bool endswith(std::string const& value, std::string const& search){
    if (value.length() >= search.length())
    {
        return (0 == value.compare(value.length() - search.length(), search.length(), search));
    } else
    {
        return false;
    }
}



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


class HttpResponder : public net::ServerResponder
/// Basic server responder (make echo?)
{
public:

    HttpResponder(net::HttpBase* conn) :
    ServerResponder(conn) {
        STrace << "BasicResponder" << std::endl;
    }

    virtual void onClose() {
        ;
        LDebug("On close")

    }

    void onRequest(net::Request& request, net::Response& response) {
        STrace << "On complete" << std::endl;

        
         auto& request1 = connection()->_request;

        // Log incoming requests
        SInfo  << ": response:\n" << response << std::endl;
        
        
      //  response.setContentLength(14); // headers will be auto flushed

        //connection()->send((const char *) "hello universe", 14);
       // connection()->Close();
         std::string file_to_open;
                 
        if(request1.getURI() == "/")
        {
            file_to_open= "./index.html";
        }
        else
        {
             file_to_open= "." + request1.getURI() ;
        }
        

        
        SInfo << "Response file Path: " << file_to_open << std::endl;
        
        std::string result;
        FILE * f = fopen(file_to_open.c_str(), "rb");
        if (f)
        {
            std::fseek(f, 0, SEEK_END);
            unsigned size = std::ftell(f);
            std::fseek(f, 0, SEEK_SET);
            result.resize(size);
            std::fread(&result[0], size, 1, f);

            //std::cout << &closure->result[0] << std::endl;

            fclose(f);
//           
            if (endswith(file_to_open, "html"))
            {
                 response.setContentType( "text/html"); 
            } else if (endswith(file_to_open, "css"))
            {
                response.setContentType( "text/css"); 
            } else if (endswith(file_to_open, "js"))
            {
               response.setContentType( "application/javascript");
            }
            else if (endswith(file_to_open, "ico"))
            {
               response.setContentType( "image/x-icon");
            }
             else if (endswith(file_to_open, "gif"))
            {
               response.setContentType( "image/gif");
            }
             else if (endswith(file_to_open, "jpeg"))
            {
               response.setContentType( "image/jpeg");
            }
             else if (endswith(file_to_open, "png"))
            {
               response.setContentType( "image/png");
            }else
            {
                 SError << "file format not supported " << file_to_open;
            }


            
            response.setContentLength(size); // headers will be auto flushed
             

             connection()->send(result.c_str(), size);
             connection()->Close();
            
        }
        else
        {
             response.setStatusAndReason(StatusCode::BadRequest, "File not found" );
             
             response.setContentLength(file_to_open.length()); // headers will be auto flushed

             connection()->send((const char *) file_to_open.c_str(), file_to_open.length());
             connection()->Close();
            
        }
        
        
        
    }
};

class StreamingResponderFactory : public ServerConnectionFactory {
public:

        ServerResponder* createResponder(HttpBase* conn) {
        
         auto& request = conn->_request;

        // Log incoming requests
        STrace << "Incoming connection from " << ": Request:\n" << request << std::endl;
        
        SInfo << "Incoming connection from: " <<   request.getHost() << " method: " <<  request.getMethod()<<  " uri: <<  " << request.getURI()  << std::endl;

         // Handle websocket connections
        if (request.getMethod() == "GET" ) {   // || request.has("Sec-WebSocket-Key")) {
            return new HttpResponder(conn);
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
    
     testwebscoket( std::string ip, int port, ServerConnectionFactory *factory = nullptr): net::HttpServer(  ip, port,  factory)
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
      
        //connection->send("arvind", 6 );
        

        
        SInfo << "No of Connectons " << this->GetNumConnections();
        
        for (auto* connection :  this->GetConnections())
        {
             WebSocketConnection *con = ((HttpConnection*)connection)->getWebSocketCon();
             if(con)
             con->send(msg ,len );
        }
      
        
         
    }
    
};

int main(int argc, char** argv) {

    Logger::instance().add(new ConsoleChannel("debug", Level::Info));
    //test::init();
  
        Application app;
        testwebscoket socket("0.0.0.0", 8000, new StreamingResponderFactory() );
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
