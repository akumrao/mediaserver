/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */
// code changes are copied from libuv test-ipc-send-recv.c

#include "net/TcpServer.h"
#include "base/logger.h"
#include "base/application.h"
#include "net/PortManager.h"
#include <inttypes.h>
#include "net/IP.h"
#if HTTPSSL
#include "net/SslConnection.h"
#endif 

#ifdef _WIN32
#define TEST_PIPENAME "\\\\?\\pipe\\uv-test"
#else
#define TEST_PIPENAME "/tmp/uv-test-sock"
#endif


namespace base
{
    namespace net
    {

        static void after_pipe_write(uv_write_t* req, int status) {
	 #ifdef _WIN32 
              //  free(req->data);
               // free_write_req(req);
              //  uv_close((uv_handle_t*) req->handle, nullptr);
               
                free(req);
         #else
          uv_close((uv_handle_t*) req->data, nullptr);
          free(req->data);
          free(req); 
         #endif
                return;
        }
        

        /* Static methods for UV callbacks. */

        inline static void onConnection(uv_stream_t* handle, int status) {
            auto* server = static_cast<TcpServerBase*> (handle->data);

            if (server == nullptr)
                return;

            server->OnUvConnection(handle , status);
        }

        inline static void onClose(uv_handle_t* handle) {
            SDebug << " TcpServerBase::onClose"  ;
            delete handle;
        }
        
   
        void on_new_worker_connection(uv_stream_t *q, ssize_t nread, const uv_buf_t *buf) {
            
        

            if (nread == UV_EOF || nread == UV_ECONNABORTED) {
                return;
            }

            if (nread < 0) {
                if (nread != UV_EOF)
                    fprintf(stderr, "Read error %s\n", uv_err_name(nread));
                uv_close((uv_handle_t*) q, NULL);
                return;
            }

            uv_pipe_t *pipe = (uv_pipe_t*) q;

	    uv_handle_type pending;	
            do
            {
                 if (!uv_pipe_pending_count(pipe))
               {
                  SError <<   "No pending count";
                  return;
                }

             pending = uv_pipe_pending_type(pipe);
               

            child_worker *tmp = (child_worker*) pipe->data;

            uv_handle_type pending = uv_pipe_pending_type(pipe);
            assert(pending == UV_TCP);
            
            free(buf->base);
           // SDebug <<  "on_new_worker_connection  loppworker" << tmp->loppworker    <<  "  threadid "  <<  tmp->thread;
                    
               tmp->obj->worker_connection(tmp->loppworker, q);
            } while (uv_pipe_pending_count(pipe) > 0);




        }
        
        void alloc_buffer_worker(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
            buf->base = (char*) malloc(suggested_size);
            buf->len = suggested_size;
        }


#ifdef _WIN32
        static void listen_cb(uv_stream_t* handle, int status) {
          int err;
          ASSERT(status == 0);

        child_worker* tmp = (child_worker*)handle->data;

          

         err = uv_pipe_init(handle->loop, &tmp->channelqueue, 1);
         err = uv_accept((uv_stream_t*)handle /*queue */, (uv_stream_t*)&tmp->channelqueue);
          ASSERT(err == 0);

          //send_recv_start();

         tmp->channelqueue.data = tmp;

         err = uv_read_start((uv_stream_t*)&tmp->channelqueue,  alloc_buffer_worker,  on_new_worker_connection);  // arvind
          if (err != 0)
            LError("workermain() failed: %s", uv_strerror(err));

        }
#endif

        static void workermain(void* _worker) {
            child_worker *tmp = (child_worker*) _worker;
            //tmp->loppworker = (uv_loop_t*) malloc(sizeof (uv_loop_t));
            Application app;
            tmp->loppworker = Application::uvGetLoop();
            
            int err;  //= uv_loop_init(tmp->loppworker);


   #ifdef _WIN32 
            err = uv_pipe_init(tmp->loppworker, &tmp->queue, 0 /* ipc */);   //listen
            if (err != 0)
              LError("workermain() failed: %s", uv_strerror(err));

           // err = uv_pipe_open(&tmp->queue, 0);
            char pipePath[256];
            sprintf(pipePath, "%s_%d", TEST_PIPENAME, tmp->id);
            unlink(pipePath);
              
            err = uv_pipe_bind(&tmp->queue, pipePath);
            ASSERT(err == 0);


            tmp->queue.data = tmp;

            err = uv_listen((uv_stream_t*)&tmp->queue, SOMAXCONN, listen_cb);
            ASSERT(err == 0);

            if (err != 0)
              LError("workermain() failed: %s", uv_strerror(err));
             
             int r = uv_loop_close(tmp->loppworker);// for windows close

             uv_run(tmp->loppworker, UV_RUN_DEFAULT);
		
#else
  	    err = uv_pipe_init(tmp->loppworker, &tmp->queue, 1/* ipc */);
            err = uv_pipe_open(&tmp->queue, tmp->fds[1]);

            tmp->queue.data = tmp;

            err = uv_read_start((uv_stream_t*) & tmp->queue, alloc_buffer_worker, on_new_worker_connection);  // arvind
	    if (err != 0)
              LError("workermain() failed: %s", uv_strerror(err));

	    uv_run(tmp->loppworker, UV_RUN_DEFAULT);

#endif
            
            SDebug << "close workermain ";

        }

