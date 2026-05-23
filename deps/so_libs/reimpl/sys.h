/*
 * Copyright (C) 2021      Andy Nguyen
 * Copyright (C) 2022      Rinnegatamante
 * Copyright (C) 2022-2023 Volodymyr Atamanenko
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

/**
 * @file  sys.h
 * @brief Implementations and wrappers for misc. system functions.
 */

#ifndef SOLOADER_SYS_H
#define SOLOADER_SYS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/time.h>

struct statfs {
    long f_type;     /* type of filesystem */
    long f_bsize;    /* optimal transfer block size */
    long f_blocks;   /* total data blocks in file system */
    long f_bfree;    /* free blocks in fs */
    long f_bavail;   /* free blocks avail to non-superuser */
    long f_files;    /* total file nodes in file system */
    long f_ffree;    /* free file nodes in fs */
    long f_fsid;     /* file system id */
    long f_namelen;  /* maximum length of filenames */
    long f_spare[6]; /* spare for later */
};

#define PAGE_SIZE 4096

clock_t clock_soloader(void);

int clock_gettime_soloader(clockid_t clock_id, struct timespec *tp);

int clock_getres_soloader(clockid_t clock_id, struct timespec *result);

int __system_property_get_soloader(const char *name, char *value);

void assert2(const char *f, int l, const char *func, const char *msg);

void syscall(int c);

void __stack_chk_fail_soloader();

void abort_soloader();

void exit_soloader(int status);

int __atomic_dec(volatile int *ptr);

int __atomic_inc(volatile int *ptr);

int __atomic_swap(int new_value, volatile int *ptr);

int __atomic_cmpxchg(int old_value, int new_value, volatile int *ptr);

char *getenv_soloader(const char *name);

int setenv_soloader(const char *name, const char *value, int overwrite);

int getpagesize(void);

int statfs_soloader(const char *path, struct statfs *buf);

#ifdef __cplusplus
};
#endif

#endif  // SOLOADER_SYS_H
