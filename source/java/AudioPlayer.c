#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "config.h"

#include <psp2/audioout.h>
#include <psp2/kernel/threadmgr.h>

#define STB_VORBIS_NO_STDIO
#define STB_VORBIS_IMPLEMENTATION
#include "stb_vorbis.c"

#include <so_common/so_logger.h>
#include <so_jni/so_jni.h>

#include "java.h"

typedef struct AudioMediaPlayer {
    uint8_t *ogg_data;
    uint32_t ogg_size;

    int loop;
    float volume;

    int paused;
    int playing;
    int stop_request;

    int audio_port;

    pthread_t thread;
} AudioMediaPlayer;

typedef struct SoundPoolData {
    uint8_t *ogg_data;
    uint32_t ogg_size;

    int audio_port;

    int playing;
    int stop_request;

    float volume;

    pthread_t thread;
} SoundPoolData;

typedef struct SoundPool {
    int dummy;
} SoundPool;

typedef struct AudioPlayer {
    jint               m_nBGMNum;
    jint               m_nSENum;
    AudioMediaPlayer **m_apBGM;
    SoundPoolData    **m_apSE;
    SoundPool         *m_pSoundPool;
} AudioPlayer;

static jclass  AudioPlayer_jclass  = NULL;
static jobject AudioPlayer_jobject = NULL;

static char  m_pFilePath[1024];

static uint8_t *read_file_segment(
        const char *path,
        uint64_t offset,
        uint64_t length) {

    FILE *fp = fopen(path, "rb");

    if (!fp) {
        so_loge("open failed: %s", path);
        return NULL;
    }

    uint8_t *buf = malloc(length);

    if (!buf) {
        fclose(fp);
        return NULL;
    }

    fseek(fp, offset, SEEK_SET);

    fread(buf, 1, length, fp);

    fclose(fp);

    return buf;
}

static void *bgm_thread_func(void *arg) {
    AudioMediaPlayer *p = (AudioMediaPlayer *)arg;

    int error;

    stb_vorbis *vorbis = stb_vorbis_open_memory(
            p->ogg_data,
            p->ogg_size,
            &error,
            NULL);

    if (!vorbis) {
        so_loge("stb_vorbis_open_memory failed: %d", error);
        p->playing = 0;
        return NULL;
    }

    stb_vorbis_info info = stb_vorbis_get_info(vorbis);

    int channels    = info.channels;
    int sample_rate = info.sample_rate;

    so_logd("[BGM] channels=%d sample_rate=%d", channels, sample_rate);

    p->audio_port = sceAudioOutOpenPort(
            SCE_AUDIO_OUT_PORT_TYPE_BGM,
            1024,
            sample_rate,
            channels == 1
            ? SCE_AUDIO_OUT_MODE_MONO
            : SCE_AUDIO_OUT_MODE_STEREO);

    if (p->audio_port < 0) {
        so_loge("sceAudioOutOpenPort failed");
        stb_vorbis_close(vorbis);
        p->playing = 0;
        return NULL;
    }

    short pcm[1024 * 2];

    while (!p->stop_request) {

        if (p->paused) {
            sceKernelDelayThread(1000 * 10);
            continue;
        }

        int samples = stb_vorbis_get_samples_short_interleaved(
                vorbis,
                channels,
                pcm,
                1024 * channels);

        if (samples <= 0) {

            if (p->loop) {
                stb_vorbis_seek_start(vorbis);
                continue;
            }

            break;
        }

        int total = samples * channels;

        for (int i = 0; i < total; i++) {
            pcm[i] = (short)((float)pcm[i] * p->volume);
        }

        sceAudioOutOutput(p->audio_port, pcm);
    }

    sceAudioOutReleasePort(p->audio_port);

    stb_vorbis_close(vorbis);

    p->playing = 0;

    return NULL;
}

