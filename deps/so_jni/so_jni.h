#ifndef SO_JNI_H
#define SO_JNI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "jni.h"

#include "so_jarray.h"
#include "so_jbuffer.h"
#include "so_jclass.h"
#include "so_jinstance.h"
#include "so_jobject.h"
#include "so_jref.h"
#include "so_jstring.h"

typedef struct so_jargs {
    jboolean is_va;
    union {
        va_list       va;
        const jvalue *vals;
    };
    jsize index;
} so_jargs;

#define SO_JARG(args, T) _Generic((T)0,                                                         \
    jboolean: ((args)->is_va ? va_arg((args)->va, jboolean) : (args)->vals[(args)->index++].z), \
    jbyte: ((args)->is_va ? va_arg((args)->va, jbyte) : (args)->vals[(args)->index++].b),       \
    jchar: ((args)->is_va ? va_arg((args)->va, jchar) : (args)->vals[(args)->index++].c),       \
    jshort: ((args)->is_va ? va_arg((args)->va, jshort) : (args)->vals[(args)->index++].s),     \
    jint: ((args)->is_va ? va_arg((args)->va, jint) : (args)->vals[(args)->index++].i),         \
    jlong: ((args)->is_va ? va_arg((args)->va, jlong) : (args)->vals[(args)->index++].j),       \
    jfloat: ((args)->is_va ? va_arg((args)->va, jfloat) : (args)->vals[(args)->index++].f),     \
    jdouble: ((args)->is_va ? va_arg((args)->va, jdouble) : (args)->vals[(args)->index++].d),   \
    jobject: ((args)->is_va ? va_arg((args)->va, jobject) : (args)->vals[(args)->index++].l))

extern JavaVM _java_vm;
extern JNIEnv _jni_env;

int  so_jni_init(void);
void so_jni_fini(void);

jclass so_jni_find_class(const char *name);
int    so_jni_register_class(jclass clazz);
int    so_jni_register_class_take(jclass clazz);
jint   so_jni_register_class_from_meta(so_jclass_meta *meta);

uint32_t so_jni_hash(const char *name, const char *sig);

#define JNI_CALL_TYPE_NATIVE(_jtype, _func, ...)                                                       \
    ({                                                                                                 \
        so_jref_push_local_frame(0);                                                                   \
        _jtype  _ret = _func(__VA_ARGS__);                                                             \
        jobject _ref = so_jref_pop_local_frame(_Generic((_jtype)0, jobject: (_ret), default: (NULL))); \
        _Generic((_jtype)0, jobject: (_ref), default: (_ret));                                         \
    })

#define JNI_CALL_VOID_NATIVE(_func, ...) \
    do {                                 \
        so_jref_push_local_frame(0);     \
        _func(__VA_ARGS__);              \
        so_jref_pop_local_frame(NULL);   \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif
