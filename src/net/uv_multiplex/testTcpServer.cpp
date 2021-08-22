/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include "base/base.h"
#include "base/logger.h"
#include "base/application.h"
#include "net/TcpServer.h"
#include "base/test.h"
#include "base/time.h"
#include "net/netInterface.h"

#define IPC_PIPE_NAME "pearl_server_ipc"
#include "uv_multiplex.h"
#define BACKLOG 128
#define THREAD_COUNT 1


using std::endl;
using namespace base;
using namespace net;
using namespace base::test;

#if 1

#define uv_fatal(e) { SInfo << __func__; SInfo << __func__; \
        assert(0 != e); \
        fprintf(stderr, "%s:%d - err:%s: %s\n", \
                __FILE__, __LINE__, uv_err_name((e)), uv_strerror((e))); \
        exit(1); }

struct UserInfo {
    int id;
    pthread_t tid;
};

//char userdata[] = "arvind";

void uv_bind_listen_socket(uv_tcp_t* listen, const char* host, const int port, uv_loop_t* loop) {
    SInfo << __func__;
    int e;

    e = uv_tcp_init(loop, listen);
    if (e != 0)
        uv_fatal(e);

    struct sockaddr_in addr;
    e = uv_ip4_addr(host, port, &addr);
    switch (e) {
            SInfo << __func__;
        case 0:
            break;
        case EINVAL:
            fprintf(stderr, "Invalid address/port: %s %d\n", host, port);
            abort();
            break;
        default:
            uv_fatal(e);
    }

    e = uv_tcp_bind(listen, (struct sockaddr *) &addr, 0);
    if (e != 0)
        uv_fatal(e);
}

void alloc_buffer(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    SInfo << __func__;
    buf->base = (char*) malloc(suggested_size);
    buf->len = suggested_size;

}

void on_close(uv_handle_t* handle) {
    SInfo << __func__;
    SInfo << __func__;
    free(handle);
}

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

void free_write_req(uv_write_t *req) {
    SInfo << __func__;
    SInfo << __func__;
    write_req_t *wr = (write_req_t*) req;
    free(wr->buf.base);
    free(wr);
}

void echo_write(uv_write_t *req, int status) {
    SInfo << __func__;
    SInfo << __func__;
    if (status) {
        SInfo << __func__;
        SInfo << __func__;
        fprintf(stderr, "Write error %s\n", uv_err_name(status));
    }
    free_write_req(req);
}

void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
    SInfo << __func__;

    UserInfo* userInf = (UserInfo*) client->data;
    SInfo << "echo_read  thread id " << userInf->id << " tid " << userInf->tid;
    ;


    if (nread > 0) {
        SInfo << __func__;
        SInfo << __func__;
        write_req_t *req = (write_req_t*) malloc(sizeof (write_req_t));
        req->buf = uv_buf_init(buf->base, nread);
        uv_write((uv_write_t*) req, client, &req->buf, 1, echo_write);
        return;
    }
    if (nread < 0) {
        SInfo << __func__;
        SInfo << __func__;
        if (nread != UV_EOF)
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t*) client, on_close);
    }

    free(buf->base);
}

static void __on_http_connection(uv_stream_t *listener, const int status) {
    SInfo << __func__;
    UserInfo* usrInfo = (UserInfo*) listener->data;
    int e;

    if (0 != status)
        uv_fatal(status);

    //    uv_tcp_t *conn = ( uv_tcp_t *) calloc(1, sizeof(*conn));
    //
    //    e = uv_tcp_init(listener->loop, conn);
    //    if (0 != status)
    //        uv_fatal(e);
    //
    //    e = uv_accept(listener, (uv_stream_t*)conn);
    //    if (0 != e)
    //        uv_fatal(e);
    //    

    SInfo << "__on_http_connection  thread id " << usrInfo->id << " tid " << usrInfo->tid;

    uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof (uv_tcp_t));
    client->data = listener->data;
    uv_tcp_init(listener->loop, client);
    if (uv_accept(listener, (uv_stream_t*) client) == 0) {
        SInfo << __func__;
        uv_read_start((uv_stream_t*) client, alloc_buffer, echo_read);
    } else {
        uv_close((uv_handle_t*) client, on_close);
    }


    //  h2o_socket_t *sock =
    //    h2o_uv_socket_create((uv_stream_t*)conn, (uv_close_cb)free);

    // struct timeval connected_at = *h2o_get_timestamp(&thread->ctx, NULL, NULL);

    //  thread->accept_ctx.ctx = &thread->ctx;
    //  thread->accept_ctx.hosts = sv->cfg.hosts;
    //  h2o_http1_accept(&thread->accept_ctx, sock, connected_at);
}

