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
            if(!_factory)
                return nullptr;
            
            return _factory->createResponder(connection);
        }

        void HttpServer::shutdown() {

            delete tcpHTTPServer;
            tcpHTTPServer = nullptr;
        }

        void HttpServer::OnTcpConnectionClosed(TcpHTTPServer* /*TcpHTTPServer*/, TcpHTTPConnection* connection) {

             std::cout << "HttpServer server closing, LocalIP" << connection->GetLocalIp() << " PeerIP" << connection->GetPeerIp() << std::endl << std::flush;

            if (connection && connection->_responder)
            {
                connection->_responder->onClose();
            }
           
        }

        void HttpServer::OnTcpConnectionPacketReceived(TcpHTTPConnection* connection, const uint8_t* data, size_t len) {
            std::cout << "TCP server send data: " << data << "len: " << len << std::endl << std::flush;
            std::string send = RESPONSE;
            // connection->Send((const uint8_t*) send.c_str(), send.length());// Test hello world
            connection->_parser.parse((const char*) data, len);

        }

        void HttpServer::onHeaders(TcpHTTPConnection* connection) {

            // Instantiate the responder now that request headers have been parsed
            connection->_responder = createResponder(connection);

          
        }

       


    } // namespace net
} // base