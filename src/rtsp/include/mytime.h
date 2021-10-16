#ifndef AVUTIL_MYTIME_H
#define AVUTIL_MYTIME_H

#include <stdint.h>

/**
 * Get the current time in microseconds.
 */
int64_t av_gettime(void);


int64_t av_gettime_relative(void);


int av_gettime_relative_is_monotonic(void);


int av_usleep(unsigned usec);

#endif /* AVUTIL_MYTIME_H */
