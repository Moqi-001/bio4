#include <so_jni/so_jni.h>
#include <so_common/so_logger.h>

// <nativehelper/JNIHelp.h>
/**
 * 抛出指定类型的异常
 * @param env       JNI环境指针
 * @param className Java异常类的全名（如 "java/lang/NullPointerException"）
 * @param msg       异常消息
 */
void jniThrowException(JNIEnv *env, const char *className, const char *msg) {
    so_loge("[JNI] jniThrowException(%p,'%s','%s')", env, className, msg);
}

// <nativehelper/JNINativeMethod.h>
/**
 * 注册本地方法到指定的Java类
 * @param env        JNI环境指针
 * @param className  Java类的全名（如 "com/example/MyClass"）
 * @param methods    JNINativeMethod数组
 * @param numMethods 方法数组的大小
 * @return 成功返回0，失败返回负数
 */
int jniRegisterNativeMethods(JNIEnv                *env,
                             const char            *className,
                             const JNINativeMethod *methods,
                             int                    numMethods) {
    
    jclass *clazz = so_jni_find_class(className);
    if (!clazz) {
        so_logd("[JNI] jniRegisterNativeMethods(%p,'%s',%p,%d): class not found!", env, className, methods, numMethods);
        return -1;
    }

    int result = so_jclass_register_natives(clazz, methods, numMethods);

    so_logd("[JNI] jniRegisterNativeMethods(%p,'%s',%p,%d): %d", env, className, methods, numMethods, result);
    return result;
}
