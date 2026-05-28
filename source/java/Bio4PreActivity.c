#include <stdbool.h>
#include <stddef.h>
#include <sys/unistd.h>

#include <psp2/kernel/threadmgr.h>
#include <so_common/so_logger.h>
#include <so_jni/so_jni.h>
#include <vitaGL.h>

#include "Bio4PreActivity.h"

#include "android/Activity.h"
#include "android/GLSurfaceView.h"
#include "lang/System.h"

#include "config.h"
#include "controls.h"
#include "patch.h"
#include "utils/utils.h"
#include "video_player.h"

typedef struct LGUArmManager LGUArmManager;

typedef struct Bio4PreActivity {
    LGUArmManager  *m_ArmMgr;
    AGLSurfaceView *m_pView;              // = NULL
    jboolean        m_isLaunchHeapCheck;  // = true,
    jlong           m_nPrevTime;          // = 0,
    jboolean        m_isAppAlive;         // = false,
    jboolean        m_isStereoFlg;        // = true,
    jlong           m_isDebugCnt;         // = 0,
    char           *m_ArmAID;             // = "AQ00114230",
} Bio4PreActivity;

jclass                  Bio4PreActivity_jclass  = NULL;
jobject                 Bio4PreActivity_jobject = NULL;
static Bio4PreActivity *sThis;

static const long LAUNCH_HEAP  = 16777216;
static const int  LGUARM_CHECK = 1;

static jint (*native_onCreate)(JNIEnv *env, jobject thiz, jobject ctx);
static void (*native_onRestart)(JNIEnv *env, jobject thiz);
static void (*native_onStart)(JNIEnv *env, jobject thiz);
static void (*native_onResume)(JNIEnv *env, jobject thiz);
static jboolean (*native_onSurfaceCreated)(JNIEnv *env, jobject thiz, jobject gl, jobject config, jint width, jint height);
static jboolean (*native_onSurfaceChanged)(JNIEnv *env, jobject thiz, jobject gl, jint width, jint height);
static jint (*native_onDrawFrame)(JNIEnv *env, jobject thiz, jobject gl);
static void (*native_onPause)(JNIEnv *env, jobject thiz);
static void (*native_onStop)(JNIEnv *env, jobject thiz);
static void (*native_onDestroy)(JNIEnv *env, jobject thiz);
static void (*native_onConfigurationChanged)(JNIEnv *env, jobject thiz, jobject a3);
static jboolean (*native_onKeyDown)(JNIEnv *env, jobject thiz, jint keyCode, jobject event);
static jboolean (*native_onKeyUp)(JNIEnv *env, jobject thiz, jint keyCode, jobject event);
static jboolean (*native_onTouchEvent)(JNIEnv *env, jobject thiz, jint a3, jint a4, jint a5);
static void (*native_setStereoHard)(JNIEnv *env, jobject thiz, jobject pView, jboolean isEnable);
static void (*native_onTouchBegan)(JNIEnv *env, jobject thiz, jobject pTouchInfo, jfloat currX, jfloat currY, jfloat prevX, jfloat prevY);
static void (*native_onTouchMoved)(JNIEnv *env, jobject thiz, jobject pTouchInfo, jfloat currX, jfloat currY, jfloat prevX, jfloat prevY);
static void (*native_onTouchEnded)(JNIEnv *env, jobject thiz, jobject pTouchInfo, jfloat currX, jfloat currY, jfloat prevX, jfloat prevY);
static void (*native_onTouchCancelled)(JNIEnv *env, jobject thiz, jobject pTouchInfo, jfloat currX, jfloat currY, jfloat prevX, jfloat prevY);
static void (*native_onShake)(JNIEnv *env, jobject thiz);

