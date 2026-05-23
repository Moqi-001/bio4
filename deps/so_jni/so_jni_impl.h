#ifndef SO_JNI_IMPL_PRIVATE
#error "so_jni_impl.h is private to so_jni.c"
#endif

#include <string.h>

#include <so_common/so_logger.h>

#include "jni.h"

#include "so_jni.h"
#include "so_jni_internal.h"

/*
 * ============================================================================
 * JavaVM Implementation
 * ============================================================================
 */
jint DestroyJavaVM(JavaVM *vm) {
    so_logd("[JVM] DestroyJavaVM(%p): ignored", vm);
    return JNI_OK;
}

jint AttachCurrentThread(JavaVM *vm, JNIEnv **p_env, void *thr_args) {
    if (!p_env) {
        so_loge("[JVM] AttachCurrentThread(%p,%p,%p): invalid arguments!", vm, p_env, thr_args);
        return JNI_EINVAL;
    }

    *p_env = &_jni_env;

    so_logd("[JVM] AttachCurrentThread(%p,%p,%p): %d", vm, p_env, thr_args, JNI_OK);
    return JNI_OK;
}

jint DetachCurrentThread(JavaVM *vm) {
    // Since we don't operate the actual Java VM, we don't need to care about
    // it being available only in one thread. Anyway, due to this restriction,
    // client applications are guaranteed to use JNI in a safe way.
    so_logd("[JVM] DetachCurrentThread(%p): ignored", vm);
    return JNI_OK;
}

jint GetEnv(JavaVM *vm, void **p_env, jint version) {
    if (!p_env) {
        so_loge("[JVM] GetEnv(%p,%p,0x%x): invalid arguments!", vm, p_env, version);
        return JNI_EINVAL;
    }

    *p_env = &_jni_env;

    so_logd("[JVM] GetEnv(%p,%p,0x%x)", vm, p_env, version);
    return JNI_OK;
}

jint AttachCurrentThreadAsDaemon(JavaVM *vm, JNIEnv **p_env, void *thr_args) {
    if (!p_env) {
        so_loge("[JVM] AttachCurrentThreadAsDaemon(%p,%p,%p): invalid arguments!", vm, p_env, thr_args);
        return JNI_EINVAL;
    }

    *p_env = &_jni_env;

    so_logd("[JVM] AttachCurrentThreadAsDaemon(%p,%p,%p)", vm, p_env, thr_args);
    return JNI_OK;
}

/*
 * ============================================================================
 * JNIEnv Implementation
 * ============================================================================
 */
jint GetVersion(JNIEnv *env) {
    jint version = JNI_VERSION_1_6;
    so_logd("[JNI] GetVersion(env): 0x%x", version);
    return version;
}

jclass DefineClass(JNIEnv *env, const char *name, jobject loader, const jbyte *buf, jsize bufLen) {
    // Supposed to load Java class from `.class`-containing `buf`. Can not be implemented.
    so_loge("[JNI] DefineClass(%p,'%s',%p,%p,%d): not implemented", env, name, loader, buf, bufLen);
    return NULL;
}

jclass FindClass(JNIEnv *env, const char *name) {
    jclass clazz = so_jni_find_class(name);
    if (!clazz) {
        so_loge("[JNI] FindClass(%p,%s): not found!", env, name);
        return NULL;
    }

    jclass result = so_jref_new_local(clazz);
    so_logd("[JNI] FindClass(%p,%s): %p", env, name, result);
    return result;
}

jclass GetSuperclass(JNIEnv *env, jclass clazz) {
    jclass super_cls = so_jclass_get_super(clazz);
    if (!super_cls) {
        so_loge("[JNI] GetSuperclass(%p,%p): no super class!", env, clazz);
        return NULL;
    }

    jclass result = so_jref_new_local(super_cls);
    so_logd("[JNI] GetSuperclass(%p,%p): %p", env, clazz, result);
    return result;
}

jboolean IsAssignableFrom(JNIEnv *env, jclass clazz1, jclass clazz2) {
    jboolean result = so_jclass_is_assignable_from(clazz1, clazz2);
    so_logd("[JNI] IsAssignableFrom(%p,%p,%p): %u", env, clazz1, clazz2, result);
    return result;
}

jmethodID FromReflectedMethod(JNIEnv *env, jobject method) {
    so_logw("[JNI] FromReflectedMethod(%p,%p): not implemented", env, method);
    return NULL;
}

jfieldID FromReflectedField(JNIEnv *env, jobject field) {
    so_logw("[JNI] FromReflectedField(%p,%p): not implemented", env, field);
    return NULL;
}

jobject ToReflectedMethod(JNIEnv *env, jclass clazz, jmethodID methodID, jboolean isStatic) {
    so_logw("[JNI] ToReflectedMethod(%p,%p,%p,%d): not implemented", env, clazz, methodID, isStatic);
    return NULL;
}

jobject ToReflectedField(JNIEnv *env, jclass clazz, jfieldID fieldID, jboolean isStatic) {
    so_logw("[JNI] ToReflectedField(%p,%p,%p,%d): not implemented", env, clazz, fieldID, isStatic);
    return NULL;
}

jint Throw(JNIEnv *env, jthrowable obj) {
    so_logw("[JNI] Throw(%p,%p): not implemented", env, obj);
    return JNI_OK;
}

jint ThrowNew(JNIEnv *env, jclass clazz, const char *message) {
    so_logw("[JNI] ThrowNew(%p,%p,'%s'): not implemented", env, clazz, message);
    return JNI_OK;
}

jthrowable ExceptionOccurred(JNIEnv *env) {
    so_logd("[JNI] ExceptionOccurred(%p): ignored", env);
    return NULL;
}

void ExceptionDescribe(JNIEnv *env) {
    so_logd("[JNI] ExceptionDescribe(%p): ignored", env);
}

void ExceptionClear(JNIEnv *env) {
    so_logd("[JNI] ExceptionClear(%p): ignored", env);
}

void FatalError(JNIEnv *env, const char *msg) {
    so_logf("[JNI] FatalError(%p,%p): '%s'", env, msg, msg);
}

jint PushLocalFrame(JNIEnv *env, jint capacity) {
    jint result = so_jref_push_local_frame(capacity);
    so_logd("[JNI] PushLocalFrame(%p,%d)", env, capacity);
    return result;
}

jobject PopLocalFrame(JNIEnv *env, jobject result) {
    jobject preserved = so_jref_pop_local_frame(result);
    so_logd("[JNI] PopLocalFrame(%p,%p): %p", env, result, preserved);
    return preserved;
}

jobject NewGlobalRef(JNIEnv *env, jobject obj) {
    jobject result = so_jref_new_global(obj);
    if (!result) {
        so_loge("[JNI] NewGlobalRef(%p,%p): invalid object!", env, obj);
        return NULL;
    }

    so_logd("[JNI] NewGlobalRef(%p,%p): %p", env, obj, result);
    return result;
}

void DeleteGlobalRef(JNIEnv *env, jobject obj) {
    so_jref_delete_global(obj);
    so_logd("[JNI] DeleteGlobalRef(%p,%p)", env, obj);
}

void DeleteLocalRef(JNIEnv *env, jobject obj) {
    so_jref_delete_local(obj);
    so_logd("[JNI] DeleteLocalRef(%p,%p)", env, obj);
}

jboolean IsSameObject(JNIEnv *env, jobject ref1, jobject ref2) {
    jboolean result = (ref1 == ref2) ? JNI_TRUE : JNI_FALSE;
    so_logd("[JNI] IsSameObject(%p,%p,%p): %u", env, ref1, ref2, result);
    return result;
}

jobject NewLocalRef(JNIEnv *env, jobject obj) {
    jobject result = so_jref_new_local(obj);
    if (!result) {
        so_loge("[JNI] NewLocalRef(%p,%p): invalid object!", env, obj);
        return NULL;
    }

    so_logd("[JNI] NewLocalRef(%p,%p): %p", env, obj, result);
    return result;
}

jint EnsureLocalCapacity(JNIEnv *env, jint capacity) {
    so_logd("[JNI] EnsureLocalCapacity(%p,%d)", env, capacity);
    return JNI_OK;
}

jobject AllocObject(JNIEnv *env, jclass clazz) {
    jobject obj = so_jclass_new_instance(clazz);
    if (!obj) {
        so_loge("[JNI] AllocObject(%p,%p): failed to alloc a new object!", env, clazz);
        return NULL;
    }

    if (so_jref_take_local(obj) != JNI_OK) {
        so_logw("[JNI] NewObject(%p,%p): %p, register failed, manual release required!", env, clazz, obj);
        return obj;
    }

    so_logd("[JNI] AllocObject(%p,%p): %p", env, clazz, obj);
    return obj;
}

jobject NewObject(JNIEnv *env, jclass clazz, jmethodID methodID, ...) {
    if (!clazz || !methodID) {
        so_logd("[JNI] NewObject(%p,%p,%p,...): invalid arguments!", env, clazz, methodID);
        return NULL;
    }

    jobject obj = so_jclass_new_instance(clazz);
    if (!obj) {
        so_loge("[JNI] NewObject(%p,%p,%p,...): failed to alloc a new object!", env, clazz, methodID);
        return NULL;
    }

    const so_jsymbol *sym = (const so_jsymbol *)methodID;
    if (!sym->addr) {
        so_loge("[JNI] NewObject(%p,%p,%p,...): invalid methodID!", env, obj, methodID);
        so_jobject_release(obj);
        return NULL;
    }

    typedef void (*MethodFunc)(void *, so_jargs *);
    MethodFunc method = (MethodFunc)sym->addr;

    va_list args;
    va_start(args, methodID);
    so_jargs jargs = {.is_va = 1, .va = args, .index = 0};
    method(so_jobject_get_instance(obj), &jargs);
    va_end(args);

    if (so_jref_take_local(obj) != JNI_OK) {
        so_logw("[JNI] NewObject(%p,%p,%p,...): %p, register failed, manual release required!", env, clazz, methodID, obj);
        return obj;
    }

    so_logd("[JNI] NewObject(%p,%p,%p,...): %p", env, clazz, methodID, obj);
    return obj;
}

