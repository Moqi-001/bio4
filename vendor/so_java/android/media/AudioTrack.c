#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

#include <psp2/audioout.h>
#include <psp2/kernel/clib.h>
#include <psp2/kernel/processmgr.h>
#include <so_common/so_logger.h>
#include <so_java/java/lang/Class.h>
#include <speex/speex_resampler.h>

#include "AudioFormat.h"
#include "AudioTrack.h"

// android
#define VOLUME_MIN 0.0f
#define VOLUME_MAX 1.0f

#define SAMPLE_RATE_HZ_MIN 4000
#define SAMPLE_RATE_HZ_MAX 48000

#define PLAYSTATE_STOPPED 1
#define PLAYSTATE_PAUSED  2
#define PLAYSTATE_PLAYING 3

#define MODE_STATIC 0
#define MODE_STREAM 1

#define STATE_UNINITIALIZED  0
#define STATE_INITIALIZED    1
#define STATE_NO_STATIC_DATA 2

#define SUCCESS                 0
#define ERROR                   (-1)
#define ERROR_BAD_VALUE         (-2)
#define ERROR_INVALID_OPERATION (-3)

#define ERROR_NATIVESETUP_AUDIOSYSTEM        (-16)
#define ERROR_NATIVESETUP_INVALIDCHANNELMASK (-17)
#define ERROR_NATIVESETUP_INVALIDFORMAT      (-18)
#define ERROR_NATIVESETUP_INVALIDSTREAMTYPE  (-19)
#define ERROR_NATIVESETUP_NATIVEINITFAILED   (-20)

// ------------------------------------------------------------------
#define OUTPUT_SAMPLE_RATE_HZ 48000                 // 输出采样率
#define OUTPUTS_NUM           3                     // 输出缓冲区的缓存数量
#define OUTPUT_FRAMES         512u                  // 每次输出的帧数
#define TRACKS_NUM            32                    // 音轨数量（须大于引擎侧 mixer 池的 20 槽）
#define TRACK_BUFFER_FRAMES   (OUTPUT_FRAMES << 4)  // 音轨缓冲区的帧数

typedef struct AudioResampler {
    SpeexResamplerState *state;
    int16_t             *buffer;
    JInt                 buffer_len;
    JInt                 in_frames_max;  // 单次可送入的源帧数上限（输出缓冲就是按它定容的）
} AudioResampler;

typedef struct AudioTrackImpl {
    JInt                 slot;
    volatile JInt        state;
    volatile JInt        play_state;
    JInt                 channel_count;
    JInt                 channels;
    JInt                 audio_format;
    JInt                 sample_rate;
    JFloat               left_volume;
    JFloat               right_volume;
    int16_t             *buffer;
    JInt                 buffer_len;
    volatile size_t      read_pos;
    volatile size_t      write_pos;
    size_t               stop_pos;
    SceKernelLwMutexWork mutex __attribute__((aligned(8)));  // 需8字节对齐
    SceKernelLwCondWork  cond __attribute__((aligned(8)));   // 需8字节对齐
    AudioResampler       resampler;
} AudioTrackImpl;

struct _AudioTrack {
    AudioTrackImpl *impl;
};

static JClass sClass = NULL;

static volatile JInt        audio_inited               = 0;
static SceUID               audio_thid                 = -1;
static SceUID               audio_port                 = -1;
static JBoolean             audio_pause                = JAVA_FALSE;
static JBoolean             audio_stop                 = JAVA_FALSE;
static AudioTrackImpl      *audio_tracks[TRACKS_NUM]   = {0};
static int16_t             *audio_outputs[OUTPUTS_NUM] = {0};
static size_t               audio_output_len           = 0;
static size_t               audio_output_size          = 0;
static SceKernelLwMutexWork audio_main_mutex;

static size_t mix_track(int16_t *output, size_t output_len, const int16_t *input, size_t input_len, float left_volume, float right_volume) {
    size_t mixed_len = output_len < input_len ? output_len : input_len;

    for (size_t i = 0; i < mixed_len; i += 2) {
        int32_t sum = (int32_t)output[i] + (int32_t)input[i] * left_volume;
        if (sum > 32767) {
            sum = 32767;
        }
        if (sum < -32768) {
            sum = -32768;
        }
        output[i] = (int16_t)sum;

        sum = (int32_t)output[i + 1] + (int32_t)input[i + 1] * right_volume;
        if (sum > 32767) {
            sum = 32767;
        }
        if (sum < -32768) {
            sum = -32768;
        }
        output[i + 1] = (int16_t)sum;
    }

    return mixed_len;
}

