#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

#include <sys/fcntl.h>
#include <sys/unistd.h>

#include <so_common/so_logger.h>

#include "bionic_fcntl.h"

#define SC_INLINE static inline __attribute__((always_inline))

SC_INLINE int oflags_bionic_to_newlib(int flags) {
    int out = 0;
    if (flags & BIONIC_O_RDWR)
        out |= O_RDWR;
    else if (flags & BIONIC_O_WRONLY)
        out |= O_WRONLY;
    else
        out |= O_RDONLY;
    if (flags & BIONIC_O_NONBLOCK)
        out |= O_NONBLOCK;
    if (flags & BIONIC_O_APPEND)
        out |= O_APPEND;
    if ((flags & BIONIC_O_CREAT) || (flags & BIONIC_O_TMPFILE))
        out |= O_CREAT;
    if (flags & BIONIC_O_TRUNC)
        out |= O_TRUNC;
    if (flags & BIONIC_O_EXCL)
        out |= O_EXCL;
    return out;
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
