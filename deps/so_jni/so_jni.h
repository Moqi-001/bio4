#ifndef SO_JNI_H
#define SO_JNI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#include "jni.h"

#include <so_java/java/lang/Array.h>
#include <so_java/java/lang/Class.h>
#include <so_java/java/lang/Object.h>
#include <so_java/java/lang/String.h>
#include <so_java/java/lang/types.h>
#include <so_java/java/nio/Buffer.h>

extern JavaVM _java_vm;
extern JNIEnv _jni_env;

int  so_jni_init(void);
void so_jni_fini(void);

// ============================================
// JNI <=> JAVA
// ============================================
JObject JniObjectToJava(jobject object);
jobject JniNewLocalRefFromJava(JObject obj);
jobject JniNewGlobalRefFromJava(JObject obj);
jweak   JniNewWeakGlobalRefFromJava(JObject obj);

// ============================================
// JNI 标准函数实现
// ============================================
jclass   JniFindClass(const char *name);
jclass   JniGetSuperclass(jclass clazz);
jboolean JniIsAssignableFrom(jclass from, jclass thiz);
jint     JniRegisterNatives(jclass clazz, const JNINativeMethod *methods, jint num_methods);
jint     JniUnregisterNatives(jclass clazz);

jint    JniPushLocalFrame(jint capacity);
jobject JniPopLocalFrame(jobject result);
jobject JniNewLocalRef(jobject object);
void    JniDeleteLocalRef(jobject object);
jobject JniNewGlobalRef(jobject object);
void    JniDeleteGlobalRef(jobject object);
jweak   JniNewWeakGlobalRef(jobject object);
void    JniDeleteWeakGlobalRef(jweak object);

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
void         JniReleaseStringChars(jstring string, const jchar *utf16);
jint         JniGetStringRegion(jstring string, jsize start, jsize length, jchar *buf);

jstring     JniNewStringUTF(const char *utf8);
jsize       JniGetStringUTFLength(jstring string);
const char *JniGetStringUTFChars(jstring string, jboolean *isCopy);
void        JniReleaseStringUTFChars(jstring string, const char *utf8);
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

jmethodID JniGetMethodID(const jclass clazz, const char *name, const char *sig);
jfieldID  JniGetFieldID(const jclass clazz, const char *name, const char *sig);
jmethodID JniGetStaticMethodID(const jclass clazz, const char *name, const char *sig);
jfieldID  JniGetStaticFieldID(const jclass clazz, const char *name, const char *sig);

// ============================================
// JNI native 调用进出宏 (确保每次 native 函数调用后释放其创建的局部引用，防止内存泄漏)
// ============================================
#define ENTER_JNI_CALL(capacity) \
    {                            \
        JniPushLocalFrame(capacity);

#define EXIT_JNI_CALL(result) \
    JniPopLocalFrame(result); \
    }

#ifdef __cplusplus
}
#endif

#endif
