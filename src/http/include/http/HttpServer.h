
#ifndef HttpServer_H
#define HttpServer_H

#include "http/HttpServer.h"
#include "http/HttpConn.h"
#include "net/TcpServer.h"
#include "http/parser.h"
#include "http/responder.h"

namespace base {
    namespace net {

        /*******************************************************************************************************************************************************/

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

        /*************************************************************************************************/
        class HttpServer : public TcpHTTPServer::Listener, public TcpHTTPConnection::Listener {
        public:

            HttpServer(std::string ip, int port, ServerConnectionFactory *factory = nullptr);

            ServerResponder* createResponder(TcpHTTPConnection* conn);

            void start();

            void shutdown();

            void OnTcpConnectionClosed(TcpHTTPServer* /*TcpHTTPServer*/, TcpHTTPConnection* connection);

            void OnTcpConnectionPacketReceived(TcpHTTPConnection* connection, const uint8_t* data, size_t len);

            void onHeaders(TcpHTTPConnection* connection);
      

            TcpHTTPServer *tcpHTTPServer;

            ServerConnectionFactory* _factory;
            
        protected:
            std::string ip; int port;

        };


    } // namespace net
} // base


#endif // HttpServer_H