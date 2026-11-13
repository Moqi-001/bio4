#include <malloc.h>

#include <so_common/so_logger.h>

#include "bionic_mman.h"

void *valloc_soloader(size_t size) {
    void *result = valloc(size);
    so_logd("valloc(%u): %p", size, result);
    return result;
}
