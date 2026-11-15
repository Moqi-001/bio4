#include <stdint.h>

#include <sys/time.h>

#include <psp2/kernel/processmgr.h>
#include <psp2/rtc.h>
#include <so_common/so_logger.h>

#include "bionic_time.h"

// 1969 years in microseconds, used to adjust SCE tick to UNIX timestamp
#define __epoch 62135587294000000

clock_t clock_soloader(void) {
    return sceKernelGetProcessTimeLow();
}

int clock_gettime_soloader(clockid_t clock_id, struct timespec *tp) {
    switch (clock_id) {
        case BIONIC_CLOCK_MONOTONIC:
        case BIONIC_CLOCK_MONOTONIC_RAW:
        case BIONIC_CLOCK_MONOTONIC_COARSE:
        case BIONIC_CLOCK_BOOTTIME:
        case BIONIC_CLOCK_BOOTTIME_ALARM:
        case BIONIC_CLOCK_SGI_CYCLE:
        case BIONIC_CLOCK_PROCESS_CPUTIME_ID:
        case BIONIC_CLOCK_THREAD_CPUTIME_ID: {
            uint64_t proctime = sceKernelGetProcessTimeWide();

            tp->tv_sec  = (proctime / 1000000);
            tp->tv_nsec = ((proctime - (tp->tv_sec * 1000000)) * 1000);
            break;
        }
        case BIONIC_CLOCK_REALTIME:
        case BIONIC_CLOCK_REALTIME_COARSE:
        case BIONIC_CLOCK_REALTIME_ALARM:
        case BIONIC_CLOCK_TAI: {
            SceRtcTick tick;
            sceRtcGetCurrentTick(&tick);
            tick.tick -= __epoch;

            tp->tv_sec  = (tick.tick / 1000000);
            tp->tv_nsec = ((tick.tick - (tp->tv_sec * 1000000)) * 1000);
            break;
        }
        default:
            so_loge("clock_gettime / unexpected clock id %lu", clock_id);
    }

    return 0;
}

int clock_getres_soloader(clockid_t clock_id, struct timespec *result) {
    result->tv_sec  = 0;
    result->tv_nsec = 1000;
    return 0;
}
