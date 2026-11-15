/*
 * OpenSL ES 最小实现：满足引擎的音频初始化流程（无实际声音输出）
 *
 * 引擎（Black::Audio::Device_impl）的调用链：
 *   slCreateEngine → (*engine)->Realize
 *                  → (*engine)->GetInterface(SL_IID_ENGINE)
 *                  → (*engineItf)->CreateOutputMix
 *                  → (*mix)->Realize / GetInterface(SL_IID_VOLUME) → SetVolumeLevel
 *                  → (*engineItf)->CreateAudioPlayer
 *                  → (*player)->Realize
 *                  → (*player)->GetInterface(SL_IID_PLAY | SL_IID_ANDROIDSIMPLEBUFFERQUEUE)
 *                  → (*play)->SetPlayState / (*bq)->RegisterCallback + Enqueue
 * 任何一步返回非 SUCCESS，引擎会走错误路径。
 *
 * 结构说明：SLObjectItf / 各 SLxxxItf 都是「指向虚表指针的指针」。
 * 因此对象/接口的值 = 某个静态存储单元的地址，该单元保存虚表指针。
 * 虚表按 SLES 头文件中的成员顺序排列；未实现的方法填通用空实现
 * （返回 SL_RESULT_SUCCESS），需要回填输出参数的方法单独实现。
 */
#include <stdint.h>
#include <string.h>

#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>

#include <psp2/kernel/threadmgr.h>

#include <so_common/so_logger.h>

#define SLOT_COUNT_OBJECT 9
#define SLOT_COUNT_ENGINE 15
#define SLOT_COUNT_PLAY   12
#define SLOT_COUNT_BQ     4
#define SLOT_COUNT_VOLUME 9

static const void *g_bq_storage;

static SLresult sl_noop(void) {
    return SL_RESULT_SUCCESS;
}

#define SLOT_NOOP ((const void *)sl_noop)

// 所有对象共用同一套对象方法；对象自身只保存状态
typedef struct SlObjectImpl {
    const void *vtable;  // 首字段：SLObjectItf 指向它
    const void *iface_vtable;
    SLuint32    state;
} SlObjectImpl;

static SlObjectImpl g_engine_obj;
static SlObjectImpl g_output_mix_obj;
static SlObjectImpl g_player_obj;

// 接口的虚表指针存储单元（接口值 = 这些单元的地址）
static const void *g_engine_storage      = NULL;
static const void *g_play_storage        = NULL;
static const void *g_volume_storage      = NULL;
static const void *g_generic_storage     = NULL;

// ==== 对象方法 ====

static SlObjectImpl *impl_of(const void *self) {
    return (SlObjectImpl *)self;
}

static SLresult object_Realize(SlObjectImpl *self, SLboolean async) {
    (void)async;
    self->state = SL_OBJECT_STATE_REALIZED;
    return SL_RESULT_SUCCESS;
}

static SLresult object_Resume(SlObjectImpl *self, SLboolean async) {
    (void)async;
    (void)self;
    return SL_RESULT_SUCCESS;
}

static SLresult object_GetState(SlObjectImpl *self, SLuint32 *pState) {
    if (pState) {
        *pState = self->state;
    }
    return SL_RESULT_SUCCESS;
}

// 接口分发：按 iid 指针比对（游戏导入的 IID 地址即本项目 sl_iid_defs.c 的符号）
static const void *storage_for_iid(const SLInterfaceID iid) {
    if (iid == NULL) {
        return NULL;
    }
    if (iid == SL_IID_ENGINE) {
        return &g_engine_storage;
    }
    if (iid == SL_IID_PLAY) {
        return &g_play_storage;
    }
    if (iid == SL_IID_ANDROIDSIMPLEBUFFERQUEUE) {
        return &g_bq_storage;
    }
    if (iid == SL_IID_VOLUME) {
        return &g_volume_storage;
    }
    return &g_generic_storage;
}

