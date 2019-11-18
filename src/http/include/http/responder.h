#ifndef HTTPRESPONDER_H
#define HTTPRESPONDER_H



namespace base {
    namespace net {



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


    } // namespace net
} // base


#endif // HTTPRESPONDER_H