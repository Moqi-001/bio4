#include <errno.h>
#include <malloc.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sys/fcntl.h>
#include <sys/unistd.h>

#include <psp2/kernel/error.h>
#include <psp2/kernel/threadmgr/msgpipe.h>

#include "sys/bionic_epoll.h"
#include "bionic_fd.h"
#include "bionic_malloc.h"

#include <so_common/so_logger.h>

#define PAGE_SIZE 4096

const unsigned int __page_size = PAGE_SIZE;

int getpagesize_soloader(void) {
    return __page_size;
}

int close_soloader(int fd) {
    // epoll fd 是偏移量方案（EPOLL_FD_BASE + 槽位），不在 newlib fd 表里
    if (fd >= EPOLL_FD_BASE) {
        int result = epoll_close_soloader(fd);
        so_logd("close(%d): %d (epoll)", fd, result);
        return result;
    }

    bionic_fd_pipe_clear_flags(fd);

    int result = close(fd);
    so_logd("close(%d): %d", fd, result);
    return result;
}

off_t lseek_soloader(int fd, off_t offset, int whence) {
    int result = lseek(fd, offset, whence);
    // so_logd("lseek(%d,%ld,%d): %d", fd, offset, whence, result);
    return result;
}

// msgpipe 的 O_NONBLOCK 支持。
// newlib 的 _read_r 对 PIPE 一律调阻塞的 sceKernelReceiveMsgPipe（不看 flags），
// 所以标志置位时这里改走 TryReceive；MSG_PIPE_EMPTY 即 EAGAIN。
// 注意 Try 版本的 unk1 必须是 0：传 1 是「收满 size」语义，try 模式下永远返回 EMPTY。
ssize_t read_soloader(int fd, void *buf, size_t count) {
    if (bionic_fd_type(fd) == BIONIC_FD_PIPE && (bionic_fd_pipe_flags(fd) & O_NONBLOCK)) {
        size_t  len = count > 4 * PAGE_SIZE ? 4 * PAGE_SIZE : count;
        SceSize received = 0;
        int     result   = sceKernelTryReceiveMsgPipe(bionic_fd_sce_uid(fd), buf, len, 0, &received);
        if (result == SCE_KERNEL_ERROR_MSG_PIPE_EMPTY) {
            errno = EAGAIN;
            return -1;
        }
        if (result < 0) {
            errno = EIO;
            so_loge("read(%d): TryReceiveMsgPipe error=0x%08X", fd, result);
            return -1;
        }
        return (ssize_t)received;
    }

    ssize_t result = read(fd, buf, count);
    // so_logd("read(%d,%p,%d): %d", fd, buf, count, result);
    return result;
}

ssize_t write_soloader(int fd, const void *buf, size_t count) {
    if (bionic_fd_type(fd) == BIONIC_FD_PIPE && (bionic_fd_pipe_flags(fd) & O_NONBLOCK)) {
        size_t  len = count > 4 * PAGE_SIZE ? 4 * PAGE_SIZE : count;
        SceSize sent = 0;
        int     uid = bionic_fd_sce_uid(fd);
        int result = sceKernelTrySendMsgPipe(uid, (void *)buf, len, 1, &sent);
        SceKernelMppInfo mi;
        memset(&mi, 0, sizeof(mi));
        mi.size = sizeof(mi);
        int ir  = sceKernelGetMsgPipeInfo(uid, &mi);
        so_logd("[WRITE] fd=%d uid=%d len=%d ret=0x%08X sent=%u info=%d free=%d/%d", fd, uid,
                (int)len, result, sent, ir, mi.freeSize, mi.bufSize);
        if (result == SCE_KERNEL_ERROR_MSG_PIPE_FULL) {
            errno = EAGAIN;
            return -1;
        }
        if (result < 0) {
            errno = EIO;
            so_loge("write(%d): TrySendMsgPipe error=0x%08X", fd, result);
            return -1;
        }
        return (ssize_t)sent;
    }

    ssize_t result = write(fd, buf, count);
    // so_logd("write(%d,%p,%d): %d", fd, buf, count, result);
    return result;
}

// 诊断：记录 abort() 的调用来源。游戏侧主动 abort 时没有其他线索，
// 这里把栈上落在代码段范围内的返回地址打印出来。
void abort_soloader(void) {
    so_loge("abort() called! ra=0x%08X", (unsigned)__builtin_return_address(0));

    struct mallinfo mi = mallinfo();
    so_loge("  heap: arena=%d fordblks=%d uordblks=%d", mi.arena, mi.fordblks, mi.uordblks);
    so_loge("  live tracked bytes=%u (alloc calls=%u, free calls=%u)", malloc_live_bytes(), malloc_alloc_calls(), malloc_free_calls());

    uint32_t *sp = (uint32_t *)__builtin_frame_address(0);
    for (int i = 0; i < 48 && sp; i++) {
        uint32_t v = sp[i];
        if (v >= 0x80400000u && v < 0x81700000u) {
            so_loge("  stack[%02d] = 0x%08X", i, v);
        }
    }

    abort();
}

int chdir_soloader(const char *dir) {
    char buf[1024];
    int  len = strlen(dir);
    if (dir[len - 1] != '/') {  // psvita的chdir需以/结尾
        if (dir == buf) {
            strcat(buf, "/");
        } else {
            strcpy(buf, dir);
            buf[len++] = '/';
            buf[len]   = '\0';
            dir        = buf;
        }
    }

    int result = chdir(dir);
    so_logd("chdir(%s): %d", dir, result);
    return result;
}

int symlink_soloader(const char *target, const char *linkpath) {
    so_loge("symlink(%s, %s): unsupport!", target, linkpath);
    return 0;
}
