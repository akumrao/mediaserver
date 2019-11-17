
#ifndef HttpServer_H
#define HttpServer_H


#include "net/TcpServer.h"

#include "http/parser.h"

namespace base {
    namespace net {

        /*******************************************************************************************************************************************************/

        class ServerResponder;

        class TcpHTTPConnection : public TcpConnectionBase, public ParserObserver {
        public:

            class Listener {
            public:
                virtual void OnTcpConnectionPacketReceived(
                        TcpHTTPConnection* connection, const uint8_t* data, size_t len) = 0;

                virtual void onHeaders(TcpHTTPConnection* connection) = 0;
                virtual void onPayload(TcpHTTPConnection* connection, const std::string& buffer) = 0;
                virtual void onComplete(TcpHTTPConnection* connection) = 0;
                virtual void onClose(TcpHTTPConnection* connection) = 0;

                virtual Message* incomingHeader(TcpHTTPConnection* connection) = 0;
                virtual Message* outgoingHeader(TcpHTTPConnection* connection) = 0;

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
             ServerResponder* _responder;

        };

        inline size_t TcpHTTPConnection::GetRecvBytes() const {
            return this->recvBytes;
        }

        inline size_t TcpHTTPConnection::GetSentBytes() const {
            return this->sentBytes;
        }

        /*******************************************************************************************************************************************************/


        class TcpHTTPServer : public TcpServerBase {
        public:

            class Listener {
            public:
                virtual void OnTcpConnectionClosed(
                        TcpHTTPServer* tcphttpServer, TcpHTTPConnection* connection) = 0;
            };

        public:
            TcpHTTPServer(Listener* listener, TcpHTTPConnection::Listener* connListener, std::string ip, int port);

            ~TcpHTTPServer() override;

            /* Pure virtual methods inherited from ::TcpHTTPServer. */
        public:
            void UserOnTcpConnectionAlloc(TcpConnectionBase** connection) override;
            bool UserOnNewTcpConnection(TcpConnectionBase* connection) override;
            void UserOnTcpConnectionClosed(TcpConnectionBase* connection) override;

        private:
            // Passed by argument.
            Listener* listener{ nullptr};
            uv_tcp_t* uvHandle{ nullptr};
            TcpHTTPConnection::Listener* connListener{ nullptr};

        protected:
           
        };

        /**********************************************************************************************************************/
        /******************************************************/
        ///

        class ServerResponder {
        public:

            ServerResponder(TcpHTTPConnection* connection) :
            _connection(connection) {
            }

            virtual ~ServerResponder() = default;

            virtual void onHeaders(Request& /* request */) {
            }

            virtual void onPayload(const std::string& /* body */) {
            }

            virtual void onRequest(Request& /* request */, Response& /* response */) {
            }

            virtual void onClose() {
            };

            TcpHTTPConnection* connection() {
                return _connection;
            }

            Request& request() {
                return _connection->_request;
            }

            Response& response() {
                return _connection->_response;
            }

        protected:
            TcpHTTPConnection* _connection;

        private:
            ServerResponder(const ServerResponder&) = delete;
            ServerResponder& operator=(const ServerResponder&) = delete;
        };


        /// This implementation of a ServerConnectionFactory
        /// is used by HTTP Server to create ServerConnection objects.

        class ServerConnectionFactory {
        public:
            ServerConnectionFactory() = default;
            virtual ~ServerConnectionFactory() = default;

            /// Factory method for instantiating the ServerConnection
            /// instance using the given Socket.
            //   virtual ServerConnection::Ptr createConnection(Server& server, const net::TCPSocket::Ptr& socket)
            // {
            //      return std::make_shared<ServerConnection>(server, socket);
            //  }

            /// Factory method for instantiating the ServerResponder
            /// instance using the given ServerConnection.

            virtual ServerResponder* createResponder(TcpHTTPConnection* connection) {
                return nullptr;
            }
        };

        /*************************************************************************************************/
        class HttpServer : public TcpHTTPServer::Listener, public TcpHTTPConnection::Listener {
        public:

            HttpServer(ServerConnectionFactory *factory= nullptr);
            
            ServerResponder* createResponder(TcpHTTPConnection* conn);

            void start();

            void shutdown();

            void OnTcpConnectionClosed(TcpHTTPServer* /*TcpHTTPServer*/, TcpHTTPConnection* connection);

            void OnTcpConnectionPacketReceived(TcpHTTPConnection* connection, const uint8_t* data, size_t len);

            void onHeaders(TcpHTTPConnection* connection);
            void onPayload(TcpHTTPConnection* connection, const std::string& buffer);
            void onComplete(TcpHTTPConnection* connection);
            void onClose(TcpHTTPConnection* connection);
            Message* incomingHeader(TcpHTTPConnection* connection);
            Message* outgoingHeader(TcpHTTPConnection* connection);

            TcpHTTPServer *tcpHTTPServer;

              ServerConnectionFactory* _factory;

        };


    } // namespace net
} // base


#endif // HttpServer_H