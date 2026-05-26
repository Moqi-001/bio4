#include <stdlib.h>
#include <string.h>

#include <so_common/so_shared_buffer.h>

#include "java/JavaInstance.h"
#include "so_jni_internal.h"

typedef struct JniObject {
    uint32_t     magic;
    so_jref     *ref;
    jclass       clazz;
    JavaInstance instance;
} JniObject;

#define JNI_OBJECT_MAGIC 0xBD5D3B24  // "JniObject".hashCode()
#define JNI_OBJECT_CAST(_o) \
    ((JniObject *)((_o) && ((JniObject *)(_o))->magic == JNI_OBJECT_MAGIC ? (_o) : NULL))

/********************* internal *********************/
jobject JniCreateObject(jclass clazz, JavaInstance instance) {
    JniObject *obj = (JniObject *)so_shared_buffer_alloc(sizeof(JniObject));
    if (!obj) {
        return NULL;
    }

    obj->magic    = JNI_OBJECT_MAGIC;
    obj->ref      = NULL;
    obj->clazz    = clazz;
    obj->instance = instance;

    return (jobject)obj;
}

jobject JniRetainObject(jobject object) {
    JniObject *obj = JNI_OBJECT_CAST(object);
    if (!obj) return NULL;

    so_shared_buffer_retain(obj);
    JavaInstanceRetain(obj->instance);

    return object;
}

int32_t JniReleaseObject(jobject object) {
    JniObject *obj = JNI_OBJECT_CAST(object);
    if (!obj) return -1;

    if (obj->instance) {
        int32_t inst_prev = JavaInstanceRelease(obj->instance);
        if (inst_prev == 1) {
            obj->instance = NULL;
        }
    }

    int32_t prev = so_shared_buffer_release(obj);

    return prev;
}

void JniForceReleaseObject(jobject object) {
    JniObject *obj = JNI_OBJECT_CAST(object);
    if (!obj) return;

    JavaInstanceReleaseForce(obj->instance);
    so_shared_buffer_release_force(obj);
}

jobject JniEditObject(jobject object) {
    JniObject *obj = JNI_OBJECT_CAST(object);
    if (!obj) return NULL;

    return (jobject)so_shared_buffer_edit(obj);
}

jobject JniAttemptEditObject(jobject object) {
    JniObject *obj = JNI_OBJECT_CAST(object);
    if (!obj) return NULL;

    return (jobject)so_shared_buffer_attempt_edit(obj);
}

so_jref *JniGetObjectRef(jobject object) {
    JniObject *obj = JNI_OBJECT_CAST(object);
    if (!obj) return NULL;

    return obj->ref;
}

jint JniSetObjectRef(jobject object, so_jref *ref) {
    JniObject *obj = JNI_OBJECT_CAST(object);
    if (!obj) return JNI_EINVAL;

    obj->ref = ref;
    return JNI_OK;
}

/********************* exports *********************/
static jobject _alloc_object(jclass clazz) {
    jsize instance_size = JniGetClassInstanceSize(clazz);
    if (instance_size <= 0) return NULL;

    JavaInstance instance = JavaInstanceNew(instance_size, NULL);
    if (!instance) {
        return NULL;
    }

    jclass object = JniCreateObject(clazz, instance);
    if (!object) {
        JavaInstanceRelease(instance);
        return NULL;
    }

    return object;
}

jobject JniAllocObject(jclass clazz) {
    jobject object = _alloc_object(clazz);
    if (!object) {
        return NULL;
    }

    JniAddLocalRef(object);

    return object;
}

jobject JniNewObject(jclass clazz, jmethodID methodID, ...) {
    jobject object = _alloc_object(clazz);
    if (!object) {
        return NULL;
    }

    const jsymbol *sym = (const jsymbol *)methodID;
    if (!sym->addr) {
        JniReleaseObject(object);
        return NULL;
    }

    typedef void (*MethodFunc)(void *, jargs *);
    MethodFunc method = (MethodFunc)sym->addr;

    va_list args;
    va_start(args, methodID);
    jargs _args = {.is_va = 1, .va = args, .index = 0};
    method(JniGetObjectInstance(object), &_args);
    va_end(args);

    JniAddLocalRef(object);

    return object;
}

jobject JniNewObjectV(jclass clazz, jmethodID methodID, va_list args) {
    jobject object = _alloc_object(clazz);
    if (!object) {
        return NULL;
    }

    const jsymbol *sym = (const jsymbol *)methodID;
    if (!sym->addr) {
        JniReleaseObject(object);
        return NULL;
    }

    typedef void (*MethodFunc)(void *, jargs *);
    MethodFunc method = (MethodFunc)sym->addr;
    jargs      _args  = {.is_va = 1, .va = args, .index = 0};
    method(JniGetObjectInstance(object), &_args);

    JniAddLocalRef(object);

    return object;
}

jobject JniNewObjectA(jclass clazz, jmethodID methodID, const jvalue *args) {
    jobject object = _alloc_object(clazz);
    if (!object) {
        return NULL;
    }

    const jsymbol *sym = (const jsymbol *)methodID;
    if (!sym->addr) {
        JniReleaseObject(object);
        return NULL;
    }

    typedef void (*MethodFunc)(void *, jargs *);
    MethodFunc method = (MethodFunc)sym->addr;
    jargs      _args  = {.is_va = 0, .vals = args, .index = 0};
    method(JniGetObjectInstance(object), &_args);

    JniAddLocalRef(object);

    return object;
}

void *JniGetObjectInstance(jobject object) {
    JniObject *obj = JNI_OBJECT_CAST(object);
    if (!obj) return NULL;

    return (void *)obj->instance;
}

jclass JniGetObjectClass(jobject object) {
    JniObject *obj = JNI_OBJECT_CAST(object);
    if (!obj || !obj->clazz) return NULL;

    return JniNewLocalRef(obj->clazz);
}

jboolean JniIsSameObject(jobject ref1, jobject ref2) {
    void    *inst1  = JniGetObjectInstance(ref1);
    void    *inst2  = JniGetObjectInstance(ref2);
    jboolean result = (inst1 == inst2) ? JNI_TRUE : JNI_FALSE;
    return result;
}

jboolean JniIsInstanceOf(jobject obj, jclass clazz) {
    return JniIsAssignableFrom(JniGetObjectClass(obj), clazz);
}

jobjectRefType JniGetObjectRefType(jobject object) {
    JniObject *obj = JNI_OBJECT_CAST(object);
    if (!obj || !obj->ref) return JNIInvalidRefType;

    return obj->ref->type;
}
