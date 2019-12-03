
#include "net/UdpSocket.h"
#include "base/logger.h"
//#include "base/loop.h"
#include <inttypes.h>
#include "net/IP.h"
#include "base/application.h"
#include <cstring> // std::memcpy()

namespace base {
    namespace net {
        /* Static. */

        static constexpr size_t ReadBufferSize{ 65536};
        static char ReadBuffer[ReadBufferSize];

        /* Static methods for UV callbacks. */

        inline static void onAlloc(uv_handle_t* handle, size_t suggestedSize, uv_buf_t* buf) {
            auto* socket = static_cast<UdpSocket*> (handle->data);

            if (socket == nullptr)
                return;

            socket->OnUvRecvAlloc(suggestedSize, buf);
        }

        inline static void onRecv(
                uv_udp_t* handle, ssize_t nread, const uv_buf_t* buf, const struct sockaddr* addr, unsigned int flags) {
            auto* socket = static_cast<UdpSocket*> (handle->data);

            if (socket == nullptr)
                return;

            socket->OnUvRecv(nread, buf, (struct sockaddr*)addr, flags);
        }

        inline static void onSend(uv_udp_send_t* req, int status) {
            auto* sendData = static_cast<UdpSocket::UvSendData*> (req->data);
            auto* handle = req->handle;
            auto* socket = static_cast<UdpSocket*> (handle->data);

            // Delete the UvSendData struct (which includes the uv_req_t and the store char[]).
            std::free(sendData);

            if (socket == nullptr)
                return;

            // Just notify the UdpSocket when error.
            if (status != 0)
                socket->OnUvSendError(status);
        }

        inline static void onClose(uv_handle_t* handle) {
            delete handle;
        }

        /* Instance methods. */

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)

        UdpSocket::UdpSocket(std::string ip, int port) : localIp(ip), localPort(port) {

           
        }

        void UdpSocket::startRead()
        {
             int err;

            this->uvHandle->data = (void*) this;

            err = uv_udp_recv_start(
                    this->uvHandle, static_cast<uv_alloc_cb> (onAlloc), static_cast<uv_udp_recv_cb> (onRecv));

            if (err != 0) {
                uv_close(reinterpret_cast<uv_handle_t*> (this->uvHandle), static_cast<uv_close_cb> (onClose));

                LError("uv_udp_recv_start() failed: %s", uv_strerror(err));
            }

            // Set local address.
           /* if (!SetLocalAddress()) {
                uv_close(reinterpret_cast<uv_handle_t*> (this->uvHandle), static_cast<uv_close_cb> (onClose));

                LError("error setting local IP and port");
            }
            */
            
           // SetPeerAddress();
        }
        
        
        UdpSocket::~UdpSocket() {

            if (!this->closed)
                Close();
            
            if (uvHandle)
           delete uvHandle;
        }

        void UdpSocket::Close() {


            if (this->closed)
                return;

            this->closed = true;

            // Tell the UV handle that the UdpSocket has been closed.
            this->uvHandle->data = nullptr;

            // Don't read more.
            int err = uv_udp_recv_stop(this->uvHandle);

            if (err != 0)
                LError("uv_udp_recv_stop() failed: %s", uv_strerror(err));

            uv_close(reinterpret_cast<uv_handle_t*> (this->uvHandle), static_cast<uv_close_cb> (onClose));
        }

        void UdpSocket::Dump() const {
            LInfo("<UdpSocket>");
            LInfo("  [UDP, local:%s :%" PRIu16 ", status:%s]",
                    this->localIp.c_str(),
                    static_cast<uint16_t> (this->localPort),
                    (!this->closed) ? "open" : "closed");
            LInfo("</UdpSocket>");
        }

