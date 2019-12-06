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

        HttpServerBase::HttpServerBase(Listener *listener, std::string ip, int port, bool ssl)
        : TcpServerBase( BindTcp(ip, port), 256),listener(listener),ssl(ssl)
        {

        }

        HttpServerBase::~HttpServerBase() {

            if (uvHandle)
                delete uvHandle;
            //UnbindTcp(this->localIp, this->localPort);
        }

        void HttpServerBase::UserOnTcpConnectionAlloc(TcpConnectionBase** connection) {

            LTrace(" On acccept-> UserOnTcpConnectionAlloc"  )
            // Allocate a new RTC::HttpConnection for the HttpServerBase to handle it.
            if(ssl)
            *connection = new HttpsConnection(listener, HTTP_REQUEST,  65536);
            else
            *connection = new HttpConnection(listener, HTTP_REQUEST,  65536);
               
            
        }

        bool HttpServerBase::UserOnNewTcpConnection(TcpConnectionBase* connection) {


            if (GetNumConnections() >= MaxTcpConnectionsPerServer) {
                LError("cannot handle more than %zu connections", MaxTcpConnectionsPerServer);

                return false;
            }

            return true;
        }

        void HttpServerBase::UserOnTcpConnectionClosed(TcpConnectionBase* connection) {

            //this->listener->on_close(connection);
        }

        /*******************************************************************************************************************/



        HttpServer::HttpServer( std::string ip, int port, ServerConnectionFactory *factory) 
        : HttpServerBase( this,  ip, port )
        ,ip(ip), port(port), _factory(factory)
        {
 
        }

        void HttpServer::start() {

       //     tcpHTTPServer = new HttpServerBase(this,  ip, port);

        }

        ServerResponder* HttpServer::createResponder(HttpBase* connection) {
            LTrace("createResponder")
                    // The initial HTTP request headers have already
                    // been parsed at this point, but the request body may
                    // be incomplete (especially if chunked).
            if (!_factory)
                return nullptr;

            return _factory->createResponder(connection);
        }

        void HttpServer::shutdown() {

            //delete tcpHTTPServer;
           // tcpHTTPServer = nullptr;
        }

        void HttpServer::on_close(Listener* connection) {

            STrace << "HttpServer::on_close, LocalIP" << connection->GetLocalIp() << " PeerIP" << connection->GetPeerIp() << std::endl << std::flush;

        }

       


        void HttpServer::on_read(Listener* connection, const char* data, size_t len) {
            STrace << "on_read:TCP server send data: " << data << "len: " << len << std::endl << std::flush;
            
             HttpConnection *con = (HttpConnection*)connection;
             // WebSocketConnection *con = (WebSocketConnection*)connection;
    
             //if(con->wsAdapter)
             connection->send( data , len);// Test hello world
           

        }

        void HttpServer::on_header(Listener* connection) {
              HttpConnection *con = (HttpConnection*)connection;
             // Instantiate the responder now that request headers have been parsed
            con->_responder = createResponder(con);
         
               LTrace("HttpServer::on_header" )
        }


/***********************************************************************************************/
        

        HttpsServer::HttpsServer( std::string ip, int port, ServerConnectionFactory *factory) 
        : HttpServerBase( this,  ip, port , true)
        ,ip(ip), port(port), _factory(factory)
        {
 
        }

        void HttpsServer::start() {

       //     tcpHTTPServer = new HttpServerBase(this,  ip, port);

        }

        ServerResponder* HttpsServer::createResponder(HttpBase* connection) {
            LTrace("createResponder")
                    // The initial HTTP request headers have already
                    // been parsed at this point, but the request body may
                    // be incomplete (especially if chunked).
            if (!_factory)
                return nullptr;

            return _factory->createResponder(connection);
        }

        void HttpsServer::shutdown() {

            //delete tcpHTTPServer;
           // tcpHTTPServer = nullptr;
        }

        void HttpsServer::on_close(Listener* connection) {

            STrace << "HttpsServer::on_close, LocalIP" << connection->GetLocalIp() << " PeerIP" << connection->GetPeerIp() << std::endl << std::flush;

        }

       


        void HttpsServer::on_read(Listener* connection, const char* data, size_t len) {
            STrace << "on_read:TCP server send data: " << data << "len: " << len << std::endl << std::flush;
            
             HttpsConnection *con = (HttpsConnection*)connection;
             // WebSocketConnection *con = (WebSocketConnection*)connection;
    
             //if(con->wsAdapter)
             connection->send( data , len);// Test hello world
           

        }

        void HttpsServer::on_header(Listener* connection) {
              HttpsConnection *con = (HttpsConnection*)connection;
             // Instantiate the responder now that request headers have been parsed
            con->_responder = createResponder(con);
         
               LTrace("HttpsServer::on_header" )
        }

    } // namespace net
} // base