/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */

#include "base/application.h"
//#include "base/memory.h"
#include "base/logger.h"
#include "base/error.h"
#include "base/singleton.h"
#include "base/singleton.h"

namespace base {


    namespace internal {

        static Singleton<Application> singleton;

        struct ShutdownCmd {
            Application* self;
            void* opaque;
            std::function<void(void*) > callback;
        };

    }

    std::map<uv_thread_t, uv_loop_t*> m_mapLoop;
       
    Application& Application::getDefault() {
        return *internal::singleton.get();
    }

    Application::Application() {

        //if(Application::loop == uv_default_loop() )
            uvInit();
    }
    
   // uv_loop_t* Application::loop ;
    

     uv_loop_t* Application::uvGetLoop()
    {
        uv_thread_t x =  uv_thread_self();
        if( m_mapLoop.find(x) != m_mapLoop.end())
        {
            return m_mapLoop[x];
        }
        else
        {
            SError << "No possible to come here";
             throw;
        }
    }
    
     
    void Application::uvInit() {
        
        uv_thread_t x =  uv_thread_self();
        
        SInfo << " uvInit Tidid " << x;
        
        if( m_mapLoop.find(x) == m_mapLoop.end())
        {
           m_mapLoop[x]=new uv_loop_t;
            int err = uv_loop_init(uvGetLoop());
            if (err != 0)
               LError("libuv initialization failed");
        }
        else
        {  
            throw;
            SError << "No possible to come here";
        }
        

    }

    void Application::uvDestroy() {

        uv_thread_t x =  uv_thread_self();
        
        SInfo << " uvDestroy Tidid " << x;
       
        std::map<uv_thread_t, uv_loop_t*>::iterator it=m_mapLoop.find (x);
        
        if( it != m_mapLoop.end())
         {
            int result = uv_loop_close(m_mapLoop[x]);
            delete m_mapLoop[x];
            m_mapLoop.erase (x);    //
         }
        else
         {  
             throw;
             SError << "No possible to come here";
         }
        LTrace("uvDestroy")
   

    }

        /*
        static void close_walk_cb(uv_handle_t* handle1, void* arg) {
          if (!uv_is_closing(handle1))
            uv_close(handle1, NULL);
        }
        static void async_cb(uv_async_t* handle) {
          uv_close((uv_handle_t*)handle->data, nullptr);
          uv_walk(Application::uvGetLoop(), close_walk_cb, NULL);
      
            uv_stop(Application::uvGetLoop());
      
        }
             void Application::stopAsync() {
           int  r = uv_async_send(&async);
            assert(r == 0);

        }
         void Application::runAsync() {
            async.data = &async;
            int r = uv_async_init(loop, &async, async_cb);
            assert(r == 0);
            uv_run(loop, UV_RUN_DEFAULT);
        }
     */
    Application::~Application() {
        uvDestroy();
    }

    void Application::run() {
        uv_run(uvGetLoop(), UV_RUN_DEFAULT);
    }

    void Application::stop() {
        uv_stop(uvGetLoop());
    }



    // no need any more, we will delete this function later

    void Application::finalize() {
        LDebug("Finalizing")

#ifdef _DEBUG
                // Print active handles
                uv_walk(loop, Application::onPrintHandle, nullptr);
#endif

        // Shutdown the garbage collector to safely free memory before the app exists

        //uv_unref(handles )
        // Run until handles are closed
        run();
        assert(uvGetLoop()->active_handles == 0);
        //assert(loop->active_reqs == 0);

        LDebug("Finalization complete")
    }

    void Application::bindShutdownSignal(std::function<void(void*) > callback, void* opaque) {
        auto cmd = new internal::ShutdownCmd;
        cmd->self = this;
        cmd->opaque = opaque;
        cmd->callback = callback;

        auto sig = new uv_signal_t;
        sig->data = cmd;
        uv_signal_init(uvGetLoop(), sig);
        uv_signal_start(sig, Application::onShutdownSignal, SIGINT);
    }

    void Application::waitForShutdown(std::function<void(void*) > callback, void* opaque) {
        LDebug("Wait for shutdown")
        bindShutdownSignal(callback, opaque);
        run();
    }

    void Application::onShutdownSignal(uv_signal_t* req, int /* signum */) {
        auto cmd = reinterpret_cast<internal::ShutdownCmd*> (req->data);
        LDebug("Got shutdown signal")

        uv_close((uv_handle_t*) req, [](uv_handle_t * handle) {
            delete handle;
        });
        if (cmd->callback)
            cmd->callback(cmd->opaque);
        delete cmd;
    }

    void Application::onPrintHandle(uv_handle_t* handle, void* /* arg */) {
        LDebug("Active handle: ", handle, ": ", handle->type)
    }

    //
    // Command-line option parser
    //

    OptionParser::OptionParser(int argc, char* argv[], const char* delim) {
        char* lastkey = 0;
        auto dlen = strlen(delim);
        for (int i = 0; i < argc; i++) {

            // Get the application exe path
            if (i == 0) {
                exepath.assign(argv[i]);
                continue;
            }

            // Get option keys
            if (strncmp(argv[i], delim, dlen) == 0) {
                lastkey = (&argv[i][dlen]);
                args[lastkey] = "";
            }                // Get value for current key
            else if (lastkey) {
                args[lastkey] = argv[i];
                lastkey = 0;
            } else {
                LDebug("Unrecognized option:", argv[i]);
            }
        }
    }


} // namespace base


