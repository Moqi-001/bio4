#include <sys/select.h>

#include <so_common/so_logger.h>

int select_soloader(int n, fd_set *readfds, fd_set *writefds,
                    fd_set *exceptfds, struct timeval *timeout) {
    int result = 0;
    // so_logd("select_soloader(%d,%p,%p,%p,%p): %d", n, readfds, writefds, exceptfds, timeout, result);
    return result;
}
