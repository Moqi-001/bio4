#ifndef BIONIC_TIME_H
#define BIONIC_TIME_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/time.h>

#define BIONIC_CLOCK_REALTIME           0
#define BIONIC_CLOCK_MONOTONIC          1
#define BIONIC_CLOCK_PROCESS_CPUTIME_ID 2
#define BIONIC_CLOCK_THREAD_CPUTIME_ID  3
#define BIONIC_CLOCK_MONOTONIC_RAW      4
#define BIONIC_CLOCK_REALTIME_COARSE    5
#define BIONIC_CLOCK_MONOTONIC_COARSE   6
#define BIONIC_CLOCK_BOOTTIME           7
#define BIONIC_CLOCK_REALTIME_ALARM     8
#define BIONIC_CLOCK_BOOTTIME_ALARM     9
#define BIONIC_CLOCK_SGI_CYCLE          10
#define BIONIC_CLOCK_TAI                11

clock_t clock_soloader(void);
int     clock_gettime_soloader(clockid_t clock_id, struct timespec *tp);
int     clock_getres_soloader(clockid_t clock_id, struct timespec *result);

#ifdef __cplusplus
};
#endif

#endif