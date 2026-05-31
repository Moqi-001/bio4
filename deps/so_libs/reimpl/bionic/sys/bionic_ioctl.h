#ifndef BIONIC_SYS_IOCTL_H_
#define BIONIC_SYS_IOCTL_H_

#ifdef __cplusplus
extern "C" {
#endif

int ioctl_soloader(int fd, int request, ...);

#ifdef __cplusplus
};
#endif

#endif