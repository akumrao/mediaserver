/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 *
 * https://dzone.com/articles/parallel-tcpip-socket-server-with-multi-threading
 */

#include <signal.h>
#include <cstdio>
#include <cstdlib>
#include <cassert>
#ifdef __unix__
#include <unistd.h> // _SC_NPROCESSORS_ONLN on OS X
#endif
#include "uv.h"
#include "http_parser.h"

#include <string>
#include <sstream>
#include <string>
#include <iostream>

#define DEBUG
#ifdef DEBUG
#define CHECK(status, msg) \
  if (status != 0){ \
    fprintf(stderr, "%s: %s\n", msg, uv_err_name(status)); \
    exit(1); \
  }
#define UVERR(err, msg) fprintf(stderr, "%s: %s\n", msg, uv_err_name(err))
#define LOG_ERROR(msg) puts(msg);
#define LOG(msg) puts(msg);
#define LOGF(...) printf(__VA_ARGS__);
#else
#define CHECK(status, msg)
#define UVERR(err, msg)
#define LOG_ERROR(msg)
#define LOG(msg)
#define LOGF(...)
#endif

//#define MultiThreadedWebServer 1

//or 

#define MultiThreadedReadWritewithPipe 1


static int request_num = 1;
static uv_loop_t* uv_loop;
static uv_tcp_t servertcp;
static http_parser_settings parser_settings;

struct client_t
{
    uv_tcp_t handle;
    http_parser parser;
    uv_write_t write_req;
    int request_num;
    std::string path;
    
     uv_loop_t *myloop{nullptr};
     
};


#if MultiThreadedReadWritewithPipe

struct child_worker 
  {
            //uv_process_t req;
          ////  uv_process_options_t options;
            uv_pipe_t pipe;
            uv_thread_t thread;

            uv_loop_t *loppworker;
            uv_pipe_t queue;

            int fds[2];
            
           // TcpServerBase *obj;

  } ;//*workers;

void alloc_buffer_worker(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
      buf->base = (char*) malloc(suggested_size);
    buf->len = suggested_size;
}

#endif


void on_close(uv_handle_t* handle){
    client_t* client = (client_t*) handle->data;
    LOGF("[ %5d ] connection closed client %x, handle %x \n\n", client->request_num, client , handle);
    delete client;
}

void alloc_cb(uv_handle_t * /*handle*/, size_t suggested_size, uv_buf_t* buf){
    *buf = uv_buf_init((char*) malloc(suggested_size), suggested_size);
}

void on_read(uv_stream_t* tcp, ssize_t nread, const uv_buf_t * buf){
    ssize_t parsed;
    LOGF("on read: %ld handle %x \n", nread , tcp );
    client_t* client = (client_t*) tcp->data;
    if (nread >= 0)
    {
        parsed = (ssize_t) http_parser_execute(
                &client->parser, &parser_settings, buf->base, nread);
        if (client->parser.upgrade)
        {
            LOG_ERROR("parse error: cannot handle http upgrade");
            uv_close((uv_handle_t*) & client->handle, on_close);
        } else if (parsed < nread)
        {
            LOG_ERROR("parse error");
            uv_close((uv_handle_t*) & client->handle, on_close);
        }
    } else
    {
        if (nread != UV_EOF)
        {
            UVERR(nread, "read");
        }
        LOGF("on close:handle %x \n",  tcp );
        uv_close((uv_handle_t*) & client->handle, on_close);
    }
    free(buf->base);
}

struct render_baton
{

    render_baton(client_t * _client) :
    client(_client),
    request(),
    result(),
    response_code("200 OK"),
    content_type("text/plain"),
    error(false){
        request.data = this;
    }
    client_t* client;
    uv_work_t request;
    std::string result;
    std::string response_code;
    std::string content_type;
    bool error;
};

void after_write(uv_write_t* req, int status){
    CHECK(status, "write");
    if (!uv_is_closing((uv_handle_t*) req->handle))
    {
        LOGF("after_write close handle %x\n",  req->handle);
        render_baton *closure = static_cast<render_baton *> (req->data);
        delete closure;
        uv_close((uv_handle_t*) req->handle, on_close);
    }
}

