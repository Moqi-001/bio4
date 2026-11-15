#ifndef BIONIC_FCNTL_H
#define BIONIC_FCNTL_H

#ifdef __cplusplus
extern "C" {
#endif

#define BIONIC_O_RDONLY    00000000
#define BIONIC_O_WRONLY    00000001
#define BIONIC_O_RDWR      00000002
#define BIONIC_O_CREAT     00000100
#define BIONIC_O_EXCL      00000200
#define BIONIC_O_TRUNC     00001000
#define BIONIC_O_APPEND    00002000
#define BIONIC_O_NONBLOCK  00004000

int open_soloader(const char *path, int oflag, ...);

// fcntl 包装：为 msgpipe（PIPE 类描述符）补上 newlib 不支持的 F_GETFL/F_SETFL
int fcntl_soloader(int fd, int cmd, ...);

#ifdef __cplusplus
};
#endif

#endif