static int audio_thread(SceSize args, void *argp) {
    so_logd("[AUDIO] Audio thread start run.\n");

    size_t   active_tracks;
    size_t   input_len;
    int16_t *output_buffer;
    size_t   output_len  = audio_output_len;
    size_t   output_size = audio_output_size;
    size_t   buffer_idx  = 0;

    while (!audio_stop) {
        active_tracks = 0;
        output_buffer = audio_outputs[buffer_idx];

        // 混音是累加操作，输出缓冲区必须初始化为 0，否则会有上一帧残留数据。
        memset(output_buffer, 0, output_size);

        sceKernelLockLwMutex(&audio_main_mutex, 1, NULL);

        for (int i = 0; i < TRACKS_NUM; i++) {
            AudioTrackImpl *track = audio_tracks[i];
            if (!track) {
                continue;
            }

            sceKernelLockLwMutex(&track->mutex, 1, NULL);

            size_t read_end;
            if (track->play_state == PLAYSTATE_PLAYING) {
                read_end = track->write_pos;
            } else if (track->play_state == PLAYSTATE_STOPPED) {
                read_end = track->stop_pos;
            } else {
                sceKernelUnlockLwMutex(&track->mutex, 1);
                continue;
            }

            if (read_end < track->read_pos) {
                input_len = read_end + track->buffer_len - track->read_pos;
            } else {
                input_len = read_end - track->read_pos;
            }
            if (input_len == 0) {
                JBoolean should_free = (track->state == STATE_UNINITIALIZED);
                sceKernelUnlockLwMutex(&track->mutex, 1);
                if (should_free) {
                    audio_tracks[i] = NULL;
                    sceKernelDeleteLwMutex(&track->mutex);
                    sceKernelDeleteLwCond(&track->cond);
                    free(track->buffer);
                    free(track);
                }
                continue;
            }

            size_t mixed_len  = input_len < output_len ? input_len : output_len;
            size_t first_part = track->buffer_len - track->read_pos;
            if (first_part < mixed_len) {
                size_t second_part = mixed_len - first_part;

                mix_track(output_buffer, output_len,
                          track->buffer + track->read_pos, first_part, track->left_volume, track->right_volume);
                mix_track(output_buffer + first_part, output_len - first_part,
                          track->buffer, second_part, track->left_volume, track->right_volume);
            } else {
                mix_track(output_buffer, output_len,
                          track->buffer + track->read_pos, mixed_len, track->left_volume, track->right_volume);
            }

            track->read_pos = (track->read_pos + mixed_len) % track->buffer_len;

            sceKernelSignalLwCond(&track->cond);
            sceKernelUnlockLwMutex(&track->mutex, 1);

            active_tracks++;
        }

        sceKernelUnlockLwMutex(&audio_main_mutex, 1);

        if (active_tracks == 0) {
            sceKernelDelayThread(10 * 1000);
            continue;
        }

        sceAudioOutOutput(audio_port, output_buffer);
        buffer_idx = (buffer_idx + 1) % OUTPUTS_NUM;
    }

    so_logd("[AUDIO] Audio thread stop run.\n");
    sceKernelExitThread(0);
    return 0;
}