        void UdpSocket::send( const char* data, unsigned int len, const struct sockaddr* addr ) {

            if (this->closed)
                return;

            if (len == 0)
                return;
           
          //  if(!addr)
            // addr = GetLocalAddress();

            // First try uv_udp_try_send(). In case it can not directly send the datagram
            // then build a uv_req_t and use uv_udp_send().

            uv_buf_t buffer = uv_buf_init((char*)data , len);
            int sent = uv_udp_try_send(this->uvHandle, &buffer, 1, addr);

            // Entire datagram was sent. Done.
            if (sent == static_cast<int> (len)) {
                // Update sent bytes.
                this->sentBytes += sent;

                return;
            }
            if (sent >= 0) {
             //   LWarn("datagram truncated (just %d of %zu bytes were sent)", sent, len); // will cause recursion lock
                  printf("datagram truncated (just %d of %zu bytes were sent)", sent, len);
                // Update sent bytes.
                this->sentBytes += sent;

                return;
            }
            // Error,
            if (sent != UV_EAGAIN) {
              //  LWarn("uv_udp_try_send() failed: %s", uv_strerror(sent)); // will cause recursion lock
                printf("uv_udp_try_send() failed: %s", uv_strerror(sent ));
                return;
            }
            // Otherwise UV_EAGAIN was returned so cannot send data at first time. Use uv_udp_send().

            // MS_DEBUG_DEV("could not send the datagram at first time, using uv_udp_send() now");

            // Allocate a special UvSendData struct pointer.
            auto* sendData = static_cast<UvSendData*> (std::malloc(sizeof (UvSendData) + len));

            std::memcpy(sendData->store, data, len);
            sendData->req.data = (void*) sendData;

            buffer = uv_buf_init(reinterpret_cast<char*> (sendData->store), len);

            int err = uv_udp_send(
                    &sendData->req, this->uvHandle, &buffer, 1, addr, static_cast<uv_udp_send_cb> (onSend));

            if (err != 0) {
                // NOTE: uv_udp_send() returns error if a wrong INET family is given
                // (IPv6 destination on a IPv4 binded socket), so be ready.
               // LWarn("uv_udp_send() failed: %s", uv_strerror(err));// will cause recursion lock
                printf("uv_udp_send() failed: %s", uv_strerror(err));

                // Delete the UvSendData struct (which includes the uv_req_t and the store char[]).
                std::free(sendData);
            } else {
                // Update sent bytes.
                this->sentBytes += len;
            }
        }

        /*
        bool UdpSocket::SetPeerAddress() {


            int err;
            int len = sizeof (this->peerAddr);

            err = uv_udp_getpeername(this->uvHandle, reinterpret_cast<struct sockaddr*> (&this->peerAddr), &len);

            if (err != 0) {
                LError("uv_tcp_getpeername() failed: %s", uv_strerror(err));

                return false;
            }

            int family;

            IP::GetAddressInfo(
                    reinterpret_cast<struct sockaddr*> (&this->peerAddr), family, this->peerIp, this->peerPort);

            return true;
        }
        */ 
        void UdpSocket::send(const char* data, unsigned int len, const std::string ip, int port) {

            if (this->closed)
                return;

            int err;

            if (len == 0)
                return;

            struct sockaddr_storage addr; // NOLINT(cppcoreguidelines-pro-type-member-init)

            switch (IP::GetFamily(ip)) {
                case AF_INET:
                {
                    err = uv_ip4_addr(
                            ip.c_str(), static_cast<int> (port), reinterpret_cast<struct sockaddr_in*> (&addr));

                    if (err != 0)
                        LError("uv_ip4_addr() failed: %s", uv_strerror(err));

                    break;
                }

                case AF_INET6:
                {
                    err = uv_ip6_addr(
                            ip.c_str(), static_cast<int> (port), reinterpret_cast<struct sockaddr_in6*> (&addr));

                    if (err != 0)
                        LError("uv_ip6_addr() failed: %s", uv_strerror(err));

                    break;
                }

                default:
                {
                    LError("invalid destination IP '%s'", ip.c_str());

                    return;
                }
            }

            send(data, len, reinterpret_cast<struct sockaddr*> (&addr));
        }

      /*
       *   bool UdpSocket::SetLocalAddress() {


            int err;
            int len = sizeof (this->localAddr);

            err =
                    uv_udp_getsockname(this->uvHandle, reinterpret_cast<struct sockaddr*> (&this->localAddr), &len);

            if (err != 0) {
                LError("uv_udp_getsockname() failed: %s", uv_strerror(err));

                return false;
            }

            int family;

            IP::GetAddressInfo(
                    reinterpret_cast<struct sockaddr*> (&this->localAddr), family, this->localIp, this->localPort);

            return true;
        }
       */