jobject NewObjectV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args) {
    if (!clazz || !methodID) {
        so_logd("[JNI] NewObjectV(%p,%p,%p,args): invalid arguments!", env, clazz, methodID);
        return NULL;
    }

    jobject obj = so_jclass_new_instance(clazz);
    if (!obj) {
        so_loge("[JNI] NewObjectV(%p,%p,%p,args): failed to alloc a new object!", env, clazz, methodID);
        return NULL;
    }

    const so_jsymbol *sym = (const so_jsymbol *)methodID;
    if (!sym->addr) {
        so_loge("[JNI] NewObjectV(%p,%p,%p,args): invalid methodID!", env, obj, methodID);
        so_jobject_release(obj);
        return NULL;
    }

    typedef void (*MethodFunc)(void *, so_jargs *);
    MethodFunc method = (MethodFunc)sym->addr;

    so_jargs jargs = {.is_va = 1, .va = args, .index = 0};
    method(so_jobject_get_instance(obj), &jargs);

    if (so_jref_take_local(obj) != JNI_OK) {
        so_logw("[JNI] NewObjectV(%p,%p,%p,args): %p, register failed, manual release required!", env, clazz, methodID, obj);
        return obj;
    }

    so_logd("[JNI] NewObjectV(%p,%p,%p,args): %p", env, clazz, methodID, obj);
    return obj;
}

jobject NewObjectA(JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args) {
    if (!clazz || !methodID) {
        so_logd("[JNI] NewObjectA(%p,%p,%p,args): invalid arguments!", env, clazz, methodID);
        return NULL;
    }

    jobject obj = so_jclass_new_instance(clazz);
    if (!obj) {
        so_loge("[JNI] NewObjectA(%p,%p,%p,args): failed to alloc a new object!", env, clazz, methodID);
        return NULL;
    }

    const so_jsymbol *sym = (const so_jsymbol *)methodID;
    if (!sym->addr) {
        so_loge("[JNI] NewObjectA(%p,%p,%p,args): invalid methodID!", env, obj, methodID);
        so_jobject_release(obj);
        return NULL;
    }

    typedef void (*MethodFunc)(void *, so_jargs *);
    MethodFunc method = (MethodFunc)sym->addr;

    so_jargs jargs = {.is_va = 0, .vals = args, .index = 0};
    method(so_jobject_get_instance(obj), &jargs);

    if (so_jref_take_local(obj) != JNI_OK) {
        so_logw("[JNI] NewObjectA(%p,%p,%p,args): %p, register failed, manual release required!", env, clazz, methodID, obj);
        return obj;
    }

    so_logd("[JNI] NewObjectA(%p,%p,%p,args): %p", env, clazz, methodID, obj);
    return obj;
}

jclass GetObjectClass(JNIEnv *env, jobject obj) {
    jclass clazz = so_jobject_get_class(obj);
    if (!clazz) {
        so_loge("[JNI] GetObjectClass(%p,%p): no class!", env, obj);
        return NULL;
    }

    jclass result = so_jref_new_local(clazz);
    so_logd("[JNI] GetObjectClass(%p,%p): %p ('%s')", env, obj, result, so_jclass_get_name(clazz));
    return result;
}

jboolean IsInstanceOf(JNIEnv *env, jobject obj, jclass clazz) {
    jboolean result = so_jclass_is_assignable_from(so_jobject_get_class(obj), clazz);
    so_logd("[JNI] IsInstanceOf(%p,%p,%p): %u", env, obj, clazz, result);
    return result;
}

jmethodID GetMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
    if (!clazz || !name || !sig) {
        so_loge("[JNI] GetMethodID(%p,%p,'%s','%s'): invalid arguments!", env, clazz, name, sig);
        return NULL;
    }

    const so_jsymbol *sym = so_jclass_find_method(clazz, name, sig, 0);
    if (!sym) {
        so_loge("[JNI] GetMethodID(%p,%p,'%s','%s'): not found!", env, clazz, name, sig);
        return NULL;
    }

    jmethodID id = (jmethodID)sym;
    so_logd("[JNI] GetMethodID(%p,%p,'%s','%s'): %p", env, clazz, name, sig, id);
    return id;
}

