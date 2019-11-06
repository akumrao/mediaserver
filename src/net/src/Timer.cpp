
#include "net/Timer.h"
//#include "base/loop.h"
#include "base/logger.h"
#include "base/application.h"

namespace base
{

    /* Static methods for UV callbacks. */

    inline static void onTimer(uv_timer_t* handle) {
        static_cast<Timer*> (handle->data)->OnUvTimer();
    }

    inline static void onClose(uv_handle_t* handle) {
        delete handle;
    }

    /* Instance methods. */

    Timer::Timer(Listener* listener) : listener(listener) {
        LOG_CALL;

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
        LOG_CALL;

        if (!this->closed)
            Close();
    }

    void Timer::Close() {
        LOG_CALL;

        if (this->closed)
            return;

        this->closed = true;

        uv_close(reinterpret_cast<uv_handle_t*> (this->uvHandle), static_cast<uv_close_cb> (onClose));
    }

    void Timer::Start(uint64_t timeout, uint64_t repeat) {
        LOG_CALL;

        if (this->closed)
            LError("closed");

        this->timeout = timeout;
        this->repeat = repeat;

        if (uv_is_active(reinterpret_cast<uv_handle_t*> (this->uvHandle)) != 0)
            Stop();

        int err = uv_timer_start(this->uvHandle, static_cast<uv_timer_cb> (onTimer), timeout, repeat);

        if (err != 0)
            LError("uv_timer_start() failed: %s", uv_strerror(err));
    }

    void Timer::Stop() {
        LOG_CALL;

        if (this->closed)
            LError("closed");

        int err = uv_timer_stop(this->uvHandle);

        if (err != 0)
            LError("uv_timer_stop() failed: %s", uv_strerror(err));
    }

    void Timer::Reset() {
        LOG_CALL;

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
        LOG_CALL;

        if (this->closed)
            LError("closed");

        if (uv_is_active(reinterpret_cast<uv_handle_t*> (this->uvHandle)) != 0)
            Stop();

        int err =
                uv_timer_start(this->uvHandle, static_cast<uv_timer_cb> (onTimer), this->timeout, this->repeat);

        if (err != 0)
            LError("uv_timer_start() failed: %s", uv_strerror(err));
    }

    inline void Timer::OnUvTimer() {
        LOG_CALL;

        // Notify the listener.
        this->listener->OnTimer(this);
    }
} // namespace base