static SLresult object_GetInterface(SlObjectImpl *self, const SLInterfaceID iid, void *pInterface) {
    (void)self;
    if (!pInterface) {
        return SL_RESULT_PARAMETER_INVALID;
    }

    const void *storage = storage_for_iid(iid);
    if (!storage) {
        return SL_RESULT_FEATURE_UNSUPPORTED;
    }

    *(const void **)pInterface = storage;
    return SL_RESULT_SUCCESS;
}

static void object_AbortAsyncOperation(SlObjectImpl *self) {
    (void)self;
}

static void object_Destroy(SlObjectImpl *self) {
    if (self) {
        self->state = SL_OBJECT_STATE_UNREALIZED;
    }
}

static SLresult object_RegisterCallback(SlObjectImpl *self, void *callback, void *context) {
    (void)self;
    (void)callback;
    (void)context;
    return SL_RESULT_SUCCESS;
}

static SLresult object_SetPriority(SlObjectImpl *self, SLint32 priority, SLboolean block) {
    (void)self;
    (void)priority;
    (void)block;
    return SL_RESULT_SUCCESS;
}

static SLresult object_GetPriority(SlObjectImpl *self, SLint32 *pPriority) {
    (void)self;
    if (pPriority) {
        *pPriority = 0;
    }
    return SL_RESULT_SUCCESS;
}

// ==== 引擎接口 ====

static SLresult engine_CreateAudioPlayer(void *self, SLObjectItf *pPlayer,
                                         const void *pAudioSrc, const void *pAudioSnk,
                                         SLuint32 numInterfaces,
                                         const SLInterfaceID *pInterfaceIds,
                                         const SLboolean *pInterfaceRequired) {
    (void)self;
    (void)pAudioSrc;
    (void)pAudioSnk;
    (void)numInterfaces;
    (void)pInterfaceIds;
    (void)pInterfaceRequired;
    so_logd("OpenSL: CreateAudioPlayer()");
    if (!pPlayer) {
        return SL_RESULT_PARAMETER_INVALID;
    }
    *pPlayer = (SLObjectItf)&g_player_obj.vtable;
    return SL_RESULT_SUCCESS;
}

static SLresult engine_CreateOutputMix(void *self, SLObjectItf *pMix,
                                       SLuint32 numInterfaces,
                                       const SLInterfaceID *pInterfaceIds,
                                       const SLboolean *pInterfaceRequired) {
    (void)self;
    (void)numInterfaces;
    (void)pInterfaceIds;
    (void)pInterfaceRequired;
    so_logd("OpenSL: CreateOutputMix()");
    if (!pMix) {
        return SL_RESULT_PARAMETER_INVALID;
    }
    *pMix = (SLObjectItf)&g_output_mix_obj.vtable;
    return SL_RESULT_SUCCESS;
}

// ==== 播放接口 ====

static SLresult play_SetPlayState(void *self, SLuint32 state) {
    (void)self;
    so_logd("OpenSL: SetPlayState(%u)", state);
    return SL_RESULT_SUCCESS;
}

static SLresult play_GetPlayState(void *self, SLuint32 *pState) {
    (void)self;
    if (pState) {
        *pState = SL_PLAYSTATE_PLAYING;
    }
    return SL_RESULT_SUCCESS;
}

// ==== Android 缓冲队列接口 ====

// 缓冲队列：引擎通过回调得知"缓冲已消费"，再补入下一个。
// 我们的桩自己按 16ms 周期驱动该回调（3200B@48k 立体声 16bit ≈ 16.7ms）。
static volatile int g_bq_pending       = 0;
static void        *g_bq_callback      = NULL;
static void        *g_bq_context       = NULL;
static SceUID       g_bq_thread        = -1;

static int bq_driver_thread(SceSize args, void *argp) {
    (void)args;
    (void)argp;

    for (;;) {
        sceKernelDelayThread(16000);

        if (!g_bq_callback || g_bq_pending <= 0) {
            continue;
        }
        g_bq_pending--;

        void (*cb)(void *, void *) = (void (*)(void *, void *))g_bq_callback;
        cb((void *)&g_bq_storage, g_bq_context);
    }

    return 0;
}

