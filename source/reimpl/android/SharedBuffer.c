#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "reimpl/android/atomic.h"
#include "reimpl/android/SharedBuffer.h"
#include "utils/logger.h"

struct SharedBuffer {
    int32_t  refs;
    size_t   size;
    uint32_t reserved[2];
};

#define onlyOwner(sb) (sb->refs == 1)
#define getData(sb)   ((void *)(sb + 1))

SharedBuffer *android_SharedBuffer_alloc(size_t size) {
    //l_debug("android_SharedBuffer_alloc(%u)", size);

    SharedBuffer *sb = (SharedBuffer *)malloc(sizeof(SharedBuffer) + size);
    if (!sb) {
        l_debug("android_SharedBuffer_alloc failed: cannot alloc memory");
        return NULL;
    }

    sb->refs = 1;
    sb->size = size;

    //l_debug("android_SharedBuffer_alloc over: sb=%p,data=%p", sb, getData(sb));
    return sb;
}

int android_SharedBuffer_dealloc(SharedBuffer *sb) {
    //l_debug("android_SharedBuffer_dealloc(%p)", sb);

    if (!sb) {
        l_debug("android_SharedBuffer_dealloc failed: null ptr");
        return -1;
    }

    if (sb->refs != 0) {
        l_debug("android_SharedBuffer_dealloc invalid operation: refs=%d", sb->refs);
        return -1;  // XXX: invalid operation
    }

    free(sb);

    //l_debug("android_SharedBuffer_dealloc ok");
    return 0;
}

SharedBuffer *android_SharedBuffer_attemptEdit(SharedBuffer *sb) {
    l_debug("android_SharedBuffer_attemptEdit(%p)", sb);

    if (!sb) {
        l_debug("android_SharedBuffer_attemptEdit failed: null ptr");
        return NULL;
    }

    SharedBuffer *buf = onlyOwner(sb) ? sb : NULL;

    l_debug("android_SharedBuffer_attemptEdit over: %p", buf);
    return buf;
}

SharedBuffer *android_SharedBuffer_edit(SharedBuffer *sb) {
    l_debug("android_SharedBuffer_edit(%p)", sb);

    if (!sb) {
        l_debug("android_SharedBuffer_edit failed: null ptr");
        return NULL;
    }

    SharedBuffer *buf;

    if (onlyOwner(sb)) {
        buf = sb;
    } else {
        buf = android_SharedBuffer_alloc(sb->size);
        if (buf) {
            memcpy(getData(buf), getData(sb), sb->size);
            android_SharedBuffer_release(sb, 0);
        }
    }

    l_debug("android_SharedBuffer_edit over: %p", buf);
    return sb;
}

SharedBuffer *android_SharedBuffer_editResize(SharedBuffer *sb, size_t new_size) {
    l_debug("android_SharedBuffer_editResize(%p,%u)", sb, new_size);

    if (!sb) {
        l_debug("android_SharedBuffer_editResize failed: null ptr");
        return NULL;
    }

    SharedBuffer *buf;

    if (onlyOwner(sb)) {
        buf = sb;
        if (buf->size != new_size) {
            buf = (SharedBuffer *)realloc(buf, sizeof(SharedBuffer) + new_size);
            if (buf != NULL)
                buf->size = new_size;
        }
    } else {
        buf = android_SharedBuffer_alloc(new_size);
        if (buf) {
            memcpy(getData(buf), getData(sb), buf->size < sb->size ? buf->size : sb->size);
            android_SharedBuffer_release(sb, 0);
        }
    }

    l_debug("android_SharedBuffer_editResize over: %p", buf);
    return buf;
}

SharedBuffer *android_SharedBuffer_reset(SharedBuffer *sb, size_t new_size) {
    l_debug("android_SharedBuffer_reset(%p,%u)", sb, new_size);

    // cheap-o-reset.
    SharedBuffer *buf = android_SharedBuffer_alloc(new_size);
    if (buf)
        android_SharedBuffer_release(sb, 0);

    l_debug("android_SharedBuffer_reset over: %p", buf);
    return buf;
}

void android_SharedBuffer_acquire(SharedBuffer *sb) {
    l_debug("android_SharedBuffer_acquire(%p)", sb);

    if (!sb) {
        l_debug("android_SharedBuffer_acquire failed: null ptr");
        return;
    }

    android_atomic_inc(&sb->refs);

    l_debug("android_SharedBuffer_acquire over: %d", sb->refs);
}

int32_t android_SharedBuffer_release(SharedBuffer *sb, uint32_t flags) {
    //l_debug("android_SharedBuffer_release(%p,%u)", sb, flags);

    if (!sb) {
        l_debug("android_SharedBuffer_release failed: null ptr");
        return 0;
    }

    int32_t prev = 1;
    if (onlyOwner(sb) || ((prev = android_atomic_dec(&sb->refs)) == 1)) {
        sb->refs = 0;
        if ((flags & ANDFROID_SHARED_BUFFER_KEEP_STORAGE) == 0)
            free(sb);
    }

    //l_debug("android_SharedBuffer_release over: prev_refs=%d", prev);
    return prev;
}

size_t android_SharedBuffer_size(SharedBuffer *sb) {
    return sb ? sb->size : 0;
}

void *android_SharedBuffer_data(SharedBuffer *sb) {
    return sb ? (void *)(sb + 1) : NULL;
}

SharedBuffer *android_SharedBuffer_bufferFromData(void *data) {
    return data ? (SharedBuffer *)data - 1 : NULL;
}

size_t android_SharedBuffer_sizeFromData(void *data) {
    return data ? (android_SharedBuffer_bufferFromData(data))->size : 0;
}

bool android_SharedBuffer_onlyOwner(SharedBuffer *sb) {
    return sb ? sb->refs == 1 : false;
}