static int audio_init() {
    if (atomic_load(&audio_inited) == 2) {
        return 0;
    }

    int expected = 0;
    if (atomic_compare_exchange_strong(&audio_inited, &expected, 1)) {
        audio_output_len  = OUTPUT_FRAMES * 2;
        audio_output_size = audio_output_len * sizeof(int16_t);

        for (JInt i = 0; i < OUTPUTS_NUM; i++) {
            if (audio_outputs[i]) {
                free(audio_outputs[i]);
            }

            audio_outputs[i] = malloc(audio_output_size);
            if (!audio_outputs[i]) {
                so_loge("[AudioTrack] audio_init: malloc output buffers failed!");
                return JAVA_ERR;
            }
        }

        audio_port = sceAudioOutOpenPort(SCE_AUDIO_OUT_PORT_TYPE_BGM, OUTPUT_FRAMES, OUTPUT_SAMPLE_RATE_HZ, SCE_AUDIO_OUT_MODE_STEREO);
        if (audio_port < 0) {
            so_loge("[AudioTrack] audio_init: sceAudioOutOpenPort failed! 0x%x", audio_port);
            return JAVA_ERR;
        }

        audio_thid = sceKernelCreateThread("audio_thread", audio_thread, 0x10000100, 0x40000, 0, 0, NULL);
        if (audio_thid < 0) {
            sceAudioOutReleasePort(audio_port);
            audio_port = -1;
            so_loge("[AudioTrack] audio_init: failed to create thread!");
            return JAVA_ERR;
        }

        sceKernelCreateLwMutex(&audio_main_mutex, "audio_main_mutex", 0, 0, NULL);

        audio_stop = 0;
        if (sceKernelStartThread(audio_thid, 0, NULL) < 0) {
            sceAudioOutReleasePort(audio_port);
            audio_port = -1;

            sceKernelDeleteLwMutex(&audio_main_mutex);
            sceKernelDeleteThread(audio_thid);
            audio_thid = -1;

            so_loge("[AudioTrack] audio_init: failed to start thread!");
            return JAVA_ERR;
        }

        audio_inited = 1;
        so_logd("[AudioTrack] audio_init: initialize OK!");
        atomic_store(&audio_inited, 2);
        return JAVA_OK;
    }

    while (atomic_load(&audio_inited) != 2) {
        sceKernelDelayThread(1000);
    }

    return JAVA_OK;
}

void AudioTrack_ctor(AudioTrack thiz, JInt streamType, JInt sampleRateInHz, JInt channelConfig,
                     JInt audioFormat, JInt bufferSizeInBytes, JInt mode) {  // (3,16000,4,2,4096,1)
    thiz->impl = NULL;

    AudioTrackImpl *impl = calloc(1, sizeof(AudioTrackImpl));
    if (!impl) {
        return;
    }

    switch (channelConfig) {
        case CHANNEL_OUT_DEFAULT:  // CHANNEL_CONFIGURATION_DEFAULT
        case CHANNEL_OUT_MONO:
        case CHANNEL_CONFIGURATION_MONO:
            impl->channel_count = 1;
            impl->channels      = CHANNEL_OUT_MONO;
            break;
        case CHANNEL_OUT_STEREO:
        case CHANNEL_CONFIGURATION_STEREO:
            impl->channel_count = 2;
            impl->channels      = CHANNEL_OUT_STEREO;
            break;
        default:
            so_loge("[AudioTrack] ctor: Unsupported channel configuration.");
            free(impl);
            return;
    }

    switch (audioFormat) {
        case ENCODING_DEFAULT:
        case ENCODING_PCM_16BIT:
            impl->audio_format = ENCODING_PCM_16BIT;
            break;
        case ENCODING_PCM_8BIT:
            impl->audio_format = audioFormat;
            break;
        default:
            so_loge("[AudioTrack] ctor: Unsupported sample encoding. Should be ENCODING_PCM_8BIT or ENCODING_PCM_16BIT.");
            free(impl);
            return;
    }

    if (!audio_inited && audio_init() != JAVA_OK) {
        free(impl);
        return;
    }

    impl->slot         = -1;
    impl->play_state   = 0;
    impl->left_volume  = 1.0f;
    impl->right_volume = 1.0f;
    impl->read_pos     = 0;
    impl->write_pos    = 0;
    impl->stop_pos     = 0;

    impl->buffer_len = TRACK_BUFFER_FRAMES * 2;  // 输出双声道
    impl->buffer     = malloc(impl->buffer_len * sizeof(int16_t));
    if (!impl->buffer) {
        so_loge("[AudioTrack] ctor: no free memory!");
        free(impl);
        return;
    }

    if (sampleRateInHz != OUTPUT_SAMPLE_RATE_HZ) {
        impl->resampler.state = speex_resampler_init(impl->channel_count, sampleRateInHz, OUTPUT_SAMPLE_RATE_HZ, 1, NULL);
        if (!impl->resampler.state) {
            so_loge("[AudioTrack] ctor: speex_resampler_init failed!");
            free(impl->buffer);
            free(impl);
            return;
        }

        JInt input_frames             = bufferSizeInBytes / (sizeof(int16_t) * impl->channel_count);
        JInt output_frames            = (input_frames * OUTPUT_SAMPLE_RATE_HZ + sampleRateInHz - 1) / sampleRateInHz + 64;
        impl->resampler.buffer_len    = output_frames * impl->channel_count;
        impl->resampler.in_frames_max = input_frames;
        impl->resampler.buffer        = (int16_t *)malloc(impl->resampler.buffer_len * sizeof(int16_t));
        if (!impl->resampler.buffer) {
            so_loge("[AudioTrack] ctor: no free memory!");
            speex_resampler_destroy(impl->resampler.state);
            free(impl->buffer);
            free(impl);
            return;
        }
    }

    sceKernelCreateLwMutex(&impl->mutex, "audio_track_mutex", 2, 0, NULL);
    sceKernelCreateLwCond(&impl->cond, "audio_track_cond", 0, &impl->mutex, NULL);

    sceKernelLockLwMutex(&audio_main_mutex, 1, NULL);

    for (JInt i = 0; i < TRACKS_NUM; i++) {
        if (audio_tracks[i] == NULL) {
            impl->slot = i;
            break;
        }
    }
    if (impl->slot == -1) {
        so_loge("[AudioTrack] ctor: no free slot!");
        sceKernelDeleteLwMutex(&impl->mutex);
        sceKernelDeleteLwCond(&impl->cond);
        free(impl->buffer);
        free(impl);
        sceKernelUnlockLwMutex(&audio_main_mutex, 1);
        return;
    }

    impl->state              = STATE_INITIALIZED;
    thiz->impl               = impl;
    audio_tracks[impl->slot] = impl;
    sceKernelUnlockLwMutex(&audio_main_mutex, 1);

    so_logd("[AudioTrack][%d] ctor(%p,%d,%d,%d,%d,%d,%d)",
            impl->slot, thiz, streamType, sampleRateInHz, channelConfig, audioFormat, bufferSizeInBytes, mode);
}

