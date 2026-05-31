
#include <so_common/so_logger.h>

int syscall_soloader(int number, ...) {
    so_logw("syscall(%d,...): not implemented", number);
    return -1;
}
