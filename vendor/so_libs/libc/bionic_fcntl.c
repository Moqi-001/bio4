#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sys/fcntl.h>
#include <sys/unistd.h>

#include <so_common/so_logger.h>

#include "bionic_fcntl.h"
#include "bionic_fd.h"

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

/*
 * fcntl：补上 newlib 缺失的 msgpipe 标志支持。
 *
 * newlib 的 _fcntl_r 只给 FILE 和 SOCKET 处理 F_GETFL/F_SETFL，
 * PIPE（msgpipe）会落到函数末尾返回 EBADF。而 Android 引擎（含 AOSP 的
 * ALooper）会在自己的管道上设置 O_NONBLOCK。这里为 PIPE 自行记录标志，
 * 并由 read_soloader/write_soloader 在置位时改走 Try* 非阻塞调用；
 * 其它类型仍交给 newlib 处理。
 */
#define SETFL_FLAGS (O_NONBLOCK | O_APPEND)

int fcntl_soloader(int fd, int cmd, ...) {
    void   *arg = NULL;
    va_list args;
    va_start(args, cmd);
    arg = va_arg(args, void *);
    va_end(args);

    if (bionic_fd_type(fd) == BIONIC_FD_PIPE) {
        switch (cmd) {
            case F_GETFD:
            case F_SETFD:
                // FD_CLOEXEC 无对应实现，与 newlib 的处理保持一致
                return 0;

            case F_GETFL:
                return bionic_fd_pipe_flags(fd);

            case F_SETFL:
                bionic_fd_pipe_set_flags(fd, (int)(intptr_t)arg & SETFL_FLAGS);
                so_logd("fcntl(%d,F_SETFL,0x%X): 0 (msgpipe flags)", fd, (int)(intptr_t)arg);
                return 0;

            default:
                errno = EINVAL;
                so_loge("fcntl(%d,%d): unsupported command for msgpipe!", fd, cmd);
                return -1;
        }
    }

    return fcntl(fd, cmd, arg);
}