static SLresult bq_Enqueue(void *self, const void *pBuffer, SLuint32 size) {
    (void)self;
    (void)pBuffer;
    (void)size;
    g_bq_pending++;
    return SL_RESULT_SUCCESS;
}

static SLresult bq_Clear(void *self) {
    (void)self;
    return SL_RESULT_SUCCESS;
}

static SLresult bq_GetState(void *self, SLAndroidSimpleBufferQueueState *pState) {
    (void)self;
    if (pState) {
        pState->count = 0;
        pState->index = 0;
    }
    return SL_RESULT_SUCCESS;
}

static SLresult bq_RegisterCallback(void *self, void *callback, void *context) {
    (void)self;
    g_bq_callback = callback;
    g_bq_context  = context;

    // 诊断开关：0 = 不启动回调驱动线程（用于排查引擎侧死锁）
#define SL_DRIVE_CALLBACK 1
#if SL_DRIVE_CALLBACK
    if (g_bq_thread < 0) {
        g_bq_thread = sceKernelCreateThread("sl_bq", bq_driver_thread, 0x10000100, 0x2000, 0, 0, NULL);
        if (g_bq_thread >= 0) {
            sceKernelStartThread(g_bq_thread, 0, NULL);
        } else {
            so_loge("[SL] failed to create buffer queue driver thread (0x%X)", g_bq_thread);
        }
    }

#endif
    so_logd("[SL] RegisterCallback(cb=%p ctx=%p) driver=%d", callback, context, g_bq_thread);
    return SL_RESULT_SUCCESS;
}

// ==== 音量接口 ====

static SLresult volume_SetVolumeLevel(void *self, SLmillibel level) {
    (void)self;
    (void)level;
    return SL_RESULT_SUCCESS;
}

static SLresult volume_GetVolumeLevel(void *self, SLmillibel *pLevel) {
    (void)self;
    if (pLevel) {
        *pLevel = 0;
    }
    return SL_RESULT_SUCCESS;
}

static SLresult volume_GetMaxVolumeLevel(void *self, SLmillibel *pMaxLevel) {
    (void)self;
    if (pMaxLevel) {
        *pMaxLevel = 0;
    }
    return SL_RESULT_SUCCESS;
}

static SLresult volume_SetMute(void *self, SLboolean mute) {
    (void)self;
    (void)mute;
    return SL_RESULT_SUCCESS;
}

static SLresult volume_GetMute(void *self, SLboolean *pMute) {
    (void)self;
    if (pMute) {
        *pMute = SL_BOOLEAN_FALSE;
    }
    return SL_RESULT_SUCCESS;
}

// ==== 虚表（槽位顺序与 SLES 头文件一致） ====

static const void *const g_object_vtable[SLOT_COUNT_OBJECT] = {
    (const void *)object_Realize,      // 0
    (const void *)object_Resume,       // 1
    (const void *)object_GetState,     // 2
    (const void *)object_GetInterface, // 3
    (const void *)object_RegisterCallback,
    (const void *)object_AbortAsyncOperation,
    (const void *)object_Destroy,
    (const void *)object_SetPriority,
    (const void *)object_GetPriority,
};

static const void *const g_engine_vtable[SLOT_COUNT_ENGINE] = {
    SLOT_NOOP,                        // 0  CreateLEDDevice
    SLOT_NOOP,                        // 1  CreateVibraDevice
    (const void *)engine_CreateAudioPlayer,  // 2
    SLOT_NOOP,                        // 3  CreateAudioRecorder
    SLOT_NOOP,                        // 4  CreateMidiPlayer
    SLOT_NOOP,                        // 5  CreateListener
    SLOT_NOOP,                        // 6  Create3DGroup
    (const void *)engine_CreateOutputMix,    // 7
    SLOT_NOOP,                        // 8
    SLOT_NOOP,                        // 9
    SLOT_NOOP,                        // 10
    SLOT_NOOP,                        // 11
    SLOT_NOOP,                        // 12
    SLOT_NOOP,                        // 13
    SLOT_NOOP,                        // 14
};

