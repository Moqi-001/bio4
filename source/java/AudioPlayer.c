#include <string.h>

#include <so_common/so_logger.h>
#include <so_jni/so_jni.h>

#include "java.h"

typedef struct AudioMediaPlayer AudioMediaPlayer;
typedef struct SoundPoolData    SoundPoolData;
typedef struct SoundPool        SoundPool;

typedef struct AudioPlayer {
    jint               m_nBGMNum;
    jint               m_nSENum;
    AudioMediaPlayer **m_apBGM;
    SoundPoolData    **m_apSE;
    SoundPool         *m_pSoundPool;
} AudioPlayer;

static jclass  AudioPlayer_jclass  = NULL;
static jobject AudioPlayer_jobject = NULL;

static jobject getInstance() {
    jobject result = JniNewLocalRef(AudioPlayer_jobject);
    so_logd("[AudioPlayer] getInstance(): %p", result);
    return result;
}

void AudioPlayer_ctor(AudioPlayer *thiz) {
}

// "init","(II)Z"
jboolean init(AudioPlayer *thiz, jargs *args) {  // (int nBGMNum, int nSENum)
    jint nBGMNum = JARG(args, jint);
    jint nSENum  = JARG(args, jint);

    jboolean result = JNI_TRUE;
    so_logd("[AudioPlayer] init(%d,%d)", nBGMNum, nSENum);
    return result;
}

// "term","(II)Z"
jboolean term(AudioPlayer *thiz, jargs *args) {  // (int nBGMNum, int nSENum)
    jint nBGMNum = JARG(args, jint);
    jint nSENum  = JARG(args, jint);

    jboolean result = JNI_TRUE;
    so_logd("[AudioPlayer] term(%d,%d)", nBGMNum, nSENum);
    return result;
}

// "loadBGM","(ILjava/lang/String;JJZ)Z"
jboolean loadBGM(AudioPlayer *thiz, jargs *args) {  // (int nSlotID, String pFileName, long nOffset, long nLength, boolean isLoop)
    jint        nSlotID   = JARG(args, jint);
    const char *pFileName = JniGetStringUTFChars(JARG(args, jstring), NULL);
    jlong       nOffset   = JARG(args, jlong);
    jlong       nLength   = JARG(args, jlong);
    jboolean    isLoop    = JARG(args, jboolean);

    jboolean result = JNI_TRUE;
    so_logd("[AudioPlayer] loadBGM(%d,'%s',%lld,%lld,%u): %d", nSlotID, pFileName, nOffset, nLength, isLoop, result);
    return result;
}

// "loadSE","(ILjava/lang/String;JJ)Z"
jboolean loadSE(AudioPlayer *thiz, jargs *args) {  // (int nSlotID, String pFileName, long nOffset, long nLength)
    jint        nSlotID   = JARG(args, jint);
    const char *pFileName = JniGetStringUTFChars(JARG(args, jstring), NULL);
    jlong       nOffset   = JARG(args, jlong);
    jlong       nLength   = JARG(args, jlong);

    jboolean result = JNI_TRUE;
    so_logd("[AudioPlayer] loadSE(%d,'%s',%lld,%lld): %d", nSlotID, pFileName, nOffset, nLength, result);

    return result;
}

// "termBGM","(I)V"
void termBGM(AudioPlayer *thiz, jargs *args) {  // (int nSlotID)
    jint nSlotID = JARG(args, jint);
    so_logd("[AudioPlayer] termBGM(%d)", nSlotID);
}

// "termSE","(I)V"
void termSE(AudioPlayer *thiz, jargs *args) {  // (int nSlotID)
    jint nSlotID = JARG(args, jint);
    so_logd("[AudioPlayer] termSE(%d)", nSlotID);
}

// "playBGM","(I)V"
void playBGM(AudioPlayer *thiz, jargs *args) {  // (int nSlotID)
    jint nSlotID = JARG(args, jint);
    so_logd("[AudioPlayer] playBGM(%d)", nSlotID);
}

// "playSE","(IF)V"
void playSE(AudioPlayer *thiz, jargs *args) {  // (int nSlotID, float nVolume)
    jint   nSlotID = JARG(args, jint);
    jfloat nVolume = JARG(args, jfloat);
    so_logd("[AudioPlayer] playSE(%d,%f)", nSlotID, nVolume);
}