static void *se_thread_func(void *arg) {
    SoundPoolData *p = (SoundPoolData *)arg;

    int error;

    stb_vorbis *vorbis = stb_vorbis_open_memory(
            p->ogg_data,
            p->ogg_size,
            &error,
            NULL);

    if (!vorbis) {
        so_loge("SE stb_vorbis_open_memory failed: %d", error);
        p->playing = 0;
        return NULL;
    }

    stb_vorbis_info info = stb_vorbis_get_info(vorbis);

    int channels    = info.channels;
    int sample_rate = info.sample_rate;

    so_logd("[SE] Original: channels=%d sample_rate=%d", channels, sample_rate);

    p->audio_port = sceAudioOutOpenPort(
            SCE_AUDIO_OUT_PORT_TYPE_VOICE,
            1024,
            sample_rate,
            channels == 1
            ? SCE_AUDIO_OUT_MODE_MONO
            : SCE_AUDIO_OUT_MODE_STEREO);

    if (p->audio_port < 0) {
        so_loge("SE: sceAudioOutOpenPort failed with error: 0x%08X", p->audio_port);
        so_loge("SE: channels=%d, sample_rate=%d", channels, sample_rate);
        stb_vorbis_close(vorbis);
        p->playing = 0;
        return NULL;
    }

    short pcm[1024 * 2];

    while (!p->stop_request) {

        int samples = stb_vorbis_get_samples_short_interleaved(
                vorbis,
                channels,
                pcm,
                1024 * channels);

        if (samples <= 0)
            break;

        int total = samples * channels;

        for (int i = 0; i < total; i++) {
            pcm[i] = (short)((float)pcm[i] * p->volume);
        }

        sceAudioOutOutput(p->audio_port, pcm);
    }

    sceAudioOutReleasePort(p->audio_port);
    stb_vorbis_close(vorbis);
    p->playing = 0;

    return NULL;
}

static jobject getInstance() {
    jobject result = JniNewLocalRef(AudioPlayer_jobject);
    so_logd("[AudioPlayer] getInstance(): %p", result);
    return result;
}

void AudioPlayer_ctor(AudioPlayer *thiz) {
    memset(thiz, 0, sizeof(AudioPlayer));
}

// "init","(II)Z"
jboolean init(AudioPlayer *thiz, jargs *args) {
    jint nBGMNum = JARG(args, jint);
    jint nSENum  = JARG(args, jint);

    thiz->m_nBGMNum = nBGMNum;
    thiz->m_nSENum  = nSENum;

    thiz->m_apBGM = calloc(
            nBGMNum,
            sizeof(AudioMediaPlayer *));

    thiz->m_apSE = calloc(
            nSENum,
            sizeof(SoundPoolData *));

    for (int i = 0; i < nBGMNum; i++) {

        thiz->m_apBGM[i] = calloc(
                1,
                sizeof(AudioMediaPlayer));

        thiz->m_apBGM[i]->volume = 1.0f;
    }

    for (int i = 0; i < nSENum; i++) {

        thiz->m_apSE[i] = calloc(
                1,
                sizeof(SoundPoolData));

        thiz->m_apSE[i]->volume = 1.0f;
    }

    thiz->m_pSoundPool = calloc(1, sizeof(SoundPool));

    so_logd("[AudioPlayer] init(%d,%d)", nBGMNum, nSENum);

    return JNI_TRUE;
}

// "term","(II)Z"
jboolean term(AudioPlayer *thiz, jargs *args) {
    jint nBGMNum = JARG(args, jint);
    jint nSENum  = JARG(args, jint);

    for (int i = 0; i < thiz->m_nBGMNum; i++) {

        AudioMediaPlayer *p = thiz->m_apBGM[i];

        if (!p)
            continue;

        p->stop_request = 1;

        if (p->playing)
            pthread_join(p->thread, NULL);

        free(p->ogg_data);

        free(p);
    }

    free(thiz->m_apBGM);
    thiz->m_apBGM = NULL;

    for (int i = 0; i < thiz->m_nSENum; i++) {

        SoundPoolData *p = thiz->m_apSE[i];

        if (!p)
            continue;

        p->stop_request = 1;

        if (p->playing)
            pthread_join(p->thread, NULL);

        free(p->ogg_data);

        free(p);
    }

    free(thiz->m_apSE);
    thiz->m_apSE = NULL;

    free(thiz->m_pSoundPool);
    thiz->m_pSoundPool = NULL;

    so_logd("[AudioPlayer] term(%d,%d)", nBGMNum, nSENum);

    return JNI_TRUE;
}

