#ifndef SO_JNI_INTERNAL_H
#define SO_JNI_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "so_jni.h"

typedef enum {
    JNI_TYPE_UNKNOWN = 0,
    JNI_TYPE_VOID    = 1 << 0,
    JNI_TYPE_BOOLEAN = 1 << 1,
    JNI_TYPE_BYTE    = 1 << 2,
    JNI_TYPE_CHAR    = 1 << 3,
    JNI_TYPE_SHORT   = 1 << 4,
    JNI_TYPE_INT     = 1 << 5,
    JNI_TYPE_LONG    = 1 << 6,
    JNI_TYPE_FLOAT   = 1 << 7,
    JNI_TYPE_DOUBLE  = 1 << 8,
    JNI_TYPE_OBJECT  = 1 << 9,
    JNI_TYPE_ARRAY   = 1 << 10,
} JniType;

#define JNI_TYPE_IS_OBJECT(_t) (((_t) & (JNI_TYPE_OBJECT | JNI_TYPE_ARRAY)) != 0)
#define JNI_TYPE_IS_ARRAY(_t)  (((_t) & JNI_TYPE_ARRAY) != 0)
#define JNI_TYPE_IS_FLOAT(_t)  (((_t) & (JNI_TYPE_FLOAT | JNI_TYPE_DOUBLE)) != 0)
#define JNI_TYPE_IS_WIDE(_t)   (((_t) & (JNI_TYPE_LONG | JNI_TYPE_DOUBLE)) != 0)

typedef struct JniArgParam {
    JniType type;      // 参数类型
    jint    stackOff;  // 栈偏移
} JniArgParam;

typedef struct JniMethod {
    char        *name;        // 方法名
    char        *signature;   // 签名
    void        *fnPtr;       // 函数指针
    jboolean     isStatic;    // 是否静态函数
    JniType      retType;     // 返回类型
    jsize        argCount;    // 参数数量
    JniArgParam *argParams;   // 参数信息列表
    jsize        stackBytes;  // 参数占用的栈大小
} JniMethod;

// Reference
jint JniInitLocalRefContext(void);
void JniFiniLocalRefContext(void);
jint JniInitGlobalRefContext(void);
void JniFiniGlobalRefContext(void);
jint JniInitWeakGlobalRefContext(void);
void JniFiniWeakGlobalRefContext(void);

// Class
const char *JniGetClassName(jclass clazz);

// Method
jint JniCallCommonMethod(void *result, jobject object, jmethodID methodID, ...);
jint JniCallCommonMethodV(void *result, jobject object, jmethodID methodID, va_list args);
jint JniCallCommonMethodA(void *result, jobject object, jmethodID methodID, const jvalue *args);
jint JniCallNonvirtualCommonMethod(void *result, jobject object, jclass clazz, jmethodID methodID, ...);
jint JniCallNonvirtualCommonMethodV(void *result, jobject object, jclass clazz, jmethodID methodID, va_list args);
jint JniCallNonvirtualCommonMethodA(void *result, jobject object, jclass clazz, jmethodID methodID, const jvalue *args);

#ifdef __cplusplus
}
#endif

#endif