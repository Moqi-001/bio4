#ifndef BIONIC_SYS_VFS_H
#define BIONIC_SYS_VFS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    int val[2];
} __kernel_fsid_t;

struct statfs_bionic {
    uint32_t        f_type;
    uint32_t        f_bsize;
    uint64_t        f_blocks;
    uint64_t        f_bfree;
    uint64_t        f_bavail;
    uint64_t        f_files;
    uint64_t        f_ffree;
    __kernel_fsid_t f_fsid;
    uint32_t        f_namelen;
    uint32_t        f_frsize;
    uint32_t        f_flags;
    uint32_t        f_spare[4];
};

int fstatfs_soloader(int fd, struct statfs_bionic *buf);
int statfs_soloader(const char *path, struct statfs_bionic *buf);

#ifdef __cplusplus
};
#endif

#endif