bool endswith(std::string const& value, std::string const& search){
    if (value.length() >= search.length())
    {
        return (0 == value.compare(value.length() - search.length(), search.length(), search));
    } else
    {
        return false;
    }
}

void render(uv_work_t* req){
    render_baton *closure = static_cast<render_baton *> (req->data);
    client_t* client = (client_t*) closure->client;
    LOGF("[ %5d ] render client %x\n", client->request_num , client);

    //closure->result = "hello universe";
    closure->response_code = "200 OK";

    //#if 0
    std::string filepath(".");
    filepath += client->path;
//    std::string index_path = (filepath + "index.html");
     std::string index_path = filepath;
             
    std::cout << "file Path: " << filepath <<  std::endl << std::flush ;

    bool has_index = (access(index_path.c_str(), R_OK) != -1);
    if (/*!has_index &&*/ filepath[filepath.size() - 1] == '/')
    {
        uv_fs_t scandir_req;
        int r = uv_fs_scandir(req->loop, &scandir_req, filepath.c_str(), 0, NULL);
        uv_dirent_t dent;
        closure->content_type = "text/html";
        closure->result = "<html><body><ul>";
        while (UV_EOF != uv_fs_scandir_next(&scandir_req, &dent))
        {
            std::string name = dent.name;
            if (dent.type == UV_DIRENT_DIR)
            {
                name += "/";
            }
            closure->result += "<li><a href='";
            closure->result += name;
            closure->result += "'>";
            closure->result += name;
            closure->result += "</a></li>";
            closure->result += "\n";
        }
        closure->result += "</ul></body></html>";
        uv_fs_req_cleanup(&scandir_req);
    } else
    {
        std::string file_to_open = filepath;
        if (has_index)
        {
            file_to_open = index_path;
        }
       // std::cout << "file Path: " << file_to_open << std::endl;
        bool exists = (access(file_to_open.c_str(), R_OK) != -1);
        if (!exists)
        {
            closure->result = "no access";
            closure->response_code = "404 Not Found";
            return;
        }
        FILE * f = fopen(file_to_open.c_str(), "rb");
        if (f)
        {
            std::fseek(f, 0, SEEK_END);
            unsigned size = std::ftell(f);
            std::fseek(f, 0, SEEK_SET);
            closure->result.resize(size);
            std::fread(&closure->result[0], size, 1, f);

            //std::cout << &closure->result[0] << std::endl;

            fclose(f);
            if (endswith(file_to_open, "html"))
            {
                closure->content_type = "text/html";
            } else if (endswith(file_to_open, "css"))
            {
                closure->content_type = "text/css";
            } else if (endswith(file_to_open, "js"))
            {
                closure->content_type = "application/javascript";
            }
        } else
        {
            closure->result = "failed to open";
            closure->response_code = "404 Not Found";
        }
    }
    //#endif
}

void after_render(uv_work_t* req){
    render_baton *closure = static_cast<render_baton *> (req->data);
    client_t* client = (client_t*) closure->client;

    LOGF("[ %5d ] after render , cline %x \n", client->request_num, client);

    std::ostringstream rep;
    rep << "HTTP/1.1 " << closure->response_code << "\r\n"
            << "Content-Type: " << closure->content_type << "\r\n"
            //<< "Connection: keep-alive\r\n"
            << "Connection: close\r\n"
            << "Content-Length: " << closure->result.size() << "\r\n"
            << "Access-Control-Allow-Origin: *" << "\r\n"
            << "\r\n";
    rep << closure->result;
    std::string res = rep.str();
    uv_buf_t resbuf;
    resbuf.base = (char *) res.c_str();
    resbuf.len = res.size();

    client->write_req.data = closure;

    // https://github.com/joyent/libuv/issues/344
    int r = uv_write(&client->write_req,
            (uv_stream_t*) & client->handle,
            &resbuf, 1, after_write);
    CHECK(r, "write buff");
}

int on_message_begin(http_parser* /*parser*/){
    LOGF("\n***MESSAGE BEGIN***\n");
    return 0;
}

int on_headers_complete(http_parser* /*parser*/){
    LOGF("\n***HEADERS COMPLETE***\n");
    return 0;
}

