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
#include "base/base.h"
#include "base/logger.h"
#include "base/util.h"
//#include "base/application.h"


#define RESPONSE                  \
 "HTTP/1.1 200 OK\r\n"           \
 "Content-Type: text/plain\r\n"  \
 "Content-Length: 14\r\n"        \
 "\r\n"                          \
 "Hello, World!\n"


namespace base {
    namespace net {




        /*************************************************************************************************************/
        /******************************************************************************************************************/
        static constexpr size_t MaxTcpConnectionsPerServer{ 1000};

        /* Instance methods. */

        HttpServerBase::HttpServerBase(Listener *listener, std::string ip, int port, bool multithreaded,  bool ssl)
        : TcpServerBase( BindTcp(ip, port), 256, multithreaded),listener(listener),ssl(ssl)
        {

        }

        HttpServerBase::~HttpServerBase() {

            if (uvHandle)
                delete uvHandle;
            //UnbindTcp(this->localIp, this->localPort);
        }

        void HttpServerBase::UserOnTcpConnectionAlloc(TcpConnectionBase** connection) {

            LTrace(" On acccept-> UserOnTcpConnectionAlloc"  )
            // Allocate a new RTC::HttpConnection for the HttpServerBase to handle it.
#if HTTPSSL
            if(ssl)
            *connection = new HttpsConnection(listener, HTTP_REQUEST);
            else
#endif
            *connection = new HttpConnection(listener, HTTP_REQUEST);
               
            
        }

        bool HttpServerBase::UserOnNewTcpConnection(TcpConnectionBase* connection) {


            if (GetNumConnections() >= MaxTcpConnectionsPerServer) {
                LError("cannot handle more than %zu connections", MaxTcpConnectionsPerServer);

                return false;
            }

            return true;
        }

        void HttpServerBase::UserOnTcpConnectionClosed(TcpConnectionBase* connection) {

            //override this function

        }

        /*******************************************************************************************************************/



        HttpServer::HttpServer( std::string ip, int port, ServerConnectionFactory *factory, bool multithreaded ) 
        : HttpServerBase( this,  ip, port, multithreaded )
        ,ip(ip), port(port), _factory(factory)
        {
 
        }

        void HttpServer::start() {

       //     tcpHTTPServer = new HttpServerBase(this,  ip, port);

        }

        ServerResponder* HttpServer::createResponder(HttpBase* connection) {
            LTrace("createResponder")
                    // The initial HTTP request headers have already
                    // been parsed at this point, but the request body may
                    // be incomplete (especially if chunked).
            if (!_factory)
                return nullptr;

            return _factory->createResponder(connection);
        }

        void HttpServer::shutdown() {

            //delete tcpHTTPServer;
           // tcpHTTPServer = nullptr;
        }

        void HttpServer::on_close(Listener* connection) {
	     //override this function

            TcpConnectionBase *con = (TcpConnectionBase*)connection;
            STrace << "HttpServer::on_close, LocalIP" << con->GetLocalIp() << " PeerIP" << con->GetPeerIp() << std::endl << std::flush;
          
        }

       


        void HttpServer::on_read(Listener* connection, const char* BODY, size_t len) {
            STrace  << BODY << std::endl << std::flush;
            
             HttpConnection *con = (HttpConnection*)connection;
             // WebSocketConnection *con = (WebSocketConnection*)connection;
    
             //if(con->wsAdapter)
          //   connection->send( data , len);// Test hello world
           
           // size_t sz =  multipartparser_execute(&parser, &g_callbacks, BODY, len); 

             // assert(multipartparser_execute(&parser, &callbacks, BODY, len) == len);
        }

        
        void HttpServer::on_header(Listener* connection) {
              HttpConnection *con = (HttpConnection*)connection;
              
              
                  auto& request = con->_request;

                 // Log incoming requests
                    STrace << "Incoming connection from " << ": URI:\n" << request.getURI() << ": Request:\n" << request << std::endl;
                  /*  
                   bool b =  request.has("boundary");
        
                  std::string df =  request.getContentType();
                  LTrace(df)
                  std::string df1 =  request.get("Content-Type");
                   LTrace(df1)
                           
                 // Base_API std::vector<std::string> split(const std::string& str, const std::string& delim, int limit = -1);
                           
                  std::vector<std::string> str =  util::split( df1, ";");
                   
                    LTrace(str[0])
                    LTrace(str[1])
                    
                    std::vector<std::string> str1 =  util::split( str[1], "=");
              
                  
                   multipartparser_callbacks_init(&callbacks); // It only sets all callbacks to NULL.
                    callbacks.on_body_begin = &on_body_begin;
                    callbacks.on_part_begin = &on_part_begin;
                    callbacks.on_header_field = &on_header_field;
                    callbacks.on_header_value = &on_header_value;
                    callbacks.on_headers_complete = &on_headers_complete;
                    callbacks.on_data = &on_data;
                    callbacks.on_part_end = &on_part_end;
                    callbacks.on_body_end = &on_body_end;

                    multipartparser_init(&parser, str1[1].c_str());
                    parser.data = this;
                    */

                  
             // Instantiate the responder now that request headers have been parsed
                con->_responder = createResponder(con);
         
               LTrace("HttpServer::on_header" )
        }


/***********************************************************************************************/
        
#if HTTPSSL
        HttpsServer::HttpsServer( std::string ip, int port, ServerConnectionFactory *factory,bool multithreaded  ) 
        : HttpServerBase( this,  ip, port ,multithreaded, true)
        ,ip(ip), port(port), _factory(factory)
        {
 
        }

        void HttpsServer::start() {

       //     tcpHTTPServer = new HttpServerBase(this,  ip, port);

        }

        ServerResponder* HttpsServer::createResponder(HttpBase* connection) {
            LTrace("createResponder")
                    // The initial HTTP request headers have already
                    // been parsed at this point, but the request body may
                    // be incomplete (especially if chunked).
            if (!_factory)
                return nullptr;

            return _factory->createResponder(connection);
        }

        void HttpsServer::shutdown() {

            //delete tcpHTTPServer;
           // tcpHTTPServer = nullptr;
        }

        void HttpsServer::on_close(Listener* connection) {

            STrace << "HttpsServer::on_close, LocalIP" << connection->GetLocalIp() << " PeerIP" << connection->GetPeerIp() << std::endl << std::flush;

        }

       


        void HttpsServer::on_read(Listener* connection, const char* data, size_t len) {
            STrace << "on_read:TCP server send data: " << data << "len: " << len << std::endl << std::flush;
            
            // HttpsConnection *con = (HttpsConnection*)connection;
             // WebSocketConnection *con = (WebSocketConnection*)connection;
    
             //if(con->wsAdapter)
            // connection->send( data , len);// Test hello world
           

        }

        void HttpsServer::on_header(Listener* connection) {
              HttpsConnection *con = (HttpsConnection*)connection;
             // Instantiate the responder now that request headers have been parsed
            con->_responder = createResponder(con);
         
               LTrace("HttpsServer::on_header" )
        }
#endif

    } // namespace net
} // base
