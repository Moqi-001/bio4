/*
 * Copyright (C) 2021      Andy Nguyen
 * Copyright (C) 2022      Rinnegatamante
 * Copyright (C) 2022-2024 Volodymyr Atamanenko
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include "reimpl/io.h"

#include <dirent.h>
#include <psp2/kernel/threadmgr.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/unistd.h>

#ifdef USE_SCELIBC_IO
#include <libc_bridge/libc_bridge.h>
#endif

#include "utils/logger.h"
#include "utils/utils.h"

// Includes the following inline utilities:
// int oflags_musl_to_newlib(int flags);
// dirent64_bionic * dirent_newlib_to_bionic(struct dirent* dirent_newlib);
// void stat_newlib_to_bionic(struct stat * src, stat64_bionic * dst);
#include "reimpl/bits/_struct_converters.c"

void replace(char str[], char old, char new, int count) {
    for (size_t i = 0; i < count; i++) {
        if (str[i] == old) {
            str[i] = new;
        }
    }
}
FILE *fopen_soloader(const char *filename, const char *mode) {
    debugPrintf("fopen_soloader:%s", filename);
    // TODO: 判断文件名称, 帮他引导到 DATA_PATH/filename 下去 open
#ifdef USE_SCELIBC_IO
    FILE *ret = sceLibcBridge_fopen(filename, mode);
#else
    FILE *ret = fopen(filename, mode);
#endif

    if (ret) {
        l_debug("fopen(%s, %s): %p", filename, mode, ret);
    } else {
        l_warn("error fopen(%s, %s): %p", filename, mode, ret);
    }

    return ret;
}

int dup_soloader(int fd) {
    debugPrintf("dup_soloader:%d", fd);
    return fd;
}

size_t fread_soloader(void *data, size_t _size, size_t _n, FILE *fd) {
    debugPrintf("fread:%x  _size:%d  _n:%d", fd, _size, _n);
    return fread(data, _size, _n, fd);
}

FILE *fdopen_soloader(int fd, const char *mode) {
    char *filename = "ux0:data/Acrodea/app.exe";
    debugPrintf("fdopen_soloader:%s", filename);
#ifdef USE_SCELIBC_IO
    FILE *ret = sceLibcBridge_fopen(filename, mode);
#else
    FILE *ret = fopen(filename, mode);
#endif

    if (ret) {
        l_debug("fdopen(%s, %s): %p", filename, mode, ret);
    } else {
        debugPrintf("fdopen_soloader not find file:%s", filename);
        l_warn("fdopen(%s, %s): %p", filename, mode, ret);
    }

    return ret;
}

int open_soloader(const char *path, int oflag, ...) {
    debugPrintf("open_soloader:%s", path);
    if (strcmp(path, "/proc/cpuinfo") == 0) {
        return open_soloader("app0:/cpuinfo", oflag);
    } else if (strcmp(path, "/proc/meminfo") == 0) {
        return open_soloader("app0:/meminfo", oflag);
    }

    mode_t mode = 0666;
    if (((oflag & BIONIC_O_CREAT) == BIONIC_O_CREAT) ||
        ((oflag & BIONIC_O_TMPFILE) == BIONIC_O_TMPFILE)) {
        va_list args;
        va_start(args, oflag);
        mode = (mode_t)(va_arg(args, int));
        va_end(args);
    }

    oflag   = oflags_bionic_to_newlib(oflag);
    int ret = open(path, oflag, mode);
    if (ret >= 0)
        l_debug("open(%s, %x): %i", path, oflag, ret);
    else
        l_warn("open(%s, %x): %i", path, oflag, ret);
    return ret;
}

int fstat_soloader(int fd, stat64_bionic *buf) {
    struct stat st;
    int         res = fstat(fd, &st);

    if (res == 0)
        stat_newlib_to_bionic(&st, buf);

    l_debug("fstat(%i): %i", fd, res);
    return res;
}

int stat_soloader(const char *path, stat64_bionic *buf) {
    struct stat st;
    int         res = stat(path, &st);

    if (res == 0)
        stat_newlib_to_bionic(&st, buf);

    l_debug("stat(%s): %i", path, res);
    return res;
}

int fclose_soloader(FILE *f) {
#ifdef USE_SCELIBC_IO
    int ret = sceLibcBridge_fclose(f);
#else
    int ret = fclose(f);
#endif

    l_debug("fclose(%p): %i", f, ret);
    return ret;
}

int close_soloader(int fd) {
    int ret = close(fd);
    l_debug("close(%i): %i", fd, ret);
    return ret;
}

DIR *opendir_soloader(char *_pathname) {
    l_debug("opendir(\"%s\")", _pathname);
    DIR *ret = opendir(_pathname);
    l_debug("opendir(\"%s\"): %p", _pathname, ret);
    return ret;
}

struct dirent64_bionic *readdir_soloader(DIR *dir) {
    static struct dirent64_bionic dirent_tmp;

    struct dirent *ret = readdir(dir);
    l_debug("readdir(%p): %p", dir, ret);

    if (ret) {
        dirent64_bionic *entry_tmp = dirent_newlib_to_bionic(ret);
        memcpy(&dirent_tmp, entry_tmp, sizeof(dirent64_bionic));
        free(entry_tmp);
        return &dirent_tmp;
    }

    return NULL;
}

int readdir_r_soloader(DIR *dirp, dirent64_bionic *entry,
                       dirent64_bionic **result) {
    struct dirent  dirent_tmp;
    struct dirent *pdirent_tmp;

    int ret = readdir_r(dirp, &dirent_tmp, &pdirent_tmp);

    if (ret == 0) {
        dirent64_bionic *entry_tmp = dirent_newlib_to_bionic(&dirent_tmp);
        memcpy(entry, entry_tmp, sizeof(dirent64_bionic));
        *result = (pdirent_tmp != NULL) ? entry : NULL;
        free(entry_tmp);
    }

    l_debug("readdir_r(%p, %p, %p): %i", dirp, entry, result, ret);
    return ret;
}

int closedir_soloader(DIR *dir) {
    int ret = closedir(dir);
    l_debug("closedir(%p): %i", dir, ret);
    return ret;
}

int fcntl_soloader(int fd, int cmd, ...) {
    l_warn("fcntl(%i, %i, ...): not implemented", fd, cmd);
    return 0;
}

int ioctl_soloader(int fd, int request, ...) {
    l_warn("ioctl(%i, %i, ...): not implemented", fd, request);
    return 0;
}

int fsync_soloader(int fd) {
    int ret = fsync(fd);
    l_debug("fsync(%i): %i", fd, ret);
    return ret;
}