int on_url(http_parser* parser, const char* url, size_t length){
    client_t* client = (client_t*) parser->data;
    LOGF("[ %5d ] on_url\n", client->request_num);
    LOGF("Url: %.*s\n", (int) length, url);
    // TODO - use https://github.com/bnoordhuis/uriparser2 instead?
    struct http_parser_url u;
    int result = http_parser_parse_url(url, length, 0, &u);
    if (result)
    {
        fprintf(stderr, "\n\n*** failed to parse URL %s ***\n\n", url);
        return -1;
    } else
    {
        if ((u.field_set & (1 << UF_PATH)))
        {
            const char * data = url + u.field_data[UF_PATH].off;
            LOGF("UrlPath: %s\n", data);
            client->path = std::string(data, u.field_data[UF_PATH].len);
            std::cout << " Url Path " << client->path << std::endl;
        } else
        {
            fprintf(stderr, "\n\n*** failed to parse PATH in URL %s ***\n\n", url);
            return -1;
        }
    }
    return 0;
}

int on_header_field(http_parser* /*parser*/, const char* at, size_t length){
    LOGF("Header field: %.*s\n", (int) length, at);
    return 0;
}

int on_header_value(http_parser* /*parser*/, const char* at, size_t length){
    LOGF("Header value: %.*s\n", (int) length, at);
    return 0;
}

int on_body(http_parser* /*parser*/, const char* at, size_t length){
    LOGF("Body: %.*s\n", (int) length, at);
    return 0;
}

int on_message_complete(http_parser* parser){
    client_t* client = (client_t*) parser->data;
    LOGF("[ %5d ] on_message_complete\n", client->request_num);
    render_baton *closure = new render_baton(client);
    int status = uv_queue_work(client->myloop,
            &closure->request,
            render,
            (uv_after_work_cb) after_render);
    CHECK(status, "uv_queue_work");
    assert(status == 0);

    return 0;
}



#if MultiThreadedReadWritewithPipe

int round_robin_counter = 0;
 int child_worker_count=0;
child_worker *workers{nullptr};


void on_new_worker_connection(uv_stream_t *q, ssize_t nread, const uv_buf_t *buf) {
  
    if (nread < 0) {

        if (nread != UV_EOF)
            fprintf(stderr, "Read error %s\n", uv_err_name(nread));
        uv_close((uv_handle_t*) q, NULL);
        return;
    }

    uv_pipe_t *pipe = (uv_pipe_t*) q;
    if (!uv_pipe_pending_count(pipe)) {
        fprintf(stderr, "No pending count\n");
        return;
    }

    child_worker *tmp = (child_worker*) pipe->data;

    uv_handle_type pending = uv_pipe_pending_type(pipe);
    assert(pending == UV_TCP);

    
    ///////////////////////////////////////////////
    
    
    
    client_t* client = new client_t();
    client->request_num = request_num;
    request_num++;
   

    client->myloop = tmp->loppworker;
    
    uv_tcp_init(tmp->loppworker, &client->handle);
    http_parser_init(&client->parser, HTTP_REQUEST);

    client->parser.data = client;
    client->handle.data = client;
    
    
    LOGF("[ %5d ] new connection, client %x, handle %x \n", request_num, client,  &client->handle);

    int r = uv_accept(q, (uv_stream_t*) & client->handle);
    CHECK(r, "accept");
   
    if(r==0)
    {
        uv_os_fd_t fd;
        uv_fileno((const uv_handle_t*) &client->handle, &fd);
        fprintf(stderr, "Worker %d: Accepted fd %d\n", getpid(), fd);
        uv_read_start((uv_stream_t*) &client->handle, alloc_cb, on_read);
    } else {
        uv_close((uv_handle_t*) &client->handle, NULL);
    }
    

}

