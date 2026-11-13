#ifndef BIONIC_SYS_SELECT_H
#define BIONIC_SYS_SELECT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/select.h>

int select_soloader(int n, fd_set *readfds, fd_set *writefds,
                    fd_set *exceptfds, struct timeval *timeout);

#ifdef __cplusplus
};
#endif

#endif