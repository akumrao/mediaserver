

#include "net/TcpServer.h"
#include "base/logger.h"
#include "base/application.h"
#include "net/PortManager.h"
#include <inttypes.h>
#include "net/IP.h"
#include "net/SslConnection.h"

namespace base
{
    namespace net
    {

        /* Static methods for UV callbacks. */

        inline static void onConnection(uv_stream_t* handle, int status) {
            auto* server = static_cast<TcpServerBase*> (handle->data);

            if (server == nullptr)
                return;

            server->OnUvConnection(status);
        }

        inline static void onClose(uv_handle_t* handle) {
            delete handle;
        }

        /* Instance methods. */

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)

        TcpServerBase::TcpServerBase(uv_tcp_t* uvHandle, int backlog) : uvHandle(uvHandle) {

            int err;

            this->uvHandle->data = (void*) this;

            err = uv_listen(
                    reinterpret_cast<uv_stream_t*> (this->uvHandle),
                    backlog,
                    static_cast<uv_connection_cb> (onConnection));

            if (err != 0)
            {
                uv_close(reinterpret_cast<uv_handle_t*> (this->uvHandle), static_cast<uv_close_cb> (onClose));

                LError("uv_listen() failed: %s", uv_strerror(err));
            }

            // Set local address.
            if (!SetLocalAddress())
            {
                uv_close(reinterpret_cast<uv_handle_t*> (this->uvHandle), static_cast<uv_close_cb> (onClose));

                LError("error setting local IP and port");
            }
        }

        TcpServerBase::~TcpServerBase() {


            if (!this->closed)
                Close();
        }

        void TcpServerBase::Close() {


            if (this->closed)
                return;

            this->closed = true;

            // Tell the UV handle that the TcpServerBase has been closed.
            this->uvHandle->data = nullptr;

            LDebug("closing %zu active connections", this->connections.size());

            for (auto* connection : this->connections)
            {
                delete connection;
            }

            uv_close(reinterpret_cast<uv_handle_t*> (this->uvHandle), static_cast<uv_close_cb> (onClose));
        }

        void TcpServerBase::Dump() const {
            LDebug("<TcpServerBase>");
            LDebug(
                    "  [TCP, local:%s :%" PRIu16 ", status:%s, connections:%zu]",
                    this->localIp.c_str(),
                    static_cast<uint16_t> (this->localPort),
                    (!this->closed) ? "open" : "closed",
                    this->connections.size());
            LDebug("</TcpServerBase>");
        }

        bool TcpServerBase::SetLocalAddress() {


            int err;
            int len = sizeof (this->localAddr);

            err =
                    uv_tcp_getsockname(this->uvHandle, reinterpret_cast<struct sockaddr*> (&this->localAddr), &len);

            if (err != 0)
            {
                LError("uv_tcp_getsockname() failed: %s", uv_strerror(err));

                return false;
            }

            int family;

            IP::GetAddressInfo(
                    reinterpret_cast<struct sockaddr*> (&this->localAddr), family, this->localIp, this->localPort);

            return true;
        }

        inline void TcpServerBase::OnUvConnection(int status) {


            if (this->closed)
                return;

            int err;

            if (status != 0)
            {
                LError("error while receiving a new TCP connection: %s", uv_strerror(status));

                return;
            }

            // Notify the subclass so it provides an allocated derived class of TCPConnection.
            TcpConnectionBase* connection = nullptr;
            UserOnTcpConnectionAlloc(&connection);

            ASSERT(connection != nullptr);

            try
            {
                connection->Setup( &(this->localAddr), this->localIp, this->localPort);
            } catch (const std::exception& error)
            {
                delete connection;

                return;
            }

            // Accept the connection.
            err = uv_accept(
                    reinterpret_cast<uv_stream_t*> (this->uvHandle),
                    reinterpret_cast<uv_stream_t*> (connection->GetUvHandle()));

            if (err != 0)
                LError("uv_accept() failed: %s", uv_strerror(err));

            // Start receiving data.
            try
            {
                // NOTE: This may throw.
                connection->Start();
            } catch (const std::exception& error)
            {
                delete connection;

                return;
            }

            // Notify the subclass and delete the connection if not accepted by the subclass.
            if (UserOnNewTcpConnection(connection))
                this->connections.insert(connection);
            else
                delete connection;
        }

        inline void TcpServerBase::OnTcpConnectionClosed(TcpConnectionBase* connection) {


            LDebug("TcpServerBase connection closed");

            // Remove the TcpConnectionBase from the set.
            this->connections.erase(connection);

            // Notify the subclass.
            UserOnTcpConnectionClosed(connection);

            // Delete it.
            delete connection;
        }
        
        uv_tcp_t* TcpServerBase::BindTcp(std::string &ip, int port) {
	    
	    if(port == -1)
            {
                return PortManager::BindTcp(ip);
            }
            int bind_flags = 0;
            uv_tcp_t *uvHandle = new uv_tcp_t;
            struct sockaddr_in6 addr6;
            struct sockaddr_in addr;
         
            int r;

            r = uv_tcp_init(Application::uvGetLoop(), uvHandle);
            ASSERT(r == 0);

            if (IP::GetFamily(ip) == AF_INET6)
            {
                bind_flags = UV_TCP_IPV6ONLY;
                ASSERT(0 == uv_ip6_addr(ip.c_str(), port, &addr6));
                r = uv_tcp_bind(uvHandle, (const struct sockaddr*) &addr6, bind_flags);
                ASSERT(r == 0);
            } else
            {
                ASSERT(0 == uv_ip4_addr(ip.c_str(), port, &addr));
                r = uv_tcp_bind(uvHandle, (const struct sockaddr*) &addr, bind_flags);
                ASSERT(r == 0);

            }
            
            LTrace("Binded to port ", ip , ":", port);

            return uvHandle;
        }

        /******************************************************************************************************************/
        static constexpr size_t MaxTcpConnectionsPerServer{ 100000};

        /* Instance methods. */

        TcpServer::TcpServer(Listener* listener, std::string ip, int port, bool ssl)
        : TcpServerBase(BindTcp(ip, port), 256), listener(listener),ssl(ssl){

        }

        TcpServer::~TcpServer() {

            if (uvHandle)
                delete uvHandle;
            //UnbindTcp(this->localIp, this->localPort);
        }

  
        void TcpServer::UserOnTcpConnectionAlloc(TcpConnectionBase** connection) {

// condition
            // Allocate a new RTC::TcpConnection for the TcpServer to handle it.
            if(ssl)
             *connection = new SslConnection(listener, true);
            else
            *connection = new TcpConnection(listener);
            
            
        }

        bool TcpServer::UserOnNewTcpConnection(TcpConnectionBase* connection) {


            if (GetNumConnections() >= MaxTcpConnectionsPerServer)
            {
                LError("cannot handle more than %zu connections", MaxTcpConnectionsPerServer);

                return false;
            }

            return true;
        }

        void TcpServer::UserOnTcpConnectionClosed(TcpConnectionBase* connection) {

            //this->listener->on_close( (TcpConnection*)connection);
        }

    } // namespace net
} // namespace base
