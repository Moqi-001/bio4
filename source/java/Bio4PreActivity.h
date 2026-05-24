#ifndef BIO4_PRE_ACTIVITY_H
#define BIO4_PRE_ACTIVITY_H

#include <stdbool.h>

#include <so_jni/so_jni.h>

#include "android/Bundle.h"

extern jclass  Bio4PreActivity_jclass;
extern jobject Bio4PreActivity_jobject;

jint     Bio4PreActivity_native_onCreate(JNIEnv *env, jobject thiz, jobject ctx);
void     Bio4PreActivity_native_onRestart(JNIEnv *env, jobject thiz);
void     Bio4PreActivity_native_onStart(JNIEnv *env, jobject thiz);
void     Bio4PreActivity_native_onResume(JNIEnv *env, jobject thiz);
jboolean Bio4PreActivity_native_onSurfaceCreated(JNIEnv *env, jobject thiz, jobject gl, jobject config, jint width, jint height);
jboolean Bio4PreActivity_native_onSurfaceChanged(JNIEnv *env, jobject thiz, jobject gl, jint width, jint height);
jint     Bio4PreActivity_native_onDrawFrame(JNIEnv *env, jobject thiz, jobject gl);
void     Bio4PreActivity_native_onPause(JNIEnv *env, jobject thiz);
void     Bio4PreActivity_native_onStop(JNIEnv *env, jobject thiz);
void     Bio4PreActivity_native_onDestroy(JNIEnv *env, jobject thiz);
void     Bio4PreActivity_native_onConfigurationChanged(JNIEnv *env, jobject thiz, jobject a3);
jboolean Bio4PreActivity_native_onKeyDown(JNIEnv *env, jobject thiz, jint keyCode, jobject event);
jboolean Bio4PreActivity_native_onKeyUp(JNIEnv *env, jobject thiz, jint keyCode, jobject event);
jboolean Bio4PreActivity_native_onTouchEvent(JNIEnv *env, jobject thiz, jint a3, jint a4, jint a5);
void     Bio4PreActivity_native_setStereoHard(JNIEnv *env, jobject thiz, jobject pView, jboolean isEnable);
void     Bio4PreActivity_native_onTouchBegan(JNIEnv *env, jobject thiz, jobject pTouchInfo, jfloat currX, jfloat currY, jfloat prevX, jfloat prevY);
void     Bio4PreActivity_native_onTouchMoved(JNIEnv *env, jobject thiz, jobject pTouchInfo, jfloat currX, jfloat currY, jfloat prevX, jfloat prevY);
void     Bio4PreActivity_native_onTouchEnded(JNIEnv *env, jobject thiz, jobject pTouchInfo, jfloat currX, jfloat currY, jfloat prevX, jfloat prevY);
void     Bio4PreActivity_native_onTouchCancelled(JNIEnv *env, jobject thiz, jobject pTouchInfo, jfloat currX, jfloat currY, jfloat prevX, jfloat prevY);
void     Bio4PreActivity_native_onShake(JNIEnv *env, jobject thiz);

void Bio4PreActivity_onCreate(ABundle *savedInstanceState);
void Bio4PreActivity_onStart();
void Bio4PreActivity_onResume();
void Bio4PreActivity_onPause();
void Bio4PreActivity_onStop();
void Bio4PreActivity_onDestroy();
void Bio4PreActivity_setStereoHard(const bool isEnable);

int Bio4PreActivity_init();

#endif
