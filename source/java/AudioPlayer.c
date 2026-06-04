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

#define MAX_CONCURRENT_SE 8
#define AUDIO_BUFFER_SAMPLES 1024

// SE播放实例
typedef struct SEInstance {
    int slot_id;
    uint8_t *ogg_data;
    uint32_t ogg_size;
    float volume;
    int playing;
    int stop_request;
    int finished;
    stb_vorbis *vorbis;
    int channels;
    int sample_rate;
    struct SEInstance *next;
} SEInstance;

// BGM播放器
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

// SE混音器
typedef struct SEMixer {
    int audio_port;
    int running;
    int sample_rate;
    int channels;
    pthread_t mixer_thread;
    pthread_mutex_t mutex;
    SEInstance *instances[MAX_CONCURRENT_SE];
    int instance_count;
    short *mix_buffer;
    int sample_rate_set;  // 标记采样率是否已设置
} SEMixer;

typedef struct SoundPoolData {
    uint8_t *ogg_data;
    uint32_t ogg_size;
    int channels;
    int sample_rate;
    int loaded;
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
    SEMixer           *m_seMixer;
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

// SE混音线程
static void *se_mixer_thread_func(void *arg) {
    SEMixer *mixer = (SEMixer *)arg;

    short **se_buffers = malloc(MAX_CONCURRENT_SE * sizeof(short*));
    for (int i = 0; i < MAX_CONCURRENT_SE; i++) {
        se_buffers[i] = malloc(AUDIO_BUFFER_SAMPLES * 2 * sizeof(short));
        memset(se_buffers[i], 0, AUDIO_BUFFER_SAMPLES * 2 * sizeof(short));
    }

    so_logd("SE mixer thread started on port %d, sample_rate=%d", mixer->audio_port, mixer->sample_rate);

    while (mixer->running) {
        // 清空混音缓冲区
        memset(mixer->mix_buffer, 0, AUDIO_BUFFER_SAMPLES * 2 * sizeof(short));

        pthread_mutex_lock(&mixer->mutex);

        // 先清理已完成的SE实例
        for (int i = 0; i < MAX_CONCURRENT_SE; i++) {
            if (mixer->instances[i] && mixer->instances[i]->finished) {
                so_logd("Cleaning up finished SE instance %d", i);
                if (mixer->instances[i]->vorbis) {
                    stb_vorbis_close(mixer->instances[i]->vorbis);
                }
                if (mixer->instances[i]->ogg_data) {
                    free(mixer->instances[i]->ogg_data);
                }
                free(mixer->instances[i]);
                mixer->instances[i] = NULL;
                mixer->instance_count--;
            }
        }

        // 混音所有活跃的SE
        for (int i = 0; i < MAX_CONCURRENT_SE; i++) {
            SEInstance *se = mixer->instances[i];
            if (se && se->playing && !se->stop_request && se->vorbis) {
                int samples = stb_vorbis_get_samples_short_interleaved(
                        se->vorbis,
                        se->channels,
                        se_buffers[i],
                        AUDIO_BUFFER_SAMPLES);

                if (samples > 0) {
                    int total = samples * se->channels;

                    if (se->channels == 1) {
                        // 单声道转立体声
                        for (int j = 0; j < samples; j++) {
                            int val = (int)((float)se_buffers[i][j] * se->volume);
                            if (val > 32767) val = 32767;
                            if (val < -32768) val = -32768;
                            mixer->mix_buffer[j * 2] += (short)val;
                            mixer->mix_buffer[j * 2 + 1] += (short)val;
                        }
                    } else {
                        // 立体声
                        for (int j = 0; j < total; j++) {
                            int val = (int)((float)se_buffers[i][j] * se->volume);
                            if (val > 32767) val = 32767;
                            if (val < -32768) val = -32768;
                            mixer->mix_buffer[j] += (short)val;
                        }
                    }
                } else {
                    // SE播放完毕，标记为已完成
                    so_logd("SE slot %d (instance %d) finished playing", se->slot_id, i);
                    se->playing = 0;
                    se->finished = 1;
                }
            }
        }

        // 防溢出处理
        for (int j = 0; j < AUDIO_BUFFER_SAMPLES * 2; j++) {
            if (mixer->mix_buffer[j] > 32767) mixer->mix_buffer[j] = 32767;
            if (mixer->mix_buffer[j] < -32768) mixer->mix_buffer[j] = -32768;
        }

        pthread_mutex_unlock(&mixer->mutex);

        // 输出混音后的数据
        if (mixer->audio_port >= 0) {
            sceAudioOutOutput(mixer->audio_port, mixer->mix_buffer);
        }
    }

    for (int i = 0; i < MAX_CONCURRENT_SE; i++) {
        free(se_buffers[i]);
    }
    free(se_buffers);

    return NULL;
}

// 初始化SE混音器 - 采样率暂时设为0，等第一个SE加载时再设置
static SEMixer* se_mixer_create() {
    SEMixer *mixer = calloc(1, sizeof(SEMixer));
    if (!mixer) return NULL;

    pthread_mutex_init(&mixer->mutex, NULL);
    mixer->running = 1;
    mixer->sample_rate = 0;  // 初始为0，等第一个SE设置
    mixer->channels = 2;
    mixer->instance_count = 0;
    mixer->sample_rate_set = 0;

    mixer->mix_buffer = malloc(AUDIO_BUFFER_SAMPLES * 2 * sizeof(short));
    if (!mixer->mix_buffer) {
        pthread_mutex_destroy(&mixer->mutex);
        free(mixer);
        return NULL;
    }
    memset(mixer->mix_buffer, 0, AUDIO_BUFFER_SAMPLES * 2 * sizeof(short));

    for (int i = 0; i < MAX_CONCURRENT_SE; i++) {
        mixer->instances[i] = NULL;
    }

    // 先不打开音频端口，等知道采样率后再打开
    mixer->audio_port = -1;

    so_logd("SE mixer created (port will be opened when first SE is played)");

    // 启动混音线程
    if (pthread_create(&mixer->mixer_thread, NULL, se_mixer_thread_func, mixer) != 0) {
        so_loge("Failed to create SE mixer thread");
        free(mixer->mix_buffer);
        pthread_mutex_destroy(&mixer->mutex);
        free(mixer);
        return NULL;
    }

    return mixer;
}

// 确保混音器端口已打开
static int se_mixer_ensure_port(SEMixer *mixer, int sample_rate) {
    if (mixer->audio_port >= 0) return 0;

    mixer->audio_port = sceAudioOutOpenPort(
            SCE_AUDIO_OUT_PORT_TYPE_VOICE,
            AUDIO_BUFFER_SAMPLES,
            sample_rate,
            SCE_AUDIO_OUT_MODE_STEREO);

    if (mixer->audio_port < 0) {
        so_loge("Failed to open SE audio port: 0x%08X", mixer->audio_port);
        return -1;
    }

    so_logd("SE audio port opened with sample_rate=%d, port=%d", sample_rate, mixer->audio_port);
    return 0;
}

// 销毁SE混音器
static void se_mixer_destroy(SEMixer *mixer) {
    if (!mixer) return;

    so_logd("Destroying SE mixer");
    mixer->running = 0;

    pthread_mutex_lock(&mixer->mutex);
    for (int i = 0; i < MAX_CONCURRENT_SE; i++) {
        if (mixer->instances[i]) {
            mixer->instances[i]->stop_request = 1;
            mixer->instances[i]->playing = 0;
            mixer->instances[i]->finished = 1;
            if (mixer->instances[i]->vorbis) {
                stb_vorbis_close(mixer->instances[i]->vorbis);
            }
            if (mixer->instances[i]->ogg_data) {
                free(mixer->instances[i]->ogg_data);
            }
            free(mixer->instances[i]);
            mixer->instances[i] = NULL;
        }
    }
    mixer->instance_count = 0;
    pthread_mutex_unlock(&mixer->mutex);

    if (mixer->mixer_thread) {
        pthread_join(mixer->mixer_thread, NULL);
    }

    if (mixer->audio_port >= 0) {
        sceAudioOutReleasePort(mixer->audio_port);
    }

    free(mixer->mix_buffer);
    pthread_mutex_destroy(&mixer->mutex);
    free(mixer);
}

// 添加SE到混音器
static int se_mixer_play(SEMixer *mixer, int slot_id, SoundPoolData *sound, float volume) {
    if (!mixer || !sound || !sound->ogg_data) return -1;

    pthread_mutex_lock(&mixer->mutex);

    // 如果是第一个SE，设置采样率并打开音频端口
    if (!mixer->sample_rate_set) {
        mixer->sample_rate = sound->sample_rate;
        mixer->sample_rate_set = 1;
        so_logd("Setting SE mixer sample_rate to %d Hz", mixer->sample_rate);

        // 打开音频端口
        pthread_mutex_unlock(&mixer->mutex);
        if (se_mixer_ensure_port(mixer, mixer->sample_rate) < 0) {
            return -1;
        }
        pthread_mutex_lock(&mixer->mutex);
    }

    // 检查采样率是否匹配
    if (sound->sample_rate != mixer->sample_rate) {
        so_loge("SE sample rate mismatch: expected %d, got %d", mixer->sample_rate, sound->sample_rate);
        pthread_mutex_unlock(&mixer->mutex);
        return -1;
    }

    // 先清理已完成的SE实例
    for (int i = 0; i < MAX_CONCURRENT_SE; i++) {
        if (mixer->instances[i] && mixer->instances[i]->finished) {
            so_logd("Cleaning up finished SE instance %d before adding new", i);
            if (mixer->instances[i]->vorbis) {
                stb_vorbis_close(mixer->instances[i]->vorbis);
            }
            if (mixer->instances[i]->ogg_data) {
                free(mixer->instances[i]->ogg_data);
            }
            free(mixer->instances[i]);
            mixer->instances[i] = NULL;
            mixer->instance_count--;
        }
    }

    // 查找空闲槽位
    int slot = -1;
    for (int i = 0; i < MAX_CONCURRENT_SE; i++) {
        if (mixer->instances[i] == NULL) {
            slot = i;
            break;
        }
    }

    if (slot == -1) {
        so_logw("SE mixer full, max concurrent: %d, current count: %d",
                MAX_CONCURRENT_SE, mixer->instance_count);
        pthread_mutex_unlock(&mixer->mutex);
        return -1;
    }

    // 创建SE实例
    SEInstance *se = calloc(1, sizeof(SEInstance));
    if (!se) {
        pthread_mutex_unlock(&mixer->mutex);
        return -1;
    }

    // 复制OGG数据
    se->ogg_data = malloc(sound->ogg_size);
    if (!se->ogg_data) {
        free(se);
        pthread_mutex_unlock(&mixer->mutex);
        return -1;
    }
    memcpy(se->ogg_data, sound->ogg_data, sound->ogg_size);
    se->ogg_size = sound->ogg_size;
    se->slot_id = slot_id;
    se->volume = volume;
    se->playing = 1;
    se->stop_request = 0;
    se->finished = 0;
    se->channels = sound->channels;
    se->sample_rate = sound->sample_rate;

    // 打开vorbis解码器
    int error;
    se->vorbis = stb_vorbis_open_memory(se->ogg_data, se->ogg_size, &error, NULL);
    if (!se->vorbis) {
        so_loge("Failed to open SE vorbis: %d", error);
        free(se->ogg_data);
        free(se);
        pthread_mutex_unlock(&mixer->mutex);
        return -1;
    }

    mixer->instances[slot] = se;
    mixer->instance_count++;

    so_logd("SE slot %d added to mixer at instance %d (ch=%d, rate=%dHz, vol=%.2f, total: %d)",
            slot_id, slot, se->channels, se->sample_rate, volume, mixer->instance_count);

    pthread_mutex_unlock(&mixer->mutex);
    return slot;
}

// 停止指定槽位的SE
static void se_mixer_stop_by_slot(SEMixer *mixer, int slot_id) {
    if (!mixer) return;

    pthread_mutex_lock(&mixer->mutex);
    for (int i = 0; i < MAX_CONCURRENT_SE; i++) {
        SEInstance *se = mixer->instances[i];
        if (se && se->slot_id == slot_id && se->playing) {
            so_logd("Stopping SE slot %d (instance %d)", slot_id, i);
            se->stop_request = 1;
            se->playing = 0;
            se->finished = 1;
        }
    }
    pthread_mutex_unlock(&mixer->mutex);
}

static jobject getInstance() {
    jobject result = JniNewLocalRef(AudioPlayer_jobject);
    so_logd("[AudioPlayer] getInstance(): %p", result);
    return result;
}


// "init","(II)Z"
jboolean init(AudioPlayer *thiz, jargs *args) {
    jint nBGMNum = JARG(args, jint);
    jint nSENum  = JARG(args, jint);

    thiz->m_nBGMNum = nBGMNum;
    thiz->m_nSENum  = nSENum;

    // 创建SE混音器
    thiz->m_seMixer = se_mixer_create();
    if (!thiz->m_seMixer) {
        so_loge("Failed to create SE mixer");
        return JNI_FALSE;
    }

    thiz->m_apBGM = calloc(
            nBGMNum,
            sizeof(AudioMediaPlayer *));

    thiz->m_apSE = calloc(
            nSENum,
            sizeof(SoundPoolData *));

    for (int i = 0; i < nBGMNum; i++) {
        thiz->m_apBGM[i] = calloc(1, sizeof(AudioMediaPlayer));
        thiz->m_apBGM[i]->volume = 1.0f;
    }

    for (int i = 0; i < nSENum; i++) {
        thiz->m_apSE[i] = calloc(1, sizeof(SoundPoolData));
        thiz->m_apSE[i]->loaded = 0;
    }

    thiz->m_pSoundPool = calloc(1, sizeof(SoundPool));

    so_logd("[AudioPlayer] init(%d,%d)", nBGMNum, nSENum);

    return JNI_TRUE;
}

// "term","(II)Z"
jboolean term(AudioPlayer *thiz, jargs *args) {
    jint nBGMNum = JARG(args, jint);
    jint nSENum  = JARG(args, jint);

    // 销毁SE混音器
    if (thiz->m_seMixer) {
        se_mixer_destroy(thiz->m_seMixer);
        thiz->m_seMixer = NULL;
    }

    for (int i = 0; i < thiz->m_nBGMNum; i++) {
        AudioMediaPlayer *p = thiz->m_apBGM[i];
        if (!p) continue;
        p->stop_request = 1;
        if (p->playing) pthread_join(p->thread, NULL);
        free(p->ogg_data);
        free(p);
    }
    free(thiz->m_apBGM);
    thiz->m_apBGM = NULL;

    for (int i = 0; i < thiz->m_nSENum; i++) {
        SoundPoolData *p = thiz->m_apSE[i];
        if (!p) continue;
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
    p->loaded = 1;

    // 获取音频信息
    int error;
    stb_vorbis *vorbis = stb_vorbis_open_memory(p->ogg_data, p->ogg_size, &error, NULL);
    if (vorbis) {
        stb_vorbis_info info = stb_vorbis_get_info(vorbis);
        p->channels = info.channels;
        p->sample_rate = info.sample_rate;
        stb_vorbis_close(vorbis);
        so_logd("[AudioPlayer] loadSE(%d) ch=%d rate=%d", nSlotID, p->channels, p->sample_rate);
    } else {
        so_loge("Failed to parse SE OGG: %d", error);
        return JNI_FALSE;
    }

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

    if (p->ogg_data) {
        free(p->ogg_data);
        p->ogg_data = NULL;
    }
    p->loaded = 0;

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

// "playSE","(IF)V" - 使用混音器
void playSE(AudioPlayer *thiz, jargs *args) {
    jint   nSlotID = JARG(args, jint);
    jfloat nVolume = JARG(args, jfloat);

    SoundPoolData *p = thiz->m_apSE[nSlotID];
    if (!p || !p->loaded) {
        so_loge("playSE: slot %d not loaded", nSlotID);
        return;
    }

    int result = se_mixer_play(thiz->m_seMixer, nSlotID, p, nVolume);
    if (result < 0) {
        so_loge("playSE: failed to play slot %d (mixer full)", nSlotID);
    } else {
        so_logd("[AudioPlayer] playSE(%d,%f) assigned to instance %d", nSlotID, nVolume, result);
    }
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

// "stopSE","(I)V" - 通过混音器停止
void stopSE(AudioPlayer *thiz, jargs *args) {
    jint nSlotID = JARG(args, jint);

    se_mixer_stop_by_slot(thiz->m_seMixer, nSlotID);

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
        JNI_SYMBOL_STATIC_METHOD("getInstance", "()Ljjp/co/capcom/android/googleplay/Evil4/AudioPlayer;", &getInstance),

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
        .name          = "jjp/co/capcom/android/googleplay/Evil4/AudioPlayer",
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