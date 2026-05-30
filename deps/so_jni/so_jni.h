#ifndef SO_JNI_H
#define SO_JNI_H

#ifdef __cplusplus
extern "C" {
#endif

#include "jni.h"

typedef struct jargs {
    jboolean is_va;
    union {
        va_list       va;
        const jvalue *vals;
    };
    jsize index;
} jargs;

#define JARG(args, T) _Generic((T)0,                                                                  \
    jboolean: ((args)->is_va ? (jboolean)va_arg((args)->va, jint) : (args)->vals[(args)->index++].z), \
    jbyte: ((args)->is_va ? va_arg((args)->va, jbyte) : (args)->vals[(args)->index++].b),             \
    jchar: ((args)->is_va ? va_arg((args)->va, jchar) : (args)->vals[(args)->index++].c),             \
    jshort: ((args)->is_va ? va_arg((args)->va, jshort) : (args)->vals[(args)->index++].s),           \
    jint: ((args)->is_va ? va_arg((args)->va, jint) : (args)->vals[(args)->index++].i),               \
    jlong: ((args)->is_va ? va_arg((args)->va, jlong) : (args)->vals[(args)->index++].j),             \
    jfloat: ((args)->is_va ? (jfloat)va_arg((args)->va, jdouble) : (args)->vals[(args)->index++].f),  \
    jdouble: ((args)->is_va ? va_arg((args)->va, jdouble) : (args)->vals[(args)->index++].d),         \
    jobject: ((args)->is_va ? va_arg((args)->va, jobject) : (args)->vals[(args)->index++].l))

enum {
    JNI_SYMBOL_FLAGS_UNDEF  = 0,       // 未定义
    JNI_SYMBOL_FLAGS_STATIC = 1 << 0,  // static
    JNI_SYMBOL_FLAGS_NATIVE = 1 << 1,  // native（仅方法）
    JNI_SYMBOL_FLAGS_METHOD = 1 << 2,  // 方法
    JNI_SYMBOL_FLAGS_FIELD  = 1 << 3,  // 字段
};

typedef struct jsymbol {
    char     *name;       // 符号名
    char     *signature;  // 符号签名
    uintptr_t addr;       // 符号地址
    uint32_t  flags;      // 符号标志
} jsymbol;

#define JNI_SYMBOL_STATIC_METHOD(name, sig, p_func) \
    {name, sig, (uintptr_t)(p_func), (JNI_SYMBOL_FLAGS_STATIC | JNI_SYMBOL_FLAGS_METHOD)}
#define JNI_SYMBOL_STATIC_FIELD(name, sig, offset) \
    {name, sig, (uintptr_t)(offset), (JNI_SYMBOL_FLAGS_STATIC | JNI_SYMBOL_FLAGS_FIELD)}
#define JNI_SYMBOL_METHOD(name, sig, p_func) \
    {name, sig, (uintptr_t)(p_func), JNI_SYMBOL_FLAGS_METHOD}
#define JNI_SYMBOL_FIELD(name, sig, offset) \
    {name, sig, (uintptr_t)(offset), JNI_SYMBOL_FLAGS_FIELD}
#define JNI_SYMBOL_STATIC_NATIVE(name, sig, pp_func) \
    {name, sig, (uintptr_t)(pp_func), JNI_SYMBOL_FLAGS_STATIC | JNI_SYMBOL_FLAGS_NATIVE | JNI_SYMBOL_FLAGS_METHOD}
#define JNI_SYMBOL_NATIVE(name, sig, pp_func) \
    {name, sig, (uintptr_t)(pp_func), JNI_SYMBOL_FLAGS_NATIVE | JNI_SYMBOL_FLAGS_METHOD}

typedef struct jclassMeta {
    char    *name;           // 类名
    char    *super_name;     // 父类名
    jsymbol *symbols;        // 符号数组
    jsize    num_symbols;    // 符号数量
    jsize    instance_size;  // 实例数据大小
} jclassMeta;

#define JNI_CALL_TYPE_NATIVE(_jtype, _func, ...)                                                \
    ({                                                                                          \
        JniPushLocalFrame(0);                                                                   \
        _jtype  _ret = _func(__VA_ARGS__);                                                      \
        jobject _ref = JniPopLocalFrame(_Generic((_jtype)0, jobject: (_ret), default: (NULL))); \
        _Generic((_jtype)0, jobject: (_ref), default: (_ret));                                  \
    })

#define JNI_CALL_VOID_NATIVE(_func, ...) \
    do {                                 \
        JniPushLocalFrame(0);            \
        _func(__VA_ARGS__);              \
        JniPopLocalFrame(NULL);          \
    } while (0)

extern JavaVM _java_vm;
extern JNIEnv _jni_env;

int  so_jni_init(void);
void so_jni_fini(void);

jclass   JniCreateClass(jclassMeta *meta);
jclass   JniFindClass(const char *name);
jclass   JniGetSuperclass(jclass clazz);
jboolean JniIsAssignableFrom(jclass clazz1, jclass clazz2);
jint     JniRegisterNatives(jclass clazz, const JNINativeMethod *methods, jint num_nethods);
jint     JniUnregisterNatives(jclass clazz);

jobject JniNewLocalRef(jobject object);
void    JniDeleteLocalRef(jobject object);
jobject JniNewGlobalRef(jobject object);
jobject JniNewGlobalRefTake(jobject object);  // 创建新全局引用并释放旧引用
void    JniDeleteGlobalRef(jobject object);
jint    JniPushLocalFrame(jint capacity);
jobject JniPopLocalFrame(jobject result);

