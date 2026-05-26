#include <string.h>

#include <so_common/so_shared_buffer.h>

#include "JavaArray.h"

#include "../jni.h"
#include "JavaInstance.h"

typedef struct _JavaArray {
    uint32_t        magic;
    jint            type;
    jsize           length;
    jsize           elem_size;
    void           *elems;
    JavaElemsDtor_t elems_dtor;
} _JavaArray;

#define JAVA_JARRAY_MAGIC 0x4388FD37  // "JavaArray".hashCode()
#define JAVA_JARRAY_CAST(_o) \
    ((_JavaArray *)((_o) && ((_JavaArray *)(_o))->magic == JAVA_JARRAY_MAGIC ? (_o) : NULL))

static jsize _jtype_size(jint type) {
    switch (type) {
        case JAVA_ARRAY_TYPE_OBJECT:
            return sizeof(jobject);
        case JAVA_ARRAY_TYPE_BOOLEAN:
            return sizeof(jboolean);
        case JAVA_ARRAY_TYPE_BYTE:
            return sizeof(jbyte);
        case JAVA_ARRAY_TYPE_CHAR:
            return sizeof(jchar);
        case JAVA_ARRAY_TYPE_SHORT:
            return sizeof(jshort);
        case JAVA_ARRAY_TYPE_INT:
            return sizeof(jint);
        case JAVA_ARRAY_TYPE_LONG:
            return sizeof(jlong);
        case JAVA_ARRAY_TYPE_FLOAT:
            return sizeof(jfloat);
        case JAVA_ARRAY_TYPE_DOUBLE:
            return sizeof(jdouble);
        default:
            return 0;
    }
}

static jint _jarray_ctor(_JavaArray *arr, jsize length, jint type, JavaElemsDtor_t elems_dtor) {
    if (!arr || length <= 0) return JNI_EINVAL;

    arr->magic     = JAVA_JARRAY_MAGIC;
    arr->type      = type;
    arr->elem_size = _jtype_size(type);
    arr->elems     = so_shared_buffer_alloc(length * arr->elem_size);
    if (!arr->elems) {
        arr->length     = 0;
        arr->elems_dtor = NULL;
        return JNI_ERR;
    }
    arr->length     = length;
    arr->elems_dtor = elems_dtor;

    return JNI_OK;
}

static void _jarray_dtor(_JavaArray *arr) {
    if (!arr || !arr->elems) return;

    int32_t prev = so_shared_buffer_release_keep(arr->elems);
    if (prev == 1) {
        if (arr->elems_dtor) {
            arr->elems_dtor(arr->elems, arr->length);
        }
        so_shared_buffer_release_force(arr->elems);
    }
}

JavaArray JavaArrayNew(jsize length, jint type, JavaElemsDtor_t elems_dtor) {
    _JavaArray *arr = (_JavaArray *)JavaInstanceNew(sizeof(_JavaArray), (JavaDtor_t)_jarray_dtor);
    if (!arr) {
        return NULL;
    }

    if (_jarray_ctor(arr, length, type, elems_dtor) != JNI_OK) {
        JavaInstanceRelease(arr);
        return NULL;
    }

    return (JavaArray)arr;
}

void *JavaArrayRetainElements(JavaArray array) {
    _JavaArray *arr = JAVA_JARRAY_CAST(array);
    if (!arr) return NULL;

    so_shared_buffer_retain(arr->elems);
    return arr->elems;
}

int32_t JavaArrayReleaseElements(JavaArray array, void *elems) {
    _JavaArray *arr = JAVA_JARRAY_CAST(array);
    if (!arr || !elems) return -1;

    int32_t prev = so_shared_buffer_release_keep(elems);
    if (prev == 1) {
        if (arr->elems_dtor) {
            arr->elems_dtor(elems, arr->length);
        }
        if (arr->elems == elems) {
            arr->elems  = NULL;
            arr->length = 0;
        }
        so_shared_buffer_release_force(elems);
    }

    return prev;
}

void *JavaArrayEditElements(JavaArray array) {
    _JavaArray *arr = JAVA_JARRAY_CAST(array);
    if (!arr || !arr->elems) return NULL;

    void *elems = so_shared_buffer_edit(arr->elems);
    if (!elems) {
        return NULL;
    }
    if (elems != arr->elems) {
        arr->elems = elems;
    }

    return elems;
}

jint JavaArrayGetType(JavaArray array) {
    _JavaArray *arr = JAVA_JARRAY_CAST(array);
    if (!arr) return JAVA_ARRAY_TYPE_UNKNOWN;

    return arr->type;
}

jsize JavaArrayGetLength(JavaArray array) {
    _JavaArray *arr = JAVA_JARRAY_CAST(array);
    if (!arr) return 0;

    return arr->length;
}

void *JavaArrayGetElements(JavaArray array) {
    _JavaArray *arr = JAVA_JARRAY_CAST(array);
    if (!arr) return NULL;

    return arr->elems;
}

jint JavaArrayGetRegion(JavaArray array, jsize start, jsize length, void *dst) {
    _JavaArray *arr = JAVA_JARRAY_CAST(array);
    if (!arr || !dst || start < 0 || length <= 0) return JNI_EINVAL;

    if (!arr->elems || start + length > arr->length) return JNI_ERR;

    void *src = (char *)arr->elems + start * arr->elem_size;
    memcpy(dst, src, length * arr->elem_size);

    return JNI_OK;
}

jint JavaArraySetRegion(JavaArray array, jsize start, jsize length, const void *src) {
    _JavaArray *arr = JAVA_JARRAY_CAST(array);
    if (!arr || !src || start < 0 || length <= 0) return JNI_EINVAL;

    if (!arr->elems || start + length > arr->length) return JNI_ERR;

    void *dst = (char *)arr->elems + start * arr->elem_size;
    if (src == dst) return JNI_OK;  // 自拷贝

    void *elems = so_shared_buffer_edit(arr->elems);
    if (!elems) return JNI_ERR;
    if (arr->elems != elems) arr->elems = elems;

    dst = (char *)elems + start * arr->elem_size;
    memcpy(dst, src, length * arr->elem_size);

    return JNI_OK;
}
