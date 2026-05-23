#ifndef ANDROID_NATIVEHELPER_H
#define ANDROID_NATIVEHELPER_H

#include <so_jni/jni.h>

void jniThrowException(JNIEnv *env, const char *className, const char *msg);
int  jniRegisterNativeMethods(JNIEnv                *env,
                              const char            *className,
                              const JNINativeMethod *methods,
                              int                    numMethods);
#endif