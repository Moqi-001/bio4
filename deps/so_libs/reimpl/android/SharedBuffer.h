#ifndef ANDROID_SHARED_BUFFER_H
#define ANDROID_SHARED_BUFFER_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    eKeepStorage = 0x00000001
};

typedef struct ASharedBuffer ASharedBuffer;

ASharedBuffer *ASharedBuffer_alloc(size_t size);
ASharedBuffer *SharedBuffer_allocWithData(size_t size, void *data);
int            ASharedBuffer_dealloc(ASharedBuffer *thiz);
ASharedBuffer *ASharedBuffer_attemptEdit(ASharedBuffer *thiz);
ASharedBuffer *ASharedBuffer_edit(ASharedBuffer *thiz);
ASharedBuffer *ASharedBuffer_editResize(ASharedBuffer *thiz, size_t new_size);
ASharedBuffer *ASharedBuffer_reset(ASharedBuffer *thiz, size_t new_size);
void           ASharedBuffer_acquire(ASharedBuffer *thiz);
int32_t        ASharedBuffer_release(ASharedBuffer *thiz, uint32_t flags);

void          *ASharedBuffer_data(ASharedBuffer *thiz);
size_t         ASharedBuffer_size(ASharedBuffer *thiz);
ASharedBuffer *ASharedBuffer_bufferFromData(void *data);
size_t         ASharedBuffer_sizeFromData(void *data);
bool           ASharedBuffer_onlyOwner(ASharedBuffer *thiz);

#ifdef __cplusplus
}
#endif

#endif