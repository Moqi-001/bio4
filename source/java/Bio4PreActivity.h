#ifndef BIO4_PRE_ACTIVITY_H
#define BIO4_PRE_ACTIVITY_H

#include <so_java/android/os/Bundle.h>
#include <so_jni/so_jni.h>

void setup_Bio4PreActivity();

JInt     Bio4PreActivity_native_onCreate(JObject ctx);
void     Bio4PreActivity_native_onRestart(void);
void     Bio4PreActivity_native_onStart(void);
void     Bio4PreActivity_native_onResume(void);
JBoolean Bio4PreActivity_native_onSurfaceCreated(JObject gl, JObject config, JInt width, JInt height);
JBoolean Bio4PreActivity_native_onSurfaceChanged(JObject gl, JInt width, JInt height);
JInt     Bio4PreActivity_native_onDrawFrame(JObject gl);
void     Bio4PreActivity_native_onPause(void);
void     Bio4PreActivity_native_onStop(void);
void     Bio4PreActivity_native_onDestroy(void);
void     Bio4PreActivity_native_onConfigurationChanged(JObject a3);
JBoolean Bio4PreActivity_native_onKeyDown(JInt keyCode, JObject event);
JBoolean Bio4PreActivity_native_onKeyUp(JInt keyCode, JObject event);
JBoolean Bio4PreActivity_native_onTouchEvent(JInt a3, JInt a4, JInt a5);
void     Bio4PreActivity_native_setStereoHard(JObject pView, JBoolean isEnable);
void     Bio4PreActivity_native_onTouchBegan(JObject pTouchInfo, JFloat currX, JFloat currY, JFloat prevX, JFloat prevY);
void     Bio4PreActivity_native_onTouchMoved(JObject pTouchInfo, JFloat currX, JFloat currY, JFloat prevX, JFloat prevY);
void     Bio4PreActivity_native_onTouchEnded(JObject pTouchInfo, JFloat currX, JFloat currY, JFloat prevX, JFloat prevY);
void     Bio4PreActivity_native_onTouchCancelled(JObject pTouchInfo, JFloat currX, JFloat currY, JFloat prevX, JFloat prevY);
void     Bio4PreActivity_native_onShake(void);

void Bio4PreActivity_onCreate(Bundle savedInstanceState);
void Bio4PreActivity_onStart(void);
void Bio4PreActivity_onRestart(void);
void Bio4PreActivity_onResume(void);
void Bio4PreActivity_onPause(void);
void Bio4PreActivity_onStop(void);
void Bio4PreActivity_onDestroy(void);
void Bio4PreActivity_setStereoHard(JBoolean isEnable);
void Bio4PreActivity_changeStereoHard(JBoolean isEnable);

#endif
