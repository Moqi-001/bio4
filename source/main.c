#include <stdio.h>

#include <falso_jni/FalsoJNI.h>
#include <psp2/kernel/threadmgr.h>
#include <so_util/so_util.h>

#include "config.h"
#include "init.h"
#include "reimpl/controls.h"
#include "utils/glutil.h"
#include "utils/logger.h"
#include "patch.h"

#define RES_PATH "/com.acrodea.gamecenter/res"

#ifdef USE_SCELIBC_IO
int sceLibcHeapSize = 4 * 1024 * 1024;
#endif
int _newlib_heap_size_user = 256 * 1024 * 1024;
#define SCREEN_W 960
#define SCREEN_H 544
so_module so_mod;
so_module libmc_so_mod;

jobjectArray JobjectArray;
int StartOffset;
char *dspUserID;

int main_thread(SceSize args, void *argp)
{
    debugPrintf("native_onDrawFrame strart \n");
    int (*native_onDrawFrame)(void *, int) = (void *)so_symbol(&so_mod, "_ZN9Bio4_Main11onDrawFrameEP7_JNIEnvP8_jobjectS3_");
    int (*native_setStereoHard)(void *, int, int, char) = (void *)so_symbol(&so_mod, "_ZN9Bio4_Main13setStereoHardEP7_JNIEnvP8_jobjectS3_h");
    int fpsLock = 44;
    uint32_t last_render_time = sceKernelGetProcessTimeLow();
    uint32_t delta = (1000000 / (fpsLock + 1));

    while (1)
    {
        //controls_poll();
        if (native_onDrawFrame(&jni, 0) == 1)
        {
            debugPrintf("native_setStereoHard strart %p\n", native_setStereoHard);
            if (native_setStereoHard == 0)
            {
                native_setStereoHard = LOAD_ADDRESS + 0xD1258 + 1 + 0x1000000;
            }
            native_setStereoHard(&StartOffset, 0, 0, 0);
            debugPrintf("native_setStereoHard end \n");
        }

        while (sceKernelGetProcessTimeLow() - last_render_time < delta)
        {
            sched_yield();
        }
        last_render_time = sceKernelGetProcessTimeLow();
        gl_swap();
    }
    debugPrintf("RenderUpdate over \n");
    return 0;
}

jobjectArray JobjectArray;

int main()
{
    remove(LOG_PATH);
    soloader_init_all();
    int (*JNI_OnLoad)(void *jvm, int reserved) = (void *)so_symbol(&so_mod, "JNI_OnLoad");
    JNI_OnLoad(&jvm, 0);
    debugPrintf("JNI_OnLoad over \n");

    int (*JNI_OnLoad0)(void *jvm, int reserved) = (void *)so_symbol(&libmc_so_mod, "JNI_OnLoad");
    JNI_OnLoad0(&jvm, 0);
    debugPrintf("JNI_OnLoad0 over \n");

    gl_init();
    eglSwapInterval(0, 2);

    int (*native_onCreate)(void *, int) = (void *)so_symbol(&so_mod, "_ZN9Bio4_Main8onCreateEP7_JNIEnvP8_jobjectS3_");
    native_onCreate(&jni, 0);
    debugPrintf("native_onCreate over \n");

    int (*native_onStart)(void *, int) = (void *)so_symbol(&so_mod, "_ZN9Bio4_Main7onStartEP7_JNIEnvP8_jobject");
    native_onStart(&jni, 0);
    debugPrintf("native_onStart over \n");

    int (*native_onSurfaceCreated)(void *, int, int, int, int, int, char) = (void *)so_symbol(&so_mod, "_ZN9Bio4_Main16onSurfaceCreatedEP7_JNIEnvP8_jobjectS3_S3_ii");
    native_onSurfaceCreated(&jni, 0, 0, 0, SCREEN_W, SCREEN_H, 0);
    debugPrintf("native_onSurfaceCreated over \n");

    int (*native_onSurfaceChanged)(void *, int, int, int, int) = (void *)so_symbol(&so_mod, "_ZN9Bio4_Main16onSurfaceChangedEP7_JNIEnvP8_jobjectS3_ii");
    native_onSurfaceChanged(&jni, 0, 0, SCREEN_W, SCREEN_H);
    debugPrintf("native_onSurfaceChanged over \n");

    SceUID thid = sceKernelCreateThread("main_thread", (SceKernelThreadEntry)main_thread, 0x40, 2 * 128 * 1024, 0, 0, NULL);
    sceKernelStartThread(thid, 0, NULL);
    return sceKernelExitDeleteThread(0);
}