   #ifdef _WIN32     
        static void connect_cb(uv_connect_t* req, int status) {
  
            SDebug << "Pipe Connected ";

            // ASSERT(status == 0);

        }
    #endif   
        
        void  TcpServerBase::setup_workers() {
            SDebug << __func__;
            //size_t path_size = 500;
            // uv_exepath(worker_path, &path_size);
            // strcpy(worker_path + (strlen(worker_path) - strlen("multi-echo-server")), "worker");
            // fprintf(stderr, "Worker path: %s\n", worker_path);

            // char* args[2];
            //  args[0] = worker_path;
            //  args[1] = NULL;

            round_robin_counter = 0;
            // ...

            // launch same number of workers as number of CPUs
            uv_cpu_info_t *info;
            int cpu_count = 3;
            //uv_cpu_info(&info, &cpu_count);
            //uv_free_cpu_info(info, cpu_count);

            child_worker_count = cpu_count;

            workers = (child_worker*) calloc(cpu_count, sizeof (struct child_worker));
            while (cpu_count--) {
                struct child_worker *worker = &workers[cpu_count];
                worker->id = cpu_count;
                worker->obj = this;
                
#ifdef _WIN32
                int err = uv_thread_create(&worker->thread, workermain,      (void*)worker);
                if (err != 0)
                   LError("setup_workers() failed: %s", uv_strerror(err));

                
                 uv_sleep(1000);

                err = uv_pipe_init(Application::uvGetLoop(), &worker->pipe, 1);
                 if (err != 0)
                   LError("setup_workers() failed: %s", uv_strerror(err));

                 //uv_connect_t connect_req;
                 
                 char pipePath[256];
                 sprintf(pipePath, "%s_%d", TEST_PIPENAME, worker->id);

                 uv_pipe_connect(&worker->connect_req, &worker->pipe, pipePath,
                                 connect_cb);
#else

 	        uv_pipe_init(Application::uvGetLoop(), &worker->pipe, 1);

                socketpair(AF_UNIX, SOCK_STREAM, 0, worker->fds);
               // uv_socketpair(SOCK_STREAM, 0, worker->fds, 0, 0);
                
                uv_pipe_open(&worker->pipe, worker->fds[0]);

                int err = uv_thread_create(&worker->thread, workermain, (void *) worker);
                if (err != 0)
                 LError("setup_workers() failed: %s", uv_strerror(err));

#endif         

            }
        }

        
        
        
        
        
        
        /* Instance methods. */

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)

        TcpServerBase::TcpServerBase(uv_tcp_t* uvHandle, int backlog, bool multiThreaded) : uvHandle(uvHandle), multithreaded(multiThreaded) {

            int err;

            if(multithreaded)
            setup_workers();
            
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


            SDebug << "~TcpServerBase()";
            
            if(workers)
            {
                 SDebug << "delete worker";
                 
                for( int x =0; x < child_worker_count ; ++x )
                {
                     struct child_worker *worker = &workers[x];
                     worker->obj->Close();
                      SDebug << "delete worker1";
                     uv_close((uv_handle_t*) &worker->queue, NULL);
                      SDebug << "delete worker2";
                     uv_close((uv_handle_t*) &worker->pipe, NULL);
                }

               free( workers);
            }
            
            if (!this->closed)
                Close();
                
        }

