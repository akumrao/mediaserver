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



class testwebscoket: public net::HttpServer 
{
public:
    
     testwebscoket( std::string ip, int port, ServerConnectionFactory *factory = nullptr,  bool multithreaded =false) : net::HttpServer(  ip, port,  factory, multithreaded)
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
            
#if HTTPSSL
                    
             WebSocketConnection *con = ((HttpConnection*)connection)->getWebSocketCon();
#else
             WebSocketConnection *con = ((HttpConnection*)connection)->getWebSocketCon();
#endif
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

   ConsoleChannel *ch =  new ConsoleChannel("debug", Level::Info);
            
   Logger::instance().add(ch);
    //test::init();
  
    
   StreamingResponderFactory *stream =   new StreamingResponderFactory();
            
   Application app;
   testwebscoket  *socket = new testwebscoket("0.0.0.0", 1111, stream , true  );
    //socket.start();

   app.waitForShutdown([&](void*)
   {
     
        SInfo << "Main shutdwon1";
        socket->Close();
        socket->shutdown();
        delete socket;

        SInfo << "Main shutdwon";

        delete stream;

        SInfo << "Main shutdwon2";

        app.stop();
        //app.uvDestroy();
        delete ch;

    }
    
    );

    

/*Leak test  without multithreaded server
pmap -x 18321
    Total kB          322044    6112    1084
  
    Total kB          322044    6112    1084   RSS /nerver goes above 6112
 
 
 
 ==19630== LEAK SUMMARY:
==19630==    definitely lost: 0 bytes in 0 blocks
==19630==    indirectly lost: 0 bytes in 0 blocks
==19630==      possibly lost: 1,152 bytes in 4 blocks
==19630==    still reachable: 5,138 bytes in 31 blocks
==19630==         suppressed: 0 bytes in 0 blocks

  
 */  
    
/*
valgrind --leak-check=full   --show-leak-kinds=all  --track-origins=yes   ./runHttp      
valgrind --leak-check=full   --show-leak-kinds=all  --track-origins=yes  --verbose 

  total kB          469284    6244    1216
EAK SUMMARY:
==25134==    definitely lost: 0 bytes in 0 blocks
==25134==    indirectly lost: 0 bytes in 0 blocks
==25134==      possibly lost: 1,728 bytes in 6 blocks
==25134==    still reachable: 11,326 bytes in 49 blocks
==25134==         suppressed: 0 bytes in 0 blocks

*/



    return 0;

    //  test::runAll();

    // return test::finalize();
}