#define CALL_TYPE_METHOD(_jtype, _jname)                                                                  \
    _jtype Call##_jname##Method(JNIEnv *env, jobject obj, jmethodID methodID, ...) {                      \
        if (!obj || !methodID) {                                                                          \
            so_loge("[JNI] Call" #_jname "Method(%p,%p,%p,...): invalid arguments!", env, obj, methodID); \
            return (_jtype)0;                                                                             \
        }                                                                                                 \
        const so_jsymbol *sym = (const so_jsymbol *)methodID;                                             \
        if (!sym->addr) {                                                                                 \
            so_loge("[JNI] Call" #_jname "Method(%p,%p,%p,...): invalid methodID!", env, obj, methodID);  \
            return (_jtype)0;                                                                             \
        }                                                                                                 \
        typedef _jtype (*MethodFunc)(void *, so_jargs *);                                                 \
        MethodFunc method = (MethodFunc)sym->addr;                                                        \
        va_list    args;                                                                                  \
        va_start(args, methodID);                                                                         \
        so_jargs jargs  = {.is_va = 1, .va = args, .index = 0};                                           \
        _jtype   result = method(so_jobject_get_instance(obj), &jargs);                                   \
        va_end(args);                                                                                     \
        so_logd("[JNI] Call" #_jname "Method(%p,%p,%p,...): ('%s')", env, obj, methodID, sym->name);      \
        return result;                                                                                    \
    }

#define CALL_TYPE_METHOD_V(_jtype, _jname)                                                                  \
    _jtype Call##_jname##MethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list args) {              \
        if (!obj || !methodID) {                                                                            \
            so_loge("[JNI] Call" #_jname "MethodV(%p,%p,%p,args): invalid arguments!", env, obj, methodID); \
            return (_jtype)0;                                                                               \
        }                                                                                                   \
        const so_jsymbol *sym = (const so_jsymbol *)methodID;                                               \
        if (!sym->addr) {                                                                                   \
            so_loge("[JNI] Call" #_jname "MethodV(%p,%p,%p,args): invalid methodID!", env, obj, methodID);  \
            return (_jtype)0;                                                                               \
        }                                                                                                   \
        typedef _jtype (*MethodFunc)(void *, so_jargs *);                                                   \
        MethodFunc method = (MethodFunc)sym->addr;                                                          \
        so_jargs   jargs  = {.is_va = 1, .va = args, .index = 0};                                           \
        _jtype     result = method(so_jobject_get_instance(obj), &jargs);                                   \
        so_logd("[JNI] Call" #_jname "MethodV(%p,%p,%p,args): ('%s')", env, obj, methodID, sym->name);      \
        return result;                                                                                      \
    }

#define CALL_TYPE_METHOD_A(_jtype, _jname)                                                                  \
    _jtype Call##_jname##MethodA(JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args) {        \
        if (!obj || !methodID) {                                                                            \
            so_loge("[JNI] Call" #_jname "MethodA(%p,%p,%p,args): invalid arguments!", env, obj, methodID); \
            return (_jtype)0;                                                                               \
        }                                                                                                   \
        const so_jsymbol *sym = (const so_jsymbol *)methodID;                                               \
        if (!sym->addr) {                                                                                   \
            so_loge("[JNI] Call" #_jname "MethodA(%p,%p,%p,args): invalid methodID!", env, obj, methodID);  \
            return (_jtype)0;                                                                               \
        }                                                                                                   \
        typedef _jtype (*MethodFunc)(void *, so_jargs *);                                                   \
        MethodFunc method = (MethodFunc)sym->addr;                                                          \
        so_jargs   jargs  = {.is_va = 0, .vals = args, .index = 0};                                         \
        _jtype     result = method(so_jobject_get_instance(obj), &jargs);                                   \
        so_logd("[JNI] Call" #_jname "MethodA(%p,%p,%p,args): ('%s')", env, obj, methodID, sym->name);      \
        return result;                                                                                      \
    }

CALL_TYPE_METHOD(jobject, Object)
CALL_TYPE_METHOD_V(jobject, Object)
CALL_TYPE_METHOD_A(jobject, Object)
CALL_TYPE_METHOD(jboolean, Boolean)
CALL_TYPE_METHOD_V(jboolean, Boolean)
CALL_TYPE_METHOD_A(jboolean, Boolean)
CALL_TYPE_METHOD(jbyte, Byte)
CALL_TYPE_METHOD_V(jbyte, Byte)
CALL_TYPE_METHOD_A(jbyte, Byte)
CALL_TYPE_METHOD(jchar, Char)
CALL_TYPE_METHOD_V(jchar, Char)
CALL_TYPE_METHOD_A(jchar, Char)
CALL_TYPE_METHOD(jshort, Short)
CALL_TYPE_METHOD_V(jshort, Short)
CALL_TYPE_METHOD_A(jshort, Short)
CALL_TYPE_METHOD(jint, Int)
CALL_TYPE_METHOD_V(jint, Int)
CALL_TYPE_METHOD_A(jint, Int)
CALL_TYPE_METHOD(jlong, Long)
CALL_TYPE_METHOD_V(jlong, Long)
CALL_TYPE_METHOD_A(jlong, Long)
CALL_TYPE_METHOD(jfloat, Float)
CALL_TYPE_METHOD_V(jfloat, Float)
CALL_TYPE_METHOD_A(jfloat, Float)
CALL_TYPE_METHOD(jdouble, Double)
CALL_TYPE_METHOD_V(jdouble, Double)
CALL_TYPE_METHOD_A(jdouble, Double)

void CallVoidMethod(JNIEnv *env, jobject obj, jmethodID methodID, ...) {
    if (!obj || !methodID) {
        so_loge("[JNI] CallVoidMethod(%p,%p,%p,...): invalid arguments!", env, obj, methodID);
        return;
    }
    const so_jsymbol *sym = (const so_jsymbol *)methodID;
    if (!sym->addr) {
        so_loge("[JNI] CallVoidMethod(%p,%p,%p,...): invalid methodID!", env, obj, methodID);
        return;
    }
    typedef void (*MethodFunc)(void *, so_jargs *);
    MethodFunc method = (MethodFunc)sym->addr;
    va_list    args;
    va_start(args, methodID);
    so_jargs jargs = {.is_va = 1, .va = args, .index = 0};
    method(so_jobject_get_instance(obj), &jargs);
    va_end(args);
    so_logd("[JNI] CallVoidMethod(%p,%p,%p,...): ('%s')", env, obj, methodID, sym->name);
}

void CallVoidMethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list args) {
    if (!obj || !methodID) {
        so_loge("[JNI] CallVoidMethodV(%p,%p,%p,args): invalid arguments!", env, obj, methodID);
        return;
    }
    const so_jsymbol *sym = (const so_jsymbol *)methodID;
    if (!sym->addr) {
        so_loge("[JNI] CallVoidMethodV(%p,%p,%p,args): invalid methodID!", env, obj, methodID);
        return;
    }
    typedef void (*MethodFunc)(void *, so_jargs *);
    MethodFunc method = (MethodFunc)sym->addr;
    so_jargs   jargs  = {.is_va = 1, .va = args, .index = 0};
    method(so_jobject_get_instance(obj), &jargs);
    so_logd("[JNI] CallVoidMethodV(%p,%p,%p,args): ('%s')", env, obj, methodID, sym->name);
}

void CallVoidMethodA(JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args) {
    if (!obj || !methodID) {
        so_loge("[JNI] CallVoidMethodA(%p,%p,%p,args): invalid arguments!", env, obj, methodID);
        return;
    }
    const so_jsymbol *sym = (const so_jsymbol *)methodID;
    if (!sym->addr) {
        so_loge("[JNI] CallVoidMethodA(%p,%p,%p,args): invalid methodID!", env, obj, methodID);
        return;
    }
    typedef void (*MethodFunc)(void *, so_jargs *);
    MethodFunc method = (MethodFunc)sym->addr;
    so_jargs   jargs  = {.is_va = 0, .vals = args, .index = 0};
    method(so_jobject_get_instance(obj), &jargs);
    so_logd("[JNI] CallVoidMethodA(%p,%p,%p,args): ('%s')", env, obj, methodID, sym->name);
}

#define CALL_NONVIRT_TYPE_METHOD(_jtype, _jname)                                                                              \
    _jtype CallNonvirtual##_jname##Method(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...) {                  \
        if (!obj || !clazz || !methodID) {                                                                                    \
            so_loge("[JNI] CallNonvirtual" #_jname "Method(%p,%p,%p,%p,...): invalid arguments!", env, obj, clazz, methodID); \
            return (_jtype)0;                                                                                                 \
        }                                                                                                                     \
        const so_jsymbol *ori_sym = (const so_jsymbol *)methodID;                                                             \
        if (ori_sym->flags & SO_JSYMBOL_FLAGS_STATIC) {                                                                       \
            so_loge("[JNI] CallNonvirtual" #_jname "Method(%p,%p,%p,%p,...): invalid methodID!", env, obj, clazz, methodID);  \
            return (_jtype)0;                                                                                                 \
        }                                                                                                                     \
        const so_jsymbol *sym = so_jclass_find_method(clazz, ori_sym->name, ori_sym->signature, 0);                           \
        if (!sym || !sym->addr) {                                                                                             \
            so_loge("[JNI] CallNonvirtual" #_jname "Method(%p,%p,%p,%p,...): invalid methodID!", env, obj, clazz, methodID);  \
            return (_jtype)0;                                                                                                 \
        }                                                                                                                     \
        typedef _jtype (*MethodFunc)(void *, so_jargs *);                                                                     \
        MethodFunc method = (MethodFunc)sym->addr;                                                                            \
        va_list    args;                                                                                                      \
        va_start(args, methodID);                                                                                             \
        so_jargs jargs  = {.is_va = 1, .va = args, .index = 0};                                                               \
        _jtype   result = method(so_jobject_get_instance(obj), &jargs);                                                       \
        va_end(args);                                                                                                         \
        so_logd("[JNI] CallNonvirtual" #_jname "Method(%p,%p,%p,%p,...): ('%s')", env, obj, clazz, methodID, sym->name);      \
        return result;                                                                                                        \
    }

#define CALL_NONVIRT_TYPE_METHOD_V(_jtype, _jname)                                                                              \
    _jtype CallNonvirtual##_jname##MethodV(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list args) {          \
        if (!obj || !clazz || !methodID) {                                                                                      \
            so_loge("[JNI] CallNonvirtual" #_jname "MethodV(%p,%p,%p,%p,args): invalid arguments!", env, obj, clazz, methodID); \
            return (_jtype)0;                                                                                                   \
        }                                                                                                                       \
        const so_jsymbol *ori_sym = (const so_jsymbol *)methodID;                                                               \
        if (ori_sym->flags & SO_JSYMBOL_FLAGS_STATIC) {                                                                         \
            so_loge("[JNI] CallNonvirtual" #_jname "MethodV(%p,%p,%p,%p,args): invalid methodID!", env, obj, clazz, methodID);  \
            return (_jtype)0;                                                                                                   \
        }                                                                                                                       \
        const so_jsymbol *sym = so_jclass_find_method(clazz, ori_sym->name, ori_sym->signature, 0);                             \
        if (!sym || !sym->addr) {                                                                                               \
            so_loge("[JNI] CallNonvirtual" #_jname "MethodV(%p,%p,%p,%p,args): invalid methodID!", env, obj, clazz, methodID);  \
            return (_jtype)0;                                                                                                   \
        }                                                                                                                       \
        typedef _jtype (*MethodFunc)(void *, so_jargs *);                                                                       \
        MethodFunc method = (MethodFunc)sym->addr;                                                                              \
        so_jargs   jargs  = {.is_va = 1, .va = args, .index = 0};                                                               \
        _jtype     result = method(so_jobject_get_instance(obj), &jargs);                                                       \
        so_logd("[JNI] CallNonvirtual" #_jname "MethodV(%p,%p,%p,%p,args): ('%s')", env, obj, clazz, methodID, sym->name);      \
        return result;                                                                                                          \
    }

#define CALL_NONVIRT_TYPE_METHOD_A(_jtype, _jname)                                                                              \
    _jtype CallNonvirtual##_jname##MethodA(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, const jvalue *args) {    \
        if (!obj || !clazz || !methodID) {                                                                                      \
            so_loge("[JNI] CallNonvirtual" #_jname "MethodA(%p,%p,%p,%p,args): invalid arguments!", env, obj, clazz, methodID); \
            return (_jtype)0;                                                                                                   \
        }                                                                                                                       \
        const so_jsymbol *ori_sym = (const so_jsymbol *)methodID;                                                               \
        if (ori_sym->flags & SO_JSYMBOL_FLAGS_STATIC) {                                                                         \
            so_loge("[JNI] CallNonvirtual" #_jname "MethodA(%p,%p,%p,%p,args): invalid methodID!", env, obj, clazz, methodID);  \
            return (_jtype)0;                                                                                                   \
        }                                                                                                                       \
        const so_jsymbol *sym = so_jclass_find_method(clazz, ori_sym->name, ori_sym->signature, 0);                             \
        if (!sym || !sym->addr) {                                                                                               \
            so_loge("[JNI] CallNonvirtual" #_jname "MethodA(%p,%p,%p,%p,args): invalid methodID!", env, obj, clazz, methodID);  \
            return (_jtype)0;                                                                                                   \
        }                                                                                                                       \
        typedef _jtype (*MethodFunc)(void *, so_jargs *);                                                                       \
        MethodFunc method = (MethodFunc)sym->addr;                                                                              \
        so_jargs   jargs  = {.is_va = 0, .vals = args, .index = 0};                                                             \
        _jtype     result = method(so_jobject_get_instance(obj), &jargs);                                                       \
        so_logd("[JNI] CallNonvirtual" #_jname "MethodA(%p,%p,%p,%p,args): ('%s')", env, obj, clazz, methodID, sym->name);      \
        return result;                                                                                                          \
    }

CALL_NONVIRT_TYPE_METHOD(jobject, Object)
CALL_NONVIRT_TYPE_METHOD_V(jobject, Object)
CALL_NONVIRT_TYPE_METHOD_A(jobject, Object)
CALL_NONVIRT_TYPE_METHOD(jboolean, Boolean)
CALL_NONVIRT_TYPE_METHOD_V(jboolean, Boolean)
CALL_NONVIRT_TYPE_METHOD_A(jboolean, Boolean)
CALL_NONVIRT_TYPE_METHOD(jbyte, Byte)
CALL_NONVIRT_TYPE_METHOD_V(jbyte, Byte)
CALL_NONVIRT_TYPE_METHOD_A(jbyte, Byte)
CALL_NONVIRT_TYPE_METHOD(jchar, Char)
CALL_NONVIRT_TYPE_METHOD_V(jchar, Char)
CALL_NONVIRT_TYPE_METHOD_A(jchar, Char)
CALL_NONVIRT_TYPE_METHOD(jshort, Short)
CALL_NONVIRT_TYPE_METHOD_V(jshort, Short)
CALL_NONVIRT_TYPE_METHOD_A(jshort, Short)
CALL_NONVIRT_TYPE_METHOD(jint, Int)
CALL_NONVIRT_TYPE_METHOD_V(jint, Int)
CALL_NONVIRT_TYPE_METHOD_A(jint, Int)
CALL_NONVIRT_TYPE_METHOD(jlong, Long)
CALL_NONVIRT_TYPE_METHOD_V(jlong, Long)
CALL_NONVIRT_TYPE_METHOD_A(jlong, Long)
CALL_NONVIRT_TYPE_METHOD(jfloat, Float)
CALL_NONVIRT_TYPE_METHOD_V(jfloat, Float)
CALL_NONVIRT_TYPE_METHOD_A(jfloat, Float)
CALL_NONVIRT_TYPE_METHOD(jdouble, Double)
CALL_NONVIRT_TYPE_METHOD_V(jdouble, Double)
CALL_NONVIRT_TYPE_METHOD_A(jdouble, Double)

void CallNonvirtualVoidMethod(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...) {
    if (!obj || !methodID) {
        so_loge("[JNI] CallNonvirtualVoidMethod(%p,%p,%p,%p,...): invalid arguments!", env, obj, clazz, methodID);
        return;
    }
    const so_jsymbol *ori_sym = (const so_jsymbol *)methodID;
    if (ori_sym->flags & SO_JSYMBOL_FLAGS_STATIC) {
        so_loge("[JNI] CallNonvirtualVoidMethod(%p,%p,%p,%p,...): invalid methodID!", env, obj, clazz, methodID);
        return;
    }
    const so_jsymbol *sym = so_jclass_find_method(clazz, ori_sym->name, ori_sym->signature, 0);
    if (!sym || !sym->addr) {
        so_loge("[JNI] CallNonvirtualVoidMethod(%p,%p,%p,%p,...): invalid methodID!", env, obj, clazz, methodID);
        return;
    }
    typedef void (*MethodFunc)(void *, so_jargs *);
    MethodFunc method = (MethodFunc)sym->addr;
    va_list    args;
    va_start(args, methodID);
    so_jargs jargs = {.is_va = 1, .va = args, .index = 0};
    method(so_jobject_get_instance(obj), &jargs);
    va_end(args);
    so_logd("[JNI] CallNonvirtualVoidMethod(%p,%p,%p,%p,...): ('%s')", env, obj, clazz, methodID, sym->name);
}

void CallNonvirtualVoidMethodV(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list args) {
    if (!obj || !methodID) {
        so_loge("[JNI] CallNonvirtualVoidMethodV(%p,%p,%p,%p,args): invalid arguments!", env, obj, clazz, methodID);
        return;
    }
    const so_jsymbol *ori_sym = (const so_jsymbol *)methodID;
    if (ori_sym->flags & SO_JSYMBOL_FLAGS_STATIC) {
        so_loge("[JNI] CallNonvirtualVoidMethodV(%p,%p,%p,%p,args): invalid methodID!", env, obj, clazz, methodID);
        return;
    }
    const so_jsymbol *sym = so_jclass_find_method(clazz, ori_sym->name, ori_sym->signature, 0);
    if (!sym || !sym->addr) {
        so_loge("[JNI] CallNonvirtualVoidMethodV(%p,%p,%p,%p,args): invalid methodID!", env, obj, clazz, methodID);
        return;
    }
    typedef void (*MethodFunc)(void *, so_jargs *);
    MethodFunc method = (MethodFunc)sym->addr;
    so_jargs   jargs  = {.is_va = 1, .va = args, .index = 0};
    method(so_jobject_get_instance(obj), &jargs);
    so_logd("[JNI] CallNonvirtualVoidMethodV(%p,%p,%p,%p,args): ('%s')", env, obj, clazz, methodID, sym->name);
}

void CallNonvirtualVoidMethodA(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, const jvalue *args) {
    if (!obj || !methodID) {
        so_loge("[JNI] CallNonvirtualVoidMethodA(%p,%p,%p,%p,args): invalid arguments!", env, obj, clazz, methodID);
        return;
    }
    const so_jsymbol *ori_sym = (const so_jsymbol *)methodID;
    if (ori_sym->flags & SO_JSYMBOL_FLAGS_STATIC) {
        so_loge("[JNI] CallNonvirtualVoidMethodA(%p,%p,%p,%p,args): invalid methodID!", env, obj, clazz, methodID);
        return;
    }
    const so_jsymbol *sym = so_jclass_find_method(clazz, ori_sym->name, ori_sym->signature, 0);
    if (!sym || !sym->addr) {
        so_loge("[JNI] CallNonvirtualVoidMethodA(%p,%p,%p,%p,args): invalid methodID!", env, obj, clazz, methodID);
        return;
    }
    typedef void (*MethodFunc)(void *, so_jargs *);
    MethodFunc method = (MethodFunc)sym->addr;
    so_jargs   jargs  = {.is_va = 0, .vals = args, .index = 0};
    method(so_jobject_get_instance(obj), &jargs);
    so_logd("[JNI] CallNonvirtualVoidMethodA(%p,%p,%p,%p,args): ('%s')", env, obj, clazz, methodID, sym->name);
}

jfieldID GetFieldID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
    if (!clazz || !name || !sig) {
        so_loge("[JNI] GetFieldID(%p,%p,'%s','%s'): invalid arguments!", env, clazz, name, sig);
        return NULL;
    }
    const so_jsymbol *sym = so_jclass_find_field(clazz, name, sig, 0);
    if (!sym) {
        so_loge("[JNI] GetFieldID(%p,%p,'%s','%s'): not found!", env, clazz, name, sig);
        return NULL;
    }
    jfieldID id = (jfieldID)sym;
    so_logd("[JNI] GetFieldID(%p,%p,'%s','%s'): %p", env, clazz, name, sig, id);
    return id;
}

#define GET_TYPE_FIELD(_jtype, _jname)                                                             \
    _jtype Get##_jname##Field(JNIEnv *env, jobject obj, jfieldID fieldID) {                        \
        if (!obj || !fieldID) {                                                                    \
            so_loge("[JNI] Get" #_jname "Field(%p,%p,%p): invalid arguments!", env, obj, fieldID); \
            return (_jtype)0;                                                                      \
        }                                                                                          \
        const so_jsymbol *sym      = (const so_jsymbol *)fieldID;                                  \
        void             *instance = so_jobject_get_instance(obj);                                 \
        if (!instance) {                                                                           \
            so_loge("[JNI] Get" #_jname "Field(%p,%p,%p): invalid object!", env, obj, fieldID);    \
            return (_jtype)0;                                                                      \
        }                                                                                          \
        _jtype result = *(_jtype *)((uintptr_t)instance + sym->addr);                              \
        so_logd("[JNI] Get" #_jname "Field(%p,%p,%p): ('%s')", env, obj, fieldID, sym->name);      \
        return result;                                                                             \
    }

GET_TYPE_FIELD(jobject, Object)
GET_TYPE_FIELD(jboolean, Boolean)
GET_TYPE_FIELD(jbyte, Byte)
GET_TYPE_FIELD(jchar, Char)
GET_TYPE_FIELD(jshort, Short)
GET_TYPE_FIELD(jint, Int)
GET_TYPE_FIELD(jlong, Long)
GET_TYPE_FIELD(jfloat, Float)
GET_TYPE_FIELD(jdouble, Double)

#define SET_TYPE_FIELD(_jtype, _jname)                                                             \
    void Set##_jname##Field(JNIEnv *env, jobject obj, jfieldID fieldID, _jtype value) {            \
        if (!obj || !fieldID) {                                                                    \
            so_loge("[JNI] Set" #_jname "Field(%p,%p,%p): invalid arguments!", env, obj, fieldID); \
            return;                                                                                \
        }                                                                                          \
        const so_jsymbol *sym      = (const so_jsymbol *)fieldID;                                  \
        void             *instance = so_jobject_get_instance(obj);                                 \
        if (!instance) {                                                                           \
            so_loge("[JNI] Set" #_jname "Field(%p,%p,%p): invalid object!", env, obj, fieldID);    \
            return;                                                                                \
        }                                                                                          \
        *(_jtype *)((uintptr_t)instance + sym->addr) = value;                                      \
        so_logd("[JNI] Set" #_jname "Field(%p,%p,%p): ('%s')", env, obj, fieldID, sym->name);      \
    }

SET_TYPE_FIELD(jobject, Object)
SET_TYPE_FIELD(jboolean, Boolean)
SET_TYPE_FIELD(jbyte, Byte)
SET_TYPE_FIELD(jchar, Char)
SET_TYPE_FIELD(jshort, Short)
SET_TYPE_FIELD(jint, Int)
SET_TYPE_FIELD(jlong, Long)
SET_TYPE_FIELD(jfloat, Float)
SET_TYPE_FIELD(jdouble, Double)

jmethodID GetStaticMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
    if (!clazz || !name || !sig) {
        so_loge("[JNI] GetStaticMethodID(%p,%p,'%s','%s'): invalid arguments!", env, clazz, name, sig);
        return NULL;
    }
    const so_jsymbol *sym = so_jclass_find_method(clazz, name, sig, 1);
    if (!sym) {
        so_loge("[JNI] GetStaticMethodID(%p,%p,'%s','%s'): not found!", env, clazz, name, sig);
        return NULL;
    }
    jmethodID id = (jmethodID)sym;
    so_logd("[JNI] GetStaticMethodID(%p,%p,'%s','%s'): %p", env, clazz, name, sig, id);
    return id;
}

#define CALL_STATIC_TYPE_METHOD(_jtype, _jname)                                                                   \
    _jtype CallStatic##_jname##Method(JNIEnv *env, jclass clazz, jmethodID methodID, ...) {                       \
        if (!clazz || !methodID) {                                                                                \
            so_loge("[JNI] CallStatic" #_jname "Method(%p,%p,%p,...): invalid arguments!", env, clazz, methodID); \
            return (_jtype)0;                                                                                     \
        }                                                                                                         \
        const so_jsymbol *sym = (const so_jsymbol *)methodID;                                                     \
        if (!sym->addr) {                                                                                         \
            so_loge("[JNI] CallStatic" #_jname "Method(%p,%p,%p,...): invalid methodID!", env, clazz, methodID);  \
            return (_jtype)0;                                                                                     \
        }                                                                                                         \
        typedef _jtype (*MethodFunc)(so_jargs *);                                                                 \
        MethodFunc method = (MethodFunc)sym->addr;                                                                \
        va_list    args;                                                                                          \
        va_start(args, methodID);                                                                                 \
        so_jargs jargs  = {.is_va = 1, .va = args, .index = 0};                                                   \
        _jtype   result = method(&jargs);                                                                         \
        va_end(args);                                                                                             \
        so_logd("[JNI] CallStatic" #_jname "Method(%p,%p,%p,...): ('%s')", env, clazz, methodID, sym->name);      \
        return result;                                                                                            \
    }

#define CALL_STATIC_TYPE_METHOD_V(_jtype, _jname)                                                                   \
    _jtype CallStatic##_jname##MethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args) {               \
        if (!clazz || !methodID) {                                                                                  \
            so_loge("[JNI] CallStatic" #_jname "MethodV(%p,%p,%p,args): invalid arguments!", env, clazz, methodID); \
            return (_jtype)0;                                                                                       \
        }                                                                                                           \
        const so_jsymbol *sym = (const so_jsymbol *)methodID;                                                       \
        if (!sym->addr) {                                                                                           \
            so_loge("[JNI] CallStatic" #_jname "MethodV(%p,%p,%p,args): invalid methodID!", env, clazz, methodID);  \
            return (_jtype)0;                                                                                       \
        }                                                                                                           \
        typedef _jtype (*MethodFunc)(so_jargs *);                                                                   \
        MethodFunc method = (MethodFunc)sym->addr;                                                                  \
        so_jargs   jargs  = {.is_va = 1, .va = args, .index = 0};                                                   \
        _jtype     result = method(&jargs);                                                                         \
        so_logd("[JNI] CallStatic" #_jname "MethodV(%p,%p,%p,args): ('%s')", env, clazz, methodID, sym->name);      \
        return result;                                                                                              \
    }

#define CALL_STATIC_TYPE_METHOD_A(_jtype, _jname)                                                                   \
    _jtype CallStatic##_jname##MethodA(JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args) {         \
        if (!clazz || !methodID) {                                                                                  \
            so_loge("[JNI] CallStatic" #_jname "MethodA(%p,%p,%p,args): invalid arguments!", env, clazz, methodID); \
            return (_jtype)0;                                                                                       \
        }                                                                                                           \
        const so_jsymbol *sym = (const so_jsymbol *)methodID;                                                       \
        if (!sym->addr) {                                                                                           \
            so_loge("[JNI] CallStatic" #_jname "MethodA(%p,%p,%p,args): invalid methodID!", env, clazz, methodID);  \
            return (_jtype)0;                                                                                       \
        }                                                                                                           \
        typedef _jtype (*MethodFunc)(so_jargs *);                                                                   \
        MethodFunc method = (MethodFunc)sym->addr;                                                                  \
        so_jargs   jargs  = {.is_va = 0, .vals = args, .index = 0};                                                 \
        _jtype     result = method(&jargs);                                                                         \
        so_logd("[JNI] CallStatic" #_jname "MethodA(%p,%p,%p,args): ('%s')", env, clazz, methodID, sym->name);      \
        return result;                                                                                              \
    }

CALL_STATIC_TYPE_METHOD(jobject, Object)
CALL_STATIC_TYPE_METHOD_V(jobject, Object)
CALL_STATIC_TYPE_METHOD_A(jobject, Object)
CALL_STATIC_TYPE_METHOD(jboolean, Boolean)
CALL_STATIC_TYPE_METHOD_V(jboolean, Boolean)
CALL_STATIC_TYPE_METHOD_A(jboolean, Boolean)
CALL_STATIC_TYPE_METHOD(jbyte, Byte)
CALL_STATIC_TYPE_METHOD_V(jbyte, Byte)
CALL_STATIC_TYPE_METHOD_A(jbyte, Byte)
CALL_STATIC_TYPE_METHOD(jchar, Char)
CALL_STATIC_TYPE_METHOD_V(jchar, Char)
CALL_STATIC_TYPE_METHOD_A(jchar, Char)
CALL_STATIC_TYPE_METHOD(jshort, Short)
CALL_STATIC_TYPE_METHOD_V(jshort, Short)
CALL_STATIC_TYPE_METHOD_A(jshort, Short)
CALL_STATIC_TYPE_METHOD(jint, Int)
CALL_STATIC_TYPE_METHOD_V(jint, Int)
CALL_STATIC_TYPE_METHOD_A(jint, Int)
CALL_STATIC_TYPE_METHOD(jlong, Long)
CALL_STATIC_TYPE_METHOD_V(jlong, Long)
CALL_STATIC_TYPE_METHOD_A(jlong, Long)
CALL_STATIC_TYPE_METHOD(jfloat, Float)
CALL_STATIC_TYPE_METHOD_V(jfloat, Float)
CALL_STATIC_TYPE_METHOD_A(jfloat, Float)
CALL_STATIC_TYPE_METHOD(jdouble, Double)
CALL_STATIC_TYPE_METHOD_V(jdouble, Double)
CALL_STATIC_TYPE_METHOD_A(jdouble, Double)

void CallStaticVoidMethod(JNIEnv *env, jclass clazz, jmethodID methodID, ...) {
    if (!clazz || !methodID) {
        so_loge("[JNI] CallStaticVoidMethod(%p,%p,%p,...): invalid arguments!", env, clazz, methodID);
        return;
    }
    const so_jsymbol *sym = (const so_jsymbol *)methodID;
    if (!sym->addr) {
        so_loge("[JNI] CallStaticVoidMethod(%p,%p,%p,...): invalid methodID!", env, clazz, methodID);
        return;
    }
    typedef void (*MethodFunc)(so_jargs *);
    MethodFunc method = (MethodFunc)sym->addr;
    va_list    args;
    va_start(args, methodID);
    so_jargs jargs = {.is_va = 1, .va = args, .index = 0};
    method(&jargs);
    va_end(args);
    so_logd("[JNI] CallStaticVoidMethod(%p,%p,%p): ('%s')", env, clazz, methodID, sym->name);
}

void CallStaticVoidMethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args) {
    if (!clazz || !methodID) {
        so_loge("[JNI] CallStaticVoidMethodV(%p,%p,%p,args): invalid arguments!", env, clazz, methodID);
        return;
    }
    const so_jsymbol *sym = (const so_jsymbol *)methodID;
    if (!sym->addr) {
        so_loge("[JNI] CallStaticVoidMethodV(%p,%p,%p,args): invalid methodID!", env, clazz, methodID);
        return;
    }
    typedef void (*MethodFunc)(so_jargs *);
    MethodFunc method = (MethodFunc)sym->addr;
    so_jargs   jargs  = {.is_va = 1, .va = args, .index = 0};
    method(&jargs);
    so_logd("[JNI] CallStaticVoidMethodV(%p,%p,%p,args): ('%s')", env, clazz, methodID, sym->name);
}

void CallStaticVoidMethodA(JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args) {
    if (!clazz || !methodID) {
        so_loge("[JNI] CallStaticVoidMethodA(%p,%p,%p,args): invalid arguments!", env, clazz, methodID);
        return;
    }
    const so_jsymbol *sym = (const so_jsymbol *)methodID;
    if (!sym->addr) {
        so_loge("[JNI] CallStaticVoidMethodA(%p,%p,%p,args): invalid methodID!", env, clazz, methodID);
        return;
    }
    typedef void (*MethodFunc)(so_jargs *);
    MethodFunc method = (MethodFunc)sym->addr;
    so_jargs   jargs  = {.is_va = 0, .vals = args, .index = 0};
    method(&jargs);
    so_logd("[JNI] CallStaticVoidMethodA(%p,%p,%p,args): ('%s')", env, clazz, methodID, sym->name);
}

jfieldID GetStaticFieldID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
    if (!clazz || !name || !sig) {
        so_loge("[JNI] GetStaticFieldID(%p,%p,'%s','%s'): invalid arguments!", env, clazz, name, sig);
        return NULL;
    }
    const so_jsymbol *sym = so_jclass_find_field(clazz, name, sig, 1);
    if (!sym) {
        so_loge("[JNI] GetStaticFieldID(%p,%p,'%s','%s'): not found!", env, clazz, name, sig);
        return NULL;
    }
    jfieldID id = (jfieldID)sym;
    so_logd("[JNI] GetStaticFieldID(%p,%p,'%s','%s'): %p", env, clazz, name, sig, id);
    return id;
}

#define GET_STATIC_TYPE_FIELD(_jtype, _jname)                                                              \
    _jtype GetStatic##_jname##Field(JNIEnv *env, jclass clazz, jfieldID fieldID) {                         \
        if (!clazz || !fieldID) {                                                                          \
            so_loge("[JNI] GetStatic" #_jname "Field(%p,%p,%p): invalid arguments!", env, clazz, fieldID); \
            return (_jtype)0;                                                                              \
        }                                                                                                  \
        const so_jsymbol *sym    = (const so_jsymbol *)fieldID;                                            \
        _jtype            result = *(_jtype *)sym->addr;                                                   \
        so_logd("[JNI] GetStatic" #_jname "Field(%p,%p,%p): ('%s')", env, clazz, fieldID, sym->name);      \
        return result;                                                                                     \
    }

GET_STATIC_TYPE_FIELD(jobject, Object)
GET_STATIC_TYPE_FIELD(jboolean, Boolean)
GET_STATIC_TYPE_FIELD(jbyte, Byte)
GET_STATIC_TYPE_FIELD(jchar, Char)
GET_STATIC_TYPE_FIELD(jshort, Short)
GET_STATIC_TYPE_FIELD(jint, Int)
GET_STATIC_TYPE_FIELD(jlong, Long)
GET_STATIC_TYPE_FIELD(jfloat, Float)
GET_STATIC_TYPE_FIELD(jdouble, Double)

#define SET_STATIC_TYPE_FIELD(_jtype, _jname)                                                              \
    void SetStatic##_jname##Field(JNIEnv *env, jclass clazz, jfieldID fieldID, _jtype value) {             \
        if (!clazz || !fieldID) {                                                                          \
            so_loge("[JNI] SetStatic" #_jname "Field(%p,%p,%p): invalid arguments!", env, clazz, fieldID); \
            return;                                                                                        \
        }                                                                                                  \
        const so_jsymbol *sym = (const so_jsymbol *)fieldID;                                               \
        *(_jtype *)sym->addr  = value;                                                                     \
        so_logd("[JNI] SetStatic" #_jname "Field(%p,%p,%p): ('%s')", env, clazz, fieldID, sym->name);      \
    }

SET_STATIC_TYPE_FIELD(jobject, Object)
SET_STATIC_TYPE_FIELD(jboolean, Boolean)
SET_STATIC_TYPE_FIELD(jbyte, Byte)
SET_STATIC_TYPE_FIELD(jchar, Char)
SET_STATIC_TYPE_FIELD(jshort, Short)
SET_STATIC_TYPE_FIELD(jint, Int)
SET_STATIC_TYPE_FIELD(jlong, Long)
SET_STATIC_TYPE_FIELD(jfloat, Float)
SET_STATIC_TYPE_FIELD(jdouble, Double)

jstring NewString(JNIEnv *env, const jchar *unicodeChars, jsize len) {
    if (!unicodeChars || len <= 0) {
        so_loge("[JNI] NewString(%p,%p,%d): invalid arguments!", env, unicodeChars, len);
        return NULL;
    }

    so_jstring *instance = so_jstring_new_with_utf16(unicodeChars, len);
    if (!instance) {
        so_loge("[JNI] NewString(%p,%p,%d): failed to alloc a new instance!", env, unicodeChars, len);
        return NULL;
    }

    jstring string = so_jobject_new(NULL, instance);
    if (!string) {
        so_loge("[JNI] NewString(%p,%p,%d): failed to alloc a new object!", env, unicodeChars, len);
        so_jinstance_release(instance);
        return NULL;
    }

    if (so_jref_take_local(string) != JNI_OK) {
        so_logw("[JNI] NewString(%p,%p,%d): %p, register failed, manual release required!", env, unicodeChars, len, string);
        return string;
    }

    so_logd("[JNI] NewString(%p,%p,%d): %p", env, unicodeChars, len, string);
    return string;
}

jsize GetStringLength(JNIEnv *env, jstring string) {
    if (!string) {
        so_loge("[JNI] GetStringLength(%p,%p): invalid arguments!", env, string);
        return 0;
    }

    jsize length = so_jstring_get_utf16_length(so_jstring_from_object(string));
    so_logd("[JNI] GetStringLength(%p,%p): %d", env, string, length);
    return length;
}

const jchar *GetStringChars(JNIEnv *env, jstring string, jboolean *isCopy) {
    if (!string) {
        so_loge("[JNI] GetStringChars(%p,%p,%p): invalid arguments!", env, string, isCopy);
        return NULL;
    }

    so_jstring  *str       = so_jstring_from_object(string);
    const jchar *ori_chars = so_jstring_get_utf16_chars(str);
    const jchar *chars     = so_jstring_retain_utf16_chars(str);
    if (!chars) {
        so_loge("[JNI] GetStringChars(%p,%p,%p) error reading element!", env, string, isCopy);
        return NULL;
    }

    if (isCopy) *isCopy = (ori_chars != chars);

    so_logd("[JNI] GetStringChars(%p,%p,%p): %p", env, string, isCopy, chars);
    return chars;
}

void ReleaseStringChars(JNIEnv *env, jstring string, const jchar *chars) {
    if (!string || !chars) {
        so_loge("[JNI] ReleaseStringChars(%p,%p,%p): invalid arguments!", env, string, chars);
        return;
    }

    so_logd("[JNI] ReleaseStringChars(%p,%p,%p)", env, string, chars);
    so_jstring_release_utf16_chars(so_jstring_from_object(string), (jchar *)chars);
}

jstring NewStringUTF(JNIEnv *env, const char *bytes) {
    if (!bytes) {
        so_loge("[JNI] NewStringUTF(%p,%p): invalid arguments!", env, bytes);
        return NULL;
    }

    so_jstring *instance = so_jstring_new_with_utf8(bytes);
    if (!instance) {
        so_loge("[JNI] NewStringUTF(%p,%p): failed to alloc a new instance!", env, bytes);
        return NULL;
    }

    jstring string = so_jobject_new(NULL, instance);
    if (!string) {
        so_loge("[JNI] NewStringUTF(%p,%p): failed to alloc a new object!", env, bytes);
        so_jinstance_release(instance);
        return NULL;
    }

    if (so_jref_take_local(string) != JNI_OK) {
        so_logw("[JNI] NewStringUTF(%p,'%s'): %p, register failed, manual release required!", env, bytes, string);
        return string;
    }

    so_logd("[JNI] NewStringUTF(%p,'%s'): %p", env, bytes, string);
    return string;
}

jsize GetStringUTFLength(JNIEnv *env, jstring string) {
    if (!string) {
        so_loge("[JNI] GetStringUTFLength(%p,%p): invalid arguments!", env, string);
        return 0;
    }

    jsize length = so_jstring_get_utf8_length(so_jstring_from_object(string));
    so_logd("[JNI] GetStringUTFLength(%p,%p): %d", env, string, length);
    return length;
}

const char *GetStringUTFChars(JNIEnv *env, jstring string, jboolean *isCopy) {
    if (!string) {
        so_loge("[JNI] GetStringUTFChars(%p,%p,%p): invalid arguments!", env, string, isCopy);
        return NULL;
    }

    so_jstring *str     = so_jstring_from_object(string);
    const char *ori_utf = so_jstring_get_utf8_chars(str);
    const char *utf     = so_jstring_retain_utf8_chars(str);
    if (!utf) {
        so_loge("[JNI] GetStringUTFChars(%p,%p,%p) error reading element!", env, string, isCopy);
        return NULL;
    }

    if (isCopy) *isCopy = (ori_utf != utf);

    so_logd("[JNI] GetStringUTFChars(%p,%p,%p): '%s'", env, string, isCopy, utf);
    return utf;
}

void ReleaseStringUTFChars(JNIEnv *env, jstring string, const char *utf) {
    if (!string || !utf) {
        so_loge("[JNI] ReleaseStringUTFChars(%p,%p,%p): invalid arguments!", env, string, utf);
        return;
    }

    so_logd("[JNI] ReleaseStringUTFChars(%p,%p,'%s')", env, string, utf);
    so_jstring_release_utf8_chars(so_jstring_from_object(string), (char *)utf);
}

jsize GetArrayLength(JNIEnv *env, jarray array) {
    if (!array) {
        so_loge("[JNI] GetArrayLength(%p,%p): invalid arguments!", env, array);
        return 0;
    }

    jsize length = so_jarray_get_length(so_jarray_from_object(array));
    so_logd("[JNI] GetArrayLength(%p,%p): %d", env, array, length);
    return length;
}

jobjectArray NewObjectArray(JNIEnv *env, jsize length, jclass elementClass, jobject initialElement) {
    if (length <= 0) {
        so_loge("[JNI] NewObjectArray(%p,%d,%p,%p): invalid arguments!", env, length, elementClass, initialElement);
        return NULL;
    }

    if (initialElement && elementClass) {
        jclass initialClass = so_jobject_get_class(initialElement);
        if (!initialClass) {
            so_loge("[JNI] NewObjectArray(%p,%d,%p,%p): failed to get class of initialElement!", env, length, elementClass, initialElement);
            return NULL;
        }
        if (!so_jclass_is_assignable_from(initialClass, elementClass)) {
            so_loge("[JNI] NewObjectArray(%p,%d,%p,%p): type mismatch!", env, length, elementClass, initialElement);
            return NULL;
        }
    }

    so_jarray *instance = so_jarray_new(length, SO_JARRAY_TYPE_OBJECT);
    if (!instance) {
        so_loge("[JNI] NewObjectArray(%p,%d,%p,%p): failed to alloc a new instance!", env, length, elementClass, initialElement);
        return NULL;
    }

    jobjectArray array = so_jobject_new(NULL, instance);
    if (!array) {
        so_loge("[JNI] NewObjectArray(%p,%d,%p,%p): failed to alloc a new object!", env, length, elementClass, initialElement);
        so_jinstance_release(instance);
        return NULL;
    }

    jobject *elems = (jobject *)so_jarray_get_elems(instance);
    if (elems) {
        for (jsize i = 0; i < length; i++) {
            elems[i] = so_jref_new_local(initialElement);
        }
    }

    if (so_jref_take_local(array) != JNI_OK) {
        so_logw("[JNI] NewObjectArray(%p,%d,%p,%p): %p, register failed, manual release required!", env, length, elementClass, initialElement, array);
        return array;
    }

    so_logd("[JNI] NewObjectArray(%p,%d,%p,%p): %p", env, length, elementClass, initialElement, array);
    return array;
}

jobject GetObjectArrayElement(JNIEnv *env, jobjectArray array, jsize index) {
    if (!array) {
        so_loge("[JNI] GetObjectArrayElement(%p,%p,%d): invalid arguments!", env, array, index);
        return NULL;
    }

    so_jarray *arr    = so_jarray_from_object(array);
    jobject   *elems  = (jobject *)so_jarray_get_elems(arr);
    jsize      length = so_jarray_get_length(arr);
    if (!elems || index < 0 || index >= length) {
        so_loge("[JNI] GetObjectArrayElement(%p,%p,%d): invalid arguments!", env, array, index);
        return NULL;
    }

    jobject result = so_jref_new_local(elems[index]);
    so_logd("[JNI] GetObjectArrayElement(%p,%p,%d): %p", env, array, index, result);
    return result;
}

void SetObjectArrayElement(JNIEnv *env, jobjectArray array, jsize index, jobject value) {
    if (!array) {
        so_loge("[JNI] SetObjectArrayElement(%p,%p,%d,%p): invalid arguments!", env, array, index, value);
        return;
    }

    so_jarray *arr    = so_jarray_from_object(array);
    jobject   *elems  = (jobject *)so_jarray_get_elems(arr);
    jsize      length = so_jarray_get_length(arr);
    if (!elems || index < 0 || index >= length) {
        so_loge("[JNI] GetObjectArrayElement(%p,%p,%d): invalid arguments!", env, array, index);
        return;
    }

    if (elems[index]) {
        so_jref_delete_local(elems[index]);
    }

    elems[index] = so_jref_new_local(value);

    so_logd("[JNI] SetObjectArrayElement(%p,%p,%d,%p)", env, array, index, value);
}

#define NEW_ARRAY(_jtype, _jname, _jtype_enum)                                                                              \
    _jtype##Array New##_jname##Array(JNIEnv *env, jsize length) {                                                           \
        if (length <= 0) {                                                                                                  \
            so_loge("[JNI] New" #_jname "Array(%p,%d): invalid arguments!", env, length);                                   \
            return NULL;                                                                                                    \
        }                                                                                                                   \
                                                                                                                            \
        so_jarray *instance = so_jarray_new(length, _jtype_enum);                                                           \
        if (!instance) {                                                                                                    \
            so_loge("[JNI] New" #_jname "Array(%p,%d): failed to alloc a new instance!", env, length);                      \
            return NULL;                                                                                                    \
        }                                                                                                                   \
                                                                                                                            \
        _jtype##Array array = so_jobject_new(NULL, instance);                                                               \
        if (!array) {                                                                                                       \
            so_loge("[JNI] New" #_jname "Array(%p,%d): failed to alloc a new object!", env, length);                        \
            so_jinstance_release(instance);                                                                                 \
            return NULL;                                                                                                    \
        }                                                                                                                   \
                                                                                                                            \
        if (so_jref_take_local(array) != JNI_OK) {                                                                          \
            so_logw("[JNI] New" #_jname "Array(%p,%d): %p, register failed, manual release required!", env, length, array); \
            return array;                                                                                                   \
        }                                                                                                                   \
                                                                                                                            \
        so_logd("[JNI] New" #_jname "Array(%p,%d): %p", env, length, array);                                                \
        return array;                                                                                                       \
    }

NEW_ARRAY(jboolean, Boolean, SO_JARRAY_TYPE_BOOLEAN)
NEW_ARRAY(jbyte, Byte, SO_JARRAY_TYPE_BYTE)
NEW_ARRAY(jchar, Char, SO_JARRAY_TYPE_CHAR)
NEW_ARRAY(jshort, Short, SO_JARRAY_TYPE_SHORT)
NEW_ARRAY(jint, Int, SO_JARRAY_TYPE_INT)
NEW_ARRAY(jlong, Long, SO_JARRAY_TYPE_LONG)
NEW_ARRAY(jfloat, Float, SO_JARRAY_TYPE_FLOAT)
NEW_ARRAY(jdouble, Double, SO_JARRAY_TYPE_DOUBLE)

#define GET_ARRAY_ELEMENTS(_jtype, _jname)                                                                       \
    _jtype *Get##_jname##ArrayElements(JNIEnv *env, _jtype##Array array, jboolean *isCopy) {                     \
        if (!array) {                                                                                            \
            so_loge("[JNI] Get" #_jname "ArrayElements(%p,%p,%p): invalid arguments!", env, array, isCopy);      \
            return NULL;                                                                                         \
        }                                                                                                        \
                                                                                                                 \
        so_jarray *arr       = so_jarray_from_object(array);                                                     \
        _jtype    *ori_elems = (_jtype *)so_jarray_get_elems(arr);                                               \
        _jtype    *elems     = (_jtype *)so_jarray_retain_elems(arr);                                            \
        if (!elems) {                                                                                            \
            so_loge("[JNI] Get" #_jname "ArrayElements(%p,%p,%p): invalid array elements!", env, array, isCopy); \
            return NULL;                                                                                         \
        }                                                                                                        \
                                                                                                                 \
        if (isCopy) *isCopy = (ori_elems != elems);                                                              \
                                                                                                                 \
        so_logd("[JNI] Get" #_jname "ArrayElements(%p,%p,%p): %p", env, array, isCopy, elems);                   \
        return elems;                                                                                            \
    }

GET_ARRAY_ELEMENTS(jboolean, Boolean)
GET_ARRAY_ELEMENTS(jbyte, Byte)
GET_ARRAY_ELEMENTS(jchar, Char)
GET_ARRAY_ELEMENTS(jshort, Short)
GET_ARRAY_ELEMENTS(jint, Int)
GET_ARRAY_ELEMENTS(jlong, Long)
GET_ARRAY_ELEMENTS(jfloat, Float)
GET_ARRAY_ELEMENTS(jdouble, Double)

#define RELEASE_ARRAY_ELEMENTS(_jtype, _jname)                                                        \
    void Release##_jname##ArrayElements(JNIEnv *env, _jtype##Array array, _jtype *elems, jint mode) { \
        if (!array || !elems) {                                                                       \
            so_loge("[JNI] Release" #_jname "ArrayElements(%p,%p,%p,%d): invalid arguments!",         \
                    env, array, elems, mode);                                                         \
            return;                                                                                   \
        }                                                                                             \
                                                                                                      \
        so_jarray *arr = so_jarray_from_object(array);                                                \
        if (mode != JNI_ABORT) {                                                                      \
            so_jarray_write_elems(arr, 0, so_jarray_get_length(arr), elems);                          \
        }                                                                                             \
                                                                                                      \
        if (mode != JNI_COMMIT) {                                                                     \
            so_jarray_release_elems(arr, elems);                                                      \
        }                                                                                             \
                                                                                                      \
        so_logd("[JNI] Release" #_jname "ArrayElements(%p,%p,%p,%d)", env, array, elems, mode);       \
    }

RELEASE_ARRAY_ELEMENTS(jboolean, Boolean)
RELEASE_ARRAY_ELEMENTS(jbyte, Byte)
RELEASE_ARRAY_ELEMENTS(jchar, Char)
RELEASE_ARRAY_ELEMENTS(jshort, Short)
RELEASE_ARRAY_ELEMENTS(jint, Int)
RELEASE_ARRAY_ELEMENTS(jlong, Long)
RELEASE_ARRAY_ELEMENTS(jfloat, Float)
RELEASE_ARRAY_ELEMENTS(jdouble, Double)

#define GET_ARRAY_REGION(_jtype, _jname)                                                                   \
    void Get##_jname##ArrayRegion(JNIEnv *env, _jtype##Array array, jsize start, jsize len, _jtype *buf) { \
        if (!array || !buf || len <= 0) {                                                                  \
            so_loge("[JNI] Get" #_jname "ArrayRegion(%p,%p,%d,%d,%p): invalid arguments!",                 \
                    env, array, start, len, buf);                                                          \
            return;                                                                                        \
        }                                                                                                  \
                                                                                                           \
        so_jarray *arr = so_jarray_from_object(array);                                                     \
        if (so_jarray_read_elems(arr, start, len, buf) != JNI_OK) {                                        \
            so_loge("[JNI] Get" #_jname "ArrayRegion(%p,%p,%d,%d,%p): error reading element!",             \
                    env, array, start, len, buf);                                                          \
            return;                                                                                        \
        }                                                                                                  \
                                                                                                           \
        so_logd("[JNI] Get" #_jname "ArrayRegion(%p,%p,%d,%d,%p)", env, array, start, len, buf);           \
    }

GET_ARRAY_REGION(jboolean, Boolean)
GET_ARRAY_REGION(jbyte, Byte)
GET_ARRAY_REGION(jchar, Char)
GET_ARRAY_REGION(jshort, Short)
GET_ARRAY_REGION(jint, Int)
GET_ARRAY_REGION(jlong, Long)
GET_ARRAY_REGION(jfloat, Float)
GET_ARRAY_REGION(jdouble, Double)

#define SET_ARRAY_REGION(_jtype, _jname)                                                                         \
    void Set##_jname##ArrayRegion(JNIEnv *env, _jtype##Array array, jsize start, jsize len, const _jtype *buf) { \
        if (!array || !buf || len <= 0) {                                                                        \
            so_loge("[JNI] Set" #_jname "ArrayRegion(%p,%p,%d,%d,%p): invalid arguments!",                       \
                    env, array, start, len, buf);                                                                \
            return;                                                                                              \
        }                                                                                                        \
                                                                                                                 \
        so_jarray *arr = so_jarray_from_object(array);                                                           \
        if (so_jarray_write_elems(arr, start, len, buf) != JNI_OK) {                                             \
            so_loge("[JNI] Set" #_jname "ArrayRegion(%p,%p,%d,%d,%p): error writing element!",                   \
                    env, array, start, len, buf);                                                                \
            return;                                                                                              \
        }                                                                                                        \
                                                                                                                 \
        so_logd("[JNI] Set" #_jname "ArrayRegion(%p,%p,%d,%d,%p)", env, array, start, len, buf);                 \
    }

SET_ARRAY_REGION(jboolean, Boolean)
SET_ARRAY_REGION(jbyte, Byte)
SET_ARRAY_REGION(jchar, Char)
SET_ARRAY_REGION(jshort, Short)
SET_ARRAY_REGION(jint, Int)
SET_ARRAY_REGION(jlong, Long)
SET_ARRAY_REGION(jfloat, Float)
SET_ARRAY_REGION(jdouble, Double)

jint RegisterNatives(JNIEnv *env, jclass clazz, const JNINativeMethod *methods, jint numMethods) {
    jint result = so_jclass_register_natives(clazz, methods, numMethods);
    so_logd("[JNI] RegisterNatives(%p,%p,%p,%d): %d", env, clazz, methods, numMethods, result);
    return result;
}

jint UnregisterNatives(JNIEnv *env, jclass clazz) {
    jint result = so_jclass_unregister_natives(clazz);
    so_logd("[JNI] UnregisterNatives(%p,%p): %d", env, clazz, result);
    return result;
}

jint MonitorEnter(JNIEnv *env, jobject obj) {
    so_logd("[JNI] MonitorEnter(%p,%p): not implemented", env, obj);
    return JNI_OK;
}

jint MonitorExit(JNIEnv *env, jobject obj) {
    so_logd("[JNI] MonitorExit(%p,%p): not implemented", env, obj);
    return JNI_OK;
}

jint GetJavaVM(JNIEnv *env, JavaVM **vm) {
    if (!vm) {
        so_loge("[JNI] GetJavaVM(%p,%p): invalid arguments!", env, vm);
        return JNI_ERR;
    }

    *vm = &_java_vm;
    so_logd("[JNI] GetJavaVM(%p,%p): %d", env, vm, JNI_OK);
    return JNI_OK;
}

void GetStringRegion(JNIEnv *env, jstring string, jsize start, jsize len, jchar *buf) {
    if (!string || !buf || len <= 0) {
        so_loge("[JNI] GetStringRegion(%p,%p,%d,%d,%p): invalid arguments!", env, string, start, len, buf);
        return;
    }

    so_jstring *str = so_jstring_from_object(string);
    if (so_jstring_read_utf16_chars(str, start, len, buf) != JNI_OK) {
        so_loge("[JNI] GetStringRegion(%p,%p,%d,%d,%p): error writing element!", env, string, start, len, buf);
        return;
    }

    so_logd("[JNI] GetStringRegion(%p,%p,%d,%d,%p)", env, string, start, len, buf);
}

void GetStringUTFRegion(JNIEnv *env, jstring string, jsize start, jsize len, char *buf) {
    if (!string || !buf || len <= 0) {
        so_loge("[JNI] GetStringUTFRegion(%p,%p,%d,%d,%p): invalid arguments!", env, string, start, len, buf);
        return;
    }

    so_jstring *str = so_jstring_from_object(string);
    if (so_jstring_read_utf8_chars(str, start, len, buf) != JNI_OK) {
        so_loge("[JNI] GetStringUTFRegion(%p,%p,%d,%d,%p): error writing element!", env, string, start, len, buf);
        return;
    }

    so_logd("[JNI] GetStringUTFRegion(%p,%p,%d,%d,%p)", env, string, start, len, buf);
}

void *GetPrimitiveArrayCritical(JNIEnv *env, jarray array, jboolean *isCopy) {
    if (!array) {
        so_loge("[JNI] GetPrimitiveArrayCritical(%p,%p,%p): invalid arguments!", env, array, isCopy);
        return NULL;
    }

    so_jarray *arr       = so_jarray_from_object(array);
    void      *ori_elems = so_jarray_get_elems(arr);
    void      *carray    = so_jarray_retain_elems(arr);
    if (!carray) {
        so_loge("[JNI] GetPrimitiveArrayCritical(%p,%p,%p): invalid array elements!", env, array, isCopy);
        return NULL;
    }

    if (isCopy) *isCopy = (ori_elems != carray);

    so_logd("[JNI] GetPrimitiveArrayCritical(%p,%p,%p): %p", env, array, isCopy, carray);
    return carray;
}

void ReleasePrimitiveArrayCritical(JNIEnv *env, jarray array, void *carray, jint mode) {
    if (!array || !carray) {
        so_loge("[JNI] ReleasePrimitiveArrayCritical(%p,%p,%p,%d): invalid arguments!", env, array, carray, mode);
        return;
    }

    so_jarray *arr = so_jarray_from_object(array);
    if (mode != JNI_ABORT) {
        so_jarray_write_elems(arr, 0, so_jarray_get_length(arr), carray);
    }
    if (mode != JNI_COMMIT) {
        so_jarray_release_elems(arr, carray);
    }

    so_logd("[JNI] ReleasePrimitiveArrayCritical(%p,%p,%p,%d)", env, array, carray, mode);
}

const jchar *GetStringCritical(JNIEnv *env, jstring string, jboolean *isCopy) {
    if (!string) {
        so_loge("[JNI] GetStringCritical(%p,%p,%p): invalid arguments!", env, string, isCopy);
        return NULL;
    }

    so_jstring  *str       = so_jstring_from_object(string);
    const jchar *ori_chars = so_jstring_get_utf16_chars(str);
    const jchar *carray    = so_jstring_retain_utf16_chars(str);
    if (!carray) {
        so_loge("[JNI] GetStringCritical(%p,%p,%p): invalid string utf16!", env, string, isCopy);
        return NULL;
    }

    if (isCopy) *isCopy = (ori_chars != carray);

    so_logd("[JNI] GetStringCritical(%p,%p,%p): %p", env, string, isCopy, carray);
    return carray;
}

void ReleaseStringCritical(JNIEnv *env, jstring string, const jchar *carray) {
    if (!string || !carray) {
        so_loge("[JNI] ReleaseStringCritical(%p,%p,%p): invalid arguments!", env, string, carray);
        return;
    }

    so_logd("[JNI] ReleaseStringCritical(%p,%p,%p)", env, string, carray);
    so_jstring *str = so_jstring_from_object(string);
    so_jstring_release_utf16_chars(str, (jchar *)carray);
}

jweak NewWeakGlobalRef(JNIEnv *env, jobject obj) {
    so_logw("[JNI] NewWeakGlobalRef(%p,%p): ignored", env, obj);
    return (jweak)obj;
}

void DeleteWeakGlobalRef(JNIEnv *env, jweak obj) {
    so_logw("[JNI] DeleteWeakGlobalRef(%p,%p): ignored", env, obj);
}

jboolean ExceptionCheck(JNIEnv *env) {
    so_logw("[JNI] ExceptionCheck(%p): ignored", env);
    return JNI_FALSE;
}

jobject NewDirectByteBuffer(JNIEnv *env, void *address, jlong capacity) {
    if (!address || capacity <= 0) {
        so_loge("[JNI] NewDirectByteBuffer(%p,%p,%lld): invalid arguments!", env, address, capacity);
        return NULL;
    }

    so_jbuffer *instance = so_jbuffer_new_direct(address, capacity);
    if (!instance) {
        so_loge("[JNI] NewDirectByteBuffer(%p,%p,%lld): failed to alloc a new instance!", env, address, capacity);
        return NULL;
    }

    jobject buffer = so_jobject_new(NULL, instance);
    if (!buffer) {
        so_loge("[JNI] NewDirectByteBuffer(%p,%p,%lld): failed to alloc a new object!", env, address, capacity);
        so_jinstance_release(instance);
        return NULL;
    }

    if (so_jref_take_local(buffer) != JNI_OK) {
        so_logw("[JNI] NewDirectByteBuffer(%p,%p,%lld): %p, register failed, manual release required!", env, address, capacity, buffer);
        return buffer;
    }

    so_logd("[JNI] NewDirectByteBuffer(%p,%p,%lld): %p", env, address, capacity, buffer);
    return buffer;
}

void *GetDirectBufferAddress(JNIEnv *env, jobject buffer) {
    if (!buffer) {
        so_loge("[JNI] GetDirectBufferAddress(%p,%p): invalid arguments!", env, buffer);
        return NULL;
    }

    void *address = so_jbuffer_get_data(so_jbuffer_from_object(buffer));
    if (!address) {
        so_loge("[JNI] GetDirectBufferAddress(%p,%p): invalid address!", env, buffer);
        return NULL;
    }

    so_logd("[JNI] GetDirectBufferAddress(%p,%p): %p", env, buffer, address);
    return address;
}

jlong GetDirectBufferCapacity(JNIEnv *env, jobject buffer) {
    if (!buffer) {
        so_loge("[JNI] GetDirectBufferCapacity(%p,%p): invalid arguments!", env, buffer);
        return 0;
    }

    jlong capacity = so_jbuffer_get_size(so_jbuffer_from_object(buffer));
    if (capacity <= 0) {
        so_loge("[JNI] GetDirectBufferCapacity(%p,%p): invalid capacity!", env, buffer);
        return 0;
    }

    so_logd("[JNI] GetDirectBufferCapacity(%p,%p): %lld", env, buffer, capacity);
    return capacity;
}

jobjectRefType GetObjectRefType(JNIEnv *env, jobject obj) {
    jobjectRefType type = so_jobject_get_ref_type(obj);
    so_logd("[JNI] GetObjectRefType(%p,%p): %d", env, obj, type);
    return type;
}

/*
 * ============================================================================
 * JNI Invocation API
 * ============================================================================
 */
jint JNI_GetDefaultJavaVMInitArgs(void *vm_args) {
    if (!vm_args) {
        so_loge("[JNI] JNI_GetDefaultJavaVMInitArgs(%p): invalid arguments!", vm_args);
        return JNI_ERR;
    }

    so_logd("[JNI] JNI_GetDefaultJavaVMInitArgs(%p): version=0x%x, ignored", vm_args, *(jint *)vm_args);
    return JNI_OK;
}

jint JNI_CreateJavaVM(JavaVM **p_vm, JNIEnv **p_env, void *vm_args) {
    if (!p_vm || !p_env || !vm_args) {
        so_loge("[JNI] JNI_CreateJavaVM(%p,%p,%p): invalid arguments!", p_vm, p_env, vm_args);
        return JNI_ERR;
    }

    *p_vm  = &_java_vm;
    *p_env = &_jni_env;

    so_logd("[JNI] JNI_CreateJavaVM(%p,%p,%p): version=0x%x", p_vm, p_env, vm_args, *(jint *)vm_args);
    return JNI_OK;
}

jint JNI_GetCreatedJavaVMs(JavaVM **vm_buf, jsize buf_len, jsize *n_vms) {
    if (vm_buf) {
        if (buf_len < 1) {
            so_loge("[JNI] JNI_GetCreatedJavaVMs(%p,%d,%p): invalid arguments!", vm_buf, buf_len, n_vms);
            return JNI_ERR;
        }
        *vm_buf = &_java_vm;
    }

    if (n_vms) {
        *n_vms = 1;
    }

    so_logd("[JNI] JNI_GetCreatedJavaVMs(%p,%d,%p)", vm_buf, buf_len, n_vms);
    return JNI_OK;
}