void AudioTrack_dtor(AudioTrack thiz) {
    so_logd("[AudioTrack] dtor(%p)", thiz);
    AudioTrack_release(thiz);
}

JInt AudioTrack_write(AudioTrack thiz, JByte *audioData, JInt offsetInBytes, JInt sizeInBytes) {
    AudioTrackImpl *impl = thiz->impl;
    if (!impl || impl->state == STATE_UNINITIALIZED) {
        return -1;
    }

    if (!audioData || sizeInBytes == 0) {
        return sizeInBytes;
    }

    JInt     result = sizeInBytes;
    int16_t *src    = (int16_t *)(audioData + offsetInBytes);
    JInt     frames = sizeInBytes / sizeof(int16_t) / impl->channel_count;

    JInt chunk_frames = frames;
    if (impl->resampler.state && impl->resampler.in_frames_max > 0 && chunk_frames > impl->resampler.in_frames_max) {
        chunk_frames = impl->resampler.in_frames_max;
    }

    for (JInt off = 0; off < frames;) {
        JInt in_frames = frames - off;
        if (in_frames > chunk_frames) {
            in_frames = chunk_frames;
        }

        int16_t *in_buf = src + off * impl->channel_count;
        JInt     in_len = in_frames * impl->channel_count;

        if (impl->resampler.state) {
            JInt out_frames = impl->resampler.buffer_len / impl->channel_count;
            speex_resampler_process_interleaved_int(impl->resampler.state, in_buf, &in_frames, impl->resampler.buffer, &out_frames);
            if (in_frames == 0) {
                break;
            }

            off += in_frames;
            in_buf = impl->resampler.buffer;
            in_len = out_frames * impl->channel_count;
        } else {
            off += in_frames;
        }

        sceKernelLockLwMutex(&impl->mutex, 1, NULL);

        int16_t *out_buf    = (int16_t *)(impl->buffer);
        JInt     buffer_len = impl->buffer_len;
        size_t   write_pos  = impl->write_pos;

        JInt need_len = in_len;
        if (impl->channel_count == 1) {
            need_len = in_len * 2;
        }

        for (;;) {
            size_t free_len;
            if (impl->read_pos > write_pos) {
                free_len = impl->read_pos - write_pos;
            } else {
                free_len = impl->read_pos + buffer_len - write_pos;
            }
            if (free_len > need_len) {
                break;
            }

            sceKernelWaitLwCond(&impl->cond, NULL);  // 等待，防止音频播放跟不上
            if (impl->state == STATE_UNINITIALIZED) {
                sceKernelUnlockLwMutex(&impl->mutex, 1);
                return -1;
            }
        }

        if (impl->channel_count == 1) {
            for (JInt i = 0; i < in_len; i++) {
                out_buf[write_pos++] = in_buf[i];
                if (write_pos >= buffer_len) {
                    write_pos = 0;
                }
                out_buf[write_pos++] = in_buf[i];
                if (write_pos >= buffer_len) {
                    write_pos = 0;
                }
            }
            impl->write_pos = write_pos;
        } else {
            size_t first_part = buffer_len - write_pos;
            if (first_part < in_len) {
                size_t second_part = in_len - first_part;
                memcpy(out_buf + write_pos, in_buf, first_part * sizeof(int16_t));
                memcpy(out_buf, in_buf + first_part, second_part * sizeof(int16_t));
            } else {
                memcpy(out_buf + write_pos, in_buf, in_len * sizeof(int16_t));
            }
            impl->write_pos = (impl->write_pos + in_len) % buffer_len;
        }

        sceKernelUnlockLwMutex(&impl->mutex, 1);
    }

    so_logd("[AudioTrack][%d] write(%p,%p,%d,%d): %d", impl->slot, thiz, audioData, offsetInBytes, sizeInBytes, result);
    return result;
}

