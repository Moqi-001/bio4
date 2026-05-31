#ifndef BIONIC_SYS_VFS_H
#define BIONIC_SYS_VFS_H

#ifdef __cplusplus
extern "C" {
#endif

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

int statfs_soloader(const char *path, struct statfs *buf);

#ifdef __cplusplus
};
#endif

#endif