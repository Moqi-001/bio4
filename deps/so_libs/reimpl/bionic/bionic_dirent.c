#include <stdlib.h>
#include <string.h>

#include <sys/dir.h>
#include <sys/dirent.h>

#include <so_common/so_logger.h>

#include "bionic_dirent.h"

#define SC_INLINE static inline __attribute__((always_inline))

struct DIR_bionic {
    DIR            *newlib_dir;
    dirent64_bionic dirent;
};

SC_INLINE void dirent_newlib_to_bionic(const struct dirent *src, struct dirent64_bionic *dst) {
    strncpy(dst->d_name, src->d_name, sizeof(dst->d_name));
    dst->d_off    = 0;
    dst->d_reclen = 0;
    dst->d_type   = SCE_S_ISDIR(src->d_stat.st_mode) ? DT_DIR : DT_REG;
}

DIR_bionic *opendir_soloader(char *dirpath) {
    DIR_bionic *result = NULL;
    DIR        *dirp   = opendir(dirpath);
    if (dirp) {
        result             = malloc(sizeof(DIR_bionic));
        result->newlib_dir = dirp;
    }

    so_logd("opendir(%s): %p", dirpath, result);
    return result;
}

DIR_bionic *fdopendir_soloader(int fd) {
    DIR_bionic *result = NULL;
    DIR        *dirp   = fdopendir(fd);
    if (dirp) {
        result             = malloc(sizeof(DIR_bionic));
        result->newlib_dir = dirp;
    }

    so_logd("opendir(%d): %p", fd, result);
    return result;
}

dirent64_bionic *readdir_soloader(DIR_bionic *dirp) {
    dirent64_bionic *result = NULL;

    struct dirent *dp = readdir(dirp->newlib_dir);
    if (dp) {
        dirent_newlib_to_bionic(dp, &dirp->dirent);
        result = &dirp->dirent;
    }

    so_logd("readdir(%p): %p", dirp, result);
    return result;
}

int readdir_r_soloader(DIR_bionic *dirp, dirent64_bionic *entry, dirent64_bionic **result) {
    struct dirent  d;
    struct dirent *dp;

    int res = readdir_r(dirp->newlib_dir, &d, &dp);
    if (res == 0) {
        dirent_newlib_to_bionic(&d, entry);
        *result = dp ? entry : NULL;
    }

    so_logd("readdir_r(%p,%p,%p): %d", dirp, entry, result, res);
    return res;
}

int closedir_soloader(DIR_bionic *dirp) {
    int result = closedir(dirp->newlib_dir);
    free(dirp);

    so_logd("closedir(%p): %d", dirp, result);
    return result;
}

void rewinddir_soloader(DIR_bionic *dirp) {
    rewinddir(dirp->newlib_dir);
    so_logd("rewinddir(%p)", dirp);
}

int dirfd_soloader(DIR_bionic *dirp) {
    int fd = dirfd(dirp->newlib_dir);
    so_logd("dirfd(%p): %d", dirp, fd);
    return fd;
}
