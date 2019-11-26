
#include "base/process.h"
#include "base/logger.h"
#include <iostream>
#include <cassert>

namespace base {

    static int output_used;

    #define OUTPUT_SIZE 8*1024
    static char output[OUTPUT_SIZE];
    

    Process::Process(uv_loop_t* loop):m_loop(loop)
    {
        init();
    }

    Process::Process(std::initializer_list<std::string> args, uv_loop_t* loop)
    : args(args),m_loop(loop)

    {

        init();
    }

    Process::~Process() {
        //kill();
        uv_thread_sleep(1000);
    }

    void Process::init() {

        on_read_cb_called=0;
        int r;
        r = uv_pipe_init(m_loop, &out, 0);
        r = uv_pipe_init(m_loop, &in, 0);
        r = uv_pipe_init(m_loop, &err, 0);
        ASSERT(r == 0);

        options.args = nullptr;
        options.env = nullptr;
        options.cwd = nullptr;
        options.flags = 0;
        options.stdio_count = 0;
        // options.uid = 0;
        // options.gid = 0;
        options.exit_cb = [](uv_process_t* req, int64_t exitStatus, int /*termSignal*/) {
            auto self = reinterpret_cast<Process*> (req->data);
            if (self->onexit)
                self->onexit(exitStatus);
            // We could call close() here to free the uv_process_t content
        };

        options.stdio = _stdio;
        options.stdio[0].flags = uv_stdio_flags(UV_CREATE_PIPE | UV_READABLE_PIPE);
        options.stdio[0].data.stream = (uv_stream_t*) & in;
        //options.stdio[0].flags = uv_stdio_flags(UV_IGNORE);
        options.stdio[1].flags = uv_stdio_flags(UV_CREATE_PIPE | UV_WRITABLE_PIPE);
        options.stdio[1].data.stream = (uv_stream_t*) & out;
        options.stdio[2].flags = uv_stdio_flags(UV_CREATE_PIPE | UV_WRITABLE_PIPE);
        options.stdio[2].data.stream = (uv_stream_t*)&err;
        options.stdio_count = 3;
  

        _handle.data = this;
    }

    static void on_read(uv_stream_t* tcp, ssize_t nread, const uv_buf_t* rdbuf) {
        uv_write_t* req;
        uv_buf_t wrbuf;
        int r;

        Process *ptr = ( Process*) tcp->data;
        
        if(nread == UV_EOF)
        {
            //ptr->onstdout(std::string("Ping end ..."));
//            uv_close((uv_handle_t*)tcp, close_cb);
        }


        if (nread > 0) {
            output_used += nread;
            
             ptr->onstdout(std::string(output));
           
           /* if (output_used == 12) {
                ASSERT(memcmp("hello world\n", output, 12) == 0);
                wrbuf = uv_buf_init(output, output_used);
                req = (uv_write_t*) malloc(sizeof (*req));
                //  r = uv_write(req, (uv_stream_t*)&in, &wrbuf, 1, after_write);
                // ASSERT(r == 0);
            }*/
        }

        
        ptr->on_read_cb_called++;
    }

    static void on_alloc(uv_handle_t* handle,
            size_t suggested_size,
            uv_buf_t* buf) {
        buf->base = output + output_used;
        buf->len = OUTPUT_SIZE - output_used;
    }

    void Process::spawn() {
        // Sanity checks
        //if (options.file == nullptr)
        //    throw std::runtime_error("Cannot spawn process: File path must be set.");
        if (args.size() > 10)
            throw std::runtime_error("Cannot spawn process: Maximum of 10 command line arguments are supported.");

        // Override c style args if STL containers have items.
        _cargs.clear();
        if (!args.empty()) {
            //assert(!!options.args && "setting both args and options.args");
            for (auto& arg : args)
                _cargs.push_back(&arg[0]);
            _cargs.push_back(nullptr);
        }

        if (!cwd.empty()) {
            options.cwd = &cwd[0];
        }

        if (!file.empty()) {
            options.file = &file[0];
            if (_cargs.empty()) {
                _cargs.push_back(&file[0]);
                _cargs.push_back(nullptr);
            }
        } else if (!_cargs.empty()) {
            options.file = _cargs[0];
        }

        assert(!_cargs.empty() && "args must not be empty");
        options.args = &_cargs[0];

        int r = uv_spawn(m_loop, &_handle, &options);
       
       if(r)
          LDebug("Starting Server responder.",  uv_strerror(r))
            
        
        ASSERT(r == 0);
        out.data = this;
        r = uv_read_start((uv_stream_t*) & out, on_alloc, on_read);
        ASSERT(r == 0);
         in.data = this;
        r = uv_read_start((uv_stream_t*) & in, on_alloc, on_read);
        //ASSERT(r == 0);
        err.data = this;
        r = uv_read_start((uv_stream_t*) & err, on_alloc, on_read);
        //ASSERT(r == 0);
       
    }

    bool Process::kill(int signum) {
        // if (!_handle.initialized())
        //     return false;
        assert(pid() > 0);
        return uv_kill(pid(), signum) == 0;
    }

    int Process::pid() const {
        // assert(_handle.initialized());
        return _handle.pid;
    }
} // namespace base



