/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


#ifndef HttpServer_H
#define HttpServer_H
#include "net/netInterface.h"
#include "http/HttpServer.h"
#include "http/HttpConn.h"
#include "http/HttpsConn.h"
#include "net/TcpServer.h"
#include "http/parser.h"
#include "http/responder.h"
#include "http/websocket.h"
//#include "multipartparser.h"


namespace base {
    namespace net {

        /*******************************************************************************************************************************************************/

        /*******************************************************************************************************************************************************/


        class HttpServerBase : public TcpServerBase  {
        public:

        public:
            HttpServerBase(Listener *listener, std::string ip, int port , bool multithreaded =false, bool ssl=false );

            ~HttpServerBase() override;

            /* Pure virtual methods inherited from ::HttpServerBase. */
        public:
            void UserOnTcpConnectionAlloc(TcpConnectionBase** connection) override;
            bool UserOnNewTcpConnection(TcpConnectionBase* connection) override;
            void UserOnTcpConnectionClosed(TcpConnectionBase* connection) override;

        private:
            // Passed by argument.
            Listener* listener{ nullptr};
            uv_tcp_t* uvHandle{ nullptr};
            //HttpConnection::Listener* connListener{ nullptr};
           // WebSocketConnection::Listener* wsConListener{ nullptr};

        protected:
            
            bool ssl;
 
        };

        /**********************************************************************************************************************/
        /******************************************************/
        ///

        /*************************************************************************************************/
        class HttpServer : public HttpServerBase {
        public:

            HttpServer( std::string ip, int port, ServerConnectionFactory *factory = nullptr, bool multithreaded =false);

            ServerResponder* createResponder(HttpBase* conn);

            void start();

            void shutdown();

            void on_close(Listener* connection);

            void on_read(Listener* connection, const char* data, size_t len);
                     
            void on_header(Listener* connection);
      

           // HttpServerBase *tcpHTTPServer;

            ServerConnectionFactory* _factory;
            
        protected:
            std::string ip; int port;
            //Listener* listener{ nullptr};
            
            
           // multipartparser_callbacks callbacks;
           // multipartparser           parser;

        };


        /*************************************************************************************************/
        class HttpsServer : public HttpServerBase {
        public:

            HttpsServer( std::string ip, int port, ServerConnectionFactory *factory = nullptr);

            ServerResponder* createResponder(HttpBase* conn);

            void start();

            void shutdown();

            void on_close(Listener* connection);

            void on_read(Listener* connection, const char* data, size_t len);
                     
            void on_header(Listener* connection);
      

           // HttpServerBase *tcpHTTPServer;

            ServerConnectionFactory* _factory;
            
        protected:
            std::string ip; int port;
            //Listener* listener{ nullptr};

        };
    } // namespace net
} // base


#endif // HttpServer_H