static void __worker_start(void* _worker) {
    SInfo << __func__;
    SInfo << __func__;
    // assert(uv_tcp);

    uv_multiplex_worker_t* worker = (uv_multiplex_worker_t*) _worker;

    uv_tcp_t* listener = (uv_tcp_t*) & worker->listener;
    UserInfo* usrInfo = (UserInfo*) listener->data;

    usrInfo->tid = worker->thread;

    //  h2o_context_init(&thread->ctx, listener->loop, &sv->cfg);

    int e = uv_listen((uv_stream_t*) listener, BACKLOG,
            __on_http_connection);
    if (e != 0)
        uv_fatal(e);

    while (1)
        uv_run(listener->loop, UV_RUN_DEFAULT);
}

int main(int argc, char** argv) {
    SInfo << __func__;

    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

    //        int port = 51038;
    //        
    //        std::string ip = "0.0.0.0";
    //        
    //        if (argc > 1) { SInfo << __func__; SInfo << __func__;
    //            ip = argv[1];
    //        }
    //        
    //        if(argc > 2)
    //        {
    //            port = atoi(argv[2]);
    //        }
    //
    //        Application app;
    //
    //        tesTcpServer socket;
    //        socket.start(ip, port);
    //
    //        app.waitForShutdown([&](void*) { SInfo << __func__; SInfo << __func__;
    //
    //            socket.shutdown();
    //
    //        });

    int e, i;
    uv_loop_t loop;
    e = uv_loop_init(&loop);
    if (0 != e)
        uv_fatal(e);

    uv_tcp_t listen;
    uv_multiplex_t m;

    /* Create an HTTP socket which is multiplexed across our worker threads. We
     * close this socket once it's been multiplexed completely. */
    uv_bind_listen_socket(&listen, "0.0.0.0", 1111, &loop);

    //    sv->nworkers = atoi(opts.workers);
    uv_multiplex_init(&m, &listen, IPC_PIPE_NAME, THREAD_COUNT, __worker_start);

    // UserInfo *threads = calloc(sv->nworkers + 1, sizeof(UserInfo));
    // if (!sv->threads)
    //   exit(-1);

    /* If there are not enough resources to sustain our workers, we abort */
    for (i = 0; i < THREAD_COUNT; i++) {
        UserInfo *userInf = new UserInfo();
        userInf->id = i + 1;
        uv_multiplex_worker_create(&m, i, userInf);
    }

    uv_multiplex_dispatch(&m);

    /* Create a thread responsible for coalescing lmdb puts into a single
     * transaction to amortize disk sync latency */


    while (1)
        pause();



    return 0;
}
#else


#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>
///////////////////////////////////////////////////////////////////////////////////////////////////////

struct child_worker {
    //uv_process_t req;
  ////  uv_process_options_t options;
    uv_pipe_t pipe;
    uv_thread_t thread;


    uv_loop_t *loppworker;
    uv_pipe_t queue;

    int fds[2];

} *workers;

typedef struct {
    uv_write_t req;
    uv_buf_t buf;
} write_req_t;

void free_write_req_worker(uv_write_t *req) {
    SInfo << __func__;
    write_req_t *wr = (write_req_t*) req;
    free(wr->buf.base);
    free(wr);
}

void alloc_buffer_worker(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
    SInfo << __func__;
    buf->base = (char*) malloc(suggested_size);
    buf->len = suggested_size;
}

void echo_write(uv_write_t *req, int status) {
    SInfo << __func__;
    if (status) {
        SInfo << __func__;
        fprintf(stderr, "Write error %s\n", uv_err_name(status));
    }
    free_write_req_worker(req);
}

void echo_read(uv_stream_t *client, ssize_t nread, const uv_buf_t *buf) {
    SInfo << __func__;
    if (nread > 0) {
        SInfo << __func__;
        write_req_t *req = (write_req_t*) malloc(sizeof (write_req_t));
        req->buf = uv_buf_init(buf->base, nread);
        uv_write((uv_write_t*) req, client, &req->buf, 1, echo_write);
        return;
    }

    if (nread < 0) {
        SInfo << __func__;
        if (nread != UV_EOF)
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t*) client, NULL);
    }

    free(buf->base);
}

void on_new_worker_connection(uv_stream_t *q, ssize_t nread, const uv_buf_t *buf) {
    SInfo << __func__;
    if (nread < 0) {
        SInfo << __func__;
        if (nread != UV_EOF)
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t*) q, NULL);
        return;
    }

    uv_pipe_t *pipe = (uv_pipe_t*) q;
    if (!uv_pipe_pending_count(pipe)) {
        SInfo << __func__;
        fprintf(stderr, "No pending count\n");
        return;
    }

    child_worker *tmp = (child_worker*) pipe->data;

    uv_handle_type pending = uv_pipe_pending_type(pipe);
    assert(pending == UV_TCP);

    uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof (uv_tcp_t));
    uv_tcp_init(tmp->loppworker, client); //arvind
    if (uv_accept(q, (uv_stream_t*) client) == 0) {
        SInfo << __func__;
        uv_os_fd_t fd;
        uv_fileno((const uv_handle_t*) client, &fd);
        fprintf(stderr, "Worker %d: Accepted fd %d\n", getpid(), fd);
        uv_read_start((uv_stream_t*) client, alloc_buffer_worker, echo_read);
    } else {
        uv_close((uv_handle_t*) client, NULL);
    }
}

