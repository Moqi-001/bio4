#ifndef SO_JNI_INTERNAL_H
#define SO_JNI_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "so_jni.h"

typedef struct JniObject JniObject;
typedef struct JniClass  JniClass;

typedef struct JniFrame   JniFrame;
typedef struct JniRef     JniRef;
typedef struct jclassNode jclassNode;

struct JniRef {
    jobjectRefType type;   // 引用类型
    jobject        obj;    // JNI 对象
    JniFrame      *frame;  // 所属帧
    JniRef        *prev;
    JniRef        *next;
};

struct JniFrame {
    JniRef   *refs;      // 局部引用链表
    jint      capacity;  // 局部引用容量
    jint      count;     // 当前已用数量
    JniFrame *prev;
};

struct jclassNode {
    jclass     *clazz;
    jclassNode *prev;
    jclassNode *next;
};

// Reference
jint JniAddLocalRef(jobject object);
jint JniPushGlobalFrame(void);
void JniPopGlobalFrame(void);

// jobject
jobject JniRetainObject(jobject object);
int32_t JniReleaseObject(jobject object);
void    JniForceReleaseObject(jobject object);
jobject JniEditObject(jobject object);
jobject JniAttemptEditObject(jobject object);
JniRef *JniGetObjectRef(jobject object);
jint    JniSetObjectRef(jobject object, JniRef *ref);

// jclass
const char    *JniGetClassName(jclass clazz);
jsize          JniGetClassInstanceSize(jclass clazz);
const jsymbol *JniFindMethod(const jclass clazz, const char *name, const char *sig, jboolean is_static);
const jsymbol *JniFindField(const jclass clazz, const char *name, const char *sig, jboolean is_static);
const jsymbol *JniFindNative(const jclass clazz, const char *name, const char *sig);
void           JniClearClasses(void);

#ifdef __cplusplus
}
#endif

#endif