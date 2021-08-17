/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include "net/netInterface.h"
#include "net/TcpConnection.h"
#include <uv.h>
#include <string>
#include <unordered_set>

//#include <mutex>

namespace base
{
    namespace net
    {
        
        struct child_worker {
            //uv_process_t req;
          ////  uv_process_options_t options;
            uv_pipe_t pipe;
            uv_thread_t thread;

            uv_loop_t *loppworker{nullptr};
            uv_pipe_t queue;

            uv_os_sock_t fds[2];
            
            TcpServerBase *obj{nullptr};;

        } ;//*workers;

        class TcpServerBase : public Listener, public TcpConnectionBase::ListenerClose
        {
        public:
            /**
             * uvHandle must be an already initialized and binded uv_tcp_t pointer.
             */
            TcpServerBase(uv_tcp_t* uvHandle, int backlog, bool multiThreaded=false);
            virtual ~TcpServerBase() ;

        public:
            void Close();
            virtual void Dump() const;
            const struct sockaddr* GetLocalAddress() const;
            int GetLocalFamily() const;
            const std::string& GetLocalIp() const;
            uint16_t GetLocalPort() const;
            size_t GetNumConnections() const;
	    std::unordered_set<TcpConnectionBase*>& GetConnections();	
            
            
            void setup_workers();
             

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
            
            child_worker *workers{nullptr};

            /* Pure virtual methods that must be implemented by the subclass. */
        public:
            virtual void UserOnTcpConnectionAlloc(TcpConnectionBase** connection) = 0;
            virtual bool UserOnNewTcpConnection(TcpConnectionBase* connection) = 0;
            virtual void UserOnTcpConnectionClosed(TcpConnectionBase* connection) = 0;

            /* Callbacks fired by UV events. */
        public:
            void OnUvConnection(uv_stream_t* uvh, int status);

            /* Methods inherited from TcpConnectionBase::Listener. */
        public:
            void OnTcpConnectionClosed(TcpConnectionBase* connection) ;
            void worker_connection( uv_loop_t *loppworker, uv_stream_t *q);

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
            
            bool multithreaded{false};
            
            int round_robin_counter{0};
            int child_worker_count{0};
            
            
            //std::mutex g_num_mutex2;
            
        };

        /* Inline methods. */

        inline size_t TcpServerBase::GetNumConnections() const {
            return this->connections.size();
        }
	
	inline std::unordered_set<TcpConnectionBase*> & TcpServerBase::GetConnections(){
            return this->connections;
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
        class TcpServer : public TcpServerBase
        {
        public:

     
        public:
            TcpServer(Listener* listener, std::string ip, int port, bool multiThreaded=false, bool ssl=false );

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