JInt AudioTrack_setStereoVolume(AudioTrack thiz, JFloat leftVolume, JFloat rightVolume) {
    AudioTrackImpl *impl = thiz->impl;
    if (!impl || impl->state == STATE_UNINITIALIZED) {
        return -1;
    }

    sceKernelLockLwMutex(&impl->mutex, 1, NULL);
    if (leftVolume < VOLUME_MIN) {
        impl->left_volume = VOLUME_MIN;
    } else if (leftVolume > VOLUME_MAX) {
        impl->left_volume = VOLUME_MAX;
    } else {
        impl->left_volume = leftVolume;
    }
    if (rightVolume < VOLUME_MIN) {
        impl->right_volume = VOLUME_MIN;
    } else if (rightVolume > VOLUME_MAX) {
        impl->right_volume = VOLUME_MAX;
    } else {
        impl->right_volume = rightVolume;
    }
    sceKernelUnlockLwMutex(&impl->mutex, 1);

    so_logd("[AudioTrack][%d] setStereoVolume(%p,%f,%f): %d", impl->slot, thiz, leftVolume, rightVolume, 0);
    return 0;
}

void AudioTrack_play(AudioTrack thiz) {
    AudioTrackImpl *impl = thiz->impl;
    if (!impl || impl->state == STATE_UNINITIALIZED) {
        return;
    }

    sceKernelLockLwMutex(&impl->mutex, 1, NULL);
    impl->play_state = PLAYSTATE_PLAYING;
    sceKernelUnlockLwMutex(&impl->mutex, 1);
    so_logd("[AudioTrack][%d] play(%p)", impl->slot, thiz);
}

void AudioTrack_pause(AudioTrack thiz) {
    AudioTrackImpl *impl = thiz->impl;
    if (!impl || impl->state == STATE_UNINITIALIZED) {
        return;
    }

    sceKernelLockLwMutex(&impl->mutex, 1, NULL);
    impl->play_state = PLAYSTATE_PAUSED;
    sceKernelUnlockLwMutex(&impl->mutex, 1);
    so_logd("[AudioTrack][%d] pause(%p)", impl->slot, thiz);
}

void AudioTrack_stop(AudioTrack thiz) {
    AudioTrackImpl *impl = thiz->impl;
    if (!impl || impl->state == STATE_UNINITIALIZED) {
        return;
    }

    sceKernelLockLwMutex(&impl->mutex, 1, NULL);
    impl->play_state = PLAYSTATE_STOPPED;
    impl->stop_pos   = impl->write_pos;
    sceKernelUnlockLwMutex(&impl->mutex, 1);
    so_logd("[AudioTrack][%d] stop(%p)", impl->slot, thiz);
}