static const void *const g_play_vtable[SLOT_COUNT_PLAY] = {
    (const void *)play_SetPlayState,  // 0
    (const void *)play_GetPlayState,  // 1
    SLOT_NOOP,                        // 2  GetDuration
    SLOT_NOOP,                        // 3  GetPosition
    SLOT_NOOP,                        // 4  RegisterCallback
    SLOT_NOOP,                        // 5  SetCallbackEventsMask
    SLOT_NOOP,                        // 6  GetCallbackEventsMask
    SLOT_NOOP,                        // 7
    SLOT_NOOP,                        // 8
    SLOT_NOOP,                        // 9
    SLOT_NOOP,                        // 10
    SLOT_NOOP,                        // 11
};

static const void *const g_bq_vtable[SLOT_COUNT_BQ] = {
    (const void *)bq_Enqueue,           // 0
    (const void *)bq_Clear,             // 1
    (const void *)bq_GetState,          // 2
    (const void *)bq_RegisterCallback,  // 3
};

static const void *const g_volume_vtable[SLOT_COUNT_VOLUME] = {
    (const void *)volume_SetVolumeLevel,   // 0
    (const void *)volume_GetVolumeLevel,   // 1
    (const void *)volume_GetMaxVolumeLevel,// 2
    (const void *)volume_SetMute,          // 3
    (const void *)volume_GetMute,          // 4
    SLOT_NOOP,                             // 5
    SLOT_NOOP,                             // 6
    SLOT_NOOP,                             // 7
    SLOT_NOOP,                             // 8
};

// 未知接口：一张足够大的空实现表
static const void *const g_generic_vtable[16] = {
    SLOT_NOOP, SLOT_NOOP, SLOT_NOOP, SLOT_NOOP, SLOT_NOOP, SLOT_NOOP, SLOT_NOOP, SLOT_NOOP,
    SLOT_NOOP, SLOT_NOOP, SLOT_NOOP, SLOT_NOOP, SLOT_NOOP, SLOT_NOOP, SLOT_NOOP, SLOT_NOOP,
};

static void opensl_init_once(void) {
    g_engine_obj.vtable          = g_object_vtable;
    g_output_mix_obj.vtable      = g_object_vtable;
    g_player_obj.vtable          = g_object_vtable;
    g_engine_obj.state           = SL_OBJECT_STATE_UNREALIZED;
    g_output_mix_obj.state       = SL_OBJECT_STATE_UNREALIZED;
    g_player_obj.state           = SL_OBJECT_STATE_UNREALIZED;

    g_engine_storage  = g_engine_vtable;
    g_play_storage    = g_play_vtable;
    g_bq_storage      = g_bq_vtable;
    g_volume_storage  = g_volume_vtable;
    g_generic_storage = g_generic_vtable;
}

SLresult SLAPIENTRY slCreateEngine(SLObjectItf *pEngine, SLuint32 numOptions,
                                   const SLEngineOption *pEngineOptions,
                                   SLuint32 numInterfaces,
                                   const SLInterfaceID *pInterfaceIds,
                                   const SLboolean *pInterfaceRequired) {
    (void)numOptions;
    (void)pEngineOptions;
    (void)numInterfaces;
    (void)pInterfaceIds;
    (void)pInterfaceRequired;

    so_logd("OpenSL: slCreateEngine(%p)", pEngine);
    if (!pEngine) {
        return SL_RESULT_PARAMETER_INVALID;
    }

    opensl_init_once();
    *pEngine = (SLObjectItf)&g_engine_obj.vtable;
    return SL_RESULT_SUCCESS;
}
