#include <errno.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <sys/socket.h>

#include <psp2/kernel/processmgr.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/kernel/threadmgr/msgpipe.h>
#include <psp2/net/net.h>

#include <so_common/so_logger.h>

#include "bionic_epoll.h"
#include "../bionic_fd.h"

/*
 * 用户态 epoll。
 *
 * 不用 sceNetEpoll：它只接受 socket（newlib 的 msgpipe 传进去返回
 * SCE_NET_ERROR_EBADF），而 Android 引擎的 ALooper/glue 恰恰是围绕
 * msgpipe 做事件循环的。这里改为轮询各 fd 的就绪状态：
 *   * PIPE（newlib msgpipe）：sceKernelGetMsgPipeInfo 的 freeSize
 *   * SOCKET：sceNetRecv(MSG_PEEK | MSG_DONTWAIT) 探测，不消费数据
 *   * FILE/TTY：恒可读可写（与 newlib 自带 poll() 的约定一致）
 * 无就绪时按 4ms 粒度休眠再查（同 FalsoNDK 的轮询间隔），不会空转。
 */

#define EPOLL_MAX_INTERESTS    32
#define EPOLL_POLL_INTERVAL_US 4000

typedef struct {
    int                 fd;
    uint32_t            events;
    epoll_data_t_bionic data;
    bool                used;
} EpollInterest;

typedef struct {
    bool          used;
    EpollInterest items[EPOLL_MAX_INTERESTS];
} EpollInstance;

static EpollInstance g_epoll_slots[EPOLL_SLOT_MAX];

int epoll_create_soloader(int size) {
    if (size <= 0) {
        errno = EINVAL;
        so_loge("epoll_create(%d): invalid arguments!", size);
        return -1;
    }

    // epoll fd 采用偏移量方案（EPOLL_FD_BASE + 槽号），不进入 newlib fd 表
    for (int i = 0; i < EPOLL_SLOT_MAX; i++) {
        if (!g_epoll_slots[i].used) {
            memset(&g_epoll_slots[i], 0, sizeof(g_epoll_slots[i]));
            g_epoll_slots[i].used = true;
            so_logd("epoll_create(%d): %d", size, EPOLL_FD_BASE + i);
            return EPOLL_FD_BASE + i;
        }
    }

    errno = EMFILE;
    so_loge("epoll_create(%d): too many epoll instances!", size);
    return -1;
}

static EpollInstance *instance_from_fd(int epfd) {
    if (epfd < EPOLL_FD_BASE) {
        return NULL;
    }
    int slot = epfd - EPOLL_FD_BASE;
    if (slot < 0 || slot >= EPOLL_SLOT_MAX || !g_epoll_slots[slot].used) {
        return NULL;
    }
    return &g_epoll_slots[slot];
}

int epoll_close_soloader(int fd) {
    EpollInstance *instance = instance_from_fd(fd);
    if (!instance) {
        errno = EBADF;
        so_loge("epoll_close(%d): not an epoll fd!", fd);
        return -1;
    }

    memset(instance, 0, sizeof(*instance));
    so_logd("epoll_close(%d): 0", fd);
    return 0;
}

static EpollInterest *find_interest(EpollInstance *instance, int fd) {
    for (int i = 0; i < EPOLL_MAX_INTERESTS; i++) {
        if (instance->items[i].used && instance->items[i].fd == fd) {
            return &instance->items[i];
        }
    }
    return NULL;
}

int epoll_ctl_soloader(int epfd, int op, int fd, struct epoll_event_bionic *event) {
    EpollInstance *instance = instance_from_fd(epfd);
    if (!instance) {
        errno = EBADF;
        so_loge("epoll_ctl(%d,%d,%d,%p): not an epoll fd!", epfd, op, fd, event);
        return -1;
    }
    if (fd < 0) {
        errno = EBADF;
        return -1;
    }
    if (op != EPOLL_CTL_DEL && !event) {
        errno = EINVAL;
        so_loge("epoll_ctl(%d,%d,%d,%p): event is NULL!", epfd, op, fd, event);
        return -1;
    }

    EpollInterest *item = find_interest(instance, fd);

    switch (op) {
        case EPOLL_CTL_ADD:
            if (item) {
                errno = EEXIST;
                return -1;
            }
            for (int i = 0; i < EPOLL_MAX_INTERESTS; i++) {
                if (!instance->items[i].used) {
                    instance->items[i].used   = true;
                    instance->items[i].fd     = fd;
                    instance->items[i].events = event->events;
                    instance->items[i].data   = event->data;
                    so_logd("epoll_ctl(%d,ADD,%d,0x%X): 0", epfd, fd, event->events);
                    return 0;
                }
            }
            errno = ENOMEM;
            so_loge("epoll_ctl(%d,ADD,%d): no free interest slot!", epfd, fd);
            return -1;

        case EPOLL_CTL_MOD:
            if (!item) {
                errno = ENOENT;
                return -1;
            }
            item->events = event->events;
            item->data   = event->data;
            so_logd("epoll_ctl(%d,MOD,%d,0x%X): 0", epfd, fd, event->events);
            return 0;

        case EPOLL_CTL_DEL:
            if (!item) {
                errno = ENOENT;
                return -1;
            }
            memset(item, 0, sizeof(*item));
            so_logd("epoll_ctl(%d,DEL,%d): 0", epfd, fd);
            return 0;

        default:
            errno = EINVAL;
            so_loge("epoll_ctl(%d,%d,%d,%p): invalid op!", epfd, op, fd, event);
            return -1;
    }
}

