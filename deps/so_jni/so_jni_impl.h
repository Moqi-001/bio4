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
    jclass clazz = JniFindClass(name);
    if (!clazz) {
        so_loge("[JNI] FindClass(%p,%s): not found!", env, name);
        return NULL;
    }

    so_logd("[JNI] FindClass(%p,%s): %p", env, name, clazz);
    return clazz;
}

jclass GetSuperclass(JNIEnv *env, jclass clazz) {
    jclass super_cls = JniGetSuperclass(clazz);
    if (!super_cls) {
        so_loge("[JNI] GetSuperclass(%p,%p): no super class!", env, clazz);
        return NULL;
    }

    so_logd("[JNI] GetSuperclass(%p,%p): %p", env, clazz, super_cls);
    return super_cls;
}

jboolean IsAssignableFrom(JNIEnv *env, jclass clazz1, jclass clazz2) {
    jboolean result = JniIsAssignableFrom(clazz1, clazz2);
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
    jint result = JniPushLocalFrame(capacity);
    so_logd("[JNI] PushLocalFrame(%p,%d)", env, capacity);
    return result;
}

jobject PopLocalFrame(JNIEnv *env, jobject result) {
    jobject preserved = JniPopLocalFrame(result);
    so_logd("[JNI] PopLocalFrame(%p,%p): %p", env, result, preserved);
    return preserved;
}

jobject NewGlobalRef(JNIEnv *env, jobject obj) {
    jobject result = JniNewGlobalRef(obj);
    if (!result) {
        so_loge("[JNI] NewGlobalRef(%p,%p): error!", env, obj);
        return NULL;
    }

    so_logd("[JNI] NewGlobalRef(%p,%p): %p", env, obj, result);
    return result;
}

void DeleteGlobalRef(JNIEnv *env, jobject obj) {
    JniDeleteGlobalRef(obj);
    so_logd("[JNI] DeleteGlobalRef(%p,%p)", env, obj);
}

void DeleteLocalRef(JNIEnv *env, jobject obj) {
    JniDeleteLocalRef(obj);
    so_logd("[JNI] DeleteLocalRef(%p,%p)", env, obj);
}

jboolean IsSameObject(JNIEnv *env, jobject ref1, jobject ref2) {
    jboolean result = JniIsSameObject(ref1, ref2);
    so_logd("[JNI] IsSameObject(%p,%p,%p): %u", env, ref1, ref2, result);
    return result;
}

jobject NewLocalRef(JNIEnv *env, jobject obj) {
    jobject result = JniNewLocalRef(obj);
    if (!result) {
        so_loge("[JNI] NewLocalRef(%p,%p): error!", env, obj);
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
    jobject obj = JniAllocObject(clazz);
    if (!obj) {
        so_loge("[JNI] AllocObject(%p,%p): error!", env, clazz);
        return NULL;
    }

    so_logd("[JNI] AllocObject(%p,%p): %p", env, clazz, obj);
    return obj;
}

jobject NewObject(JNIEnv *env, jclass clazz, jmethodID methodID, ...) {
    if (!clazz || !methodID) {
        so_logd("[JNI] NewObject(%p,%p,%p,...): invalid arguments!", env, clazz, methodID);
        return NULL;
    }

    va_list args;
    va_start(args, methodID);
    jobject obj = JniNewObjectV(clazz, methodID, args);
    if (!obj) {
        so_loge("[JNI] NewObject(%p,%p,%p,...): error!", env, clazz, methodID);
        return NULL;
    }
    va_end(args);

    so_logd("[JNI] NewObject(%p,%p,%p,...): %p", env, clazz, methodID, obj);
    return obj;
}

jobject NewObjectV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args) {
    if (!clazz || !methodID) {
        so_logd("[JNI] NewObjectV(%p,%p,%p,args): invalid arguments!", env, clazz, methodID);
        return NULL;
    }

    jobject obj = JniNewObjectV(clazz, methodID, args);
    if (!obj) {
        so_loge("[JNI] NewObjectV(%p,%p,%p,args): error!", env, clazz, methodID);
        return NULL;
    }

    so_logd("[JNI] NewObjectV(%p,%p,%p,args): %p", env, clazz, methodID, obj);
    return obj;
}

jobject NewObjectA(JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args) {
    if (!clazz || !methodID) {
        so_logd("[JNI] NewObjectA(%p,%p,%p,args): invalid arguments!", env, clazz, methodID);
        return NULL;
    }

    jobject obj = JniNewObjectA(clazz, methodID, args);
    if (!obj) {
        so_loge("[JNI] NewObjectA(%p,%p,%p,args): error!", env, clazz, methodID);
        return NULL;
    }

    so_logd("[JNI] NewObjectA(%p,%p,%p,args): %p", env, clazz, methodID, obj);
    return obj;
}

