/*
 * Copyright (C) 2021      Andy Nguyen
 * Copyright (C) 2022      Rinnegatamante
 * Copyright (C) 2022-2024 Volodymyr Atamanenko
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <dirent.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <sys/unistd.h>

#include <so_linker/so_linker.h>
#include <so_common/so_logger.h>

#include "io.h"

// Includes the following inline utilities:
// int oflags_musl_to_newlib(int flags);
// dirent64_bionic * dirent_newlib_to_bionic(struct dirent* dirent_newlib);
// void stat_newlib_to_bionic(struct stat * src, stat64_bionic * dst);
#define STRUCT_CONVERTERS_PRIVATE
#include "bits/_struct_converters.h"

FILE *fopen_soloader(const char *filename, const char *mode) {
    char buf[1024];
    if (strncmp(filename, "/sdcard/", 8) == 0) {
        snprintf(buf, sizeof(buf), "%s%s", DATA_PATH, filename + 8);
        filename = buf;
    }

    FILE *result = fopen(filename, mode);
    so_logd("fopen('%s','%s'): %p", filename, mode, result);
    return result;
}

int fclose_soloader(FILE *stream) {
    int result = fclose(stream);
    so_logd("fclose(%p): %d", stream, result);
    return result;
}

int fseek_soloader(FILE *stream, long offset, int whence) {
    int result = fseek(stream, offset, whence);
    so_logd("fseek(%p,%ld,%d): %d", stream, offset, whence, result);
    return result;
}

long ftell_soloader(FILE *stream) {
    long result = ftell(stream);
    so_logd("ftell(%p): %ld", stream, result);
    return result;
}

size_t fread_soloader(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t result = fread(ptr, size, nmemb, stream);
    so_logd("fread(%p,%zu,%zu,%p): %zu", ptr, size, nmemb, stream, result);
    return result;
}

size_t fwrite_soloader(void *ptr, size_t size, size_t nmemb, FILE *stream) {
    size_t result = fwrite(ptr, size, nmemb, stream);
    so_logd("fwrite(%p,%zu,%zu,%p): %zu", ptr, size, nmemb, stream, result);
    return result;
}

int fseeko_soloader(FILE *stream, long offset, int whence) {
    int result = fseeko(stream, offset, whence);
    so_logd("fseeko(%p,%ld,%d): %d", stream, offset, whence, result);
    return result;
}

off_t ftello_soloader(FILE *stream) {
    long result = ftello(stream);
    so_logd("ftello(%p): %ld", stream, result);
    return result;
}

int symlink_soloader(const char *target, const char *linkpath) {
    so_logd("symlink(%s, %s): unsupport!", target, linkpath);
    return 0;
}

int chdir_soloader(const char *dir) {
    char buf[1024];
    int  len = strlen(dir);
    if (dir[len - 1] != '/') {  // psvita的chdir需以/结尾
        strcpy(buf, dir);
        buf[len++] = '/';
        buf[len]   = '\0';
        dir        = buf;
    }

    int result = chdir(dir);
    so_logd("chdir(%s): %d", dir, result);
    return result;
}

static int fake_urandom_open() {
    int pipefd[2];
    if (pipe(pipefd) < 0)
        return -1;

    uint64_t buffer[64];
    for (int i = 0; i < sizeof(buffer) / sizeof(buffer[0]); i++) {
        buffer[i] = ((uint64_t)random() << 32) | (uint64_t)random();
    }

    write(pipefd[1], buffer, sizeof(buffer));
    close(pipefd[1]);
    return pipefd[0];
}

int open_soloader(const char *path, int oflag, ...) {
    int    result;
    mode_t mode = 0666;
    if (((oflag & BIONIC_O_CREAT) == BIONIC_O_CREAT) ||
        ((oflag & BIONIC_O_TMPFILE) == BIONIC_O_TMPFILE)) {
        va_list args;
        va_start(args, oflag);
        mode = (mode_t)(va_arg(args, int));
        va_end(args);
    }

    oflag = oflags_bionic_to_newlib(oflag);

    if (strcmp(path, "/proc/cpuinfo") == 0) {
        result = open("app0:/cpuinfo", oflag, mode);
    } else if (strcmp(path, "/proc/meminfo") == 0) {
        result = open("app0:/cpuinfo", oflag, mode);
    } else if (strcmp(path, "/dev/urandom") == 0) {
        result = fake_urandom_open();
    } else {
        result = open(path, oflag, mode);
    }

    if (result < 0) {
        so_logw("open(%s, %x): %d", path, oflag, result);
        return result;
    }

    so_logd("open(%s,0x%x): %d", path, oflag, result);
    return result;
}

int close_soloader(int fd) {
    int result = close(fd);
    so_logd("close(%d): %d", fd, result);
    return result;
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

int fcntl_soloader(int fd, int cmd, ...) {
    so_logw("fcntl(%d, %d, ...): not implemented", fd, cmd);
    return 0;
}

int ioctl_soloader(int fd, int request, ...) {
    so_logw("ioctl(%d, %d, ...): not implemented", fd, request);
    return 0;
}

int fsync_soloader(int fd) {
    int ret = fsync(fd);
    so_logd("fsync(%d): %d", fd, ret);
    return ret;
}

int dup_soloader(int fd) {
    so_logd("dup(fd): %d", fd, fd);
    return fd;
}

DIR *opendir_soloader(char *path) {
    DIR *result = opendir(path);
    so_logd("opendir(%s): %p", path, result);
    return result;
}

struct dirent64_bionic *readdir_soloader(DIR *dir) {
    static struct dirent64_bionic dirent_tmp;

    struct dirent *result = readdir(dir);
    so_logd("readdir(%p): %p", dir, result);

    if (result) {
        dirent64_bionic *entry_tmp = dirent_newlib_to_bionic(result);
        memcpy(&dirent_tmp, entry_tmp, sizeof(dirent64_bionic));
        free(entry_tmp);
        return &dirent_tmp;
    }

    return NULL;
}

int readdir_r_soloader(DIR *dirp, dirent64_bionic *entry, dirent64_bionic **result) {
    struct dirent  dirent_tmp;
    struct dirent *pdirent_tmp;

    int res = readdir_r(dirp, &dirent_tmp, &pdirent_tmp);

    if (result == 0) {
        dirent64_bionic *entry_tmp = dirent_newlib_to_bionic(&dirent_tmp);
        memcpy(entry, entry_tmp, sizeof(dirent64_bionic));
        *result = (pdirent_tmp != NULL) ? entry : NULL;
        free(entry_tmp);
    }

    so_logd("readdir_r(%p, %p, %p): %d", dirp, entry, result, result);
    return res;
}

int closedir_soloader(DIR *dir) {
    int result = closedir(dir);
    so_logd("closedir(%p): %d", dir, result);
    return result;
}
