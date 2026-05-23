#include <string.h>

#include <so_common/so_shared_buffer.h>

#include "so_jni.h"
#include "so_jni_internal.h"

struct so_jarray {
    uint32_t magic;
    jint     type;
    jsize    length;
    jsize    elem_size;
    void    *elems;
};

#define SO_JARRAY_MAGIC 0x013E7372  // "so_jarray".hashCode()

#define SO_JARRAY_CAST(_o) \
    ((so_jarray *)((_o) && ((so_jarray *)(_o))->magic == SO_JARRAY_MAGIC ? (_o) : NULL))

#define SO_JARRAY_FROM_OBJ(_o)                 \
    ({                                         \
        void *_inst = so_jobject_get_instance(_o); \
        SO_JARRAY_CAST(_inst);                 \
    })

static jsize _jtype_size(jint type) {
    switch (type) {
        case SO_JARRAY_TYPE_OBJECT:
            return sizeof(jobject);
        case SO_JARRAY_TYPE_BOOLEAN:
            return sizeof(jboolean);
        case SO_JARRAY_TYPE_BYTE:
            return sizeof(jbyte);
        case SO_JARRAY_TYPE_CHAR:
            return sizeof(jchar);
        case SO_JARRAY_TYPE_SHORT:
            return sizeof(jshort);
        case SO_JARRAY_TYPE_INT:
            return sizeof(jint);
        case SO_JARRAY_TYPE_LONG:
            return sizeof(jlong);
        case SO_JARRAY_TYPE_FLOAT:
            return sizeof(jfloat);
        case SO_JARRAY_TYPE_DOUBLE:
            return sizeof(jdouble);
        default:
            return 0;
    }
}

static jint _jarray_ctor(so_jarray *array, jsize length, jint type) {
    if (!array || length <= 0) return JNI_EINVAL;

    array->magic     = SO_JARRAY_MAGIC;
    array->type      = type;
    array->elem_size = _jtype_size(type);
    array->elems     = so_shared_buffer_alloc(length * array->elem_size);
    if (!array->elems) {
        return JNI_ERR;
    }
    array->length = length;

    return JNI_OK;
}

static void _jarray_dtor(so_jarray *array) {
    if (!array || !array->elems) return;

    if (array->type == SO_JARRAY_TYPE_OBJECT) {
        // jobject 需释放局部引用（先调用保留数据的释放函数判断是否是为唯一引用）
        int32_t prev = so_shared_buffer_release_keep(array->elems);
        if (prev == 1) {
            jobject *elems = (jobject *)array->elems;
            for (jsize i = 0; i < array->length; i++) {
                if (elems[i]) {
                    so_jref_delete_local(elems[i]);
                }
            }
            so_shared_buffer_release_force(array->elems);
        }
    } else {
        so_shared_buffer_release(array->elems);
    }
}

so_jarray *so_jarray_new(jsize length, jint type) {
    so_jarray *array = (so_jarray *)so_jinstance_new(sizeof(so_jarray), (so_jdtor_t)_jarray_dtor);
    if (!array) {
        return NULL;
    }

    if (_jarray_ctor(array, length, type) != JNI_OK) {
        so_jinstance_release(array);
        return NULL;
    }

    return array;
}

void *so_jarray_retain_elems(so_jarray *array) {
    if (!array) return NULL;

    so_shared_buffer_retain(array->elems);
    return array->elems;
}

int32_t so_jarray_release_elems(so_jarray *array, void *elems) {
    if (!array || !elems) return -1;

    int32_t prev = so_shared_buffer_release(elems);
    if (prev == 1) {
        if (array->elems == elems) {
            array->elems = NULL;
        }
    }

    return prev;
}

void *so_jarray_edit_elems(so_jarray *array) {
    if (!array) return NULL;

    void *elems = so_shared_buffer_edit(array->elems);
    if (elems && elems != array->elems) {
        array->elems = elems;
    }

    return elems;
}

jint so_jarray_read_elems(so_jarray *array, jsize start, jsize length, void *dst) {
    if (!array || !dst || start < 0 || length <= 0) return JNI_EINVAL;

    if (!array->elems || start + length > array->length) return JNI_ERR;

    void *src = (char *)array->elems + start * array->elem_size;
    memcpy(dst, src, length * array->elem_size);

    return JNI_OK;
}

jint so_jarray_write_elems(so_jarray *array, jsize start, jsize length, const void *src) {
    if (!array || !src || start < 0 || length <= 0) return JNI_EINVAL;

    if (!array->elems || start + length > array->length) return JNI_ERR;

    // dst 指向原始数组：用于判断自拷贝
    // 不可用 edit 后的 elems 判断，因为 COW 会产生副本，导致原本可能一样的 elems 变得不一样
    void *dst = (char *)array->elems + start * array->elem_size;
    if (src == dst) return JNI_OK;

    void *elems = so_jarray_edit_elems(array);
    if (!elems) return JNI_ERR;

    dst = (char *)elems + start * array->elem_size;
    memcpy(dst, src, length * array->elem_size);

    return JNI_OK;
}

jint so_jarray_get_type(so_jarray *array) {
    if (!array) return SO_JARRAY_TYPE_UNKNOWN;

    return array->type;
}

void *so_jarray_get_elems(so_jarray *array) {
    if (!array) return NULL;

    return array->elems;
}

jsize so_jarray_get_length(so_jarray *array) {
    if (!array) return 0;

    return array->length;
}

so_jarray *so_jarray_from_object(jobject obj) {
    return SO_JARRAY_FROM_OBJ(obj);
}
