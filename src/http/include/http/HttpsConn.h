/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


/* 
 * File:   HTTPConnection.h
 * Author: root
 *
 * Created on November 18, 2019, 9:42 AM
 */

#ifndef HTTPSCONNECTION_H
#define HTTPSCONNECTION_H

//#include "net/TcpConnection.h"

#include "net/SslConnection.h"
#include "http/parser.h"


namespace base {
    namespace net {

        class ServerResponder;
        class WebSocketConnection;

        class HttpsConnection : public SslConnection, public HttpBase {
        public:

        public:
            HttpsConnection(Listener* listener, http_parser_type type);
            ~HttpsConnection() override;

        public:
            void send(const char* data, size_t len, bool binary=false) override;
            void tcpsend(const char* data, size_t len) override;
            
            void Close() override;

            /* Pure virtual methods inherited from ::HttpsConnection. */
        public:
            void on_read(const char* data, size_t len) override;
            void on_close() override;
/*
            /// HTTP Parser interface
            virtual void onParserHeader(const std::string& name, const std::string& value);
            virtual void onParserHeadersEnd(bool upgrade);
            virtual void onParserChunk(const char* buf, size_t len);
            virtual void onParserError(const base::Error& err);
            virtual void onParserEnd();
*/
            /// HTTP connection and server interface
            virtual void onHeaders() override;
            virtual void on_payload(const char* data, size_t len) override;
            virtual void onComplete() override;
            // virtual void onClose() ;

         //   Message* incomingHeader();
         //   Message* outgoingHeader();

            /// Send the outdoing HTTP header.
            virtual long sendHeader() ;

            WebSocketConnection* getWebSocketCon()
            {
               return wsAdapter;
            }
           

            

        private:
            // Passed by argument.
            Listener* listener{ nullptr};
            WebSocketConnection *wsAdapter{ nullptr};


        public:

            ServerResponder* _responder{nullptr};
        protected:
           
        

        };




    } // namespace net
} // base


#endif /* HTTPSCONNECTION_H */

