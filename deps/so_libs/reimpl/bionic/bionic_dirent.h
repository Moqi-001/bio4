#ifndef BIONIC_DIRENT_H
#define BIONIC_DIRENT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include <sys/dirent.h>

#ifndef DT_UNKNOWN
#define DT_UNKNOWN 0
#define DT_FIFO    1
#define DT_CHR     2
#define DT_DIR     4
#define DT_BLK     6
#define DT_REG     8
#define DT_LNK     10
#define DT_SOCK    12
#define DT_WHT     14
#endif

typedef struct __attribute__((__packed__)) dirent64_bionic {
    int16_t       d_ino;        // 2 bytes // offset 0x0
    int64_t       d_off;        // 8 bytes // offset 0x2
    uint64_t      d_reclen;     // 8 bytes // 0xA
    unsigned char d_type;       // 1 byte // offset 0x12
    char          d_name[256];  // 256 bytes // offset 0x13
} dirent64_bionic;

typedef struct DIR_bionic DIR_bionic;

DIR_bionic      *opendir_soloader(char *dirpath);
DIR_bionic      *fdopendir_soloader(int fd);
dirent64_bionic *readdir_soloader(DIR_bionic *dirp);
int              readdir_r_soloader(DIR_bionic *dirp, dirent64_bionic *entry, dirent64_bionic **result);
int              closedir_soloader(DIR_bionic *dirp);
void             rewinddir_soloader(DIR_bionic *dirp);
int              dirfd_soloader(DIR_bionic *dirp);

#ifdef __cplusplus
};
#endif

#endif