// 计算单个 fd 的就绪事件（不消费数据）
static uint32_t interest_readiness(const EpollInterest *item) {
    uint32_t revents = 0;

    int uid = bionic_fd_sce_uid(item->fd);
    if (uid < 0) {
        return EPOLLERR;
    }

    switch (bionic_fd_type(item->fd)) {
        case BIONIC_FD_PIPE: {
            SceKernelMppInfo info;
            memset(&info, 0, sizeof(info));
            info.size = sizeof(info);
            if (sceKernelGetMsgPipeInfo(uid, &info) < 0) {
                so_logd("[EPOLL] fd=%d uid=%d GetMsgPipeInfo failed", item->fd, uid);
                return EPOLLERR;
            }
            static int dbg_count = 0;
            if (item->fd == 7 && (dbg_count++ % 2000) == 0) {
                so_logd("[EPOLL] poll fd=7 uid=%d free=%d/%d", uid, info.freeSize, info.bufSize);
            }
            if (info.freeSize < info.bufSize) {
                so_logd("[EPOLL] fd=%d readable free=%d/%d", item->fd, info.freeSize, info.bufSize);
                revents |= EPOLLIN;
            }
            if (info.freeSize > 0) {
                revents |= EPOLLOUT;
            }
            break;
        }

        case BIONIC_FD_SOCKET: {
            char c    = 0;
            int  peek = sceNetRecv(uid, &c, 1, MSG_PEEK | MSG_DONTWAIT);
            if (peek > 0) {
                revents |= EPOLLIN;
            } else if (peek == 0) {
                revents |= EPOLLHUP;
            }
            // 无数据可读时无法廉价判定可写性；socket 绝大多数时间可写
            revents |= EPOLLOUT;
            break;
        }

        case BIONIC_FD_FILE:
        case BIONIC_FD_TTY:
            // 常规文件恒可读写，与 newlib 自带 poll() 的处理一致
            revents |= EPOLLIN | EPOLLOUT;
            break;

        default:
            break;
    }

    return revents;
}

int epoll_wait_soloader(int epfd, struct epoll_event_bionic *events, int max, int timeout) {
    EpollInstance *instance = instance_from_fd(epfd);
    if (!instance) {
        errno = EBADF;
        so_loge("epoll_wait(%d,%p,%d,%d): not an epoll fd!", epfd, events, max, timeout);
        return -1;
    }
    if (!events || max <= 0) {
        errno = EINVAL;
        so_loge("epoll_wait(%d,%p,%d,%d): invalid arguments!", epfd, events, max, timeout);
        return -1;
    }

    bool     infinite = (timeout < 0);
    uint64_t deadline = infinite ? 0 : (uint64_t)sceKernelGetProcessTimeWide() + (uint64_t)timeout * 1000;

    for (;;) {
        int count = 0;

        for (int i = 0; i < EPOLL_MAX_INTERESTS && count < max; i++) {
            EpollInterest *item = &instance->items[i];
            if (!item->used) {
                continue;
            }

            uint32_t revents = interest_readiness(item);
            // 只上报被关注的位，但错误/挂断总是上报
            revents &= item->events | EPOLLERR | EPOLLHUP;
            if (!revents) {
                continue;
            }

            events[count].events = revents;
            events[count].data   = item->data;
            count++;
        }

        if (count > 0) {
            for (int i = 0; i < count; i++) {
                so_logd("[EPOLL] wait fd=%d events=0x%X", events[i].data.fd, events[i].events);
            }
            return count;
        }

        if (timeout == 0) {
            return 0;
        }

        uint32_t sleep_us = EPOLL_POLL_INTERVAL_US;
        if (!infinite) {
            uint64_t now = (uint64_t)sceKernelGetProcessTimeWide();
            if (now >= deadline) {
                return 0;
            }
            uint64_t remaining = deadline - now;
            if (remaining < sleep_us) {
                sleep_us = (uint32_t)remaining;
            }
        }

        sceKernelDelayThread(sleep_us);
    }
}
