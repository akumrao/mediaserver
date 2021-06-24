
#include "webrtc/tools.h"
#include <sys/time.h>
long int getCurrentMsTimestamp() {
  struct timeval time;
  gettimeofday(&time, nullptr);
  return time.tv_sec*1000+time.tv_usec/1000;
}


long int getMsDiff(timeval tv1, timeval tv2) {
  return (tv1.tv_sec-tv2.tv_sec)*1000 + (tv1.tv_usec-tv2.tv_usec)/1000;
}


struct timeval msToTimeval(long int mstimestamp) {
  struct timeval fPresentationTime;
  fPresentationTime.tv_sec   =(mstimestamp/1000); // secs
  fPresentationTime.tv_usec  =(mstimestamp-fPresentationTime.tv_sec*1000)*1000; // microsecs
  return fPresentationTime;
}

long int timevalToMs(struct timeval time) {
    return time.tv_sec*1000+time.tv_usec/1000;
}


//bool slotOk(SlotNumber n_slot) {
//  if (n_slot>I_MAX_SLOTS) {
//    std::cout << "WARNING! slot overflow with "<<n_slot<<" increase I_MAX_SLOTS in sizes.h"<<std::endl;
//    return false;
//  }
//  return true;
//}


// normalized_timespec : normalize to nanosec and sec
void normalize_timespec(struct timespec *ts, time_t sec, int64_t nanosec)
{
  while (nanosec >= NANOSEC_PER_SEC) {
    asm("" : "+rm"(nanosec));
    nanosec -= NANOSEC_PER_SEC;
    ++sec;
  }
  while (nanosec < 0) {
    asm("" : "+rm"(nanosec));
    nanosec += NANOSEC_PER_SEC;
    --sec;
  }
  ts->tv_sec  = sec;
  ts->tv_nsec = nanosec;
}


