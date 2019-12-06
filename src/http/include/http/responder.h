#ifndef HTTPRESPONDER_H
#define HTTPRESPONDER_H

#include "http/parser.h"

namespace base {
    namespace net {


     
  class ServerResponder {
        public:

            ServerResponder(HttpBase* connection) :
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

            HttpBase* connection() {
                return _connection;
            }

            Request& request() {
                return _connection->_request;
            }

            Response& response() {
                return _connection->_response;
            }

        protected:
            HttpBase* _connection;

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

            virtual ServerResponder* createResponder(HttpBase* connection) {
                return nullptr;
            }
        };


    } // namespace net
} // base


#endif // HTTPRESPONDER_H