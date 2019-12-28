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

    Application& Application::getDefault() {
        return *internal::singleton.get();
    }

    Application::Application() {

        //if(Application::loop == uv_default_loop() )
            uvInit();
    }

    uv_loop_t* Application::loop = uv_default_loop();

    void Application::uvInit() {
        LDebug("init")

        if(loop ==nullptr || loop == uv_default_loop()  )
        {
            loop = new uv_loop_t;
            int err = uv_loop_init(loop);
            if (err != 0)
                LError("libuv initialization failed");
        }

    }

    void Application::uvDestroy() {

        LTrace("uvDestroy")
        if (loop != nullptr) {
            int result = uv_loop_close(loop);
            delete loop;
            loop =nullptr;
        }

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
        uv_run(loop, UV_RUN_DEFAULT);
    }

    void Application::stop() {
        uv_stop(loop);
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
        assert(loop->active_handles == 0);
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
        uv_signal_init(loop, sig);
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


