#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "net/netInterface.h"
#include "net/TcpConnection.h"
#include <uv.h>
#include <string>
#include <unordered_set>

namespace base
{
    namespace net
    {

        class TcpServerBase 
        {
        public:
            /**
             * uvHandle must be an already initialized and binded uv_tcp_t pointer.
             */
            TcpServerBase(uv_tcp_t* uvHandle, int backlog);
            virtual ~TcpServerBase() ;

        public:
            void Close();
            virtual void Dump() const;
            const struct sockaddr* GetLocalAddress() const;
            int GetLocalFamily() const;
            const std::string& GetLocalIp() const;
            uint16_t GetLocalPort() const;
            size_t GetNumConnections() const;


            bool setNoDelay(bool enable)
            {
                return uv_tcp_nodelay(uvHandle, enable ? 1 : 0) == 0;
            }


            bool setKeepAlive(bool enable, int delay)
            {
                return uv_tcp_keepalive(uvHandle, enable ? 1 : 0, delay) == 0;
            }


            bool setSimultaneousAccepts(bool enable)
            {

                #ifdef base_WIN
                    return uv_tcp_simultaneous_accepts(uvHandle, enable ? 1 : 0) == 0;
                #else
                    return false;
                #endif
            }



        private:
            bool SetLocalAddress();

            /* Pure virtual methods that must be implemented by the subclass. */
        protected:
            virtual void UserOnTcpConnectionAlloc(TcpConnectionBase** connection) = 0;
            virtual bool UserOnNewTcpConnection(TcpConnectionBase* connection) = 0;
            virtual void UserOnTcpConnectionClosed(TcpConnectionBase* connection) = 0;

            /* Callbacks fired by UV events. */
        public:
            void OnUvConnection(int status);

            /* Methods inherited from TcpConnectionBase::Listener. */
        public:
            void OnTcpConnectionClosed(TcpConnectionBase* connection) ;

        protected:
               uv_tcp_t* BindTcp(std::string &ip, int port);
        protected:
            struct sockaddr_storage localAddr;
            std::string localIp;
            uint16_t localPort{ 0};

        private:
            // Allocated by this (may be passed by argument).
            uv_tcp_t* uvHandle{ nullptr};
            // Others.
            std::unordered_set<TcpConnectionBase*> connections;
            bool closed{ false};
        };

        /* Inline methods. */

        inline size_t TcpServerBase::GetNumConnections() const {
            return this->connections.size();
        }

        inline const struct sockaddr* TcpServerBase::GetLocalAddress() const {
            return reinterpret_cast<const struct sockaddr*> (&this->localAddr);
        }

        inline int TcpServerBase::GetLocalFamily() const {
            return reinterpret_cast<const struct sockaddr*> (&this->localAddr)->sa_family;
        }

        inline const std::string& TcpServerBase::GetLocalIp() const {
            return this->localIp;
        }

        inline uint16_t TcpServerBase::GetLocalPort() const {
            return this->localPort;
        }

        /**********************************************************************************************************/
        class TcpServer : public TcpServerBase, public Listener
        {
        public:

     
        public:
            TcpServer(Listener* listener, std::string ip, int port, bool ssl=false);

            ~TcpServer() override;

            /* Pure virtual methods inherited from ::TcpServer. */
        public:
            void UserOnTcpConnectionAlloc(TcpConnectionBase** connection) override;
            bool UserOnNewTcpConnection(TcpConnectionBase* connection) override;
            void UserOnTcpConnectionClosed(TcpConnectionBase* connection) override;

        private:
            // Passed by argument.
            Listener* listener{ nullptr};
            uv_tcp_t* uvHandle{ nullptr};
            bool ssl;
        };





    } // namespace net
} // namespace base

#endif //TCP_SERVER_H
