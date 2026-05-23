#include <stdlib.h>
#include <string.h>

#include <so_common/so_shared_buffer.h>

#include "so_jni.h"
#include "so_jni_internal.h"

typedef struct so_jobject {
    uint32_t magic;
    so_jref *ref;
    jclass   clazz;
    void    *instance;
} so_jobject;

#define SO_JOBJECT_MAGIC 0x3D8EC546  // "so_jobject".hashCode()

#define SO_JOBJECT_CAST(_o) \
    ((so_jobject *)((_o) && ((so_jobject *)(_o))->magic == SO_JOBJECT_MAGIC ? (_o) : NULL))

jobject so_jobject_new(jclass clazz, void *instance) {
    so_jobject *obj = (so_jobject *)so_shared_buffer_alloc(sizeof(so_jobject));
    if (!obj) {
        return NULL;
    }

    obj->magic    = SO_JOBJECT_MAGIC;
    obj->ref      = NULL;
    obj->clazz    = clazz;
    obj->instance = instance;

    return (jobject)obj;
}

jobject so_jobject_retain(jobject object) {
    so_jobject *obj = SO_JOBJECT_CAST(object);
    if (!obj) return NULL;

    so_shared_buffer_retain(obj);
    so_jinstance_retain(obj->instance);

    return object;
}

int32_t so_jobject_release(jobject object) {
    so_jobject *obj = SO_JOBJECT_CAST(object);
    if (!obj) return -1;

    if (obj->instance) {
        int32_t inst_prev = so_jinstance_release(obj->instance);
        if (inst_prev == 1) {
            obj->instance = NULL;
        }
    }

    int32_t prev = so_shared_buffer_release(obj);

    return prev;
}

void so_jobject_release_force(jobject object) {
    so_jobject *obj = SO_JOBJECT_CAST(object);
    if (!obj) return;

    so_jinstance_release_force(obj->instance);
    so_shared_buffer_release_force(obj);
}

jobject so_jobject_edit(jobject object) {
    so_jobject *obj = SO_JOBJECT_CAST(object);
    if (!obj) return NULL;

    return (jobject)so_shared_buffer_edit(obj);
}

jobject so_jobject_attempt_edit(jobject object) {
    so_jobject *obj = SO_JOBJECT_CAST(object);
    if (!obj) return NULL;

    return (jobject)so_shared_buffer_attempt_edit(obj);
}

jclass so_jobject_get_class(jobject object) {
    so_jobject *obj = SO_JOBJECT_CAST(object);
    if (!obj) return NULL;

    return obj->clazz;
}

void *so_jobject_get_instance(jobject object) {
    so_jobject *obj = SO_JOBJECT_CAST(object);
    if (!obj) return NULL;

    return obj->instance;
}

jobjectRefType so_jobject_get_ref_type(jobject object) {
    so_jobject *obj = SO_JOBJECT_CAST(object);
    if (!obj || !obj->ref) return JNIInvalidRefType;

    return so_jref_get_type(obj->ref);
}

so_jref *so_jobject_get_ref(jobject object) {
    so_jobject *obj = SO_JOBJECT_CAST(object);
    if (!obj) return NULL;

    return obj->ref;
}

jint so_jobject_set_ref(jobject object, so_jref *ref) {
    so_jobject *obj = SO_JOBJECT_CAST(object);
    if (!obj) return JNI_EINVAL;

    obj->ref = ref;
    return JNI_OK;
}
