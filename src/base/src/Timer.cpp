/* This file is part of mediaserver. A webrtc sfu server.
 * Copyright (C) 2018 Arvind Umrao <akumrao@yahoo.com> & Herman Umrao<hermanumrao@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 */


#include "base/Timer.h"
//#include "base/loop.h"
#include "base/logger.h"
#include "base/application.h"

namespace base
{

    /* Static methods for UV callbacks. */

    inline static void onTimer(uv_timer_t* handle) {
        Timer *cls = static_cast<Timer*> (handle->data);
        
       cls->OnUvTimer( cls->timerID);
    }

    inline static void onClose(uv_handle_t* handle) {
        delete handle;
    }

    /* Instance methods. */

    Timer::Timer(Listener* listener, int timerID) : listener(listener),timerID(timerID) {
        

        this->uvHandle = new uv_timer_t;
        this->uvHandle->data = (void*) this;

        int err = uv_timer_init(Application::uvGetLoop() , this->uvHandle);

        if (err != 0)
        {
            delete this->uvHandle;
            this->uvHandle = nullptr;

            LError("uv_timer_init() failed: %s", uv_strerror(err));
        }
    }

    Timer::~Timer() {
        

        if (!this->closed)
            Close();
    }

    void Timer::Close() {
        

        if (this->closed)
            return;

        this->closed = true;

        uv_close(reinterpret_cast<uv_handle_t*> (this->uvHandle), static_cast<uv_close_cb> (onClose));
    }

    void Timer::Start(uint64_t timeout_ms, uint64_t repeat_ms) {
        
        if (this->closed)
            LError("closed");

        this->timeout = timeout_ms;
        this->repeat = repeat_ms;

        if (uv_is_active(reinterpret_cast<uv_handle_t*> (this->uvHandle)) != 0)
            Stop();

        int err = uv_timer_start(this->uvHandle, static_cast<uv_timer_cb> (onTimer),  this->timeout, this->repeat);

        if (err != 0)
            LError("uv_timer_start() failed: %s", uv_strerror(err));
    }

    void Timer::Stop() {
        

        if (this->closed)
            LError("closed");

        int err = uv_timer_stop(this->uvHandle);

        if (err != 0)
            LError("uv_timer_stop() failed: %s", uv_strerror(err));
    }

    void Timer::Reset() {
        

        if (this->closed)
            LError("closed");

        if (uv_is_active(reinterpret_cast<uv_handle_t*> (this->uvHandle)) == 0)
            return;

        if (this->repeat == 0u)
            return;

        int err =
                uv_timer_start(this->uvHandle, static_cast<uv_timer_cb> (onTimer), this->repeat, this->repeat);

        if (err != 0)
            LError("uv_timer_start() failed: %s", uv_strerror(err));
    }

    void Timer::Restart() {
        

        if (this->closed)
            LError("closed");

        if (uv_is_active(reinterpret_cast<uv_handle_t*> (this->uvHandle)) != 0)
            Stop();

        int err =
                uv_timer_start(this->uvHandle, static_cast<uv_timer_cb> (onTimer), this->timeout, this->repeat);

        if (err != 0)
            LError("uv_timer_start() failed: %s", uv_strerror(err));
    }

    inline void Timer::OnUvTimer(int timerID) {
        
        // Notify the listener.
        if(this->listener)
        {
            // if(timerID > 0)
            // this->listener->OnTimer1(this,timerID);
            // else
            // {
             this->listener->OnTimer(this);
            //}
        }
        
        if(cb_timeout)
        cb_timeout();
    }
} // namespace base
