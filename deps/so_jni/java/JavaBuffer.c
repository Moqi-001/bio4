#include <stdlib.h>

#include "JavaBuffer.h"

#include "../jni.h"
#include "JavaInstance.h"

typedef struct _JavaBuffer {
    uint32_t magic;
    jboolean is_direct;
    void    *data;
    jlong    size;
} _JavaBuffer;

#define JAVA_BUFFER_MAGIC 0x2F706522  // "JavaBuffer".hashCode()
#define JAVA_BUFFER_CAST(_o) \
    ((_JavaBuffer *)((_o) && ((_JavaBuffer *)(_o))->magic == JAVA_BUFFER_MAGIC ? (_o) : NULL))

static jint _jbuffer_ctor(_JavaBuffer *buf, jlong size) {
    if (!buf || size <= 0) return JNI_EINVAL;

    buf->magic     = JAVA_BUFFER_MAGIC;
    buf->is_direct = JNI_FALSE;
    buf->data      = malloc(size);
    if (!buf->data) {
        return JNI_ENOMEM;
    }
    buf->size = size;

    return JNI_OK;
}

static jint _jbuffer_ctor_direct(_JavaBuffer *buf, void *address, jlong size) {
    if (!buf || !address || size <= 0) return JNI_EINVAL;

    buf->magic     = JAVA_BUFFER_MAGIC;
    buf->is_direct = JNI_TRUE;
    buf->data      = address;
    buf->size      = size;

    return JNI_OK;
}

static void _jbuffer_dtor(_JavaBuffer *buf) {
    if (!buf) return;

    if (!buf->is_direct) {
        free(buf->data);
    }
}

JavaBuffer JavaBufferNew(jlong size) {
    _JavaBuffer *buf = (_JavaBuffer *)JavaInstanceNew(sizeof(_JavaBuffer), (JavaDtor_t)_jbuffer_dtor);
    if (!buf) {
        return NULL;
    }

    if (_jbuffer_ctor(buf, size) != JNI_OK) {
        JavaInstanceRelease(buf);
        return NULL;
    }

    return (JavaBuffer)buf;
}

JavaBuffer JavaBufferNewDirect(void *address, jlong size) {
    _JavaBuffer *buf = (_JavaBuffer *)JavaInstanceNew(sizeof(_JavaBuffer), (JavaDtor_t)_jbuffer_dtor);
    if (!buf) {
        return NULL;
    }

    if (_jbuffer_ctor_direct(buf, address, size) != JNI_OK) {
        JavaInstanceRelease(buf);
        return NULL;
    }

    return (JavaBuffer)buf;
}

void *JavaBufferGetData(JavaBuffer buffer) {
    _JavaBuffer *buf = JAVA_BUFFER_CAST(buffer);
    if (!buf) return NULL;

    return buf->data;
}

jlong JavaBufferGetSize(JavaBuffer buffer) {
    _JavaBuffer *buf = JAVA_BUFFER_CAST(buffer);
    if (!buf) return 0;

    return buf->size;
}
