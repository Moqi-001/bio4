#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "so_logger.h"
#include "so_shared_buffer.h"
#include "so_utils.h"

typedef struct so_shared_buffer {
    int32_t  refs;
    size_t   size;
    uint32_t reserved[2];
} so_shared_buffer;

#define onlyOwner(buffer)      (buffer->refs == 1)
#define dataFromBuffer(buffer) ((void *)((buffer) + 1))
#define bufferFromData(data)   ((so_shared_buffer *)(data) - 1)

static so_shared_buffer *_shared_buffer_alloc(size_t size) {
    so_shared_buffer *buffer = (so_shared_buffer *)malloc(sizeof(so_shared_buffer) + size);
    if (!buffer) {
        return NULL;
    }

    buffer->refs = 1;
    buffer->size = size;

    return buffer;
}

static int32_t _shared_buffer_release(so_shared_buffer *buffer) {
    int32_t prev = 1;
    if ((onlyOwner(buffer) || ((prev = so_atomic_dec(&buffer->refs)) == 1))) {
        free(buffer);
    }

    return prev;
}

void *so_shared_buffer_alloc(size_t size) {
    so_shared_buffer *buffer = _shared_buffer_alloc(size);
    if (!buffer) {
        return NULL;
    }

    void *data = dataFromBuffer(buffer);
    return data;
}

void *so_shared_buffer_alloc_with_data(size_t size, const void *data) {
    so_shared_buffer *buffer = _shared_buffer_alloc(size);
    if (!buffer) {
        return NULL;
    }

    void *new_data = dataFromBuffer(buffer);
    if (data != NULL && size > 0) {
        memcpy(new_data, data, size);
    }

    return new_data;
}

void so_shared_buffer_retain(void *data) {
    if (!data) {
        return;
    }

    so_shared_buffer *buffer = bufferFromData(data);
    so_atomic_inc(&buffer->refs);
}

int32_t so_shared_buffer_release(void *data) {
    if (!data) {
        return -1;
    }

    so_shared_buffer *buffer = bufferFromData(data);
    int32_t           prev   = _shared_buffer_release(buffer);

    return prev;
}

int32_t so_shared_buffer_release_keep(void *data) {
    if (!data) {
        return -1;
    }

    so_shared_buffer *buffer = bufferFromData(data);
    int32_t           prev   = so_atomic_dec(&buffer->refs);

    return prev;
}

void so_shared_buffer_release_force(void *data) {
    if (!data) {
        return;
    }

    so_shared_buffer *buffer = bufferFromData(data);
    free(buffer);
}

void *so_shared_buffer_attempt_edit(void *data) {
    if (!data) {
        return NULL;
    }

    so_shared_buffer *buffer   = bufferFromData(data);
    void             *new_data = onlyOwner(buffer) ? data : NULL;

    return new_data;
}

void *so_shared_buffer_edit(void *data) {
    if (!data) {
        return NULL;
    }

    so_shared_buffer *buffer   = bufferFromData(data);
    size_t            size     = buffer->size;
    void             *new_data = NULL;

    if (onlyOwner(buffer)) {
        new_data = data;
    } else {
        so_shared_buffer *new_buffer = _shared_buffer_alloc(size);
        if (new_buffer != NULL) {
            new_data = dataFromBuffer(new_buffer);
            memcpy(new_data, data, size);
            _shared_buffer_release(buffer);
        }
    }

    return new_data;
}

void *so_shared_buffer_edit_resize(void *data, size_t new_size) {
    if (!data) {
        return NULL;
    }

    so_shared_buffer *buffer   = bufferFromData(data);
    size_t            old_size = buffer->size;
    void             *new_data = NULL;

    if (onlyOwner(buffer)) {
        if (old_size == new_size) {
            new_data = data;
        } else {
            so_shared_buffer *new_buffer = (so_shared_buffer *)realloc(buffer, sizeof(so_shared_buffer) + new_size);
            if (new_buffer != NULL) {
                new_buffer->size = new_size;
                new_data         = dataFromBuffer(new_buffer);
            }
        }
    } else {
        so_shared_buffer *new_buffer = _shared_buffer_alloc(new_size);
        if (new_buffer) {
            new_data = dataFromBuffer(new_buffer);
            memcpy(new_data, data, new_size < old_size ? new_size : old_size);
            _shared_buffer_release(buffer);
        }
    }

    return new_data;
}

void *so_shared_buffer_reset(void *data, size_t new_size) {
    if (!data) {
        return NULL;
    }

    so_shared_buffer *buffer     = bufferFromData(data);
    so_shared_buffer *new_buffer = _shared_buffer_alloc(new_size);
    if (!new_buffer) {
        return NULL;
    }

    _shared_buffer_release(buffer);

    void *new_data = dataFromBuffer(new_buffer);
    return new_data;
}

size_t so_shared_buffer_size(void *data) {
    if (!data) {
        return 0;
    }

    so_shared_buffer *buffer = bufferFromData(data);
    return buffer->size;
}
