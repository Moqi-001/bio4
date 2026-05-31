
#include <so_common/so_logger.h>

int ioctl_soloader(int fd, int request, ...) {
    so_logw("ioctl(%d, %d, ...): not implemented", fd, request);
    return 0;
}
