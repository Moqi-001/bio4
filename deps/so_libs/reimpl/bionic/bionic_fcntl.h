#ifndef BIONIC_FCNTL_H
#define BIONIC_FCNTL_H

#ifdef __cplusplus
extern "C" {
#endif

#define BIONIC_O_WRONLY    01
#define BIONIC_O_RDWR      02
#define BIONIC_O_CREAT     0100
#define BIONIC_O_EXCL      0200
#define BIONIC_O_TRUNC     01000
#define BIONIC_O_APPEND    02000
#define BIONIC_O_NONBLOCK  04000
#define BIONIC_O_DIRECTORY 00200000
#define BIONIC__O_TMPFILE  020000000
#define BIONIC_O_TMPFILE   (BIONIC__O_TMPFILE | BIONIC_O_DIRECTORY)

int open_soloader(const char *path, int oflag, ...);

#ifdef __cplusplus
};
#endif

#endif