jint Bio4PreActivity_native_onCreate(JNIEnv *env, jobject thiz, jobject ctx) {
    return JNI_CALL_TYPE_NATIVE(jint, native_onCreate, env, thiz, ctx);
}
void Bio4PreActivity_native_onRestart(JNIEnv *env, jobject thiz) {
    JNI_CALL_VOID_NATIVE(native_onRestart, env, thiz);
}
void Bio4PreActivity_native_onStart(JNIEnv *env, jobject thiz) {
    JNI_CALL_VOID_NATIVE(native_onStart, env, thiz);
}
void Bio4PreActivity_native_onResume(JNIEnv *env, jobject thiz) {
    JNI_CALL_VOID_NATIVE(native_onResume, env, thiz);
}
jboolean Bio4PreActivity_native_onSurfaceCreated(JNIEnv *env, jobject thiz, jobject gl, jobject config, jint width, jint height) {
    return JNI_CALL_TYPE_NATIVE(jboolean, native_onSurfaceCreated, env, thiz, gl, config, width, height);
}
jboolean Bio4PreActivity_native_onSurfaceChanged(JNIEnv *env, jobject thiz, jobject gl, jint width, jint height) {
    return JNI_CALL_TYPE_NATIVE(jboolean, native_onSurfaceChanged, env, thiz, gl, width, height);
}
jint Bio4PreActivity_native_onDrawFrame(JNIEnv *env, jobject thiz, jobject gl) {
    return JNI_CALL_TYPE_NATIVE(jint, native_onDrawFrame, env, thiz, gl);
}
void Bio4PreActivity_native_onPause(JNIEnv *env, jobject thiz) {
    JNI_CALL_VOID_NATIVE(native_onPause, env, thiz);
}
void Bio4PreActivity_native_onStop(JNIEnv *env, jobject thiz) {
    JNI_CALL_VOID_NATIVE(native_onStop, env, thiz);
}
void Bio4PreActivity_native_onDestroy(JNIEnv *env, jobject thiz) {
    JNI_CALL_VOID_NATIVE(native_onDestroy, env, thiz);
}
void Bio4PreActivity_native_onConfigurationChanged(JNIEnv *env, jobject thiz, jobject a3) {
    JNI_CALL_VOID_NATIVE(native_onConfigurationChanged, env, thiz, a3);
}
jboolean Bio4PreActivity_native_onKeyDown(JNIEnv *env, jobject thiz, jint keyCode, jobject event) {
    return JNI_CALL_TYPE_NATIVE(jboolean, native_onKeyDown, env, thiz, keyCode, event);
}
jboolean Bio4PreActivity_native_onKeyUp(JNIEnv *env, jobject thiz, jint keyCode, jobject event) {
    return JNI_CALL_TYPE_NATIVE(jboolean, native_onKeyUp, env, thiz, keyCode, event);
}
jboolean Bio4PreActivity_native_onTouchEvent(JNIEnv *env, jobject thiz, jint a3, jint a4, jint a5) {
    return JNI_CALL_TYPE_NATIVE(jboolean, native_onTouchEvent, env, thiz, a3, a4, a5);
}
void Bio4PreActivity_native_setStereoHard(JNIEnv *env, jobject thiz, jobject pView, jboolean isEnable) {
    JNI_CALL_VOID_NATIVE(native_setStereoHard, env, thiz, pView, isEnable);
}
void Bio4PreActivity_native_onTouchBegan(JNIEnv *env, jobject thiz, jobject pTouchInfo, jfloat currX, jfloat currY, jfloat prevX, jfloat prevY) {
    JNI_CALL_VOID_NATIVE(native_onTouchBegan, env, thiz, pTouchInfo, currX, currY, prevX, prevY);
}
void Bio4PreActivity_native_onTouchMoved(JNIEnv *env, jobject thiz, jobject pTouchInfo, jfloat currX, jfloat currY, jfloat prevX, jfloat prevY) {
    JNI_CALL_VOID_NATIVE(native_onTouchMoved, env, thiz, pTouchInfo, currX, currY, prevX, prevY);
}
void Bio4PreActivity_native_onTouchEnded(JNIEnv *env, jobject thiz, jobject pTouchInfo, jfloat currX, jfloat currY, jfloat prevX, jfloat prevY) {
    JNI_CALL_VOID_NATIVE(native_onTouchEnded, env, thiz, pTouchInfo, currX, currY, prevX, prevY);
}
void Bio4PreActivity_native_onTouchCancelled(JNIEnv *env, jobject thiz, jobject pTouchInfo, jfloat currX, jfloat currY, jfloat prevX, jfloat prevY) {
    JNI_CALL_VOID_NATIVE(native_onTouchCancelled, env, thiz, pTouchInfo, currX, currY, prevX, prevY);
}
void Bio4PreActivity_native_onShake(JNIEnv *env, jobject thiz) {
    JNI_CALL_VOID_NATIVE(native_onShake, env, thiz);
}

