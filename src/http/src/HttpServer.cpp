#include "http/HttpServer.h"
#include "base/base.h"
#include "base/logger.h"
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
        static constexpr size_t MaxTcpConnectionsPerServer{ 100000};

        /* Instance methods. */

        TcpHTTPServer::TcpHTTPServer(Listener* listener, TcpHTTPConnection::Listener* connListener, std::string ip, int port)
        : TcpServerBase(BindTcp(ip, port), 256), listener(listener),
        connListener(connListener) {

        }

        TcpHTTPServer::~TcpHTTPServer() {

            if (uvHandle)
                delete uvHandle;
            //UnbindTcp(this->localIp, this->localPort);
        }

        void TcpHTTPServer::UserOnTcpConnectionAlloc(TcpConnectionBase** connection) {


            // Allocate a new RTC::TcpHTTPConnection for the TcpHTTPServer to handle it.
            *connection = new TcpHTTPConnection(this->connListener, HTTP_REQUEST, 65536);
        }

        bool TcpHTTPServer::UserOnNewTcpConnection(TcpConnectionBase* connection) {


            if (GetNumConnections() >= MaxTcpConnectionsPerServer) {
                LError("cannot handle more than %zu connections", MaxTcpConnectionsPerServer);

                return false;
            }

            return true;
        }

        void TcpHTTPServer::UserOnTcpConnectionClosed(TcpConnectionBase* connection) {

            this->listener->OnTcpConnectionClosed(this, static_cast<TcpHTTPConnection*> (connection));
        }

 /*******************************************************************************************************************/
 
        
        
        
        
        HttpServer::HttpServer(std::string ip, int port, ServerConnectionFactory *factory):ip(ip), port(port), _factory(factory) {
        }

        void HttpServer::start() {

            tcpHTTPServer = new TcpHTTPServer(this, this, ip, port);

        }

        ServerResponder* HttpServer::createResponder(TcpHTTPConnection* connection) {
            LTrace("createResponder")
                    // The initial HTTP request headers have already
                    // been parsed at this point, but the request body may
                    // be incomplete (especially if chunked).
            return _factory->createResponder(connection);
        }

        void HttpServer::shutdown() {

            delete tcpHTTPServer;
            tcpHTTPServer = nullptr;
        }

        void HttpServer::OnTcpConnectionClosed(TcpHTTPServer* /*TcpHTTPServer*/, TcpHTTPConnection* connection) {

            std::cout << "TCP server closing, LocalIP" << connection->GetLocalIp() << " PeerIP" << connection->GetPeerIp() << std::endl << std::flush;

        }

        void HttpServer::OnTcpConnectionPacketReceived(TcpHTTPConnection* connection, const uint8_t* data, size_t len) {
            std::cout << "TCP server send data: " << data << "len: " << len << std::endl << std::flush;
            std::string send = RESPONSE;
            // connection->Send((const uint8_t*) send.c_str(), send.length());// Test hello world
            connection->_parser.parse((const char*) data, len);

        }

        void HttpServer::onHeaders(TcpHTTPConnection* connection) {


            bool _upgrade = connection->_parser.upgrade();
            if (_upgrade && util::icompare(connection->_request.get("Upgrade", ""), "websocket") == 0) {
                // if (util::icompare(request().get("Connection", ""), "upgrade") == 0 &&
                //     util::icompare(request().get("Upgrade", ""), "websocket") == 0){LOG_CALL;
                LTrace("Upgrading to WebSocket: ", connection->_request)

                        // Note: To upgrade the connection we need to replace the
                        // underlying SocketAdapter instance. Since we are currently
                        // inside the default ConnectionAdapter's HTTP parser callback
                        // scope we just swap the SocketAdapter instance pointers and do
                        // a deferred delete on the old adapter. No more callbacks will be
                        // received from the old adapter after replaceAdapter is called.
                        /*  auto wsAdapter = new ws::ConnectionAdapter(this, ws::ServerSide);
                           replaceAdapter(wsAdapter);

                           // Send the handshake request to the WS adapter for handling.
                           // If the request fails the underlying socket will be closed
                           // resulting in the destruction of the current connection.

                           // std::ostringstream oss;
                           // request().write(oss);
                           // request().clear();
                           // std::string buffer(oss.str());

                           std::string buffer;
                           buffer.reserve(256);
                           request().write(buffer);
                           request().clear();

                           wsAdapter->onSocketRecv(*socket().get(), mutableBuffer(buffer), socket()->peerAddress()); */
            }

            // Notify the server the connection is ready for data flow
            //   _server.onConnectionReady(*this);

            // Instantiate the responder now that request headers have been parsed
            connection->_responder = createResponder(connection);

            // Upgraded connections don't receive the onHeaders callback
            if (connection->_responder && !_upgrade)
                connection->_responder->onHeaders(connection->_request);
        }

        void HttpServer::onPayload(TcpHTTPConnection* connection, const std::string& buffer) {
        }

        void HttpServer::onComplete(TcpHTTPConnection* connection) {

            if (connection->_responder)
                connection->_responder->onRequest(connection->_request, connection->_response);
        }

        void HttpServer::onClose(TcpHTTPConnection* connection) {

            if (connection->_responder)
                connection->_responder->onClose();
        }

        Message* HttpServer::incomingHeader(TcpHTTPConnection* connection) {
            return reinterpret_cast<Message*> (&connection->_request);
        }

        Message* HttpServer::outgoingHeader(TcpHTTPConnection* connection) {

            return reinterpret_cast<Message*> (&connection->_response);
        }


    } // namespace net
} // base