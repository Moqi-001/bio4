#ifndef ANDROID_SHARED_BUFFER_H
#define ANDROID_SHARED_BUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    ANDFROID_SHARED_BUFFER_KEEP_STORAGE = 0x00000001
};

typedef struct SharedBuffer SharedBuffer;

SharedBuffer *android_SharedBuffer_alloc(size_t size);
int           android_SharedBuffer_dealloc(SharedBuffer *sb);
SharedBuffer *android_SharedBuffer_attemptEdit(SharedBuffer *sb);
SharedBuffer *android_SharedBuffer_edit(SharedBuffer *sb);
SharedBuffer *android_SharedBuffer_editResize(SharedBuffer *sb, size_t new_size);
SharedBuffer *android_SharedBuffer_reset(SharedBuffer *sb, size_t new_size);
void          android_SharedBuffer_acquire(SharedBuffer *sb);
int32_t       android_SharedBuffer_release(SharedBuffer *sb, uint32_t flags);

size_t        android_SharedBuffer_size(SharedBuffer *sb);
void         *android_SharedBuffer_data(SharedBuffer *sb);
SharedBuffer *android_SharedBuffer_bufferFromData(void *data);
size_t        android_SharedBuffer_sizeFromData(void *data);
bool          android_SharedBuffer_onlyOwner(SharedBuffer *sb);

#ifdef __cplusplus
}
#endif

#endif