void          *JniAllocInstance(jsize size);
void          *JniGetObjectInstance(jobject object);
jobject        JniCreateObject(jclass clazz, void *instance);
jobject        JniAllocObject(jclass clazz);
jobject        JniNewObject(jclass clazz, jmethodID methodID, ...);
jobject        JniNewObjectV(jclass clazz, jmethodID methodID, va_list args);
jobject        JniNewObjectA(jclass clazz, jmethodID methodID, const jvalue *args);
jclass         JniGetObjectClass(jobject object);
jboolean       JniIsSameObject(jobject ref1, jobject ref2);
jboolean       JniIsInstanceOf(jobject obj, jclass clazz);
jobjectRefType JniGetObjectRefType(jobject object);

jstring      JniNewString(const jchar *utf16, jsize len);
jsize        JniGetStringLength(jstring string);
const jchar *JniGetStringChars(jstring string, jboolean *isCopy);
int32_t      JniReleaseStringChars(jstring string, const jchar *utf16);
jint         JniGetStringRegion(jstring string, jsize start, jsize length, jchar *buf);

jstring     JniNewStringUTF(const char *utf8);
jsize       JniGetStringUTFLength(jstring string);
const char *JniGetStringUTFChars(jstring string, jboolean *isCopy);
int32_t     JniReleaseStringUTFChars(jstring string, const char *utf8);
jint        JniGetStringUTFRegion(jstring string, jsize start, jsize length, char *buf);

jlong JniGetArrayLength(jarray array);

jobjectArray JniNewObjectArray(jsize length, jclass elementClass, jobject initialElement);
jobject      JniGetObjectArrayElement(jobjectArray array, jsize index);
void         JniSetObjectArrayElement(jobjectArray array, jsize index, jobject value);

jbooleanArray JniNewBooleanArray(jsize length);
jbyteArray    JniNewByteArray(jsize length);
jcharArray    JniNewCharArray(jsize length);
jshortArray   JniNewShortArray(jsize length);
jintArray     JniNewIntArray(jsize length);
jlongArray    JniNewLongArray(jsize length);
jfloatArray   JniNewFloatArray(jsize length);
jdoubleArray  JniNewDoubleArray(jsize length);

jboolean *JniGetBooleanArrayElements(jbooleanArray array, jboolean *isCopy);
jbyte    *JniGetByteArrayElements(jbyteArray array, jboolean *isCopy);
jchar    *JniGetCharArrayElements(jcharArray array, jboolean *isCopy);
jshort   *JniGetShortArrayElements(jshortArray array, jboolean *isCopy);
jint     *JniGetIntArrayElements(jintArray array, jboolean *isCopy);
jlong    *JniGetLongArrayElements(jlongArray array, jboolean *isCopy);
jfloat   *JniGetFloatArrayElements(jfloatArray array, jboolean *isCopy);
jdouble  *JniGetDoubleArrayElements(jdoubleArray array, jboolean *isCopy);
void      JniReleaseBooleanArrayElements(jbooleanArray array, jboolean *elems, jint mode);
void      JniReleaseByteArrayElements(jbyteArray array, jbyte *elems, jint mode);
void      JniReleaseCharArrayElements(jcharArray array, jchar *elems, jint mode);
void      JniReleaseShortArrayElements(jshortArray array, jshort *elems, jint mode);
void      JniReleaseIntArrayElements(jintArray array, jint *elems, jint mode);
void      JniReleaseLongArrayElements(jlongArray array, jlong *elems, jint mode);
void      JniReleaseFloatArrayElements(jfloatArray array, jfloat *elems, jint mode);
void      JniReleaseDoubleArrayElements(jdoubleArray array, jdouble *elems, jint mode);

void  JniGetBooleanArrayRegion(jbooleanArray array, jsize start, jsize len, jboolean *buf);
void  JniGetByteArrayRegion(jbyteArray array, jsize start, jsize len, jbyte *buf);
void  JniGetCharArrayRegion(jcharArray array, jsize start, jsize len, jchar *buf);
void  JniGetShortArrayRegion(jshortArray array, jsize start, jsize len, jshort *buf);
void  JniGetIntArrayRegion(jintArray array, jsize start, jsize len, jint *buf);
void  JniGetLongArrayRegion(jlongArray array, jsize start, jsize len, jlong *buf);
void  JniGetFloatArrayRegion(jfloatArray array, jsize start, jsize len, jfloat *buf);
void  JniGetDoubleArrayRegion(jdoubleArray array, jsize start, jsize len, jdouble *buf);
void  JniSetBooleanArrayRegion(jbooleanArray array, jsize start, jsize len, const jboolean *buf);
void  JniSetByteArrayRegion(jbyteArray array, jsize start, jsize len, const jbyte *buf);
void  JniSetCharArrayRegion(jcharArray array, jsize start, jsize len, const jchar *buf);
void  JniSetShortArrayRegion(jshortArray array, jsize start, jsize len, const jshort *buf);
void  JniSetIntArrayRegion(jintArray array, jsize start, jsize len, const jint *buf);
void  JniSetLongArrayRegion(jlongArray array, jsize start, jsize len, const jlong *buf);
void  JniSetFloatArrayRegion(jfloatArray array, jsize start, jsize len, const jfloat *buf);
void  JniSetDoubleArrayRegion(jdoubleArray array, jsize start, jsize len, const jdouble *buf);
void *JniGetPrimitiveArrayCritical(jarray array, jboolean *isCopy);
void  JniReleasePrimitiveArrayCritical(jarray array, void *carray, jint mode);

jobject        JniNewDirectByteBuffer(void *address, jlong capacity);
void          *JniGetDirectBufferAddress(jobject buffer);
jlong          JniGetDirectBufferCapacity(jobject buffer);
jobjectRefType JniGetObjectRefType(jobject object);

#ifdef __cplusplus
}
#endif

#endif
