#include <errno.h>
#include <stdint.h>
#include <string.h>

#include <psp2/net/net.h>
#include <so_common/so_logger.h>

#include "bionic_epoll.h"

#include "../vitadescriptor.h"
#include "../vitaerror.h"

int epoll_create_soloader(int size) {
    if (size <= 0) {
        errno = EINVAL;
        so_loge("epoll_create(%d): invalid arguments!", size);
        return -1;
    }

    int eid = sceNetEpollCreate("epoll", 0);
    if (eid < 0) {
        errno = __vita_scenet_errno_to_errno(eid);
        so_loge("epoll_create(%d): error=0x%x", size, eid);
        return -1;
    }

    int fd = __vita_acquire_descriptor();
    if (fd < 0) {
        errno = EMFILE;
        so_loge("epoll_create(%d): too many open files!", size);
        sceNetEpollDestroy(eid);
        return -1;
    }

    __vita_fdmap[fd]->sce_uid = eid;
    __vita_fdmap[fd]->type    = VITA_DESCRIPTOR_EPOLL;

    so_logd("epoll_create(%d): %d", size, fd);
    return fd;
}

int epoll_ctl_soloader(int epfd, int op, int fd, struct epoll_event_bionic *event) {
    if (!event) {
        errno = EINVAL;
        so_loge("epoll_ctl(%d,%d,%d,%p): invalid arguments!", epfd, op, fd, event);
        return -1;
    }

    int sce_eid = fd_to_scefd(epfd);
    int sce_fd  = fd_to_scefd(fd);
    int sce_op;

    switch (op) {
        case EPOLL_CTL_ADD:
            sce_op = SCE_NET_EPOLL_CTL_ADD;
            break;
        case EPOLL_CTL_MOD:
            sce_op = SCE_NET_EPOLL_CTL_MOD;
            break;
        case EPOLL_CTL_DEL:
            sce_op = SCE_NET_EPOLL_CTL_DEL;
            break;
        default:
            errno = EINVAL;
            so_loge("epoll_ctl(%d,%d,%d,%p): invalid arguments!", epfd, op, fd, event);
            return -1;
    }

    SceNetEpollEvent sce_event;
    memset(&sce_event, 0, sizeof(sce_event));

    sce_event.events = SCE_NET_EPOLLDESCID;

    if (event->events & EPOLLIN) {
        sce_event.events |= SCE_NET_EPOLLIN;
    }
    if (event->events & EPOLLOUT) {
        sce_event.events |= SCE_NET_EPOLLOUT;
    }
    if (event->events & EPOLLERR) {
        sce_event.events |= SCE_NET_EPOLLERR;
    }
    if (event->events & EPOLLHUP) {
        sce_event.events |= SCE_NET_EPOLLHUP;
    }

    memcpy(&sce_event.data, &event->data, sizeof(epoll_data_t_bionic));

    int result = sceNetEpollControl(sce_eid, sce_op, sce_fd, &sce_event);
    if (result < 0) {
        errno = __vita_scenet_errno_to_errno(result);
        so_loge("epoll_ctl(%d,%d,%d,%p): error! (0x%x)", epfd, op, sce_fd, event, result);
        return -1;
    }

    so_logd("epoll_ctl(%d,%d,%d,%p): 0", epfd, op, fd, event);
    return 0;
}

int epoll_wait_soloader(int epfd, struct epoll_event_bionic *events, int max, int timeout) {
    if (!events || max <= 0) {
        errno = EINVAL;
        so_loge("epoll_wait(%d,%p,%d,%d): invalid arguments!", epfd, events, max, timeout);
        return -1;
    }

    int      sce_eid     = fd_to_scefd(epfd);
    uint64_t sce_timeout = timeout * 1000;

    SceNetEpollEvent sce_events[max];
    int              nev = sceNetEpollWait(sce_eid, sce_events, max, sce_timeout);
    if (nev < 0) {
        errno = __vita_scenet_errno_to_errno(nev);
        so_loge("epoll_wait(%d,%p,%d,%d): error! (0x%x)", epfd, events, max, timeout, nev);
        return -1;
    }

    for (int i = 0; i < nev; i++) {
        events[i].events = 0;

        if (sce_events[i].events & SCE_NET_EPOLLIN) {
            events[i].events |= EPOLLIN;
        }
        if (sce_events[i].events & SCE_NET_EPOLLOUT) {
            events[i].events |= EPOLLOUT;
        }
        if (sce_events[i].events & SCE_NET_EPOLLERR) {
            events[i].events |= EPOLLERR;
        }
        if (sce_events[i].events & SCE_NET_EPOLLHUP) {
            events[i].events |= EPOLLHUP;
        }

        memcpy(&events[i].data, &sce_events[i].data, sizeof(epoll_data_t_bionic));
    }

    so_logd("epoll_wait(%d,%p,%d,%d): %d", epfd, events, max, timeout, nev);
    return nev;
}
