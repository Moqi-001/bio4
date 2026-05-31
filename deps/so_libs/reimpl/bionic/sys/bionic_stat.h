#ifndef BIONIC_SYS_STAT_H
#define BIONIC_SYS_STAT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/stat.h>

typedef struct __attribute__((__packed__)) stat64_bionic {
    unsigned long long st_dev;
    unsigned char      __pad0[4];
    unsigned long      __st_ino;
    unsigned int       st_mode;
    nlink_t            st_nlink;
    uid_t              st_uid;
    gid_t              st_gid;
    unsigned long long st_rdev;
    unsigned char      __pad3[4];
    long long          st_size;
    unsigned long      st_blksize;
    unsigned long long st_blocks;
    struct timespec    st_atim;
    struct timespec    st_mtim;
    struct timespec    st_ctim;
    unsigned long long st_ino;
} stat64_bionic;

int fstat_soloader(int fd, stat64_bionic *buf);
int stat_soloader(const char *path, stat64_bionic *buf);

#ifdef __cplusplus
};
#endif

#endif