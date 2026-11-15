#include "so_jni_internal.h"

const char *JniGetClassName(jclass clazz) {
    return Class_getName(JniObjectToJava(clazz));
}

jclass JniFindClass(const char *name) {
    return JniNewLocalRefFromJava(JavaFindClass(name));
}

jclass JniGetSuperclass(jclass clazz) {
    return JniNewLocalRefFromJava(Class_getSuper(JniObjectToJava(clazz)));
}

jboolean JniIsAssignableFrom(jclass from, jclass thiz) {
    return Class_isAssignableFrom(JniObjectToJava(thiz), JniObjectToJava(from));
}

jint JniRegisterNatives(jclass clazz, const JNINativeMethod *methods, jint num_methods) {
    // JNINativeMethod 与 JNativeMethod 内存布局一致，这里做 jni → java 的显式桥接
    return Class_registerNatives(JniObjectToJava(clazz), (const JNativeMethod *)methods, num_methods);
}

jint JniUnregisterNatives(jclass clazz) {
    return Class_unregisterNatives(JniObjectToJava(clazz));
}
