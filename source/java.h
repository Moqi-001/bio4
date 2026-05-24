#ifndef SOLOADED_JAVA_H
#define SOLOADED_JAVA_H

#include <so_jni/so_jni.h>

#include "java/Bio4PreActivity.h"

// 不带Call版需手动管理，不入帧，创建的资源不会自动释放（用于静态）
jarray JniNewArray(jsize length, int type);

#define JniGetStringUTFChars(str) so_jstring_get_utf8_chars(so_jstring_from_object(str));
#define JniGetArrayElems(arr)     so_jarray_get_elems(so_jarray_from_object(arr))

// 带Call版会调用jni_env，创建的资源会注册到帧，会在native函数执行完后自动释放（用于返回或局部）
#define JniCallNewStringUTF(utf)               _jni_env->NewStringUTF(&_jni_env, utf)
#define JniCallGetStringUTFChars(str, isCopy)  _jni_env->GetStringUTFChars(&_jni_env, str, isCopy)
#define JniCallReleaseStringUTFChars(str, utf) _jni_env->ReleaseStringUTFChars(&_jni_env, str, utf)
#define JniCallNewByteArray(len)               _jni_env->NewByteArray(&_jni_env, len)

#define JniCallNewLocalRef(obj)     _jni_env->NewLocalRef(&_jni_env, obj)
#define JniCallDeleteLocalRef(obj)  _jni_env->DeleteLocalRef(&_jni_env, obj)
#define JniCallNewGlobalRef(obj)    _jni_env->NewGlobalRef(&_jni_env, obj)
#define JniCallDeleteGlobalRef(obj) _jni_env->DeleteGlobalRef(&_jni_env, obj)

int soloader_java_init();

#endif