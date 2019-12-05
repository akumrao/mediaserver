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

//#include "http/websocket.h"



namespace base {
    namespace net {

        class ServerResponder;
        class WebSocketConnection;

        //template < class T>
        class HttpConnection : public TcpConnection, public ParserObserver {
        public:

        public:
            HttpConnection(Listener* listener, http_parser_type type, size_t bufferSize = 65536);
            ~HttpConnection() override;

        public:
            void send(const char* data, size_t len);
 

            /* Pure virtual methods inherited from ::HttpConnection. */
        public:
            void on_read(const char* data, size_t len) override;
            void on_close() override;

            /// HTTP Parser interface
            virtual void onParserHeader(const std::string& name, const std::string& value);
            virtual void onParserHeadersEnd(bool upgrade);
            virtual void onParserChunk(const char* buf, size_t len);
            virtual void onParserError(const base::Error& err);
            virtual void onParserEnd();

            /// HTTP connection and server interface
            virtual void onHeaders();
            virtual void on_payload(const char* data, size_t len);
            virtual void onComplete();
            // virtual void onClose() ;

            Message* incomingHeader();
            Message* outgoingHeader();

            /// Send the outdoing HTTP header.
            virtual long sendHeader();

            /// Return true if headers should be automatically sent.
            bool shouldSendHeader() const;

            /// Set true to prevent auto-sending HTTP headers.
            void shouldSendHeader(bool flag);

            WebSocketConnection *wsAdapter{ nullptr};

        private:
            // Passed by argument.
            Listener* listener{ nullptr};


            // ws::WebSocketConnection *wsAdapter{ nullptr};
            // Others.
            size_t frameStart{ 0}; // Where the latest frame starts.
            size_t recvBytes{ 0};
            size_t sentBytes{ 0};

        public:
            Request _request;
            Response _response;
            Parser _parser;
            http_parser_type type;
            ServerResponder* _responder;
        protected:
            bool _shouldSendHeader;

        public:

            size_t GetRecvBytes() const {
                return this->recvBytes;
            }

            size_t GetSentBytes() const {
                return this->sentBytes;
            }



        };




    } // namespace net
} // base


#endif /* HTTPCONNECTION_H */

