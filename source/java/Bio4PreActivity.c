#include <stdbool.h>
#include <stddef.h>

#include <psp2/kernel/threadmgr.h>
#include <so_common/so_logger.h>
#include <so_java/android/opengl/GLSurfaceView.h>
#include <so_java/android/os/Bundle.h>
#include <so_java/java/lang/System.h>
#include <so_jni/so_jni.h>
#include <vitaGL.h>

#include "Bio4PreActivity.h"

#include "config.h"
#include "controls.h"

typedef struct LGUArmManager *LGUArmManager;

struct _Bio4PreActivity {
    LGUArmManager m_ArmMgr;
    GLSurfaceView m_pView;              // = NULL
    JBoolean      m_isLaunchHeapCheck;  // = true,
    JLong         m_nPrevTime;          // = 0,
    JBoolean      m_isAppAlive;         // = false,
    JBoolean      m_isStereoFlg;        // = true,
    JLong         m_isDebugCnt;         // = 0,
    JString      *m_ArmAID;             // = "AQ00114230",
};
typedef struct _Bio4PreActivity *Bio4PreActivity;

static JClass          sClass    = NULL;
static Bio4PreActivity sInstance = NULL;

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

jint Bio4PreActivity_native_onCreate(JObject ctx) {
    jint ret;
    ENTER_JNI_CALL(0)
    ret = native_onCreate(&_jni_env, JniNewLocalRefFromJava(sInstance), NULL);
    EXIT_JNI_CALL(NULL)
    return ret;
}

void Bio4PreActivity_native_onRestart(void) {
    ENTER_JNI_CALL(0)
    native_onRestart(&_jni_env, JniNewLocalRefFromJava(sInstance));
    EXIT_JNI_CALL(NULL)
}

void Bio4PreActivity_native_onStart(void) {
    ENTER_JNI_CALL(0)
    native_onStart(&_jni_env, JniNewLocalRefFromJava(sInstance));
    EXIT_JNI_CALL(NULL)
}

void Bio4PreActivity_native_onResume(void) {
    ENTER_JNI_CALL(0)
    native_onResume(&_jni_env, JniNewLocalRefFromJava(sInstance));
    EXIT_JNI_CALL(NULL)
}

jboolean Bio4PreActivity_native_onSurfaceCreated(jobject gl, jobject config, jint width, jint height) {
    jboolean ret;
    ENTER_JNI_CALL(0)
    ret = native_onSurfaceCreated(&_jni_env, JniNewLocalRefFromJava(sInstance), gl, config, width, height);
    EXIT_JNI_CALL(NULL)
    return ret;
}

jboolean Bio4PreActivity_native_onSurfaceChanged(jobject gl, jint width, jint height) {
    jboolean ret;
    ENTER_JNI_CALL(0)
    ret = native_onSurfaceChanged(&_jni_env, JniNewLocalRefFromJava(sInstance), gl, width, height);
    EXIT_JNI_CALL(NULL)
    return ret;
}

jint Bio4PreActivity_native_onDrawFrame(jobject gl) {
    jint ret;
    ENTER_JNI_CALL(0)
    ret = native_onDrawFrame(&_jni_env, JniNewLocalRefFromJava(sInstance), gl);
    EXIT_JNI_CALL(NULL)
    return ret;
}

void Bio4PreActivity_native_onPause(void) {
    ENTER_JNI_CALL(0)
    native_onPause(&_jni_env, JniNewLocalRefFromJava(sInstance));
    EXIT_JNI_CALL(NULL)
}

void Bio4PreActivity_native_onStop(void) {
    ENTER_JNI_CALL(0)
    native_onStop(&_jni_env, JniNewLocalRefFromJava(sInstance));
    EXIT_JNI_CALL(NULL)
}

void Bio4PreActivity_native_onDestroy(void) {
    ENTER_JNI_CALL(0)
    native_onDestroy(&_jni_env, JniNewLocalRefFromJava(sInstance));
    EXIT_JNI_CALL(NULL)
}

void Bio4PreActivity_native_onConfigurationChanged(jobject a3) {
    ENTER_JNI_CALL(0)
    native_onConfigurationChanged(&_jni_env, JniNewLocalRefFromJava(sInstance), a3);
    EXIT_JNI_CALL(NULL)
}

jboolean Bio4PreActivity_native_onKeyDown(jint keyCode, jobject event) {
    jboolean ret;
    ENTER_JNI_CALL(0)
    ret = native_onKeyDown(&_jni_env, JniNewLocalRefFromJava(sInstance), keyCode, event);
    EXIT_JNI_CALL(NULL)
    return ret;
}