jclass GetObjectClass(JNIEnv *env, jobject object) {
    jclass clazz = JniGetObjectClass(object);
    if (!clazz) {
        so_loge("[JNI] GetObjectClass(%p,%p): error!", env, object);
        return NULL;
    }

    so_logd("[JNI] GetObjectClass(%p,%p): %p ('%s')", env, object, clazz, JniGetClassName(clazz));
    return clazz;
}

jboolean IsInstanceOf(JNIEnv *env, jobject object, jclass clazz) {
    jboolean result = JniIsInstanceOf(object, clazz);
    so_logd("[JNI] IsInstanceOf(%p,%p,%p): %u", env, object, clazz, result);
    return result;
}

jmethodID GetMethodID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
    if (!clazz || !name || !sig) {
        so_loge("[JNI] GetMethodID(%p,%p,'%s','%s'): invalid arguments!", env, clazz, name, sig);
        return NULL;
    }

    const jsymbol *sym = JniFindMethod(clazz, name, sig, 0);
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
        const jsymbol *sym = (const jsymbol *)methodID;                                                   \
        if (!sym->addr) {                                                                                 \
            so_loge("[JNI] Call" #_jname "Method(%p,%p,%p,...): invalid methodID!", env, obj, methodID);  \
            return (_jtype)0;                                                                             \
        }                                                                                                 \
        typedef _jtype (*MethodFunc)(void *, jargs *);                                                    \
        MethodFunc method = (MethodFunc)sym->addr;                                                        \
        va_list    args;                                                                                  \
        va_start(args, methodID);                                                                         \
        jargs  jargs  = {.is_va = 1, .va = args, .index = 0};                                             \
        _jtype result = method(JniGetObjectInstance(obj), &jargs);                                        \
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
        const jsymbol *sym = (const jsymbol *)methodID;                                                     \
        if (!sym->addr) {                                                                                   \
            so_loge("[JNI] Call" #_jname "MethodV(%p,%p,%p,args): invalid methodID!", env, obj, methodID);  \
            return (_jtype)0;                                                                               \
        }                                                                                                   \
        typedef _jtype (*MethodFunc)(void *, jargs *);                                                      \
        MethodFunc method = (MethodFunc)sym->addr;                                                          \
        jargs      jargs  = {.is_va = 1, .va = args, .index = 0};                                           \
        _jtype     result = method(JniGetObjectInstance(obj), &jargs);                                      \
        so_logd("[JNI] Call" #_jname "MethodV(%p,%p,%p,args): ('%s')", env, obj, methodID, sym->name);      \
        return result;                                                                                      \
    }

#define CALL_TYPE_METHOD_A(_jtype, _jname)                                                                  \
    _jtype Call##_jname##MethodA(JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args) {        \
        if (!obj || !methodID) {                                                                            \
            so_loge("[JNI] Call" #_jname "MethodA(%p,%p,%p,args): invalid arguments!", env, obj, methodID); \
            return (_jtype)0;                                                                               \
        }                                                                                                   \
        const jsymbol *sym = (const jsymbol *)methodID;                                                     \
        if (!sym->addr) {                                                                                   \
            so_loge("[JNI] Call" #_jname "MethodA(%p,%p,%p,args): invalid methodID!", env, obj, methodID);  \
            return (_jtype)0;                                                                               \
        }                                                                                                   \
        typedef _jtype (*MethodFunc)(void *, jargs *);                                                      \
        MethodFunc method = (MethodFunc)sym->addr;                                                          \
        jargs      jargs  = {.is_va = 0, .vals = args, .index = 0};                                         \
        _jtype     result = method(JniGetObjectInstance(obj), &jargs);                                      \
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
    const jsymbol *sym = (const jsymbol *)methodID;
    if (!sym->addr) {
        so_loge("[JNI] CallVoidMethod(%p,%p,%p,...): invalid methodID!", env, obj, methodID);
        return;
    }
    typedef void (*MethodFunc)(void *, jargs *);
    MethodFunc method = (MethodFunc)sym->addr;
    va_list    args;
    va_start(args, methodID);
    jargs jargs = {.is_va = 1, .va = args, .index = 0};
    method(JniGetObjectInstance(obj), &jargs);
    va_end(args);
    so_logd("[JNI] CallVoidMethod(%p,%p,%p,...): ('%s')", env, obj, methodID, sym->name);
}

void CallVoidMethodV(JNIEnv *env, jobject obj, jmethodID methodID, va_list args) {
    if (!obj || !methodID) {
        so_loge("[JNI] CallVoidMethodV(%p,%p,%p,args): invalid arguments!", env, obj, methodID);
        return;
    }
    const jsymbol *sym = (const jsymbol *)methodID;
    if (!sym->addr) {
        so_loge("[JNI] CallVoidMethodV(%p,%p,%p,args): invalid methodID!", env, obj, methodID);
        return;
    }
    typedef void (*MethodFunc)(void *, jargs *);
    MethodFunc method = (MethodFunc)sym->addr;
    jargs      jargs  = {.is_va = 1, .va = args, .index = 0};
    method(JniGetObjectInstance(obj), &jargs);
    so_logd("[JNI] CallVoidMethodV(%p,%p,%p,args): ('%s')", env, obj, methodID, sym->name);
}

void CallVoidMethodA(JNIEnv *env, jobject obj, jmethodID methodID, const jvalue *args) {
    if (!obj || !methodID) {
        so_loge("[JNI] CallVoidMethodA(%p,%p,%p,args): invalid arguments!", env, obj, methodID);
        return;
    }
    const jsymbol *sym = (const jsymbol *)methodID;
    if (!sym->addr) {
        so_loge("[JNI] CallVoidMethodA(%p,%p,%p,args): invalid methodID!", env, obj, methodID);
        return;
    }
    typedef void (*MethodFunc)(void *, jargs *);
    MethodFunc method = (MethodFunc)sym->addr;
    jargs      jargs  = {.is_va = 0, .vals = args, .index = 0};
    method(JniGetObjectInstance(obj), &jargs);
    so_logd("[JNI] CallVoidMethodA(%p,%p,%p,args): ('%s')", env, obj, methodID, sym->name);
}

#define CALL_NONVIRT_TYPE_METHOD(_jtype, _jname)                                                                              \
    _jtype CallNonvirtual##_jname##Method(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, ...) {                  \
        if (!obj || !clazz || !methodID) {                                                                                    \
            so_loge("[JNI] CallNonvirtual" #_jname "Method(%p,%p,%p,%p,...): invalid arguments!", env, obj, clazz, methodID); \
            return (_jtype)0;                                                                                                 \
        }                                                                                                                     \
        const jsymbol *ori_sym = (const jsymbol *)methodID;                                                                   \
        if (ori_sym->flags & JNI_SYMBOL_FLAGS_STATIC) {                                                                       \
            so_loge("[JNI] CallNonvirtual" #_jname "Method(%p,%p,%p,%p,...): invalid methodID!", env, obj, clazz, methodID);  \
            return (_jtype)0;                                                                                                 \
        }                                                                                                                     \
        const jsymbol *sym = JniFindMethod(clazz, ori_sym->name, ori_sym->signature, 0);                                      \
        if (!sym || !sym->addr) {                                                                                             \
            so_loge("[JNI] CallNonvirtual" #_jname "Method(%p,%p,%p,%p,...): invalid methodID!", env, obj, clazz, methodID);  \
            return (_jtype)0;                                                                                                 \
        }                                                                                                                     \
        typedef _jtype (*MethodFunc)(void *, jargs *);                                                                        \
        MethodFunc method = (MethodFunc)sym->addr;                                                                            \
        va_list    args;                                                                                                      \
        va_start(args, methodID);                                                                                             \
        jargs  jargs  = {.is_va = 1, .va = args, .index = 0};                                                                 \
        _jtype result = method(JniGetObjectInstance(obj), &jargs);                                                            \
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
        const jsymbol *ori_sym = (const jsymbol *)methodID;                                                                     \
        if (ori_sym->flags & JNI_SYMBOL_FLAGS_STATIC) {                                                                         \
            so_loge("[JNI] CallNonvirtual" #_jname "MethodV(%p,%p,%p,%p,args): invalid methodID!", env, obj, clazz, methodID);  \
            return (_jtype)0;                                                                                                   \
        }                                                                                                                       \
        const jsymbol *sym = JniFindMethod(clazz, ori_sym->name, ori_sym->signature, 0);                                        \
        if (!sym || !sym->addr) {                                                                                               \
            so_loge("[JNI] CallNonvirtual" #_jname "MethodV(%p,%p,%p,%p,args): invalid methodID!", env, obj, clazz, methodID);  \
            return (_jtype)0;                                                                                                   \
        }                                                                                                                       \
        typedef _jtype (*MethodFunc)(void *, jargs *);                                                                          \
        MethodFunc method = (MethodFunc)sym->addr;                                                                              \
        jargs      jargs  = {.is_va = 1, .va = args, .index = 0};                                                               \
        _jtype     result = method(JniGetObjectInstance(obj), &jargs);                                                          \
        so_logd("[JNI] CallNonvirtual" #_jname "MethodV(%p,%p,%p,%p,args): ('%s')", env, obj, clazz, methodID, sym->name);      \
        return result;                                                                                                          \
    }

#define CALL_NONVIRT_TYPE_METHOD_A(_jtype, _jname)                                                                              \
    _jtype CallNonvirtual##_jname##MethodA(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, const jvalue *args) {    \
        if (!obj || !clazz || !methodID) {                                                                                      \
            so_loge("[JNI] CallNonvirtual" #_jname "MethodA(%p,%p,%p,%p,args): invalid arguments!", env, obj, clazz, methodID); \
            return (_jtype)0;                                                                                                   \
        }                                                                                                                       \
        const jsymbol *ori_sym = (const jsymbol *)methodID;                                                                     \
        if (ori_sym->flags & JNI_SYMBOL_FLAGS_STATIC) {                                                                         \
            so_loge("[JNI] CallNonvirtual" #_jname "MethodA(%p,%p,%p,%p,args): invalid methodID!", env, obj, clazz, methodID);  \
            return (_jtype)0;                                                                                                   \
        }                                                                                                                       \
        const jsymbol *sym = JniFindMethod(clazz, ori_sym->name, ori_sym->signature, 0);                                        \
        if (!sym || !sym->addr) {                                                                                               \
            so_loge("[JNI] CallNonvirtual" #_jname "MethodA(%p,%p,%p,%p,args): invalid methodID!", env, obj, clazz, methodID);  \
            return (_jtype)0;                                                                                                   \
        }                                                                                                                       \
        typedef _jtype (*MethodFunc)(void *, jargs *);                                                                          \
        MethodFunc method = (MethodFunc)sym->addr;                                                                              \
        jargs      jargs  = {.is_va = 0, .vals = args, .index = 0};                                                             \
        _jtype     result = method(JniGetObjectInstance(obj), &jargs);                                                          \
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
    const jsymbol *ori_sym = (const jsymbol *)methodID;
    if (ori_sym->flags & JNI_SYMBOL_FLAGS_STATIC) {
        so_loge("[JNI] CallNonvirtualVoidMethod(%p,%p,%p,%p,...): invalid methodID!", env, obj, clazz, methodID);
        return;
    }
    const jsymbol *sym = JniFindMethod(clazz, ori_sym->name, ori_sym->signature, 0);
    if (!sym || !sym->addr) {
        so_loge("[JNI] CallNonvirtualVoidMethod(%p,%p,%p,%p,...): invalid methodID!", env, obj, clazz, methodID);
        return;
    }
    typedef void (*MethodFunc)(void *, jargs *);
    MethodFunc method = (MethodFunc)sym->addr;
    va_list    args;
    va_start(args, methodID);
    jargs jargs = {.is_va = 1, .va = args, .index = 0};
    method(JniGetObjectInstance(obj), &jargs);
    va_end(args);
    so_logd("[JNI] CallNonvirtualVoidMethod(%p,%p,%p,%p,...): ('%s')", env, obj, clazz, methodID, sym->name);
}

void CallNonvirtualVoidMethodV(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, va_list args) {
    if (!obj || !methodID) {
        so_loge("[JNI] CallNonvirtualVoidMethodV(%p,%p,%p,%p,args): invalid arguments!", env, obj, clazz, methodID);
        return;
    }
    const jsymbol *ori_sym = (const jsymbol *)methodID;
    if (ori_sym->flags & JNI_SYMBOL_FLAGS_STATIC) {
        so_loge("[JNI] CallNonvirtualVoidMethodV(%p,%p,%p,%p,args): invalid methodID!", env, obj, clazz, methodID);
        return;
    }
    const jsymbol *sym = JniFindMethod(clazz, ori_sym->name, ori_sym->signature, 0);
    if (!sym || !sym->addr) {
        so_loge("[JNI] CallNonvirtualVoidMethodV(%p,%p,%p,%p,args): invalid methodID!", env, obj, clazz, methodID);
        return;
    }
    typedef void (*MethodFunc)(void *, jargs *);
    MethodFunc method = (MethodFunc)sym->addr;
    jargs      jargs  = {.is_va = 1, .va = args, .index = 0};
    method(JniGetObjectInstance(obj), &jargs);
    so_logd("[JNI] CallNonvirtualVoidMethodV(%p,%p,%p,%p,args): ('%s')", env, obj, clazz, methodID, sym->name);
}

void CallNonvirtualVoidMethodA(JNIEnv *env, jobject obj, jclass clazz, jmethodID methodID, const jvalue *args) {
    if (!obj || !methodID) {
        so_loge("[JNI] CallNonvirtualVoidMethodA(%p,%p,%p,%p,args): invalid arguments!", env, obj, clazz, methodID);
        return;
    }
    const jsymbol *ori_sym = (const jsymbol *)methodID;
    if (ori_sym->flags & JNI_SYMBOL_FLAGS_STATIC) {
        so_loge("[JNI] CallNonvirtualVoidMethodA(%p,%p,%p,%p,args): invalid methodID!", env, obj, clazz, methodID);
        return;
    }
    const jsymbol *sym = JniFindMethod(clazz, ori_sym->name, ori_sym->signature, 0);
    if (!sym || !sym->addr) {
        so_loge("[JNI] CallNonvirtualVoidMethodA(%p,%p,%p,%p,args): invalid methodID!", env, obj, clazz, methodID);
        return;
    }
    typedef void (*MethodFunc)(void *, jargs *);
    MethodFunc method = (MethodFunc)sym->addr;
    jargs      jargs  = {.is_va = 0, .vals = args, .index = 0};
    method(JniGetObjectInstance(obj), &jargs);
    so_logd("[JNI] CallNonvirtualVoidMethodA(%p,%p,%p,%p,args): ('%s')", env, obj, clazz, methodID, sym->name);
}

jfieldID GetFieldID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
    if (!clazz || !name || !sig) {
        so_loge("[JNI] GetFieldID(%p,%p,'%s','%s'): invalid arguments!", env, clazz, name, sig);
        return NULL;
    }
    const jsymbol *sym = JniFindField(clazz, name, sig, 0);
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
        const jsymbol *sym      = (const jsymbol *)fieldID;                                        \
        void          *instance = JniGetObjectInstance(obj);                                       \
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
        const jsymbol *sym      = (const jsymbol *)fieldID;                                        \
        void          *instance = JniGetObjectInstance(obj);                                       \
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
    const jsymbol *sym = JniFindMethod(clazz, name, sig, 1);
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
        const jsymbol *sym = (const jsymbol *)methodID;                                                           \
        if (!sym->addr) {                                                                                         \
            so_loge("[JNI] CallStatic" #_jname "Method(%p,%p,%p,...): invalid methodID!", env, clazz, methodID);  \
            return (_jtype)0;                                                                                     \
        }                                                                                                         \
        typedef _jtype (*MethodFunc)(jargs *);                                                                    \
        MethodFunc method = (MethodFunc)sym->addr;                                                                \
        va_list    args;                                                                                          \
        va_start(args, methodID);                                                                                 \
        jargs  jargs  = {.is_va = 1, .va = args, .index = 0};                                                     \
        _jtype result = method(&jargs);                                                                           \
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
        const jsymbol *sym = (const jsymbol *)methodID;                                                             \
        if (!sym->addr) {                                                                                           \
            so_loge("[JNI] CallStatic" #_jname "MethodV(%p,%p,%p,args): invalid methodID!", env, clazz, methodID);  \
            return (_jtype)0;                                                                                       \
        }                                                                                                           \
        typedef _jtype (*MethodFunc)(jargs *);                                                                      \
        MethodFunc method = (MethodFunc)sym->addr;                                                                  \
        jargs      jargs  = {.is_va = 1, .va = args, .index = 0};                                                   \
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
        const jsymbol *sym = (const jsymbol *)methodID;                                                             \
        if (!sym->addr) {                                                                                           \
            so_loge("[JNI] CallStatic" #_jname "MethodA(%p,%p,%p,args): invalid methodID!", env, clazz, methodID);  \
            return (_jtype)0;                                                                                       \
        }                                                                                                           \
        typedef _jtype (*MethodFunc)(jargs *);                                                                      \
        MethodFunc method = (MethodFunc)sym->addr;                                                                  \
        jargs      jargs  = {.is_va = 0, .vals = args, .index = 0};                                                 \
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
    const jsymbol *sym = (const jsymbol *)methodID;
    if (!sym->addr) {
        so_loge("[JNI] CallStaticVoidMethod(%p,%p,%p,...): invalid methodID!", env, clazz, methodID);
        return;
    }
    typedef void (*MethodFunc)(jargs *);
    MethodFunc method = (MethodFunc)sym->addr;
    va_list    args;
    va_start(args, methodID);
    jargs jargs = {.is_va = 1, .va = args, .index = 0};
    method(&jargs);
    va_end(args);
    so_logd("[JNI] CallStaticVoidMethod(%p,%p,%p): ('%s')", env, clazz, methodID, sym->name);
}

void CallStaticVoidMethodV(JNIEnv *env, jclass clazz, jmethodID methodID, va_list args) {
    if (!clazz || !methodID) {
        so_loge("[JNI] CallStaticVoidMethodV(%p,%p,%p,args): invalid arguments!", env, clazz, methodID);
        return;
    }
    const jsymbol *sym = (const jsymbol *)methodID;
    if (!sym->addr) {
        so_loge("[JNI] CallStaticVoidMethodV(%p,%p,%p,args): invalid methodID!", env, clazz, methodID);
        return;
    }
    typedef void (*MethodFunc)(jargs *);
    MethodFunc method = (MethodFunc)sym->addr;
    jargs      jargs  = {.is_va = 1, .va = args, .index = 0};
    method(&jargs);
    so_logd("[JNI] CallStaticVoidMethodV(%p,%p,%p,args): ('%s')", env, clazz, methodID, sym->name);
}

void CallStaticVoidMethodA(JNIEnv *env, jclass clazz, jmethodID methodID, const jvalue *args) {
    if (!clazz || !methodID) {
        so_loge("[JNI] CallStaticVoidMethodA(%p,%p,%p,args): invalid arguments!", env, clazz, methodID);
        return;
    }
    const jsymbol *sym = (const jsymbol *)methodID;
    if (!sym->addr) {
        so_loge("[JNI] CallStaticVoidMethodA(%p,%p,%p,args): invalid methodID!", env, clazz, methodID);
        return;
    }
    typedef void (*MethodFunc)(jargs *);
    MethodFunc method = (MethodFunc)sym->addr;
    jargs      jargs  = {.is_va = 0, .vals = args, .index = 0};
    method(&jargs);
    so_logd("[JNI] CallStaticVoidMethodA(%p,%p,%p,args): ('%s')", env, clazz, methodID, sym->name);
}

jfieldID GetStaticFieldID(JNIEnv *env, jclass clazz, const char *name, const char *sig) {
    const jsymbol *sym = JniFindField(clazz, name, sig, 1);
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
        const jsymbol *sym    = (const jsymbol *)fieldID;                                                  \
        _jtype         result = *(_jtype *)sym->addr;                                                      \
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
        const jsymbol *sym   = (const jsymbol *)fieldID;                                                   \
        *(_jtype *)sym->addr = value;                                                                      \
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
    jstring string = JniNewString(unicodeChars, len);
    if (!string) {
        so_loge("[JNI] NewString(%p,%p,%d): error!", env, unicodeChars, len);
        return NULL;
    }

    so_logd("[JNI] NewString(%p,%p,%d): %p", env, unicodeChars, len, string);
    return string;
}

jsize GetStringLength(JNIEnv *env, jstring string) {
    jsize length = JniGetStringLength(string);
    so_logd("[JNI] GetStringLength(%p,%p): %d", env, string, length);
    return length;
}

const jchar *GetStringChars(JNIEnv *env, jstring string, jboolean *isCopy) {
    const jchar *chars = JniGetStringChars(string, isCopy);
    if (!chars) {
        so_loge("[JNI] GetStringChars(%p,%p,%p): error!", env, string, isCopy);
        return NULL;
    }

    so_logd("[JNI] GetStringChars(%p,%p,%p): %p", env, string, isCopy, chars);
    return chars;
}

void ReleaseStringChars(JNIEnv *env, jstring string, const jchar *chars) {
    so_logd("[JNI] ReleaseStringChars(%p,%p,%p)", env, string, chars);
    JniReleaseStringChars(string, (jchar *)chars);
}

jstring NewStringUTF(JNIEnv *env, const char *bytes) {
    jstring string = JniNewStringUTF(bytes);
    if (!string) {
        so_loge("[JNI] NewStringUTF(%p,%p): failed to create a new object!", env, bytes);
        return NULL;
    }

    so_logd("[JNI] NewStringUTF(%p,'%s'): %p", env, bytes, string);
    return string;
}

jsize GetStringUTFLength(JNIEnv *env, jstring string) {
    jsize length = JniGetStringUTFLength(string);
    so_logd("[JNI] GetStringUTFLength(%p,%p): %d", env, string, length);
    return length;
}

const char *GetStringUTFChars(JNIEnv *env, jstring string, jboolean *isCopy) {
    const char *utf = JniGetStringUTFChars(string, isCopy);
    if (!utf) {
        so_loge("[JNI] GetStringUTFChars(%p,%p,%p): error!", env, string, isCopy);
        return NULL;
    }

    so_logd("[JNI] GetStringUTFChars(%p,%p,%p): '%s'", env, string, isCopy, utf);
    return utf;
}

void ReleaseStringUTFChars(JNIEnv *env, jstring string, const char *utf) {
    so_logd("[JNI] ReleaseStringUTFChars(%p,%p,'%s')", env, string, utf);
    JniReleaseStringUTFChars(string, utf);
}

jsize GetArrayLength(JNIEnv *env, jarray array) {
    jsize length = JniGetArrayLength(array);
    so_logd("[JNI] GetArrayLength(%p,%p): %d", env, array, length);
    return length;
}

jobjectArray NewObjectArray(JNIEnv *env, jsize length, jclass elementClass, jobject initialElement) {
    jobjectArray *array = JniNewObjectArray(length, elementClass, initialElement);
    if (!array) {
        so_loge("[JNI] NewObjectArray(%p,%d,%p,%p): error!", env, length, elementClass, initialElement);
        return NULL;
    }

    so_logd("[JNI] NewObjectArray(%p,%d,%p,%p): %p", env, length, elementClass, initialElement, array);
    return array;
}

jobject GetObjectArrayElement(JNIEnv *env, jobjectArray array, jsize index) {
    jobject elem = JniGetObjectArrayElement(array, index);
    if (!elem) {
        so_loge("[JNI] GetObjectArrayElement(%p,%p,%d): error!", env, array, index);
        return NULL;
    }

    so_logd("[JNI] GetObjectArrayElement(%p,%p,%d): %p", env, array, index, elem);
    return elem;
}

void SetObjectArrayElement(JNIEnv *env, jobjectArray array, jsize index, jobject value) {
    so_logd("[JNI] SetObjectArrayElement(%p,%p,%d,%p)", env, array, index, value);
    JniSetObjectArrayElement(array, index, value);
}

#define NEW_ARRAY(_jtype, _jname, _jtype_enum)                                \
    _jtype##Array New##_jname##Array(JNIEnv *env, jsize length) {             \
        _jtype##Array array = JniNew##_jname##Array(length);                  \
        if (!array) {                                                         \
            so_loge("[JNI] New" #_jname "Array(%p,%d): error!", env, length); \
            return NULL;                                                      \
        }                                                                     \
        so_logd("[JNI] New" #_jname "Array(%p,%d): %p", env, length, array);  \
        return array;                                                         \
    }
NEW_ARRAY(jboolean, Boolean, SO_JARRAY_TYPE_BOOLEAN)
NEW_ARRAY(jbyte, Byte, SO_JARRAY_TYPE_BYTE)
NEW_ARRAY(jchar, Char, SO_JARRAY_TYPE_CHAR)
NEW_ARRAY(jshort, Short, SO_JARRAY_TYPE_SHORT)
NEW_ARRAY(jint, Int, SO_JARRAY_TYPE_INT)
NEW_ARRAY(jlong, Long, SO_JARRAY_TYPE_LONG)
NEW_ARRAY(jfloat, Float, SO_JARRAY_TYPE_FLOAT)
NEW_ARRAY(jdouble, Double, SO_JARRAY_TYPE_DOUBLE)

#define GET_ARRAY_ELEMENTS(_jtype, _jname)                                                      \
    _jtype *Get##_jname##ArrayElements(JNIEnv *env, _jtype##Array array, jboolean *isCopy) {    \
        _jtype *elems = JniGet##_jname##ArrayElements(array, isCopy);                           \
        if (!elems) {                                                                           \
            so_loge("[JNI] Get" #_jname "ArrayElements(%p,%p,%p): error!", env, array, isCopy); \
            return NULL;                                                                        \
        }                                                                                       \
        so_logd("[JNI] Get" #_jname "ArrayElements(%p,%p,%p): %p", env, array, isCopy, elems);  \
        return elems;                                                                           \
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
        so_logd("[JNI] Release" #_jname "ArrayElements(%p,%p,%p,%d)", env, array, elems, mode);       \
        JniRelease##_jname##ArrayElements(array, elems, mode);                                        \
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
        so_logd("[JNI] Get" #_jname "ArrayRegion(%p,%p,%d,%d,%p)", env, array, start, len, buf);           \
        JniGet##_jname##ArrayRegion(array, start, len, buf);                                               \
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
        so_logd("[JNI] Set" #_jname "ArrayRegion(%p,%p,%d,%d,%p)", env, array, start, len, buf);                 \
        JniSet##_jname##ArrayRegion(array, start, len, buf);                                                     \
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
    jint result = JniRegisterNatives(clazz, methods, numMethods);
    so_logd("[JNI] RegisterNatives(%p,%p,%p,%d): %d", env, clazz, methods, numMethods, result);
    return result;
}

jint UnregisterNatives(JNIEnv *env, jclass clazz) {
    jint result = JniUnregisterNatives(clazz);
    so_logd("[JNI] UnregisterNatives(%p,%p): %d", env, clazz, result);
    return result;
}

jint MonitorEnter(JNIEnv *env, jobject obj) {
    so_logw("[JNI] MonitorEnter(%p,%p): not implemented", env, obj);
    return JNI_OK;
}

jint MonitorExit(JNIEnv *env, jobject obj) {
    so_logw("[JNI] MonitorExit(%p,%p): not implemented", env, obj);
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
    JniGetStringRegion(string, start, len, buf);
    so_logd("[JNI] GetStringRegion(%p,%p,%d,%d,%p)", env, string, start, len, buf);
}

void GetStringUTFRegion(JNIEnv *env, jstring string, jsize start, jsize len, char *buf) {
    JniGetStringUTFRegion(string, start, len, buf);
    so_logd("[JNI] GetStringUTFRegion(%p,%p,%d,%d,%p)", env, string, start, len, buf);
}

void *GetPrimitiveArrayCritical(JNIEnv *env, jarray array, jboolean *isCopy) {
    void *carray = JniGetPrimitiveArrayCritical(array, isCopy);
    if (!carray) {
        so_loge("[JNI] GetPrimitiveArrayCritical(%p,%p,%p): invalid array elements!", env, array, isCopy);
        return NULL;
    }

    so_logd("[JNI] GetPrimitiveArrayCritical(%p,%p,%p): %p", env, array, isCopy, carray);
    return carray;
}

void ReleasePrimitiveArrayCritical(JNIEnv *env, jarray array, void *carray, jint mode) {
    JniReleasePrimitiveArrayCritical(array, carray, mode);
    so_logd("[JNI] ReleasePrimitiveArrayCritical(%p,%p,%p,%d)", env, array, carray, mode);
}

const jchar *GetStringCritical(JNIEnv *env, jstring string, jboolean *isCopy) {
    const jchar *carray = JniGetStringChars(string, isCopy);
    if (!carray) {
        so_loge("[JNI] GetStringCritical(%p,%p,%p): invalid string utf16!", env, string, isCopy);
        return NULL;
    }

    so_logd("[JNI] GetStringCritical(%p,%p,%p): %p", env, string, isCopy, carray);
    return carray;
}

void ReleaseStringCritical(JNIEnv *env, jstring string, const jchar *carray) {
    so_logd("[JNI] ReleaseStringCritical(%p,%p,%p)", env, string, carray);
    JniReleaseStringChars(string, (jchar *)carray);
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
    jobject buffer = JniNewDirectByteBuffer(address, capacity);
    if (!buffer) {
        so_loge("[JNI] NewDirectByteBuffer(%p,%p,%lld): failed to alloc a new object!", env, address, capacity);
        return NULL;
    }

    so_logd("[JNI] NewDirectByteBuffer(%p,%p,%lld): %p", env, address, capacity, buffer);
    return buffer;
}

void *GetDirectBufferAddress(JNIEnv *env, jobject buffer) {
    void *address = JniGetDirectBufferAddress(buffer);
    if (!address) {
        so_loge("[JNI] GetDirectBufferAddress(%p,%p): invalid address!", env, buffer);
        return NULL;
    }

    so_logd("[JNI] GetDirectBufferAddress(%p,%p): %p", env, buffer, address);
    return address;
}

jlong GetDirectBufferCapacity(JNIEnv *env, jobject buffer) {
    jlong capacity = JniGetDirectBufferCapacity(buffer);
    if (capacity <= 0) {
        so_loge("[JNI] GetDirectBufferCapacity(%p,%p): invalid capacity!", env, buffer);
        return 0;
    }

    so_logd("[JNI] GetDirectBufferCapacity(%p,%p): %lld", env, buffer, capacity);
    return capacity;
}

jobjectRefType GetObjectRefType(JNIEnv *env, jobject obj) {
    jobjectRefType type = JniGetObjectRefType(obj);
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
