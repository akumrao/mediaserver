/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   HTTPConnection.h
 * Author: root
 *
 * Created on November 18, 2019, 9:42 AM
 */

#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include "net/TcpConnection.h"
#include "http/parser.h"


namespace base {
    namespace net {

        class ServerResponder;
        class WebSocketConnection;

        //template < class T>
        class HttpConnection : public TcpConnection, public HttpBase {
        public:

        public:
            HttpConnection(Listener* listener, http_parser_type type);
            ~HttpConnection() override;

        public:
            void send(const char* data, size_t len);
            void Close();

            /* Pure virtual methods inherited from ::HttpConnection. */
        public:
            void on_read(const char* data, size_t len) override;
            void on_close() override;

            /// HTTP Parser interface
          //  virtual void onParserHeader(const std::string& name, const std::string& value);
           // virtual void onParserHeadersEnd(bool upgrade);
           //// virtual void onParserChunk(const char* buf, size_t len);
           // virtual void onParserError(const base::Error& err);
           // virtual void onParserEnd();

            /// HTTP connection and server interface
            virtual void onHeaders();
            virtual void on_payload(const char* data, size_t len);
            virtual void onComplete();
         

            /// Send the outdoing HTTP header.
            virtual long sendHeader();


            WebSocketConnection *wsAdapter{ nullptr};

        private:
            // Passed by argument.
            Listener* listener{ nullptr};


        public:

           ServerResponder* _responder{nullptr};
        public:

     

        };




    } // namespace net
} // base


#endif /* HTTPCONNECTION_H */

