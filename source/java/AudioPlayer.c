#include <stddef.h>

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

static AudioPlayer *sInstance = NULL;

AudioPlayer *AudioPlayer_new() {
    AudioPlayer *player = calloc(1, sizeof(AudioPlayer));

    return player;
}

static AudioPlayer *getInstance() {
    if (!sInstance) {
        sInstance = AudioPlayer_new();
    }
    return JniNewLocalRef(sInstance);
}

void AudioPlayer_ctor(AudioPlayer *thiz) {
}

// "init", "(II)Z"
jboolean init(AudioPlayer *thiz, int nBGMNum, int nSENum) {  // (int nBGMNum, int nSENum)
    thiz->m_nBGMNum = nBGMNum;
    thiz->m_nSENum  = nSENum;
    thiz->m_apBGM   = new AudioMediaPlayer[thiz->m_nBGMNum];

    for (int i = 0; i < thiz->m_nBGMNum; i++) {
        thiz->m_apBGM[i] = new AudioMediaPlayer();
        thiz->m_apBGM[i].create();
    }

    thiz->m_apSE = new SoundPoolData[thiz->m_nSENum];
    for (int i2 = 0; i2 < thiz->m_nSENum; i2++) {
        thiz->m_apSE[i2] = new SoundPoolData();
    }

    thiz->m_pSoundPool = new SoundPool(thiz->m_nSENum, 3, 0);

    return JNI_TRUE;
}

jboolean term(AudioPlayer *thiz, int nBGMNum, int nSENum) {
    if (thiz->m_apBGM != NULL) {
        for (int i = 0; i < thiz->m_nBGMNum; i++) {
            if (thiz->m_apBGM[i] != NULL) {
                thiz->m_apBGM[i].destroy();
            }
            thiz->m_apBGM[i] = NULL;
        }
        thiz->m_apBGM = NULL;
    }

    if (thiz->m_apSE != NULL) {
        for (int i2 = 0; i2 < thiz->m_nSENum; i2++) {
            if (thiz->m_pSoundPool != NULL) {
                stopSE(i2);
            }
            thiz->m_apSE[i2] = NULL;
        }
        thiz->m_apSE = NULL;
    }

    if (thiz->m_pSoundPool != NULL) {
        thiz->m_pSoundPool.release();
        thiz->m_pSoundPool = NULL;
    }

    return JNI_TRUE;
}

jboolean loadBGM(AudioPlayer *thiz, int nSlotID, String pFileName, long nOffset, long nLength, boolean isLoop) {  // (int nSlotID, String pFileName, long nOffset, long nLength, boolean isLoop)
    boolean isResult = thiz->m_apBGM[nSlotID].load(pFileName, nOffset, nLength, nSlotID);
    thiz->m_apBGM[nSlotID].setLoop(isLoop);
    return isResult;
}

jboolean loadSE(AudioPlayer *thiz, int nSlotID, String pFileName, long nOffset, long nLength) {  // (int nSlotID, String pFileName, long nOffset, long nLength)
    return thiz->m_apSE[nSlotID].load(thiz->m_pSoundPool, pFileName, nOffset, nLength);
}

void termBGM(AudioPlayer *thiz, int nSlotID) {
    thiz->m_apBGM[nSlotID].term();
}

void termSE(AudioPlayer *thiz, int nSlotID) {
    thiz->m_apSE[nSlotID].term(thiz->m_pSoundPool);
}

void playBGM(AudioPlayer *thiz, int nSlotID) {
    thiz->m_apBGM[nSlotID].play();
}

void playSE(AudioPlayer *thiz, int nSlotID, float i_Vol) {
    thiz->m_apSE[nSlotID].play(thiz->m_pSoundPool, i_Vol);
}

void stopBGM(AudioPlayer *thiz, int nSlotID) {
    thiz->m_apBGM[nSlotID].stop();
}

void stopSE(AudioPlayer *thiz, int nSlotID) {
    thiz->m_apSE[nSlotID].stop(thiz->m_pSoundPool);
}

void pauseBGM(AudioPlayer *thiz, int nSlotID) {
    thiz->m_apBGM[nSlotID].pause();
}

void pauseSE(AudioPlayer *thiz, int nSlotID) {
}

void restartBGM(AudioPlayer *thiz, int nSlotID) {
    thiz->m_apBGM[nSlotID].restart();
}

void restartSE(int nSlotID) {
}

void setVolumeBGM(int nSlotID, float i_Vol) {
    if (thiz->m_apBGM[nSlotID] != NULL) {
        thiz->m_apBGM[nSlotID].SetVolume(i_Vol);
    }
}

static jsymbol class_symbols[] = {
    // 静态方法
    JNI_SYMBOL_STATIC_METHOD("getInstance", "()Ljava/lang/String;", &getInstance),
};

static jclassMeta class_meta = {
    .name          = "jp/co/capcom/android/bio4_LGUplus0119/AudioPlayer",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = sizeof(AudioPlayer),
};

void AudioPlayer_class_init() {
    JniCreateClass(&class_meta);
}
