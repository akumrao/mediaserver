
#ifndef base_Thread_H
#define base_Thread_H


#include "base/base.h"

#include <atomic>

#include "uv.h"


#ifdef ANDROID
#define __linux__ ANDROID
#endif

#if defined (WIN32) || defined(_WIN32)
#include <windows.h>
#endif
#ifdef __linux__
#include <pthread.h>
#include <unistd.h>
#endif


#if defined (WIN32) || defined(_WIN32)
#define uv_thread_close(t) (CloseHandle(t)!=FALSE)
#define uv_thread_sleep(ms) Sleep(ms);
#define uv_thread_id GetCurrentThreadId

#elif defined(__linux__)
#define uv_thread_close(t) ()
#define uv_thread_sleep(ms) usleep((ms) * 1000)
#define uv_thread_id pthread_self

#else
#error "no supported os"
#endif


namespace base
{


    typedef void (*entry)(void* arg);

    class Base_API Thread
    {
    public:

        Thread() : isrunning_(false) { }
        virtual ~Thread(void);

        virtual void start(entry fun, void* arg) {
            if (isrunning_)
            {
                return;
            }
            uv_thread_create(&thread_, fun, arg);
            isrunning_ = true;
        }


        virtual void start();

        static void enter(void *pthis) {
            Thread *obj = static_cast<Thread *> (pthis);
            obj->run();
        }

        virtual void run() {
            printf("Thread\n");
        };

        void join();

        bool running() {
            return isrunning_;
        }

        void Sleep(int64_t millsec) {
            uv_thread_sleep(millsec);
        }

        int GetThreadID(void) const {
            return uv_thread_id();
        }

        virtual void stop(bool flag = true) {
            exit = flag;
        }

        /// Returns true when the task has been cancelled.

        virtual bool stopped() const {
            return exit.load();
        };

    protected:
        std::atomic<bool> exit;
        std::atomic<bool> isrunning_;

    private:
        uv_thread_t thread_;

    };





} // namespace base


#endif


