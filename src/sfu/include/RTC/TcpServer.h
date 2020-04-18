#ifndef MS_RTC_TCP_SERVER_HPP
#define MS_RTC_TCP_SERVER_HPP

#include "common.h"
#include "RTC/TcpConnection.h"
#include "net/TcpConnection.h"
#include "net/TcpServer.h"
#include <string>

namespace RTC
{
	class TcpServer : public base::net::TcpServerBase
        {
        public:

     
        public:
            TcpServer(Listener* listener, std::string ip);

            ~TcpServer() override;

            /* Pure virtual methods inherited from ::TcpServer. */
        public:
            void UserOnTcpConnectionAlloc(base::net::TcpConnectionBase** connection) override;
            bool UserOnNewTcpConnection(base::net::TcpConnectionBase* connection) override;
            void UserOnTcpConnectionClosed(base::net::TcpConnectionBase* connection) override;

        private:
            // Passed by argument.
            Listener* listener{ nullptr};
            uv_tcp_t* uvHandle{ nullptr};
        };
} // namespace RTC

#endif
