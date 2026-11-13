#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include <sys/fcntl.h>
#include <sys/unistd.h>

#include <so_common/so_logger.h>

#include "bionic_fcntl.h"

static inline int oflags_bionic_to_newlib(int flags) {
    int out = O_RDONLY;
    if (flags & BIONIC_O_RDWR)
        out |= O_RDWR;
    else if (flags & BIONIC_O_WRONLY)
        out |= O_WRONLY;

    if ((flags & BIONIC_O_CREAT))
        out |= O_CREAT;
    if (flags & BIONIC_O_EXCL)
        out |= O_EXCL;
    if (flags & BIONIC_O_TRUNC)
        out |= O_TRUNC;
    if (flags & BIONIC_O_APPEND)
        out |= O_APPEND;
    if (flags & BIONIC_O_NONBLOCK)
        out |= O_NONBLOCK;
    return out;
}

int open_soloader(const char *path, int oflag, ...) {
    mode_t mode = 0;
    if (oflag & BIONIC_O_CREAT) {
        va_list args;
        va_start(args, oflag);
        mode = va_arg(args, mode_t);
        va_end(args);
    }

    oflag = oflags_bionic_to_newlib(oflag);

    if (path[0] == '/') {
        if (strcmp(path, "/proc/cpuinfo") == 0) {
            path = "app0:/cpuinfo";
        } else if (strcmp(path, "/proc/meminfo") == 0) {
            path = "app0:/meminfo";
        } else if (strcmp(path, "/dev/urandom") == 0) {
            path = "app0:/urandom";
        }
    }

    int result = open(path, oflag, mode);
    if (result < 0) {
        so_loge("open('%s',0x%x): error!", path, oflag);
        return result;
    }

    so_logd("open('%s',0x%x,%d): %d", path, oflag, mode, result);
    return result;
}
