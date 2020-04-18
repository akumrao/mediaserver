#define MS_CLASS "RTC::TcpServer"
// #define MS_LOG_DEV_LEVEL 3

#include "RTC/TcpServer.h"
#include "net/PortManager.h"
#include "LoggerTag.h"
#include <string>

namespace RTC
{
	static constexpr size_t MaxTcpConnectionsPerServer{ 100000};

        /* Instance methods. */

        TcpServer::TcpServer(Listener* listener, std::string ip)
        : base::net::TcpServerBase(base::net::PortManager::BindTcp(ip), 256), listener(listener){

        }

        TcpServer::~TcpServer() {

            if (uvHandle)
                delete uvHandle;
            base::net::PortManager::UnbindTcp(this->localIp, this->localPort);
        }

  
        void TcpServer::UserOnTcpConnectionAlloc(base::net::TcpConnectionBase** connection) {

             *connection = new RTC::TcpConnection(listener);
        }

        bool TcpServer::UserOnNewTcpConnection(base::net::TcpConnectionBase* connection) {


            if (GetNumConnections() >= MaxTcpConnectionsPerServer)
            {
                LError("cannot handle more than %zu connections", MaxTcpConnectionsPerServer);

                return false;
            }

            return true;
        }

        void TcpServer::UserOnTcpConnectionClosed(base::net::TcpConnectionBase* connection) {

            //this->listener->on_close( (TcpConnection*)connection);
        }

	

        
        
//         bool TcpServer::UserOnNewTcpConnection(base::net1::TcpConnectionBase* connection ) 
//         {
//             
//         }
 
     
             
} // namespace RTC
