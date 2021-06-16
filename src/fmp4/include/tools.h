#ifndef TOOLS_HEADER_GUARD 
#define TOOLS_HEADER_GUARD



//
////#include "common.h"
//#include "constant.h"
////#include "logging.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/** posix time doodle:
*
* time_t (sec)
* 
* struct timeval (tv_sec, tv_usec)
* 
* struct timespec (tv_sec, tv_nsec)
* 
*/

static const int64_t NANOSEC_PER_SEC = 1000000000;

long int getCurrentMsTimestamp(); ///< Utility function: returns current unix epoch timestamp in milliseconds.  Uses timeval
//
long int getMsDiff(timeval tv1, timeval tv2); ///< Utility function: return timedif of two timeval structs in milliseconds

struct timeval msToTimeval(long int mstimestamp); ///< Milliseconds to timeval

long int timevalToMs(struct timeval time); /// Timeval to milliseconds

//bool slotOk(SlotNumber n_slot); ///< Checks the slot number range

void normalize_timespec(struct timespec *ts, time_t sec, int64_t nanosec);


#ifdef BIG_ENDIAN
inline uint32_t deserialize_uint32_big_endian(unsigned char *buffer)
{
    uint32_t value = 0;

    value |= buffer[0] << 24;
    value |= buffer[1] << 16;
    value |= buffer[2] << 8;
    value |= buffer[3];
    return value;
}
#else // either not defined or little endian
// deserialize value from big endian in little endian system
// byte1 byte2 byte3 byte4 => byte4 byte3 ..
inline uint32_t deserialize_uint32_big_endian(unsigned char *buffer)
{
    uint32_t value = 0;

    value |= buffer[3] << 24;
    value |= buffer[2] << 16;
    value |= buffer[1] << 8;
    value |= buffer[0];
    return value;
}
#endif

/*
// this was removed from live555 at some point
unsigned our_inet_addr(const char* cp)
{
        return inet_addr(cp);
}
*/

#endif
