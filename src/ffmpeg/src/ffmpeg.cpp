
#include "ff/ffmpeg.h"
#include "base/logger.h"
#include "base/util.h"

#ifdef HAVE_FFMPEG

#include <mutex>
#include <iostream>
#include <stdexcept>


namespace base {
namespace ff {
namespace internal {


static int LockManagerOperation(void** lock, enum AVLockOp op)
{
    switch (op) {
        case AV_LOCK_CREATE:
            *lock = new std::mutex();
            if (!*lock)
                return 1;
            return 0;

        case AV_LOCK_OBTAIN:
            static_cast<std::mutex*>(*lock)->lock();
            return 0;

        case AV_LOCK_RELEASE:
            static_cast<std::mutex*>(*lock)->unlock();
            return 0;

        case AV_LOCK_DESTROY:
            delete static_cast<std::mutex*>(*lock);
            *lock = NULL;
            return 0;
    }
    return 1;
}


static std::mutex _mutex;
static int _refCount(0);


static void logCallback(void *ptr, int level, const char *fmt, va_list vl)
{
    char logbuf[256];
    vsnprintf(logbuf, sizeof(logbuf), fmt, vl);
    logbuf[sizeof(logbuf) - 1] = '\0';

    switch (level) {
    case AV_LOG_PANIC:
    case AV_LOG_FATAL:
    case AV_LOG_ERROR:
        LError("FFmpeg: ", util::trimRight<std::string>(logbuf))
        break;
    case AV_LOG_WARNING:
        LWarn("FFmpeg: ", util::trimRight<std::string>(logbuf))
        break;
    case AV_LOG_INFO:
        LDebug("FFmpeg: ", util::trimRight<std::string>(logbuf))
        break;
    default:
    case AV_LOG_VERBOSE:
    case AV_LOG_DEBUG:
         LError("FFmpeg: ", util::trimRight<std::string>(logbuf))
        break;
    }
}


void init()
{
    std::lock_guard<std::mutex> guard(_mutex);

    if (++_refCount == 1) {
        // Use an internal log callback.
        // av_log_set_callback(logCallback);
        // av_log_set_level(AV_LOG_INFO);

        // Optionally disable logging.
        // av_log_set_level(AV_LOG_QUIET);

        // Register our protocol glue code with FFmpeg.
        av_lockmgr_register(&LockManagerOperation);

        // Now register the rest of FFmpeg.
        av_register_all();

        // And devices if available.
#ifdef HAVE_FFMPEG_AVDEVICE
        avdevice_register_all();
#endif
    }
}


void uninit()
{
    std::lock_guard<std::mutex> guard(_mutex);

    if (--_refCount == 0) {
        av_lockmgr_register(NULL);
    }
}


} // namespace internal


void initializeFFmpeg()
{
    internal::init();
}


void uninitializeFFmpeg()
{
    internal::uninit();
}


std::string averror(const int error)
{
    static char error_buffer[255];
    av_strerror(error, error_buffer, sizeof(error_buffer));
    return error_buffer;
}

/*
 
Since you asked for libav* formats, I'm guessing you're after a code example.

To get a list of all the codecs use the av_codec_next api to iterate through the list of available codecs.


 
 */
void printInputFormats(std::ostream& ost, const char* delim)
{
    initializeFFmpeg(); // init here so reference is not held
    AVInputFormat* p = av_iformat_next(nullptr);
    while (p) {
        ost << p->name << delim;
        p = av_iformat_next(p);
    }
    uninitializeFFmpeg();
}


void printOutputFormats(std::ostream& ost, const char* delim)
{
    initializeFFmpeg(); // init here so reference is not held
    AVOutputFormat* p = av_oformat_next(nullptr);
    while (p) {
        ost << p->name << delim;
        p = av_oformat_next(p);
    }
    uninitializeFFmpeg();
}

/*
 
 To get a list of the formats, use av_format_next in the same way:
 */
void printEncoders(std::ostream& ost, const char* delim)
{
    initializeFFmpeg(); // init here so reference is not held
    AVCodec* p = av_codec_next(nullptr);
    while (p) {
        if (av_codec_is_encoder(p))
            ost << p->name << delim;
        p = p->next;
    }
    uninitializeFFmpeg();
}


} // namespace ff
} // namespace base


#else

namespace base {
namespace ff {


void initializeFFmpeg()
{
}


void uninitializeFFmpeg()
{
}


} // namespace ff
} // namespace base


#endif


