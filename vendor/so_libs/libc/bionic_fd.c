#include <string.h>

#include "bionic_fd.h"

#define MAX_FD 256

/*
 * 镜像 newlib 的 DescriptorTranslation（newlib/libc/sys/vita/vitadescriptor.h）：
 *   int sce_uid; DescriptorTypes type; int ref_count; char *filename; int flags;
 * 只声明开头两个字段，后面的字段不读。
 *
 * 这些描述符是 newlib 内部对象，只能通过 newlib 导出的 __vita_fd_grab 取得，
 * 不能自己下标 __vita_fdmap（其布局随版本变化）；grab 会引用计数 +1，
 * 用完必须 __vita_fd_drop。
 */
typedef struct {
    int sce_uid;
    int type;
} DescriptorTranslation;

extern DescriptorTranslation *__vita_fd_grab(int fd);
extern int                    __vita_fd_drop(DescriptorTranslation *map);

static DescriptorTranslation *grab(int fd) {
    if (fd < 0 || fd >= MAX_FD) {
        return NULL;
    }
    return __vita_fd_grab(fd);
}

int bionic_fd_sce_uid(int fd) {
    DescriptorTranslation *map = grab(fd);
    if (!map) {
        return -1;
    }
    int uid = map->sce_uid;
    __vita_fd_drop(map);
    return uid;
}

int bionic_fd_type(int fd) {
    DescriptorTranslation *map = grab(fd);
    if (!map) {
        return -1;
    }
    int type = map->type;
    __vita_fd_drop(map);
    return type;
}

// msgpipe 的 O_* 标志：索引即 fd，close 时清理
static int g_pipe_flags[MAX_FD];

int bionic_fd_pipe_flags(int fd) {
    if (fd < 0 || fd >= MAX_FD) {
        return 0;
    }
    return g_pipe_flags[fd];
}

void bionic_fd_pipe_set_flags(int fd, int flags) {
    if (fd < 0 || fd >= MAX_FD) {
        return;
    }
    g_pipe_flags[fd] = flags;
}

void bionic_fd_pipe_clear_flags(int fd) {
    if (fd < 0 || fd >= MAX_FD) {
        return;
    }
    g_pipe_flags[fd] = 0;
}
