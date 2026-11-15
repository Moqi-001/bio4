#include "Timers.h"

#include <limits.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

nsecs_t systemTime(int clock) {
    struct timespec t;
    t.tv_sec = t.tv_nsec = 0;
    clock_gettime(CLOCK_REALTIME, &t);
    return (nsecs_t)t.tv_sec * 1000000000LL + t.tv_nsec;
}

int toMillisecondTimeoutDelay(nsecs_t referenceTime, nsecs_t timeoutTime) {
    int timeoutDelayMillis;
    if (timeoutTime > referenceTime) {
        uint64_t timeoutDelay = (uint64_t)(timeoutTime - referenceTime);
        if (timeoutDelay > (uint64_t)((INT_MAX - 1) * 1000000LL)) {
            timeoutDelayMillis = -1;
        } else {
            timeoutDelayMillis = (timeoutDelay + 999999LL) / 1000000LL;
        }
    } else {
        timeoutDelayMillis = 0;
    }
    return timeoutDelayMillis;
}
