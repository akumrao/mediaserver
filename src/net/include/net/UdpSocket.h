#ifndef UDP_SOCKET_H
#define UDP_SOCKET_H


#include <uv.h>
#include <string>
#include "net/IP.h"

namespace base {
    namespace net {

        class UdpSocket {
        public:

            /* Struct for the data field of uv_req_t when sending a datagram. */
            struct UvSendData {
                uv_udp_send_t req;
                char store[1];
            };

        public:
            /**
             * uvHandle must be an already initialized and binded uv_udp_t pointer.
             */
            UdpSocket(std::string ip, int port);
            UdpSocket& operator=(const UdpSocket&) = delete;
            UdpSocket(const UdpSocket&) = delete;
            virtual ~UdpSocket();

        public:
            void Close();
            virtual void Dump() const;

            bool setBroadcast(bool enable) {
                return uv_udp_set_broadcast(uvHandle, enable ? 1 : 0) == 0;
            }

            bool setMulticastLoop(bool enable) {
                return uv_udp_set_multicast_loop(uvHandle, enable ? 1 : 0) == 0;
            }

            bool setMulticastTTL(int ttl) {
                ASSERT(ttl > 0 && ttl <= 255);
                return uv_udp_set_multicast_ttl(uvHandle, ttl) == 0;
            }


            void send(const char* data, unsigned int len, const struct sockaddr* add=nullptr);
           // void send(const std::string& data, const struct sockaddr* addr);
            void send(const char* data, unsigned int len, const std::string ip, int port);
            void send(const std::string& data, const std::string& ip, uint16_t port);
            const struct sockaddr* GetLocalAddress() const;
            int GetLocalFamily() const;
            const std::string& GetLocalIp() const;
            uint16_t GetLocalPort() const;
            size_t GetRecvBytes() const;
            size_t GetSentBytes() const;

            //////////////////////
            /*
            const struct sockaddr* GetPeerAddress() const;
            const std::string& GetPeerIp() const;
            uint16_t GetPeerPort() const;
            struct sockaddr_storage peerAddr;
            std::string peerIp;
            uint16_t peerPort{ 0};
            bool SetPeerAddress();
             */
            /////////////////////

        private:
           

            /* Callbacks fired by UV events. */
        public:
            void OnUvRecvAlloc(size_t suggestedSize, uv_buf_t* buf);
            void OnUvRecv(ssize_t nread, const uv_buf_t* buf,  struct sockaddr* addr, unsigned int flags);
            void OnUvSendError(int error);
            void bind();
            void connect();

            /* Pure virtual methods that must be implemented by the subclass. */
        protected:
            virtual void UserOnUdpDatagramReceived(
                    const char* data, size_t len,  struct sockaddr* addr){};
            
         void startRead();
 

        protected:
            //bool SetLocalAddress();
            
            struct sockaddr_storage localAddr;
            std::string localIp;
            uint16_t localPort{ 0};

        private:
            // Allocated by this (may be passed by argument).
            uv_udp_t* uvHandle{ nullptr};
            // Others.
            bool closed{ false};
            size_t recvBytes{ 0};
            size_t sentBytes{ 0};
        };
        


        /* Inline methods. */

     
      
        inline void UdpSocket::send(const std::string& data, const std::string& ip, uint16_t port){
            send(data.c_str(), data.length(), ip, port );
        }
        inline const struct sockaddr* UdpSocket::GetLocalAddress() const {
            return reinterpret_cast<const struct sockaddr*> (&this->localAddr);
        }

        inline int UdpSocket::GetLocalFamily() const {
            return reinterpret_cast<const struct sockaddr*> (&this->localAddr)->sa_family;
        }

        inline const std::string& UdpSocket::GetLocalIp() const {
            return this->localIp;
        }

        inline uint16_t UdpSocket::GetLocalPort() const {
            return this->localPort;
        }

        inline size_t UdpSocket::GetRecvBytes() const {
            return this->recvBytes;
        }

        inline size_t UdpSocket::GetSentBytes() const {
            return this->sentBytes;
        }

        /***********************************************************************************************************************/
        class UdpServer : public UdpSocket {
        public:

            class Listener {
            public:
                virtual void OnUdpSocketPacketReceived(
                        net::UdpServer* socket, const char* data, size_t len,  struct sockaddr* remoteAddr) = 0;
            };

        public:

      
            UdpServer(Listener* listener, std::string ip, int port);
            ~UdpServer() override;

            /* Pure virtual methods inherited from ::UdpSocket. */
        public:
            void UserOnUdpDatagramReceived(const char* data, size_t len,  struct sockaddr* addr) override;

        private:
            // Passed by argument.
            Listener* listener{ nullptr};
      
        };

        

    } // namespace net
} // namespace base
#endif //UDP_SOCKET_H
