/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H

#include <uv.h>
#include <string>
#include <functional>

// TcpConnection class is for RFC 4571 for RTP transport. Please do not use it other than SFU/MCU.

namespace base
{
     using onSendCallback =  std::function<void(bool sent)>;
    namespace net
    {

        // Avoid cyclic #include problem by declaring classes instead of including
        // the corresponding header files.
        class TcpServerBase;

        class TcpConnectionBase: public Listener
        {
        protected:
       

        
        public:
            
            class ListenerClose
            {
            public:
                    virtual ~ListenerClose() = default;

            public:
                    virtual void OnTcpConnectionClosed(TcpConnectionBase* connection) = 0;
            };


            struct UvWriteData
            {
                explicit UvWriteData(size_t storeSize)
                {
                    this->store = new uint8_t[storeSize];
                }

                // Disable copy constructor because of the dynamically allocated data (store).
                UvWriteData(const UvWriteData&) = delete;

                ~UvWriteData()
                {
                    delete[] this->store;
                }

                uv_write_t req;
                uint8_t* store{ nullptr };
                onSendCallback cb{ nullptr };
            };


            // Let the TcpServerBase class directly call the destructor of TcpConnectionBase.
            friend class TcpServerBase;

        public:
            explicit TcpConnectionBase(Listener *lis = nullptr, bool tls = false);
            TcpConnectionBase& operator=(const TcpConnectionBase&) = delete;
            TcpConnectionBase(const TcpConnectionBase&) = delete;
            virtual ~TcpConnectionBase();

        public:
            void Close();
            void Connect( std::string ip, int port,  addrinfo *addrs = nullptr);
            virtual void on_connect() { }
            virtual void on_read(const char* data, size_t len) {}
            virtual void on_tls_read(const char* data, size_t len){}
            virtual void on_close();
            virtual void Dump() const;
            void Setup(
                    ListenerClose* listenerClose, uv_loop_t* _loop,
                    struct sockaddr_storage* localAddr,
                    const std::string& localIp,
                    uint16_t localPort);
            bool IsClosed() const;
            uv_tcp_t* GetUvHandle() const;
            void Start();
            int Write(const char* data, size_t len,onSendCallback cb);
            int Write(const char* data1, size_t len1, const char* data2, size_t len2,onSendCallback cb);
            int Write(const std::string& data);
            void ErrorReceiving();
            const struct sockaddr* GetLocalAddress() const;
            int GetLocalFamily() const;
            const std::string& GetLocalIp() const;
            uint16_t GetLocalPort() const;
            const struct sockaddr* GetPeerAddress() const;
            const std::string& GetPeerIp() const;
            uint16_t GetPeerPort() const;

        private:
            bool SetPeerAddress();

            /* Callbacks fired by UV events. */
        public:
            void OnUvReadAlloc(size_t suggestedSize, uv_buf_t* buf);
            void OnUvRead(ssize_t nread, const uv_buf_t* buf);
            void OnUvWrite(int status,onSendCallback cb);

            void send(const char* data, size_t len) override ;
            
            int write_queue_size();

        protected:
            // Passed by argument.
            size_t bufferSize{ 65536};
            // Allocated by this.
            char* buffer{ nullptr};

            size_t bufferDataLen{ 0};
            std::string localIp;
            uint16_t localPort{ 0};
            struct sockaddr_storage peerAddr;
            std::string peerIp;
            uint16_t peerPort{ 0};
           

        public:
             ListenerClose* listenerClose{nullptr};
            size_t GetRecvBytes() const;
            size_t GetSentBytes() const;
            
            size_t recvBytes{ 0};
            size_t sentBytes{ 0};
      
        private:

            // Allocated by this.
            uv_tcp_t* uvHandle{ nullptr};
            // Others.

            struct sockaddr_storage* localAddr
            {
                nullptr
            };
            bool closed{ false};
            bool isClosedByPeer{ false};
            bool hasError{ false};
            
            bool tls;
            
            protected:
            Listener* listener{ nullptr};
            
        };

        /* Inline methods. */

        inline bool TcpConnectionBase::IsClosed() const {
            return this->closed;
        }

        inline uv_tcp_t* TcpConnectionBase::GetUvHandle() const {
            return this->uvHandle;
        }

        inline int TcpConnectionBase::Write(const std::string& data) {
           return Write(reinterpret_cast<const char*> (data.c_str()), data.size(),nullptr);
        }

        inline const struct sockaddr* TcpConnectionBase::GetLocalAddress() const {
            return reinterpret_cast<const struct sockaddr*> (this->localAddr);
        }

        inline int TcpConnectionBase::GetLocalFamily() const {
            return reinterpret_cast<const struct sockaddr*> (this->localAddr)->sa_family;
        }

        inline const std::string& TcpConnectionBase::GetLocalIp() const {
            return this->localIp;
        }

        inline uint16_t TcpConnectionBase::GetLocalPort() const {
            return this->localPort;
        }

        inline const struct sockaddr* TcpConnectionBase::GetPeerAddress() const {
            return reinterpret_cast<const struct sockaddr*> (&this->peerAddr);
        }

        inline const std::string& TcpConnectionBase::GetPeerIp() const {
            return this->peerIp;
        }

        inline uint16_t TcpConnectionBase::GetPeerPort() const {
            return this->peerPort;
        }

      /*******************************************************************************************************************************************************/
        // TcpConnection class is for RFC 4571 for RTP transport. Please do not use it other than SFU/MCU.
        class TcpConnection : public TcpConnectionBase
        {
        public:


        public:
            TcpConnection(Listener* listener, bool tls=false);
            ~TcpConnection() override;

        public:
            void send(const char* data, size_t len) override ;


            /* Pure virtual methods inherited from ::TcpConnection. */
        public:
            void on_read( const char* data, size_t len) override;
            
            void on_close() override;
            
//            const std::string& GetLocalIp() const;
//            uint16_t GetLocalPort() const;
//             const std::string& GetPeerIp() const;
//            uint16_t GetPeerPort() const;

        public:
            // Passed by argument.
          
            // Others.
        public:
            size_t frameStart{ 0}; // Where the latest frame starts.
           
        };

//        inline size_t TcpConnection::GetRecvBytes() const {
//            return this->recvBytes;
//        }
//
//        inline size_t TcpConnection::GetSentBytes() const {
//            return this->sentBytes;
//        }
//
//        
//         inline const std::string& TcpConnection::GetLocalIp() const {
//            return this->localIp;
//        }
//
//        inline uint16_t TcpConnection::GetLocalPort() const {
//            return this->localPort;
//        }
//
//    
//        inline const std::string& TcpConnection::GetPeerIp() const {
//            return this->peerIp;
//        }
//
//        inline uint16_t TcpConnection::GetPeerPort() const {
//            return this->peerPort;
//        }
        
        /*******************************************************************************************************************************************************/


    } // namespace net
} // namespace base


#endif  //TCP_CONNECTION_H