// "stopBGM","(I)V"
void stopBGM(AudioPlayer *thiz, jargs *args) {  // (int nSlotID)
    jint nSlotID = JARG(args, jint);
    so_logd("[AudioPlayer] stopBGM(%d)", nSlotID);
}

// "stopSE","(I)V"
void stopSE(AudioPlayer *thiz, jargs *args) {  // (int nSlotID)
    jint nSlotID = JARG(args, jint);
    so_logd("[AudioPlayer] stopSE(%d)", nSlotID);
}

// "pauseBGM","(I)V"
void pauseBGM(AudioPlayer *thiz, jargs *args) {  // (int nSlotID)
    jint nSlotID = JARG(args, jint);
    so_logd("[AudioPlayer] pauseBGM(%d)", nSlotID);
}

// "pauseSE","(I)V"
void pauseSE(AudioPlayer *thiz, jargs *args) {  // (int nSlotID)
    jint nSlotID = JARG(args, jint);
    so_logd("[AudioPlayer] pauseSE(%d)", nSlotID);
}

// "restartBGM","(I)V"
void restartBGM(AudioPlayer *thiz, jargs *args) {  // (int nSlotID)
    jint nSlotID = JARG(args, jint);
    so_logd("[AudioPlayer] restartBGM(%d)", nSlotID);
}

// "restartSE","(I)V"
void restartSE(AudioPlayer *thiz, jargs *args) {  // (int nSlotID)
    jint nSlotID = JARG(args, jint);
    so_logd("[AudioPlayer] restartSE(%d)", nSlotID);
}

// "setVolumeBGM", "(IF)V"
void setVolumeBGM(AudioPlayer *thiz, jargs *args) {  // (int nSlotID, float nVolume)
    jint   nSlotID = JARG(args, jint);
    jfloat nVolume = JARG(args, jfloat);
    so_logd("[AudioPlayer] setVolumeBGM(%d,%f)", nSlotID, nVolume);
}

static jsymbol class_symbols[] = {
    // 静态方法
    JNI_SYMBOL_STATIC_METHOD("getInstance", "()Ljp/co/capcom/android/bio4_LGUplus0119/AudioPlayer;", &getInstance),

    // 实例方法
    JNI_SYMBOL_METHOD("init", "(II)Z", &init),
    JNI_SYMBOL_METHOD("term", "(II)Z", &term),
    JNI_SYMBOL_METHOD("loadBGM", "(ILjava/lang/String;JJZ)Z", &loadBGM),
    JNI_SYMBOL_METHOD("loadSE", "(ILjava/lang/String;JJ)Z", &loadSE),
    JNI_SYMBOL_METHOD("termBGM", "(ILjava/lang/String;JJZ)Z", &termBGM),
    JNI_SYMBOL_METHOD("termSE", "(ILjava/lang/String;JJ)Z", &termSE),
    JNI_SYMBOL_METHOD("playBGM", "(I)V", &playBGM),
    JNI_SYMBOL_METHOD("playSE", "(IF)V", &playSE),
    JNI_SYMBOL_METHOD("stopBGM", "(I)V", &stopBGM),
    JNI_SYMBOL_METHOD("stopSE", "(I)V", &stopSE),
    JNI_SYMBOL_METHOD("pauseBGM", "(I)V", &pauseBGM),
    JNI_SYMBOL_METHOD("pauseSE", "(I)V", &pauseSE),
    JNI_SYMBOL_METHOD("restartBGM", "(I)V", &restartBGM),
    JNI_SYMBOL_METHOD("restartSE", "(I)V", &restartSE),
    JNI_SYMBOL_METHOD("setVolumeBGM", "(IF)V", &setVolumeBGM),
};

static jclassMeta class_meta = {
    .name          = "jp/co/capcom/android/bio4_LGUplus0119/AudioPlayer",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = sizeof(AudioPlayer),
};

void AudioPlayer_class_init() {
    AudioPlayer_jclass  = JniCreateClass(&class_meta);
    AudioPlayer_jobject = JniNewGlobalRefTake(JniAllocObject(AudioPlayer_jclass));
}
