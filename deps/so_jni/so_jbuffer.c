#include <stdlib.h>

#include "so_jni.h"
#include "so_jni_internal.h"

struct so_jbuffer {
    uint32_t magic;
    jboolean is_direct;
    void    *data;
    jlong    size;
};

#define SO_JBUFFER_MAGIC 0x2869B647  // "so_jbuffer".hashCode()

#define SO_JBUFFER_CAST(_o) \
    ((so_jbuffer *)((_o) && ((so_jbuffer *)(_o))->magic == SO_JBUFFER_MAGIC ? (_o) : NULL))

#define SO_JBUFFER_FROM_OBJ(_o)                    \
    ({                                             \
        void *_inst = so_jobject_get_instance(_o); \
        SO_JBUFFER_CAST(_inst);                    \
    })

static jint _jbuffer_ctor(so_jbuffer *buffer, jlong size) {
    if (!buffer || size <= 0) return JNI_EINVAL;

    buffer->magic     = SO_JBUFFER_MAGIC;
    buffer->is_direct = JNI_FALSE;
    buffer->data      = malloc(size);
    if (!buffer->data) {
        return JNI_ENOMEM;
    }
    buffer->size = size;

    return JNI_OK;
}

static jint _jbuffer_ctor_direct(so_jbuffer *buffer, void *address, jlong size) {
    if (!buffer || !address || size <= 0) return JNI_EINVAL;

    buffer->magic     = SO_JBUFFER_MAGIC;
    buffer->is_direct = JNI_TRUE;
    buffer->data      = address;
    buffer->size      = size;

    return JNI_OK;
}

static void _jbuffer_dtor(so_jbuffer *buffer) {
    if (!buffer) return;

    if (!buffer->is_direct) {
        free(buffer->data);
    }
}

so_jbuffer *so_jbuffer_new(jlong size) {
    so_jbuffer *buffer = (so_jbuffer *)so_jinstance_new(sizeof(so_jbuffer), (so_jdtor_t)_jbuffer_dtor);
    if (!buffer) {
        return NULL;
    }

    if (_jbuffer_ctor(buffer, size) != JNI_OK) {
        so_jinstance_release(buffer);
        return NULL;
    }

    return buffer;
}

so_jbuffer *so_jbuffer_new_direct(void *address, jlong size) {
    so_jbuffer *buffer = (so_jbuffer *)so_jinstance_new(sizeof(so_jbuffer), (so_jdtor_t)_jbuffer_dtor);
    if (!buffer) {
        return NULL;
    }

    if (_jbuffer_ctor_direct(buffer, address, size) != JNI_OK) {
        so_jinstance_release(buffer);
        return NULL;
    }

    return buffer;
}

void *so_jbuffer_get_data(so_jbuffer *buffer) {
    if (!buffer) return NULL;

    return buffer->data;
}

jlong so_jbuffer_get_size(so_jbuffer *buffer) {
    if (!buffer) return 0;

    return buffer->size;
}

so_jbuffer *so_jbuffer_from_object(jobject obj) {
    return SO_JBUFFER_FROM_OBJ(obj);
}
