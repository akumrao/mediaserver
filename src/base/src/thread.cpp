
#include "base/thread.h"
#include "base/logger.h"
#include "base/platform.h"
#include <assert.h>
#include <memory>


using std::endl;


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
