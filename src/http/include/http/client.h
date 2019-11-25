///
//
// LibSourcey
// Copyright (c) 2005, Sourcey <https://sourcey.com>
//
// SPDX-License-Identifier: LGPL-2.1+
//
/// @addtogroup http
/// @{


#ifndef SCY_HTTP_Client_H
#define SCY_HTTP_Client_H


#include "net/TcpConnection.h"
#include "http/parser.h"
#include "http/url.h"

namespace base {
    namespace net {

        class ClientConnection : public TcpConnectionBase, public ParserObserver {
        public:

            class Listener {
            public:
                virtual void OnTcpConnectionPacketReceived(
                        ClientConnection* connection, const uint8_t* data, size_t len) = 0;

              //  virtual void onHeaders(ClientConnection* connection) = 0;


            protected:

            };

        public:
            ClientConnection(Listener* listener, const URL& url, http_parser_type type = HTTP_RESPONSE, size_t bufferSize = 65536);


            ~ClientConnection() override;
            void connect();

            //              ClientConnection::Ptr createConnection(const URL& url)
        public:
            virtual void Send(const uint8_t* data, size_t len);
            virtual void Send();
            virtual void Send(Request& req);
            
            void on_connect();
            void on_close();


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
            virtual void onHeaders();
            virtual void onPayload(const std::string& buffer);
            virtual void onComplete();
            std::function<void(const std::string&)> fnPayload; ///< Signals when raw data is received
            std::function<void(const Response&)> fnComplete;     ///< Signals when the HTTP transaction is complete
            std::function<void(ClientConnection&)> fnClose;          
          
       
       
            Message* incomingHeader();
            Message* outgoingHeader();

            /// Send the outdoing HTTP header.
            virtual long sendHeader();

            /// Return true if headers should be automatically sent.
            bool shouldSendHeader() const;

            /// Set true to prevent auto-sending HTTP headers.
            void shouldSendHeader(bool flag);

            void setReadStream(std::ostream* os);

        private:
            // Passed by argument.
            Listener* listener{ nullptr};
            // Others.
            size_t frameStart{ 0}; // Where the latest frame starts.
            size_t recvBytes{ 0};
            size_t sentBytes{ 0};

        public:
            Request _request;
            Response _response;
            Parser _parser;


            URL _url;
            bool _connect;
            bool _active;
            bool _complete;
            std::vector<std::string> _outgoingBuffer;
            std::unique_ptr<std::ostream> _readStream;


        protected:
            bool _shouldSendHeader;

        };









        //
        // HTTP Connection Helpers
        //

        /*
        template <class ConnectionT>
        inline ClientConnection::Ptr createConnectionT(const URL& url, uv::Loop* loop = uv::defaultLoop())
        {
            ClientConnection::Ptr conn;

            if (url.scheme() == "http") {
                conn = std::make_shared<ConnectionT>(url, std::make_shared<net::TCPSocket>(loop));
                // conn->replaceAdapter(new ConnectionAdapter(conn, HTTP_RESPONSE));
                // conn = std::shared_ptr<ConnectionT>(
                //     new ConnectionT(url, std::make_shared<net::TCPSocket>(loop)),
                //     deleter::Deferred<ConnectionT>());
            } else if (url.scheme() == "https") {
                conn = std::make_shared<ConnectionT>(url, std::make_shared<net::SSLSocket>(loop));
                // conn->replaceAdapter(new ConnectionAdapter(conn, HTTP_RESPONSE));
                // conn = std::shared_ptr<ConnectionT>(
                //    new ConnectionT(url, std::make_shared<net::SSLSocket>(loop)),
                //     deleter::Deferred<ConnectionT>());
            } else if (url.scheme() == "ws") {
                conn = std::make_shared<ConnectionT>(url, std::make_shared<net::TCPSocket>(loop));
                conn->replaceAdapter(new ws::ConnectionAdapter(conn.get(), ws::ClientSide));
                // conn = std::shared_ptr<ConnectionT>(
                //     new ConnectionT(url, std::make_shared<net::TCPSocket>(loop)),
                //    deleter::Deferred<ConnectionT>());
            } else if (url.scheme() == "wss") {
                conn = std::make_shared<ConnectionT>(url, std::make_shared<net::SSLSocket>(loop));
                conn->replaceAdapter(new ws::ConnectionAdapter(conn.get(), ws::ClientSide));
                // conn = std::shared_ptr<ConnectionT>(
                //     new ConnectionT(url, std::make_shared<net::SSLSocket>(loop)),
                //     deleter::Deferred<ConnectionT>());
            } else
                throw std::runtime_error("Unknown connection type for URL: " + url.str());

            return conn;
        }
         */

        //
        // HTTP Client
        //

        class Client : public ClientConnection::Listener {
        public:

            Client(URL url);
            ~Client();

            void start();

            void shutdown();

            void OnTcpConnectionClosed(ClientConnection* connection);

            void OnTcpConnectionPacketReceived(ClientConnection* connection, const uint8_t* data, size_t len);

            //void onHeaders(ClientConnection* connection);

            ClientConnection *clientConn;

        protected:
            
            URL _url;
          
        };
    } // namespace net
} // base

#endif

