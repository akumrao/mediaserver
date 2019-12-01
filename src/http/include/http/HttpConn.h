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
        class ConnectionAdapter;
        
        class TcpHTTPConnection : public TcpConnectionBase, public ParserObserver {
        public:

            class Listener {
            public:
                virtual void OnTcpConnectionPacketReceived(
                        TcpHTTPConnection* connection, const uint8_t* data, size_t len) = 0;

                virtual void onHeaders(TcpHTTPConnection* connection) = 0;


            protected:

            };

        public:
            TcpHTTPConnection(Listener* listener, http_parser_type type, size_t bufferSize = 65536);
            ~TcpHTTPConnection() override;

        public:
            void Send(const uint8_t* data, size_t len);
            size_t GetRecvBytes() const;
            size_t GetSentBytes() const;

            /* Pure virtual methods inherited from ::TcpHTTPConnection. */
        public:
            void UserOnTcpConnectionRead(const uint8_t* data, size_t len) override;

            /// HTTP Parser interface
            virtual void onParserHeader(const std::string& name, const std::string& value);
            virtual void onParserHeadersEnd(bool upgrade);
            virtual void onParserChunk(const char* buf, size_t len);
            virtual void onParserError(const base::Error& err);
            virtual void onParserEnd();

            /// HTTP connection and server interface
            virtual void onHeaders() ;
            virtual void onPayload(const uint8_t* data, size_t len); ;
            virtual void onComplete() ;
           // virtual void onClose() ;

            Message* incomingHeader() ;
            Message* outgoingHeader() ;
            
            /// Send the outdoing HTTP header.
            virtual long sendHeader();
            
             /// Return true if headers should be automatically sent.
            bool shouldSendHeader() const;

            /// Set true to prevent auto-sending HTTP headers.
            void shouldSendHeader(bool flag);
    
        private:
            // Passed by argument.
            Listener* listener{ nullptr};
            ConnectionAdapter *wsAdapter{ nullptr};
           // ws::ConnectionAdapter *wsAdapter{ nullptr};
            // Others.
            size_t frameStart{ 0}; // Where the latest frame starts.
            size_t recvBytes{ 0};
            size_t sentBytes{ 0};

        public:
            Request _request;
            Response _response;
            Parser _parser;
            ServerResponder* _responder;
        protected:    
            bool _shouldSendHeader;

        };

        inline size_t TcpHTTPConnection::GetRecvBytes() const {
            return this->recvBytes;
        }

        inline size_t TcpHTTPConnection::GetSentBytes() const {
            return this->sentBytes;
        }



    } // namespace net
} // base

#endif /* HTTPCONNECTION_H */

