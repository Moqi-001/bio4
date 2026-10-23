#include <string.h>

#include <psp2/apputil.h>
#include <psp2/common_dialog.h>
#include <psp2/ctrl.h>
#include <psp2/io/fcntl.h>
#include <psp2/kernel/processmgr.h>
#include <psp2/power.h>
#include <psp2/sysmodule.h>
#include <psp2/touch.h>
#include <psp2/motion.h>

#include "config.h"
#include "loader.h"

int _newlib_heap_size_user = MEMORY_NEWLIB_MB * 1024 * 1024;

int main(int argc, char *argv[]) {
    SceCommonDialogConfigParam cmnDlgCfgParam;
    SceAppUtilInitParam        appUtilParam;
    SceAppUtilBootParam        appUtilBootParam;
    memset(&appUtilParam, 0, sizeof(SceAppUtilInitParam));
    memset(&appUtilBootParam, 0, sizeof(SceAppUtilBootParam));
    sceAppUtilInit(&appUtilParam, &appUtilBootParam);
    sceCommonDialogConfigParamInit(&cmnDlgCfgParam);
    sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_LANG, (int *)&cmnDlgCfgParam.language);
    sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_ENTER_BUTTON, (int *)&cmnDlgCfgParam.enterButtonAssign);
    sceCommonDialogSetConfigParam(&cmnDlgCfgParam);

    sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, SCE_TOUCH_SAMPLING_STATE_START);
    sceCtrlSetSamplingMode(SCE_CTRL_MODE_ANALOG_WIDE);
    sceMotionStartSampling();

    scePowerSetArmClockFrequency(444);
    scePowerSetBusClockFrequency(222);
    scePowerSetGpuClockFrequency(222);
    scePowerSetGpuXbarClockFrequency(166);

    sceSysmoduleLoadModule(SCE_SYSMODULE_AVPLAYER);

#ifdef LOG_PATH
    sceIoRemove(LOG_PATH);
#endif

    soloader_init();
    soloader_run();
    soloader_term();

    return sceKernelExitProcess(0);
}