// "loadBGM","(ILjava/lang/String;JJZ)Z"
jboolean loadBGM(AudioPlayer *thiz, jargs *args) {
    jint        nSlotID   = JARG(args, jint);
    const char *pFileName = JniGetStringUTFChars(JARG(args, jstring), NULL);
    jlong       nOffset   = JARG(args, jlong);
    jlong       nLength   = JARG(args, jlong);
    jboolean    isLoop    = JARG(args, jboolean);

    AudioMediaPlayer *p = thiz->m_apBGM[nSlotID];

    if (p->ogg_data) {
        free(p->ogg_data);
        p->ogg_data = NULL;
    }

    snprintf(m_pFilePath, sizeof(m_pFilePath), "%s%s.png", SO_ASSETS_DIR, pFileName);
    p->ogg_data = read_file_segment(
            m_pFilePath,
            nOffset,
            nLength);

    if (!p->ogg_data) {
        so_loge("loadBGM failed");
        return JNI_FALSE;
    }

    p->ogg_size = nLength;
    p->loop     = isLoop;
    p->paused   = 0;

    so_logd("[AudioPlayer] loadBGM(%d,'%s',%lld,%lld,%u)",
            nSlotID,
            m_pFilePath,
            nOffset,
            nLength,
            isLoop);

    return JNI_TRUE;
}

// "loadSE","(ILjava/lang/String;JJ)Z"
jboolean loadSE(AudioPlayer *thiz, jargs *args) {
    jint        nSlotID   = JARG(args, jint);
    const char *pFileName = JniGetStringUTFChars(JARG(args, jstring), NULL);
    jlong       nOffset   = JARG(args, jlong);
    jlong       nLength   = JARG(args, jlong);

    SoundPoolData *p = thiz->m_apSE[nSlotID];

    if (p->ogg_data) {
        free(p->ogg_data);
        p->ogg_data = NULL;
    }

    snprintf(m_pFilePath, sizeof(m_pFilePath), "%s%s.png", SO_ASSETS_DIR, pFileName);
    p->ogg_data = read_file_segment(
            m_pFilePath,
            nOffset,
            nLength);

    if (!p->ogg_data) {
        so_loge("loadSE failed");
        return JNI_FALSE;
    }

    p->ogg_size = nLength;

    so_logd("[AudioPlayer] loadSE(%d,'%s',%lld,%lld)",
            nSlotID,
            m_pFilePath,
            nOffset,
            nLength);

    return JNI_TRUE;
}

// "termBGM","(I)V"
void termBGM(AudioPlayer *thiz, jargs *args) {
    jint nSlotID = JARG(args, jint);

    AudioMediaPlayer *p = thiz->m_apBGM[nSlotID];

    if (!p)
        return;

    p->stop_request = 1;

    if (p->playing)
        pthread_join(p->thread, NULL);

    free(p->ogg_data);
    p->ogg_data = NULL;
    p->ogg_size = 0;

    so_logd("[AudioPlayer] termBGM(%d)", nSlotID);
}

// "termSE","(I)V"
void termSE(AudioPlayer *thiz, jargs *args) {
    jint nSlotID = JARG(args, jint);

    SoundPoolData *p = thiz->m_apSE[nSlotID];

    if (!p)
        return;

    p->stop_request = 1;

    if (p->playing)
        pthread_join(p->thread, NULL);

    free(p->ogg_data);
    p->ogg_data = NULL;
    p->ogg_size = 0;

    so_logd("[AudioPlayer] termSE(%d)", nSlotID);
}

// "playBGM","(I)V"
void playBGM(AudioPlayer *thiz, jargs *args) {
    jint nSlotID = JARG(args, jint);

    AudioMediaPlayer *p = thiz->m_apBGM[nSlotID];

    if (!p || !p->ogg_data)
        return;

    if (p->playing) {
        p->stop_request = 1;
        pthread_join(p->thread, NULL);
    }

    p->stop_request = 0;
    p->paused       = 0;
    p->playing      = 1;

    pthread_create(
            &p->thread,
            NULL,
            bgm_thread_func,
            p);

    so_logd("[AudioPlayer] playBGM(%d)", nSlotID);
}