static void workermain(void* _worker) {
    SInfo << __func__;

    child_worker *tmp = (child_worker*) _worker;
    
    tmp->loppworker = (uv_loop_t*) malloc(sizeof (uv_loop_t));

    int e = uv_loop_init(tmp->loppworker);


    e = uv_pipe_init(tmp->loppworker, &tmp->queue, 1/* ipc */);
    e = uv_pipe_open(&tmp->queue, tmp->fds[1]);

    tmp->queue.data = tmp;

    e = uv_read_start((uv_stream_t*) & tmp->queue, alloc_buffer_worker, on_new_worker_connection);
    uv_run(tmp->loppworker, UV_RUN_DEFAULT);

}










////////////////////////////////////////////////////////////////////////////////////////////////


uv_loop_t *loppmain;


int round_robin_counter;
int child_worker_count;

uv_buf_t dummy_buf;
char worker_path[500];

void close_process_handle(uv_process_t *req, int64_t exit_status, int term_signal) {
    SInfo << __func__;
    fprintf(stderr, "Process exited with status %" PRId64 ", signal %d\n", exit_status, term_signal);
    uv_close((uv_handle_t*) req, NULL);
}



void on_new_connection(uv_stream_t *server, int status) {
    SInfo << __func__;
    if (status == -1) {
        SInfo << __func__;
        // error!
        return;
    }

    uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof (uv_tcp_t));
    uv_tcp_init(loppmain, client);
    if (uv_accept(server, (uv_stream_t*) client) == 0) {
        uv_write_t *write_req = (uv_write_t*) malloc(sizeof (uv_write_t));
        dummy_buf = uv_buf_init("a", 1);
        struct child_worker *worker = &workers[round_robin_counter];
        int e =  uv_write2(write_req, (uv_stream_t*) & worker->pipe, &dummy_buf, 1, (uv_stream_t*) client, NULL);
        round_robin_counter = (round_robin_counter + 1) % child_worker_count;
    } else {
        uv_close((uv_handle_t*) client, NULL);
    }
}

void setup_workers() {
    SInfo << __func__;
    size_t path_size = 500;
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
    int cpu_count = 2;
    //uv_cpu_info(&info, &cpu_count);
    //uv_free_cpu_info(info, cpu_count);

    child_worker_count = cpu_count;

    workers = (child_worker*) calloc(cpu_count, sizeof (struct child_worker));
    while (cpu_count--) {
        struct child_worker *worker = &workers[cpu_count];
        uv_pipe_init(loppmain, &worker->pipe, 1);
        
        socketpair(AF_UNIX, SOCK_STREAM, 0, worker->fds);
        uv_pipe_open(&worker->pipe, worker->fds[0]);

//        uv_stdio_container_t child_stdio[3];
//        child_stdio[0].flags = (uv_stdio_flags) (UV_CREATE_PIPE | UV_READABLE_PIPE);
//        child_stdio[0].data.stream = (uv_stream_t*) & worker->pipe;
//        child_stdio[1].flags = UV_IGNORE;
//        child_stdio[2].flags = UV_INHERIT_FD;
//        child_stdio[2].data.fd = 2;

       // worker->options.stdio = child_stdio;
   //     worker->options.stdio_count = 3;
//
 //       worker->options.exit_cb = close_process_handle;
  //      worker->options.file = args[0];
   //     worker->options.args = args;

        //uv_spawn(loppmain, &worker->req, &worker->options); 

        int e = uv_thread_create(&worker->thread, workermain, (void *) worker);
        if (0 != e)
            fatal(e);


       // fprintf(stderr, "Started worker %d\n", worker->req.pid);
    }
}

int main() {

    Logger::instance().add(new ConsoleChannel("debug", Level::Trace));

    loppmain = Application::uvGetLoop();

    setup_workers();

    uv_tcp_t server;
    uv_tcp_init(loppmain, &server);

    struct sockaddr_in bind_addr;
    uv_ip4_addr("0.0.0.0", 1111, &bind_addr);
    uv_tcp_bind(&server, (const struct sockaddr *) &bind_addr, 0);
    int r;
    if ((r = uv_listen((uv_stream_t*) & server, 128, on_new_connection))) {
        SInfo << __func__;
        fprintf(stderr, "Listen error %s\n", uv_err_name(r));
        return 2;
    }
    return uv_run(loppmain, UV_RUN_DEFAULT);
}

#endif