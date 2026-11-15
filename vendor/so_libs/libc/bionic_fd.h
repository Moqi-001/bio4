#ifndef BIONIC_FD_H
#define BIONIC_FD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

// 描述符类型：与 newlib 的 vitadescriptor.h 中 DescriptorTypes 一一对应。
// newlib 的 read/write/fcntl 都用这个字段分派，本模块也用它判断 fd 种类。
enum {
    BIONIC_FD_FILE      = 0,
    BIONIC_FD_DIRECTORY = 1,
    BIONIC_FD_SOCKET    = 2,
    BIONIC_FD_TTY       = 3,
    BIONIC_FD_PIPE      = 4,
    BIONIC_FD_EPOLL     = 5,
};

// 查询 newlib 描述符的 sce uid；非法 fd 返回 -1
int bionic_fd_sce_uid(int fd);

// 查询描述符类型（BIONIC_FD_*）；非法 fd 返回 -1
int bionic_fd_type(int fd);

/*
 * msgpipe（PIPE 类描述符）的 O_* 标志。
 *
 * newlib 的 _fcntl_r 只给 FILE/SOCKET 处理 F_GETFL/F_SETFL，PIPE 直接 EBADF；
 * 而它的 read/write 对 PIPE 走无条件阻塞的 sceKernelReceive/SendMsgPipe，
 * 从不检查 flags。所以 O_NONBLOCK 由本模块自行记录，并由
 * read_soloader/write_soloader 在置位时改走 TryReceive/TrySend。
 */
int  bionic_fd_pipe_flags(int fd);
void bionic_fd_pipe_set_flags(int fd, int flags);
void bionic_fd_pipe_clear_flags(int fd);

#ifdef __cplusplus
};
#endif

#endif