static void workermain(void* _worker) {


    child_worker *tmp = (child_worker*) _worker;
    
    tmp->loppworker = (uv_loop_t*) malloc(sizeof (uv_loop_t));

    int e = uv_loop_init(tmp->loppworker);


    e = uv_pipe_init(tmp->loppworker, &tmp->queue, 1/* ipc */);
    e = uv_pipe_open(&tmp->queue, tmp->fds[1]);

    tmp->queue.data = tmp;

    e = uv_read_start((uv_stream_t*) & tmp->queue, alloc_buffer_worker, on_new_worker_connection);
    uv_run(tmp->loppworker, UV_RUN_DEFAULT);

}

  void  setup_workers() 
  {
           
            //size_t path_size = 500;
            // uv_exepath(worker_path, &path_size);
            // strcpy(worker_path + (strlen(worker_path) - strlen("multi-echo-server")), "worker");
            // fprintf(stderr, "Worker path: %s\n", worker_path);

            // char* args[2];
            //  args[0] = worker_path;
            //  args[1] = NULL;

            
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
                
                //worker->obj = this;
                
                uv_pipe_init(uv_loop, &worker->pipe, 1);

                socketpair(AF_UNIX, SOCK_STREAM, 0, worker->fds);
                uv_pipe_open(&worker->pipe, worker->fds[0]);

                int e = uv_thread_create(&worker->thread, workermain, (void *) worker);
                if (0 != e)
                {
                   // SError << "Error creating thread";
                }


                // fprintf(stderr, "Started worker %d\n", worker->req.pid);
            }
}

  
static void after_pipe_write(uv_write_t* req, int status) {
 #ifdef _WIN32 
      //  free(req->data);
       // free_write_req(req);
      //  uv_close((uv_handle_t*) req->handle, nullptr);

        free(req);
 #else

   std::cout << " after_pipe_write "  <<  req  << " req->handle"  << req->data  << std::endl << std::flush;
  
  uv_close((uv_handle_t*) req->data, nullptr);
  free(req->data);
  free(req); 

#endif
        return;
}
        
  
uv_buf_t dummy_buf;
void on_new_connection(uv_stream_t *server, int status) {

    if (status == -1) {

        // error!
        return;
    }

    uv_tcp_t *client = (uv_tcp_t*) malloc(sizeof (uv_tcp_t));
    
    std::cout << " on_new_connection handle"  <<  client << std::endl << std::flush;
    
    uv_tcp_init(uv_loop, client);
    if (uv_accept(server, (uv_stream_t*) client) == 0) {
        uv_write_t *write_req = (uv_write_t*) malloc(sizeof (uv_write_t));
        dummy_buf = uv_buf_init("a", 1);
        struct child_worker *worker = &workers[round_robin_counter];
        
         write_req->data = client;
        
         std::cout << " write_req "  <<  write_req << std::endl << std::flush;
            
        int e =  uv_write2(write_req, (uv_stream_t*) & worker->pipe, &dummy_buf, 1, (uv_stream_t*) client, after_pipe_write);
        round_robin_counter = (round_robin_counter + 1) % child_worker_count;
    } else {
        uv_close((uv_handle_t*) client, NULL);
    }
}

#endif

void on_connect(uv_stream_t* server_handle, int status){
    CHECK(status, "connect");
  //  assert((uv_tcp_t*) server_handle == &servertcp);

    client_t* client = new client_t();
    client->request_num = request_num;
    request_num++;

    

    client->myloop = server_handle->loop;
    
    uv_tcp_init(server_handle->loop, &client->handle);
    http_parser_init(&client->parser, HTTP_REQUEST);

    client->parser.data = client;
    client->handle.data = client;
    
    LOGF("[ %5d ] on_connect  %x, handle %x \n", request_num , client, &client->handle );
    
    
    
    std::cout << " client  " << client <<  " handle " <<   &client->handle  << std::endl << std::flush;
        

    int r = uv_accept(server_handle, (uv_stream_t*)&client->handle);
    CHECK(r, "accept");

    uv_read_start((uv_stream_t*) & client->handle, alloc_cb, on_read);
}

#define MAX_WRITE_HANDLES 1000