jboolean Bio4PreActivity_native_onKeyUp(jint keyCode, jobject event) {
    jboolean ret;
    ENTER_JNI_CALL(0)
    ret = native_onKeyUp(&_jni_env, JniNewLocalRefFromJava(sInstance), keyCode, event);
    EXIT_JNI_CALL(NULL)
    return ret;
}

jboolean Bio4PreActivity_native_onTouchEvent(jint a3, jint a4, jint a5) {
    jboolean ret;
    ENTER_JNI_CALL(0)
    ret = native_onTouchEvent(&_jni_env, JniNewLocalRefFromJava(sInstance), a3, a4, a5);
    EXIT_JNI_CALL(NULL)
    return ret;
}

void Bio4PreActivity_native_setStereoHard(jobject pView, jboolean isEnable) {
    ENTER_JNI_CALL(0)
    native_setStereoHard(&_jni_env, JniNewLocalRefFromJava(sInstance), pView, isEnable);
    EXIT_JNI_CALL(NULL)
}

void Bio4PreActivity_native_onTouchBegan(jobject pTouchInfo, jfloat currX, jfloat currY, jfloat prevX, jfloat prevY) {
    ENTER_JNI_CALL(0)
    native_onTouchBegan(&_jni_env, JniNewLocalRefFromJava(sInstance), pTouchInfo, currX, currY, prevX, prevY);
    EXIT_JNI_CALL(NULL)
}

void Bio4PreActivity_native_onTouchMoved(jobject pTouchInfo, jfloat currX, jfloat currY, jfloat prevX, jfloat prevY) {
    ENTER_JNI_CALL(0)
    native_onTouchMoved(&_jni_env, JniNewLocalRefFromJava(sInstance), pTouchInfo, currX, currY, prevX, prevY);
    EXIT_JNI_CALL(NULL)
}

void Bio4PreActivity_native_onTouchEnded(jobject pTouchInfo, jfloat currX, jfloat currY, jfloat prevX, jfloat prevY) {
    ENTER_JNI_CALL(0)
    native_onTouchEnded(&_jni_env, JniNewLocalRefFromJava(sInstance), pTouchInfo, currX, currY, prevX, prevY);
    EXIT_JNI_CALL(NULL)
}

void Bio4PreActivity_native_onTouchCancelled(jobject pTouchInfo, jfloat currX, jfloat currY, jfloat prevX, jfloat prevY) {
    ENTER_JNI_CALL(0)
    native_onTouchCancelled(&_jni_env, JniNewLocalRefFromJava(sInstance), pTouchInfo, currX, currY, prevX, prevY);
    EXIT_JNI_CALL(NULL)
}

void Bio4PreActivity_native_onShake(void) {
    ENTER_JNI_CALL(0)
    native_onShake(&_jni_env, JniNewLocalRefFromJava(sInstance));
    EXIT_JNI_CALL(NULL)
}

void Bio4PreActivity_onCreate(Bundle savedInstanceState) {
    sInstance->m_ArmMgr            = NULL;
    sInstance->m_pView             = NULL;
    sInstance->m_isLaunchHeapCheck = true;
    sInstance->m_nPrevTime         = currentTimeMillis();
    sInstance->m_isAppAlive        = false;
    sInstance->m_isStereoFlg       = true;
    sInstance->m_isDebugCnt        = 0;
    sInstance->m_ArmAID            = String_newUTF("AQ00114230");

    so_logi("[Bio4PreActivity] onCreate");
    Bio4PreActivity_native_onCreate(NULL);
}

static void mainLoop() {
    while (sInstance->m_isAppAlive) {
        JLong nCurrTime = currentTimeMillis();
        JLong nDiffTime = nCurrTime - sInstance->m_nPrevTime;
        if (nDiffTime < 44) {
            sceKernelDelayThread((44 - nDiffTime) * 1000);
        }
        sInstance->m_nPrevTime = currentTimeMillis();

        controls_poll();
        JInt res = Bio4PreActivity_native_onDrawFrame(NULL);
        if (res == 1) {
            Bio4PreActivity_native_setStereoHard(sInstance->m_pView, sInstance->m_isStereoFlg);
        }
        vglSwapBuffers(GL_FALSE);
    }
}

void Bio4PreActivity_onStart() {
    so_logi("[Bio4PreActivity] onStart");
    Bio4PreActivity_native_onSurfaceCreated(NULL, NULL, SCREEN_W, SCREEN_H);
    Bio4PreActivity_native_onSurfaceChanged(NULL, SCREEN_W, SCREEN_H);
    sInstance->m_isAppAlive = JAVA_TRUE;
    mainLoop();
}

void Bio4PreActivity_onRestart() {
    so_logi("[Bio4PreActivity] onRestart");
}

