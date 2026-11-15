#include <sys/statvfs.h>

#include <so_common/so_logger.h>

#include "bionic_vfs.h"

static inline void statvfs_to_bionic_statfs(const struct statvfs *src, struct statfs_bionic *dst) {
    dst->f_type        = 0xEF53;  // EXT4
    dst->f_bsize       = src->f_bsize;
    dst->f_blocks      = src->f_blocks;
    dst->f_bfree       = src->f_bfree;
    dst->f_bavail      = src->f_bavail;
    dst->f_files       = src->f_files;
    dst->f_ffree       = src->f_ffree;
    dst->f_fsid.val[0] = src->f_fsid;
    dst->f_fsid.val[1] = 0;
    dst->f_namelen     = src->f_namemax;
    dst->f_frsize      = src->f_frsize;
    dst->f_flags       = src->f_flag;
}

int fstatfs_soloader(int fd, struct statfs_bionic *buf) {
    struct statvfs st;
    int            result = fstatvfs(fd, &st);
    if (result == 0)
        statvfs_to_bionic_statfs(&st, buf);

    so_logd("fstatfs(%d,%p): %d", fd, buf, result);
    return result;
}

int statfs_soloader(const char *path, struct statfs_bionic *buf) {
    struct statvfs st;
    int            result = statvfs(path, &st);
    if (result == 0)
        statvfs_to_bionic_statfs(&st, buf);

    so_logd("statfs(%s,%p): %d", path, buf, result);
    return result;
}
