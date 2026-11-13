#include <stdlib.h>
#include <string.h>

#include <psp2/io/dirent.h>
#include <psp2/io/fcntl.h>
#include <psp2/kernel/threadmgr.h>
#include <psp2/net/net.h>

#include "vitadescriptor.h"
#include "vitaerror.h"

extern SceKernelLwMutexWork _newlib_fd_mutex;

int scefd_to_fd(int sce_fd) {
    if (sce_fd < 0) return -1;

    sceKernelLockLwMutex(&_newlib_fd_mutex, 1, 0);
    for (int fd = 3; fd < MAX_OPEN_FILES; fd++) {
        if (__vita_fdmap[fd] && __vita_fdmap[fd]->sce_uid == sce_fd) {
            sceKernelUnlockLwMutex(&_newlib_fd_mutex, 1);
            return fd;
        }
    }

    sceKernelUnlockLwMutex(&_newlib_fd_mutex, 1);
    return -1;
}

int fd_to_scefd(int fd) {
    if (is_fd_valid(fd) && __vita_fdmap[fd]) {
        return __vita_fdmap[fd]->sce_uid;
    }

    return -1;
}

int __wrap___vita_fd_drop(DescriptorTranslation *map) {
    sceKernelLockLwMutex(&_newlib_fd_mutex, 1, 0);

    if (map->ref_count == 1) {
        int ret = 0;

        switch (map->type) {
            case VITA_DESCRIPTOR_FILE:
            case VITA_DESCRIPTOR_TTY: {
                ret = sceIoClose(map->sce_uid);
                if (map->filename) {
                    free(map->filename);
                }
                break;
            }
            case VITA_DESCRIPTOR_DIRECTORY: {
                ret = sceIoDclose(map->sce_uid);
                if (map->filename) {
                    free(map->filename);
                }
                break;
            }
            case VITA_DESCRIPTOR_SOCKET: {
                ret = sceNetSocketClose(map->sce_uid);
                if (ret < 0) {
                    sceKernelUnlockLwMutex(&_newlib_fd_mutex, 1);
                    return -__vita_scenet_errno_to_errno(ret);
                }
                break;
            }
            case VITA_DESCRIPTOR_PIPE: {
                ret = sceKernelDeleteMsgPipe(map->sce_uid);
                break;
            }
            case VITA_DESCRIPTOR_EPOLL: {
                ret = sceNetEpollDestroy(map->sce_uid);
                break;
            }
        }

        if (ret < 0) {
            sceKernelUnlockLwMutex(&_newlib_fd_mutex, 1);
            return -__vita_sce_errno_to_errno(ret, ERROR_GENERIC);
        }

        map->ref_count--;
        memset(map, 0, sizeof(DescriptorTranslation));
    } else {
        map->ref_count--;
    }

    sceKernelUnlockLwMutex(&_newlib_fd_mutex, 1);
    return 0;
}