void Bio4PreActivity_onResume() {
    so_logi("[Bio4PreActivity] onResume");
    Bio4PreActivity_native_onResume();
}

void Bio4PreActivity_onPause() {
    so_logi("[Bio4PreActivity] onPause");
    Bio4PreActivity_native_onPause();
}

void Bio4PreActivity_onStop() {
    so_logi("[Bio4PreActivity] onStop");
    Bio4PreActivity_setStereoHard(JAVA_FALSE);
    Bio4PreActivity_native_onStop();
}

void Bio4PreActivity_onDestroy() {
    so_logi("[Bio4PreActivity] onDestroy");
    Bio4PreActivity_native_onDestroy();
    sInstance->m_isAppAlive = JAVA_FALSE;
}

void Bio4PreActivity_setStereoHard(JBoolean isEnable) {
    so_logd("[Bio4PreActivity] setStereoHard(%u)", isEnable);
    Bio4PreActivity_native_setStereoHard(sInstance->m_pView, isEnable);
}

void Bio4PreActivity_changeStereoHard(JBoolean isEnable) {
    so_logd("[Bio4PreActivity] changeStereoHard(%u)", isEnable);
    if (isEnable != sInstance->m_isStereoFlg) {
        sInstance->m_isStereoFlg = isEnable;
        Bio4PreActivity_setStereoHard(isEnable);
    }
}

static JSymbol class_symbols[] = {
    // native 实例方法
    JAVA_SYMBOL_NATIVE("native_onCreate", "(Ljava/lang/Object;)I", &native_onCreate),
    JAVA_SYMBOL_NATIVE("native_onRestart", "()V", &native_onRestart),
    JAVA_SYMBOL_NATIVE("native_onStart", "()V", &native_onStart),
    JAVA_SYMBOL_NATIVE("native_onResume", "()V", &native_onResume),
    JAVA_SYMBOL_NATIVE("native_onSurfaceCreated", "(Ljava/lang/Object;Ljava/lang/Object;II)Z", &native_onSurfaceCreated),
    JAVA_SYMBOL_NATIVE("native_onSurfaceChanged", "(Ljava/lang/Object;II)Z", &native_onSurfaceChanged),
    JAVA_SYMBOL_NATIVE("native_onDrawFrame", "(Ljava/lang/Object;)I", &native_onDrawFrame),
    JAVA_SYMBOL_NATIVE("native_onPause", "()V", &native_onPause),
    JAVA_SYMBOL_NATIVE("native_onStop", "()V", &native_onStop),
    JAVA_SYMBOL_NATIVE("native_onDestroy", "()V", &native_onDestroy),
    JAVA_SYMBOL_NATIVE("native_onConfigurationChanged", "(Ljava/lang/Object;)V", &native_onConfigurationChanged),
    JAVA_SYMBOL_NATIVE("native_onKeyDown", "(ILjava/lang/Object;)Z", &native_onKeyDown),
    JAVA_SYMBOL_NATIVE("native_onKeyUp", "(ILjava/lang/Object;)Z", &native_onKeyUp),
    JAVA_SYMBOL_NATIVE("native_onTouchEvent", "(III)Z", &native_onTouchEvent),
    JAVA_SYMBOL_NATIVE("native_setStereoHard", "(Ljava/lang/Object;Z)V", &native_setStereoHard),
    JAVA_SYMBOL_NATIVE("native_onTouchBegan", "(Ljava/lang/Object;FFFF)V", &native_onTouchBegan),
    JAVA_SYMBOL_NATIVE("native_onTouchMoved", "(Ljava/lang/Object;FFFF)V", &native_onTouchMoved),
    JAVA_SYMBOL_NATIVE("native_onTouchEnded", "(Ljava/lang/Object;FFFF)V", &native_onTouchEnded),
    JAVA_SYMBOL_NATIVE("native_onTouchCancelled", "(Ljava/lang/Object;FFFF)V", &native_onTouchCancelled),
    JAVA_SYMBOL_NATIVE("native_onShake", "()V", &native_onShake),

    // 静态变量
    JAVA_SYMBOL_STATIC_FIELD("LAUNCH_HEAP", "I", &LAUNCH_HEAP),
    JAVA_SYMBOL_STATIC_FIELD("LGUARM_CHECK", "I", &LGUARM_CHECK),
};

static JClassMeta class_meta = {
    .name          = "jp/co/capcom/android/bio4_LGUplus0119/Bio4PreActivity",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = sizeof(struct _Bio4PreActivity),
};

void setup_Bio4PreActivity() {
    sClass    = JavaCreateClass(&class_meta);
    sInstance = Object_new(sClass);
}
