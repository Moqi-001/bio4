#include <so_common/so_logger.h>
#include <so_jni/so_jni.h>

#include "JNIHelp.h"

void jniThrowException(JNIEnv *env, const char *className, const char *msg) {
    so_loge("[JNI] jniThrowException(%p,'%s','%s')", env, className, msg);
}

int jniRegisterNativeMethods(JNIEnv                *env,
                             const char            *className,
                             const JNINativeMethod *methods,
                             int                    numMethods) {
    jclass clazz = JniFindClass(className);
    if (!clazz) {
        so_logd("[JNI] jniRegisterNativeMethods(%p,'%s',%p,%d): class not found!", env, className, methods, numMethods);
        return -1;
    }

    int result = JniRegisterNatives(clazz, methods, numMethods);

    so_logd("[JNI] jniRegisterNativeMethods(%p,'%s',%p,%d): %d", env, className, methods, numMethods, result);
    return result;
}