void Bio4PreActivity_onCreate(ABundle *savedInstanceState) {
    sThis->m_ArmMgr            = NULL;
    sThis->m_pView             = NULL;
    sThis->m_isLaunchHeapCheck = true;
    sThis->m_nPrevTime         = JSystem_currentTimeMillis();
    sThis->m_isAppAlive        = false;
    sThis->m_isStereoFlg       = true;
    sThis->m_isDebugCnt        = 0;
    sThis->m_ArmAID            = "AQ00114230";

    so_logi("Main onCreate");
    Bio4PreActivity_native_onCreate(&_jni_env, Bio4PreActivity_jobject, NULL);
}

static void mainLoop() {
    while (sThis->m_isAppAlive) {
        jlong nCurrTime = JSystem_currentTimeMillis();
        jlong nDiffTime = nCurrTime - sThis->m_nPrevTime;
        if (nDiffTime < 44) {
            sceKernelDelayThread((44 - nDiffTime) * 1000);
        }
        sThis->m_nPrevTime = JSystem_currentTimeMillis();

        controls_poll();
        jint res = Bio4PreActivity_native_onDrawFrame(&_jni_env, Bio4PreActivity_jobject, NULL);
        so_logd("[Bio4PreActivity] native_onDrawFrame: %d, Main_nState=%d", res, *Main_nState);

        vglSwapBuffers(GL_FALSE);
        if (res == 1) {
            Bio4PreActivity_native_setStereoHard(&_jni_env, Bio4PreActivity_jobject, sThis->m_pView, sThis->m_isStereoFlg);
        }
    }
}

void Bio4PreActivity_onStart() {
    so_logi("Main onStart");
    Bio4PreActivity_native_onSurfaceCreated(&_jni_env, Bio4PreActivity_jobject, NULL, NULL, SCREEN_W, SCREEN_H);
    Bio4PreActivity_native_onSurfaceChanged(&_jni_env, Bio4PreActivity_jobject, NULL, SCREEN_W, SCREEN_H);
    sThis->m_isAppAlive = 1;
    mainLoop();
}

void Bio4PreActivity_onRestart() {
    so_logi("Main onRestart");
}

void Bio4PreActivity_onResume() {
    so_logi("Main onResume");
    Bio4PreActivity_native_onResume(&_jni_env, Bio4PreActivity_jobject);
}

void Bio4PreActivity_onPause() {
    so_logi("Main onPause");
    Bio4PreActivity_native_onPause(&_jni_env, Bio4PreActivity_jobject);
}

void Bio4PreActivity_onStop() {
    so_logi("Main onStop");
    Bio4PreActivity_setStereoHard(false);
    Bio4PreActivity_native_onStop(&_jni_env, Bio4PreActivity_jobject);
}

void Bio4PreActivity_onDestroy() {
    so_logi("Main onDestroy");
    Bio4PreActivity_native_onDestroy(&_jni_env, Bio4PreActivity_jobject);
    sThis->m_isAppAlive = false;
}