void AudioTrack_flush(AudioTrack thiz) {
    so_logd("[AudioTrack][%d] flush(%p)", thiz);
}

void AudioTrack_release(AudioTrack thiz) {
    AudioTrackImpl *impl = thiz->impl;
    if (!impl) {
        return;
    }
    thiz->impl = NULL;

    sceKernelLockLwMutex(&impl->mutex, 1, NULL);
    impl->play_state = PLAYSTATE_STOPPED;
    impl->stop_pos   = impl->write_pos;
    impl->state      = STATE_UNINITIALIZED;
    sceKernelSignalLwCond(&impl->cond);
    sceKernelUnlockLwMutex(&impl->mutex, 1);

    so_logd("[AudioTrack][%d] release(%p)", impl->slot, thiz);
}

JInt AudioTrack_getMinBufferSize(JInt sampleRateInHz, JInt channelConfig, JInt audioFormat) {
    JInt channelCount;
    JInt bytesPerSample;

    switch (channelConfig) {
        case CHANNEL_OUT_DEFAULT:  // CHANNEL_CONFIGURATION_DEFAULT
        case CHANNEL_OUT_MONO:
        case CHANNEL_CONFIGURATION_MONO:
            channelCount = 1;
            break;
        case CHANNEL_OUT_STEREO:
        case CHANNEL_CONFIGURATION_STEREO:
            channelCount = 2;
            break;
        default:
            so_loge("[AudioTrack] getMinBufferSize(%d,%d,%d): Invalid channel configuration.",
                    sampleRateInHz, channelConfig, audioFormat);
            return ERROR_BAD_VALUE;
    }

    switch (audioFormat) {
        case ENCODING_DEFAULT:
        case ENCODING_PCM_16BIT:
            bytesPerSample = 2;
            break;
        case ENCODING_PCM_8BIT:
            bytesPerSample = 1;
            break;
        default:
            so_loge("[AudioTrack] getMinBufferSize(%d,%d,%d): Unsupported sample encoding. Should be ENCODING_PCM_8BIT or ENCODING_PCM_16BIT.",
                    sampleRateInHz, channelConfig, audioFormat);
            return ERROR_BAD_VALUE;
    }

    // 建议值：至少能容纳 10ms 的据数
    JInt minFrames = sampleRateInHz / 100;  // 10ms 的帧数
    minFrames      = (minFrames + 7) & ~7;
    // 确保不小于硬件输出块大小
    if (minFrames < OUTPUT_FRAMES) {
        minFrames = OUTPUT_FRAMES;
    }

    JInt minSize = minFrames * channelCount * bytesPerSample;  // 10ms 的字节数

    so_logd("[AudioTrack] getMinBufferSize(%d,%d,%d): %d", sampleRateInHz, channelConfig, audioFormat, minSize);
    return minSize;
}

static JSymbol class_symbols[] = {
    // method
    JAVA_SYMBOL_METHOD("<init>", "(IIIIII)V", &AudioTrack_ctor),
    JAVA_SYMBOL_METHOD("write", "([BII)I", &AudioTrack_write),
    JAVA_SYMBOL_METHOD("setStereoVolume", "(FF)I", &AudioTrack_setStereoVolume),
    JAVA_SYMBOL_METHOD("play", "()V", &AudioTrack_play),
    JAVA_SYMBOL_METHOD("pause", "()V", &AudioTrack_pause),
    JAVA_SYMBOL_METHOD("stop", "()V", &AudioTrack_stop),
    JAVA_SYMBOL_METHOD("flush", "()V", &AudioTrack_flush),
    JAVA_SYMBOL_METHOD("release", "()V", &AudioTrack_release),
    // static method
    JAVA_SYMBOL_STATIC_METHOD("getMinBufferSize", "(III)I", &AudioTrack_getMinBufferSize),
};

static JClassMeta class_meta = {
    .name          = "android/media/AudioTrack",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = sizeof(struct _AudioTrack),
    .dtor          = (JDtor)AudioTrack_dtor,
};

void setup_android_media_AudioTrack(void) {
    sClass = JavaCreateClass(&class_meta);
}
