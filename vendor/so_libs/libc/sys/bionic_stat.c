#include <sys/stat.h>

#include <so_common/so_logger.h>

#include "bionic_stat.h"

static inline void stat_newlib_to_bionic(const struct stat *src, struct stat_bionic *dst) {
    dst->st_dev        = src->st_dev;
    dst->__st_ino      = src->st_ino;
    dst->st_mode       = src->st_mode;
    dst->st_nlink      = src->st_nlink;
    dst->st_uid        = src->st_uid;
    dst->st_gid        = src->st_gid;
    dst->st_rdev       = src->st_rdev;
    dst->st_size       = src->st_size;
    dst->st_blksize    = src->st_blksize;
    dst->st_blocks     = src->st_blocks;
    dst->st_atime      = src->st_atime;
    dst->st_atime_nsec = 0;
    dst->st_mtime      = src->st_mtime;
    dst->st_mtime_nsec = 0;
    dst->st_ctime      = src->st_ctime;
    dst->st_ctime_nsec = 0;
    dst->st_ino        = src->st_ino;
}

int fstat_soloader(int fd, struct stat_bionic *st) {
    struct stat buf;
    int         result = fstat(fd, &buf);
    if (result == 0) {
        stat_newlib_to_bionic(&buf, st);
    }

    so_logd("fstat(%d,%p): %d", fd, st, result);
    return result;
}

int stat_soloader(const char *path, struct stat_bionic *st) {
    struct stat buf;
    int         result = stat(path, &buf);
    if (result == 0) {
        stat_newlib_to_bionic(&buf, st);
    }

    so_logd("stat(%s,%p): %d", path, st, result);
    return result;
}

int lstat_soloader(const char *path, struct stat_bionic *st) {
    struct stat buf;
    int         result = lstat(path, &buf);
    if (result == 0) {
        stat_newlib_to_bionic(&buf, st);
    }

    so_logd("lstat(%s,%p): %d", path, st, result);
    return result;
}
