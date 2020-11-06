/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


#ifndef base_Thread_H
#define base_Thread_H
/*******************************************************************
Please disable libuv thread and enable std::thread
for thread leak testing 

When you enable std::thread it will give thread resource 
issue during thread join.

********************************************************************/
#if 1 

#include "base/base.h"

#include <atomic>

#include "uv.h"


// #ifdef ANDROID
// #define __linux__ ANDROID
// #endif

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
//#error "no supported os"
#include <pthread.h>
#include <unistd.h>

#define uv_thread_sleep(ms) usleep((ms) * 1000)
#define uv_thread_id pthread_self
#endif


namespace base
{


    typedef void (*entry)(void* arg);

    class Base_API Thread
    {
    public:

        Thread() : exit(false),isrunning_(false) { }
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
        }

        void join();

        bool running() {
            return isrunning_;
        }

        void Sleep(int64_t millsec) {
            uv_thread_sleep(millsec);
        }

        int GetThreadID(void) const {
            //return uv_thread_id();
	    return 0;
        }

        virtual void stop(bool flag = true) {
            exit = flag;
        }

        /// Returns true when the task has been cancelled.

        virtual bool stopped() const {
            return exit.load();
        }

    protected:
        std::atomic<bool> exit;
        std::atomic<bool> isrunning_;

    private:
        uv_thread_t thread_;

    };



class guard {
public:
  guard(uv_mutex_t &mutex) {
    _mutex = &mutex;
    uv_mutex_lock(_mutex);
  }

  ~guard() {
    uv_mutex_unlock(_mutex);
  }

private:
  uv_mutex_t* _mutex;
};


// Container class for holding thread-safe values.
template <typename T>
class ThreadSafe {
public:
  ThreadSafe(const T value) : _value(value) {
    uv_mutex_init(&_mutex);
  }

  void set(const T value) {
    guard guard(_mutex);
    _value = value;
  }

  T get() {
    guard guard(_mutex);
    return _value;
  }

private:
  T _value;
  uv_mutex_t _mutex;
};


// Wrapper class for mutex/condition variable pair.
class CondWait {
public:
  CondWait() {
    uv_mutex_init(&mutex);
    uv_cond_init(&cond);
  }

  ~CondWait() {
    uv_mutex_destroy(&mutex);
    uv_cond_destroy(&cond);
  }

  void lock() {
    uv_mutex_lock(&mutex);
  }

  void unlock() {
    uv_mutex_unlock(&mutex);
  }

  void signal() {
    uv_cond_signal(&cond);
  }

  void wait() {
    uv_cond_wait(&cond, &mutex);
  }

  uv_mutex_t mutex;
  uv_cond_t cond;
};


// uv_barrier_t causes crashes on Windows (with libuv 1.15.0), so we have our
// own implementation here.
class Barrier {
public:
  Barrier(int n) : n(n) {}

  void wait() {
    guard guard(condwait.mutex);

    if (n == 0) {
      return;
    }

    --n;

    if (n == 0) {
      condwait.signal();
    }
    while(n > 0) {
      condwait.wait();
    }
  }

private:
  int n;
  CondWait condwait;
};

} // namespace base



#else

//std thread

#include "base/base.h"
#include <thread> 
#include <atomic>



namespace base
{


    typedef void (*entry)(void* arg);

    class Base_API Thread
    {
    public:

        Thread() : exit(false),thread_(nullptr) { }
        virtual ~Thread(void);

        virtual void start();


        virtual void run() {
            printf("Thread\n");
        };

        void join();



        std::thread::id GetThreadID(void) const {
            return std::this_thread::get_id(); //uv_thread_self();
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

    private:
        std::thread *thread_;
    };



} // namespace base

#endif
#endif