#if MultiThreadedWebServer
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void server_on_start(void *arg) { // void (*uv_thread_cb)(void* arg)
    uv_loop_t loop;
    if (uv_loop_init(&loop)) { LOG_ERROR("uv_loop_init\n"); return; } // int uv_loop_init(uv_loop_t* loop)
    //server_t *server = server_init(&loop);

    uv_tcp_t tcp;
    if (uv_tcp_init(&loop, &tcp)) { LOG_ERROR("uv_tcp_init\n");  return; } // int uv_tcp_init(uv_loop_t* loop, uv_tcp_t* handle)
    uv_os_sock_t client_sock = *((uv_os_sock_t *)arg);
    if (uv_tcp_open(&tcp, client_sock)) { LOG_ERROR("uv_tcp_open\n");  return; } // int uv_tcp_open(uv_tcp_t* handle, uv_os_sock_t sock)
    //int name[] = {CTL_NET, NET_CORE, NET_CORE_SOMAXCONN}, nlen = sizeof(name), oldval[nlen]; size_t oldlenp = sizeof(oldval);
 //   if (sysctl(name, nlen / sizeof(int), (void *)oldval, &oldlenp, NULL, 0)) { LOG_ERROR("sysctl\n");  return; } // int sysctl (int *name, int nlen, void *oldval, size_t *oldlenp, void *newval, size_t newlen)
    int backlog = SOMAXCONN; 
    if (uv_listen((uv_stream_t *)&tcp, backlog, on_connect)) { LOG_ERROR("uv_listen\n");  return; } // int uv_listen(uv_stream_t* stream, int backlog, uv_connection_cb cb)
    if (uv_run(&loop, UV_RUN_DEFAULT)) { LOG_ERROR("uv_run\n");  return; } // int uv_run(uv_loop_t* loop, uv_run_mode mode)
    if (uv_loop_close(&loop)) { LOG_ERROR("uv_loop_close\n");  return; } // int uv_loop_close(uv_loop_t* loop)
   // 
}



///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#endif


int main(){
    parser_settings.on_url = on_url;
    // notification callbacks
    parser_settings.on_message_begin = on_message_begin;
    parser_settings.on_headers_complete = on_headers_complete;
    parser_settings.on_message_complete = on_message_complete;
    // data callbacks
    parser_settings.on_header_field = on_header_field;
    parser_settings.on_header_value = on_header_value;
    parser_settings.on_body = on_body;
    uv_loop = uv_default_loop();
    int r = uv_tcp_init(uv_loop, &servertcp);
    CHECK(r, "tcp_init");
    r = uv_tcp_keepalive(&servertcp, 1, 60);
    CHECK(r, "tcp_keepalive");
    struct sockaddr_in address;
    r = uv_ip4_addr("0.0.0.0", 3000, &address);
    CHECK(r, "ip4_addr");
    r = uv_tcp_bind(&servertcp, (const struct sockaddr*) &address, 0);
    CHECK(r, "tcp_bind");
    
    
              
#if MultiThreadedWebServer
    //////////////////////////////////////////////////////////////////////////////////////////////////////
    
     uv_os_sock_t sock;
    if ((r = uv_fileno((const uv_handle_t*)&servertcp, (uv_os_fd_t *)&sock))) { LOG_ERROR("uv_fileno\n"); return r; } // int uv_fileno(const uv_handle_t* handle, uv_os_fd_t* fd)
    int cpu_count = 2;

    int thread_count = cpu_count;

    {
        uv_thread_t tid[thread_count];
        for (int i = 0; i < thread_count; i++) if ((r = uv_thread_create(&tid[i], server_on_start, (void *)&sock))) { LOG_ERROR("uv_thread_create\n"); return r; } // int uv_thread_create(uv_thread_t* tid, uv_thread_cb entry, void* arg)
        for (int i = 0; i < thread_count; i++) if ((r = uv_thread_join(&tid[i]))) { LOG_ERROR("uv_thread_join\n"); return r; } // int uv_thread_join(uv_thread_t *tid)
    }
    
    
    //////////////////////////////////////////////////////////////////////////////////////////////////
#elif MultiThreadedReadWritewithPipe
    setup_workers() ;
       r = uv_listen((uv_stream_t*) & servertcp, MAX_WRITE_HANDLES, on_new_connection); 
#else
   r = uv_listen((uv_stream_t*) & servertcp, MAX_WRITE_HANDLES, on_connect);
#endif

   CHECK(r, "uv_listen");
    LOG("listening on port 3000");
   

    
    uv_run(uv_loop, UV_RUN_DEFAULT);
}