// "playSE","(IF)V"
void playSE(AudioPlayer *thiz, jargs *args) {
    jint   nSlotID = JARG(args, jint);
    jfloat nVolume = JARG(args, jfloat);

    SoundPoolData *p = thiz->m_apSE[nSlotID];

    if (!p || !p->ogg_data)
        return;

    if (p->playing) {
        p->stop_request = 1;
        pthread_join(p->thread, NULL);
    }

    p->volume      = nVolume;
    p->stop_request = 0;
    p->playing      = 1;

    pthread_create(
            &p->thread,
            NULL,
            se_thread_func,
            p);

    so_logd("[AudioPlayer] playSE(%d,%f)", nSlotID, nVolume);
}

// "stopBGM","(I)V"
void stopBGM(AudioPlayer *thiz, jargs *args) {
    jint nSlotID = JARG(args, jint);

    AudioMediaPlayer *p = thiz->m_apBGM[nSlotID];

    if (!p || !p->playing)
        return;

    p->stop_request = 1;

    pthread_join(p->thread, NULL);

    p->playing = 0;

    so_logd("[AudioPlayer] stopBGM(%d)", nSlotID);
}

// "stopSE","(I)V"
void stopSE(AudioPlayer *thiz, jargs *args) {
    jint nSlotID = JARG(args, jint);

    SoundPoolData *p = thiz->m_apSE[nSlotID];

    if (!p || !p->playing)
        return;

    p->stop_request = 1;

    pthread_join(p->thread, NULL);

    p->playing = 0;

    so_logd("[AudioPlayer] stopSE(%d)", nSlotID);
}

// "pauseBGM","(I)V"
void pauseBGM(AudioPlayer *thiz, jargs *args) {
    jint nSlotID = JARG(args, jint);

    AudioMediaPlayer *p = thiz->m_apBGM[nSlotID];

    if (!p)
        return;

    p->paused = 1;

    so_logd("[AudioPlayer] pauseBGM(%d)", nSlotID);
}

// "pauseSE","(I)V"
void pauseSE(AudioPlayer *thiz, jargs *args) {
    jint nSlotID = JARG(args, jint);

    so_logd("[AudioPlayer] pauseSE(%d)", nSlotID);
}

// "restartBGM","(I)V"
void restartBGM(AudioPlayer *thiz, jargs *args) {
    jint nSlotID = JARG(args, jint);

    AudioMediaPlayer *p = thiz->m_apBGM[nSlotID];

    if (!p)
        return;

    p->paused = 0;

    so_logd("[AudioPlayer] restartBGM(%d)", nSlotID);
}

// "restartSE","(I)V"
void restartSE(AudioPlayer *thiz, jargs *args) {
    jint nSlotID = JARG(args, jint);

    so_logd("[AudioPlayer] restartSE(%d)", nSlotID);
}

// "setVolumeBGM", "(IF)V"
void setVolumeBGM(AudioPlayer *thiz, jargs *args) {
    jint   nSlotID = JARG(args, jint);
    jfloat nVolume = JARG(args, jfloat);

    AudioMediaPlayer *p = thiz->m_apBGM[nSlotID];

    if (!p)
        return;

    p->volume = nVolume;

    so_logd("[AudioPlayer] setVolumeBGM(%d,%f)", nSlotID, nVolume);
}

static jsymbol class_symbols[] = {
        JNI_SYMBOL_STATIC_METHOD("getInstance", "()Ljp/co/capcom/android/bio4_LGUplus0119/AudioPlayer;", &getInstance),

        JNI_SYMBOL_METHOD("init", "(II)Z", &init),
        JNI_SYMBOL_METHOD("term", "(II)Z", &term),
        JNI_SYMBOL_METHOD("loadBGM", "(ILjava/lang/String;JJZ)Z", &loadBGM),
        JNI_SYMBOL_METHOD("loadSE", "(ILjava/lang/String;JJ)Z", &loadSE),

        JNI_SYMBOL_METHOD("termBGM", "(I)V", &termBGM),
        JNI_SYMBOL_METHOD("termSE", "(I)V", &termSE),

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

    AudioPlayer_jobject = JniNewGlobalRefTake(
            JniAllocObject(AudioPlayer_jclass));
}