        inline void UdpSocket::OnUvRecvAlloc(size_t /*suggestedSize*/, uv_buf_t* buf) {


            // Tell UV to write into the static buffer.
            buf->base = reinterpret_cast<char*> (ReadBuffer);
            // Give UV all the buffer space.
            buf->len = ReadBufferSize;
        }

        inline void UdpSocket::OnUvRecv(
                ssize_t nread, const uv_buf_t* buf,  struct sockaddr* addr, unsigned int flags) {


            if (this->closed)
                return;

            // NOTE: libuv calls twice to alloc & recv when a datagram is received, the
            // second one with nread = 0 and addr = NULL. Ignore it.
            if (nread == 0)
                return;

            // Check flags.
            if ((flags & UV_UDP_PARTIAL) != 0u) {
                LError("received datagram was truncated due to insufficient buffer, ignoring it");

                return;
            }

            // Data received.
            if (nread > 0) {
                // Update received bytes.
                this->recvBytes += nread;

                // Notify the subclass.
                UserOnUdpDatagramReceived(reinterpret_cast<char*> (buf->base), nread, addr);
            }// Some error.
            else {
                LTrace("read error: %s", uv_strerror(nread));
            }
        }

        inline void UdpSocket::OnUvSendError(int error) // NOLINT(misc-unused-parameters)
        {

            if (this->closed)
                return;

            LTrace("send error: %s", uv_strerror(error));
        }
        
        
       void UdpSocket::bind() {
            int bind_flags = 0;
            uvHandle = new uv_udp_t;
            struct sockaddr_in6 addr6;
            struct sockaddr_in addr;

            int r;

            r = uv_udp_init(Application::uvGetLoop(), uvHandle);
            ASSERT(r == 0);

            if (IP::GetFamily(localIp) == AF_INET6) {
                bind_flags = UV_UDP_IPV6ONLY;
                ASSERT(0 == uv_ip6_addr(localIp.c_str(), localPort, &addr6));
                r = uv_udp_bind(uvHandle, (const struct sockaddr*) &addr6, bind_flags);
                ASSERT(r == 0);
            } else {
                ASSERT(0 == uv_ip4_addr(localIp.c_str(), localPort, &addr));
                r = uv_udp_bind(uvHandle, (const struct sockaddr*) &addr, bind_flags);
                ASSERT(r == 0);

            }

            
            startRead();
        }

       
       
        void UdpSocket::connect() {

            uvHandle = new uv_udp_t;
            struct sockaddr_in6 addr6;
            struct sockaddr_in addr;

            int r;

            r = uv_udp_init(Application::uvGetLoop(), uvHandle);
            ASSERT(r == 0);

            if (IP::GetFamily(localIp) == AF_INET6) {
                ASSERT(0 == uv_ip6_addr(localIp.c_str(), localPort, &addr6));
                r = uv_udp_connect(uvHandle, (const struct sockaddr*) &addr6);
                
                ASSERT(r == 0);
                memcpy(reinterpret_cast<char *>(&localAddr),  reinterpret_cast<char *>(&addr6), sizeof(addr6));
                
                ASSERT(r == 0);
            } else {
                ASSERT(0 == uv_ip4_addr(localIp.c_str(), localPort, &addr));
                r = uv_udp_connect(uvHandle, (const struct sockaddr*) &addr);
               // ASSERT(r == 0);
                  
                memcpy(reinterpret_cast<char *>(&localAddr),  reinterpret_cast<char *>(&addr), sizeof(addr));
            }

        }
        /**********************************************************************************************************/
        UdpServer::UdpServer(Listener* listener, std::string ip, int port)
        : UdpSocket(ip, port), listener(listener) {

        }

     
        UdpServer::~UdpServer() {

        }

        void UdpServer::UserOnUdpDatagramReceived(const char* data, size_t len,  struct sockaddr* addr) {

            if (this->listener == nullptr) {
                LError("no listener set");

                return;
            }

            // Notify the reader.
            this->listener->OnUdpSocketPacketReceived(this, data, len, addr);
        }

        /**********************************************************************************************************/
   

    } // namespace net


} // namespace base
