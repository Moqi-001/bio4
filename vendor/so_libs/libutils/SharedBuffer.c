#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <so_common/so_logger.h>

#include "../libcutils/atomic.h"
#include "SharedBuffer.h"

struct ASharedBuffer {
    int32_t  refs;
    size_t   size;
    uint32_t reserved[2];
};

#define onlyOwner(buffer)      (buffer->refs == 1)
#define dataFromBuffer(buffer) ((void *)(buffer + 1))
#define bufferFromData(data)   ((ASharedBuffer *)data - 1)

ASharedBuffer *ASharedBuffer_alloc(size_t size) {
    ASharedBuffer *buffer = (ASharedBuffer *)malloc(sizeof(ASharedBuffer) + size);
    if (!buffer) {
        so_loge("[NDK][SharedBuffer] alloc(%u): cannot alloc memory!", size);
        return NULL;
    }

    buffer->refs = 1;
    buffer->size = size;

    // so_logd("[NDK][SharedBuffer] alloc(%u): %p", size, buffer);
    return buffer;
}

ASharedBuffer *SharedBuffer_allocWithData(size_t size, void *data) {
    ASharedBuffer *buffer = (ASharedBuffer *)malloc(sizeof(ASharedBuffer) + size);
    if (!buffer) {
        so_loge("[NDK][SharedBuffer] allocWithData(%u,%p): cannot alloc memory!", size, data);
        return NULL;
    }

    buffer->refs = 1;
    buffer->size = size;

    if (data != NULL && size > 0) {
        memcpy(dataFromBuffer(buffer), data, size);
    }

    // so_logd("[NDK][SharedBuffer] allocWithData(%u,%p): %p", size, data, buffer);
    return buffer;
}

int ASharedBuffer_dealloc(ASharedBuffer *released) {
    if (!released) {
        so_loge("[NDK][SharedBuffer] dealloc(%p): null ptr", released);
        return -1;
    }

    if (released->refs != 0) {
        so_loge("[NDK][SharedBuffer] dealloc(%p): invalid operation!", released);
        return -1;  // XXX: invalid operation
    }

    free(released);

    // so_logd("[NDK][SharedBuffer] dealloc(%p): 0", thiz);
    return 0;
}

ASharedBuffer *ASharedBuffer_attemptEdit(ASharedBuffer *thiz) {
    if (!thiz) {
        so_loge("[NDK][SharedBuffer] attemptEdit(%p): null ptr!", thiz);
        return NULL;
    }

    ASharedBuffer *buffer = onlyOwner(thiz) ? thiz : NULL;

    // so_logd("[NDK][SharedBuffer] attemptEdit(%p): %p", thiz, buffer);
    return buffer;
}

ASharedBuffer *ASharedBuffer_edit(ASharedBuffer *thiz) {
    if (!thiz) {
        so_loge("[NDK][SharedBuffer] edit(%p): null ptr!", thiz);
        return NULL;
    }

    ASharedBuffer *buffer;

    if (onlyOwner(thiz)) {
        buffer = thiz;
    } else {
        buffer = ASharedBuffer_alloc(thiz->size);
        if (buffer) {
            memcpy(dataFromBuffer(buffer), dataFromBuffer(thiz), thiz->size);
            ASharedBuffer_release(thiz, 0);
        }
    }

    // so_logd("[NDK][SharedBuffer] edit(%p): %p", thiz, buffer);
    return buffer;
}

ASharedBuffer *ASharedBuffer_editResize(ASharedBuffer *thiz, size_t new_size) {
    if (!thiz) {
        so_loge("[NDK][SharedBuffer] editResize(%p, %u): null ptr!", thiz, new_size);
        return NULL;
    }

    ASharedBuffer *buffer;

    if (onlyOwner(thiz)) {
        if (thiz->size == new_size) {
            buffer = thiz;
        } else {
            buffer = (ASharedBuffer *)realloc(thiz, sizeof(ASharedBuffer) + new_size);
            if (buffer != NULL) {
                buffer->size = new_size;
            }
        }
    } else {
        buffer = ASharedBuffer_alloc(new_size);
        if (buffer) {
            memcpy(dataFromBuffer(buffer), dataFromBuffer(thiz), buffer->size < thiz->size ? buffer->size : thiz->size);
            ASharedBuffer_release(thiz, 0);
        }
    }

    // so_logd("[NDK][SharedBuffer] editResize(%p,%u): %p", thiz, new_size, buffer);
    return buffer;
}

ASharedBuffer *ASharedBuffer_reset(ASharedBuffer *thiz, size_t new_size) {
    ASharedBuffer *buffer = ASharedBuffer_alloc(new_size);
    if (!buffer) {
        so_loge("[NDK][SharedBuffer] reset(%p,%u): out of memory!", thiz, new_size);
        return NULL;
    }
    ASharedBuffer_release(thiz, 0);

    // so_logd("[NDK][SharedBuffer] reset(%p,%u): %p", thiz, new_size, buffer);
    return buffer;
}

void ASharedBuffer_acquire(ASharedBuffer *thiz) {
    if (!thiz) {
        so_loge("[NDK][SharedBuffer] acquire(%p): null ptr!", thiz);
        return;
    }

    android_atomic_inc(&thiz->refs);
    // so_logd("[NDK][SharedBuffer] acquire(%p): %d", thiz, thiz->refs);
}

int32_t ASharedBuffer_release(ASharedBuffer *thiz, uint32_t flags) {
    int32_t prev = 1;
    if (thiz) {
        if (onlyOwner(thiz) || ((prev = android_atomic_dec(&thiz->refs)) == 1)) {
            thiz->refs = 0;
            if ((flags & eKeepStorage) == 0) {
                free(thiz);
            }
        }
    }

    // so_logd("[NDK][SharedBuffer] release(%p,0x%x): %d", thiz, flags, prev);
    return prev;
}

void *ASharedBuffer_data(ASharedBuffer *thiz) {
    return thiz ? dataFromBuffer(thiz) : NULL;
}

size_t ASharedBuffer_size(ASharedBuffer *thiz) {
    return thiz ? thiz->size : 0;
}

ASharedBuffer *ASharedBuffer_bufferFromData(void *data) {
    return data ? bufferFromData(data) : NULL;
}

size_t ASharedBuffer_sizeFromData(void *data) {
    return data ? (bufferFromData(data))->size : 0;
}

bool ASharedBuffer_onlyOwner(ASharedBuffer *thiz) {
    return thiz ? onlyOwner(thiz) : false;
}
