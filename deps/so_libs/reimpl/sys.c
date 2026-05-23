/*
 * Copyright (C) 2021      Andy Nguyen
 * Copyright (C) 2022      Rinnegatamante
 * Copyright (C) 2022-2023 Volodymyr Atamanenko
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sys/errno.h>
#include <sys/statvfs.h>

#include <psp2/kernel/clib.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/rtc.h>
#include <so_linker/so_linker.h>
#include <so_common/so_logger.h>

#include "sys.h"

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

// 1969 years in microseconds, used to adjust SCE tick to UNIX timestamp
#define __epoch 62135587294000000

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

clock_t clock_soloader(void) {
    return sceKernelGetProcessTimeLow();
}

int sigaction(int signum, const struct sigaction *act, struct sigaction *oldact) {
    so_logw("sigaction(%d, ...): not implemented", signum);
    return 0;
}

int __system_property_get_soloader(const char *name, char *value) {
    so_logw("__system_property_get(%s, %p): not implemented", name, value);
    strncpy(value, "psvita", 7);
    return 7;
}

void assert2(const char *f, int l, const char *func, const char *msg) {
    so_loge("[%s:%d][%s] Assertion failed: %s", f, l, func, msg);
}

void syscall(int c) {
    so_logw("syscall(%d): not implemented", c);
}

void __stack_chk_fail_soloader() {
    so_loge("Stack collapsed at address %p", __builtin_return_address(0));
}

void abort_soloader() {
    so_loge("Abort called from address %p", __builtin_return_address(0));
    abort();
}

void exit_soloader(int status) {
    so_loge("Exit(%d) called from %p", status, __builtin_return_address(0));
    exit(status);
}

int __atomic_dec(volatile int *ptr) {
    return __sync_fetch_and_sub(ptr, 1);
}

int __atomic_inc(volatile int *ptr) {
    return __sync_fetch_and_add(ptr, 1);
}

int __atomic_swap(int new_value, volatile int *ptr) {
    int old_value;
    do {
        old_value = *ptr;
    } while (__sync_val_compare_and_swap(ptr, old_value, new_value) != old_value);
    return old_value;
}

int __atomic_cmpxchg(int old_value, int new_value, volatile int *ptr) {
    /* We must return 0 on success */
    return __sync_val_compare_and_swap(ptr, old_value, new_value) != old_value;
}

char *getenv_soloader(const char *var) {
    so_logw("getenv(\"%s\"): not implemented.", var);
    return NULL;
}

int setenv_soloader(const char *name, const char *value, int overwrite) {
    so_logw("setenv(\"%s\", \"%s\"): not implemented.", name, value);
    return 0;
}

int getpagesize(void) {
    return PAGE_SIZE;
}

int statfs_soloader(const char *path, struct statfs *buf) {
    struct statvfs vfs_buf;

    if (statvfs(path, &vfs_buf) != 0) {
        return -1;
    }

    memset(buf, 0, sizeof(struct statfs));
    buf->f_type    = 0xEF53;  // EXT4
    buf->f_bsize   = vfs_buf.f_bsize;
    buf->f_blocks  = vfs_buf.f_blocks;
    buf->f_bfree   = vfs_buf.f_bfree;
    buf->f_bavail  = vfs_buf.f_bavail;
    buf->f_files   = vfs_buf.f_files;
    buf->f_ffree   = vfs_buf.f_ffree;
    buf->f_fsid    = vfs_buf.f_fsid;
    buf->f_namelen = 255;

    return 0;
}