        void TcpServerBase::Close() {


            if (this->closed)
                return;

            this->closed = true;

            // Tell the UV handle that the TcpServerBase has been closed.
            this->uvHandle->data = nullptr;

            SDebug << " TcpServerBase::Close,  closing all active connections"  <<  this->connections.size();

            for (auto* connection : this->connections)
            {
                connection->Close();
                SDebug << " TcpServerBase::Close,  delete"  <<  connection;
                delete connection;
            }
            connections.clear();

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

        uv_buf_t dummy_buf;
        inline void TcpServerBase::OnUvConnection(uv_stream_t* uvh, int status) {


            if (this->closed)
                return;

            int err;

            if (status != 0)
            {
                LError("error while receiving a new TCP connection: %s", uv_strerror(status));

                return;
            }
            
            

            if(multithreaded)
            {
                //g_num_mutex2.lock();

                SDebug << "OnUvConnection " << round_robin_counter;
                
                uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof (uv_tcp_t));
                uv_tcp_init(Application::uvGetLoop(), client);
                if (uv_accept(reinterpret_cast<uv_stream_t*> (this->uvHandle), (uv_stream_t*) client) == 0) {
                    uv_write_t *write_req = (uv_write_t*) malloc(sizeof (uv_write_t));
                    dummy_buf = uv_buf_init("a", 1);
                    struct child_worker *worker = &workers[round_robin_counter];
                    
                    write_req->data = client;
                    
                    int err = uv_write2(write_req, (uv_stream_t*) &worker->pipe, &dummy_buf, 1, (uv_stream_t*) client, after_pipe_write);
                    
                    if (err != 0)
                    LError("uv_accept() failed: %s", uv_strerror(err));

                    
                    round_robin_counter = (round_robin_counter + 1) % child_worker_count;
                } else {
                    uv_close((uv_handle_t*) client, NULL);
                }
                
              ///  g_num_mutex2.unlock();

            }
            else
            {

            // Notify the subclass so it provides an allocated derived class of TCPConnection.
                TcpConnectionBase* connection = nullptr;
                UserOnTcpConnectionAlloc(&connection);

                ASSERT(connection != nullptr);

                try
                {
                    connection->Setup(this, Application::uvGetLoop(), &(this->localAddr), this->localIp, this->localPort);
                } catch (const std::exception& error)
                {
                    delete connection;
                    SError << error.what();
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
                    SError << error.what();
                    return;
                }

                // Notify the subclass and delete the connection if not accepted by the subclass.
                if (UserOnNewTcpConnection(connection))
                {
                       SDebug << "TcpServerBase new connection "  << connection;
                    con_mutex.lock();
                    this->connections.insert(connection);
                    con_mutex.unlock();
                }
                else
                    delete connection;
                
            }
            
            
        }
        
        
         void  TcpServerBase::worker_connection( uv_loop_t *loppworker, uv_stream_t *q) 
        {
             int err;
             
           // Notify the subclass so it provides an allocated derived class of TCPConnection.
            TcpConnectionBase* connection = nullptr;
            UserOnTcpConnectionAlloc(&connection);

            ASSERT(connection != nullptr);
            
             try
            {
                connection->Setup(this, loppworker,  &(this->localAddr), this->localIp, this->localPort);
            } catch (const std::exception& error)
            {
                delete connection;
                SError << error.what();
                return;
            }

            // Accept the connection.
            err = uv_accept(
                    q,
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
                SError << error.what();
                return;
            }

            // Notify the subclass and delete the connection if not accepted by the subclass.
            if (UserOnNewTcpConnection(connection))
            {
                SDebug << "TcpServerBase new connection "  << connection;
                con_mutex.lock();
                this->connections.insert(connection);
                con_mutex.unlock();
            }
            else
                delete connection;
        }

        void TcpServerBase::OnTcpConnectionClosed(TcpConnectionBase* connection) {

            
            SDebug << " TcpConnectionBase close "  << connection;
              
            con_mutex.lock();
            // // Remove the TcpConnectionBase from the set.
             if( this->connections.find(connection) != this->connections.end() )
             {
                 UserOnTcpConnectionClosed(connection);
                 this->connections.erase(connection);
                 // Notify the subclass.
                 // Delete it.
                 delete connection;
             }
            con_mutex.unlock();
        }
        
        uv_tcp_t* TcpServerBase::BindTcp(std::string &ip, int port) {
            //Arvind
            //please do not do it here . Drive your own class from TServerBase. 
            //Use Portmanager there itself. See the sfu example
            
//	    if(port == -1)
//            {
//                return PortManager::BindTcp(ip);
//            }
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
        static constexpr size_t MaxTcpConnectionsPerServer{ 4000};

        /* Instance methods. */

        TcpServer::TcpServer(Listener* listener, std::string ip, int port,  bool multiThreaded, bool ssl )
        : TcpServerBase(BindTcp(ip, port), 256, multiThreaded), listener(listener),ssl(ssl){

        }

        TcpServer::~TcpServer() {

            if (uvHandle)
                delete uvHandle;
            //UnbindTcp(this->localIp, this->localPort); // please do not do it here . Drive your own class from TServerBase.
        }

  
        void TcpServer::UserOnTcpConnectionAlloc(TcpConnectionBase** connection) {

// condition
            // Allocate a new RTC::TcpConnection for the TcpServer to handle it.
	#if HTTPSSL
            if(ssl)
             *connection = new SslConnection( true);
            else
	 #endif
            *connection = new TcpConnectionBase(listener);
            
            //SDebug << "TcpServer::UserOnTcpConnectionAlloc new connection "  << *connection;
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

            //override this function
        }

    } // namespace net
} // namespace base
