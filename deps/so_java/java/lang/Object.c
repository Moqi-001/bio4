#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <so_common/so_shared_buffer.h>

#include "Class.h"
#include "types.h"

#include "Object.h"

typedef struct JavaWeakRef {
    JObject            *ref_ptr;
    struct JavaWeakRef *next;
} JavaWeakRef;

typedef struct JavaObjectHeader {
    uint32_t     magic;
    JClass       clazz;
    JInt         size;
    JDtor        dtor;
    JavaWeakRef *weak_refs;
    uint32_t     _padding;  // 8字节对齐
} JavaObjectHeader;

#define JAVA_OBJECT_MAGIC    0x44957421  // "JObject".hashCode()
#define JAVA_OBJECT_CAST(_o) JAVA_BASE_CAST(JavaObjectHeader *, _o, JAVA_OBJECT_MAGIC)

#define objectFromHeader(_header) ((JObject)((_header) + 1))
#define headerFromObject(_obj)    JAVA_OBJECT_CAST(((JavaObjectHeader *)(_obj) - 1))

JObject _object_new(JClass clazz, JInt size, JDtor dtor) {
    if (size <= 0) {
        return NULL;
    }

    JavaObjectHeader *header = (JavaObjectHeader *)so_shared_buffer_alloc(sizeof(JavaObjectHeader) + size);
    if (!header) {
        return NULL;
    }

    JObject object = objectFromHeader(header);
    memset(object, 0, size);

    header->magic     = JAVA_OBJECT_MAGIC;
    header->clazz     = clazz;
    header->size      = size;
    header->dtor      = dtor;
    header->weak_refs = NULL;

    return object;
}

JObject Object_new(JClass clazz) {
    return _object_new(clazz, Class_getInstanceSize(clazz), Class_getDestructor(clazz));
}

JObject Object_newWithoutClass(JInt size, JDtor dtor) {
    return _object_new(NULL, size, dtor);
}

JObject Object_retain(JObject object) {
    if (!object) {
        return NULL;
    }

    JavaObjectHeader *header = headerFromObject(object);
    if (!header) {
        return NULL;
    }
    so_shared_buffer_retain(header);

    return object;
}

JInt Object_release(JObject object) {
    if (!object) {
        return -1;
    }

    JavaObjectHeader *header = headerFromObject(object);
    JInt              prev   = so_shared_buffer_release_keep(header);
    if (prev == 1) {
        // 调用析构函数
        if (header->dtor) {
            header->dtor(object);
        }
        // 注销弱引用
        JavaWeakRef *node = header->weak_refs;
        while (node) {
            JavaWeakRef *next = node->next;
            if (node->ref_ptr) {
                *(node->ref_ptr) = NULL;
            }
            free(node);
            node = next;
        }
        // 释放内存
        so_shared_buffer_release_force(header);
    }

    return prev;
}

JInt Object_releaseKeep(JObject object) {
    if (!object) {
        return -1;
    }

    JavaObjectHeader *header = headerFromObject(object);
    JInt              prev   = so_shared_buffer_release_keep(header);

    return prev;
}

void Object_releaseForce(JObject object) {
    if (!object) {
        return;
    }

    JavaObjectHeader *header = headerFromObject(object);
    if (header->dtor) {
        header->dtor(object);
    }

    JavaWeakRef *node = header->weak_refs;
    while (node) {
        JavaWeakRef *next = node->next;
        if (node->ref_ptr) {
            *(node->ref_ptr) = NULL;
        }
        free(node);
        node = next;
    }

    so_shared_buffer_release_force(header);
}

JInt Object_retainWeak(JObject object, JObject *ref_ptr) {
    if (!object || !ref_ptr) {
        return JAVA_ERR;
    }

    JavaObjectHeader *header = headerFromObject(object);
    JavaWeakRef      *node   = (JavaWeakRef *)malloc(sizeof(JavaWeakRef));
    if (!node) {
        return JAVA_ERR;
    }

    node->ref_ptr     = ref_ptr;
    node->next        = header->weak_refs;
    header->weak_refs = node;

    return JAVA_OK;
}

void Object_releaseWeak(JObject object, JObject *ref_ptr) {
    if (!object || !ref_ptr) {
        return;
    }

    JavaObjectHeader *header = headerFromObject(object);
    JavaWeakRef      *prev   = NULL;
    JavaWeakRef      *node   = header->weak_refs;
    while (node) {
        if (node->ref_ptr == ref_ptr) {
            if (prev) {
                prev->next = node->next;
            } else {
                header->weak_refs = node->next;
            }
            free(node);
            return;
        }
        prev = node;
        node = node->next;
    }
}

JClass Object_getClass(JObject object) {
    if (!object) {
        return NULL;
    }

    JavaObjectHeader *header = headerFromObject(object);

    return header->clazz;
}

JInt Object_getSize(JObject object) {
    if (!object) {
        return 0;
    }

    JavaObjectHeader *header = headerFromObject(object);

    return header->size;
}

JBoolean Object_isInstanceOf(JObject object, JClass clazz) {
    // object 可作为 clazz 的实例用 <=> object 的类可赋值给 clazz
    return Class_isAssignableFrom(clazz, Object_getClass(object));
}
