/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


#include "base/thread.h"
#include "base/logger.h"
#include "base/platform.h"
#include <assert.h>
#include <memory>


using std::endl;


#if 1
namespace base
{

    Thread::~Thread(void) {
        if (isrunning_)
        {
            uv_thread_join(&thread_);
        }
        isrunning_ = false;
    }

    void Thread::start() {
        if (isrunning_)
        {
            return;
        }
        uv_thread_create(&thread_, enter, this);
        isrunning_ = true;
    }

    void Thread::join() {
        if (!isrunning_)
        {
            return;
        }
        uv_thread_join(&thread_);
        isrunning_ = false;
    }



} // namespace base
#else
/// std::thread
#include <thread>

namespace base {

    Thread::~Thread(void) {
        if (thread_ && thread_->joinable()) {
            thread_->join();
        }
        if (!thread_) {
            delete thread_;
            thread_ = nullptr;
        }
    }

    void Thread::start() {

        if (!thread_)
            thread_ = new std::thread(&Thread::run, this);
        else {
             SError << "Already thread created";
            delete thread_;
            thread_ = new std::thread(&Thread::run, this);
        }


    }

    void Thread::join() {

      //assert(thread_->get_id()  != this=->currentID());

            
        if (thread_)
            thread_->join();
   
    }



} // namespace base



#endif
