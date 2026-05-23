#include <stdint.h>

#include <sys/time.h>

#include <so_common/so_logger.h>

#include "Timers.h"

nsecs_t systemTime(int clock) {
    struct timeval t;
    t.tv_sec = t.tv_usec = 0;
    gettimeofday(&t, NULL);
    nsecs_t nanoSeconds = (nsecs_t)(t.tv_sec) * 1000000000LL + (nsecs_t)(t.tv_usec) * 1000LL;
    // so_logd("systemTime(%d): (%lld)", clock, nanoSeconds);
    return nanoSeconds;
}
