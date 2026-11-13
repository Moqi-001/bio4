#include <stdarg.h>

#include <sys/unistd.h>

#include <so_common/so_logger.h>

#define FIONREAD 0x541B

static int handle_fionread(int fd, int *count) {
    off_t current = lseek(fd, 0, SEEK_CUR);
    if (current == (off_t)-1) return -1;

    off_t end = lseek(fd, 0, SEEK_END);
    if (end == (off_t)-1) return -1;

    lseek(fd, current, SEEK_SET);
    *count = (int)(end - current);
    return 0;
}

int ioctl_soloader(int fd, int request, ...) {
    int     result;
    va_list ap;
    void   *arg;

    va_start(ap, request);
    arg = va_arg(ap, void *);
    va_end(ap);

    so_logd("ioctl(%d,0x%x,...): %d", fd, request, 0);
    return 0;

    switch (request) {
        case FIONREAD:
            result = handle_fionread(fd, (int *)arg);
            break;

        default:
            so_loge("ioctl(%d,0x%x,...): unsupported request 0x%x!", fd, request, request);
            return -1;
    }

    so_logd("ioctl(%d,0x%x,...): %d", fd, request, result);
    return result;
}
