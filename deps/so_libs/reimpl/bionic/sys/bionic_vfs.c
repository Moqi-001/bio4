#include <string.h>

#include <sys/statvfs.h>

#include "bionic_vfs.h"

int statfs_soloader(const char *path, struct statfs *buf) {
    struct statvfs vfs_buf;

    int result = statvfs(path, &vfs_buf);
    if (result == 0) {
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
    }

    return result;
}
