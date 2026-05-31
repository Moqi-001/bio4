#include <sys/stat.h>

#include <so_common/so_logger.h>

#include "bionic_stat.h"

#define SC_INLINE static inline __attribute__((always_inline))

SC_INLINE void stat_newlib_to_bionic(const struct stat *src, stat64_bionic *dst) {
    dst->st_dev          = src->st_dev;
    dst->__st_ino        = src->st_ino;
    dst->st_ino          = src->st_ino;
    dst->st_mode         = src->st_mode;
    dst->st_nlink        = src->st_nlink;
    dst->st_uid          = src->st_uid;
    dst->st_gid          = src->st_gid;
    dst->st_rdev         = src->st_rdev;
    dst->st_size         = src->st_size;
    dst->st_blksize      = src->st_blksize;
    dst->st_blocks       = src->st_blocks;
    dst->st_atim.tv_sec  = src->st_atime;
    dst->st_atim.tv_nsec = 0;
    dst->st_mtim.tv_sec  = src->st_mtime;
    dst->st_mtim.tv_nsec = 0;
    dst->st_ctim.tv_sec  = src->st_ctime;
    dst->st_ctim.tv_nsec = 0;
}

int fstat_soloader(int fd, stat64_bionic *buf) {
    struct stat st;
    int         result = fstat(fd, &st);

    if (result == 0)
        stat_newlib_to_bionic(&st, buf);

    so_logd("fstat(%d): %d", fd, result);
    return result;
}

int stat_soloader(const char *path, stat64_bionic *buf) {
    struct stat st;
    int         result = stat(path, &st);

    if (result == 0)
        stat_newlib_to_bionic(&st, buf);

    so_logd("stat(%s): %d", path, result);
    return result;
}