void Bio4PreActivity_setStereoHard(const jboolean isEnable) {
    Bio4PreActivity_native_setStereoHard(&_jni_env, Bio4PreActivity_jobject, sThis->m_pView, isEnable);
}

void Bio4PreActivity_changeStereoHard(jboolean isEnable) {
    if (isEnable != sThis->m_isStereoFlg) {
        sThis->m_isStereoFlg = isEnable;
        Bio4PreActivity_setStereoHard(isEnable);
    }
}

static jsymbol class_symbols[] = {
    // native 实例方法
    JNI_SYMBOL_NATIVE("native_onCreate", "(Ljava/lang/Object;)I", &native_onCreate),
    JNI_SYMBOL_NATIVE("native_onRestart", "()V", &native_onRestart),
    JNI_SYMBOL_NATIVE("native_onStart", "()V", &native_onStart),
    JNI_SYMBOL_NATIVE("native_onResume", "()V", &native_onResume),
    JNI_SYMBOL_NATIVE("native_onSurfaceCreated", "(Ljava/lang/Object;Ljava/lang/Object;II)Z", &native_onSurfaceCreated),
    JNI_SYMBOL_NATIVE("native_onSurfaceChanged", "(Ljava/lang/Object;II)Z", &native_onSurfaceChanged),
    JNI_SYMBOL_NATIVE("native_onDrawFrame", "(Ljava/lang/Object;)I", &native_onDrawFrame),
    JNI_SYMBOL_NATIVE("native_onPause", "()V", &native_onPause),
    JNI_SYMBOL_NATIVE("native_onStop", "()V", &native_onStop),
    JNI_SYMBOL_NATIVE("native_onDestroy", "()V", &native_onDestroy),
    JNI_SYMBOL_NATIVE("native_onConfigurationChanged", "(Ljava/lang/Object;)V", &native_onConfigurationChanged),
    JNI_SYMBOL_NATIVE("native_onKeyDown", "(ILjava/lang/Object;)Z", &native_onKeyDown),
    JNI_SYMBOL_NATIVE("native_onKeyUp", "(ILjava/lang/Object;)Z", &native_onKeyUp),
    JNI_SYMBOL_NATIVE("native_onTouchEvent", "(III)Z", &native_onTouchEvent),
    JNI_SYMBOL_NATIVE("native_setStereoHard", "(Ljava/lang/Object;Z)V", &native_setStereoHard),
    JNI_SYMBOL_NATIVE("native_onTouchBegan", "(Ljava/lang/Object;FFFF)V", &native_onTouchBegan),
    JNI_SYMBOL_NATIVE("native_onTouchMoved", "(Ljava/lang/Object;FFFF)V", &native_onTouchMoved),
    JNI_SYMBOL_NATIVE("native_onTouchEnded", "(Ljava/lang/Object;FFFF)V", &native_onTouchEnded),
    JNI_SYMBOL_NATIVE("native_onTouchCancelled", "(Ljava/lang/Object;FFFF)V", &native_onTouchCancelled),
    JNI_SYMBOL_NATIVE("native_onShake", "()V", &native_onShake),

    // 静态变量
    JNI_SYMBOL_STATIC_FIELD("LAUNCH_HEAP", "I", &LAUNCH_HEAP),
    JNI_SYMBOL_STATIC_FIELD("LGUARM_CHECK", "I", &LGUARM_CHECK),
};

static jclassMeta class_meta = {
    .name          = "jp/co/capcom/android/bio4_LGUplus0119/Bio4PreActivity",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = sizeof(Bio4PreActivity),
};

void Bio4PreActivity_class_init() {
    // 创建 jclass
    Bio4PreActivity_jclass = JniCreateClass(&class_meta);
    // 创建 jobject
    Bio4PreActivity_jobject = JniNewGlobalRefTake(JniAllocObject(Bio4PreActivity_jclass));
    // 获取实例
    sThis = (Bio4PreActivity *)JniGetObjectInstance(Bio4PreActivity_jobject);
}
