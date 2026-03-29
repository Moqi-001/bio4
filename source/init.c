/*
 * Copyright (C) 2021      Andy Nguyen
 * Copyright (C) 2021-2022 Rinnegatamante
 * Copyright (C) 2022-2024 Volodymyr Atamanenko
 *
 * This software may be modified and distributed under the terms
 * of the MIT license. See the LICENSE file for details.
 */

#include <string.h>

#include <falso_jni/FalsoJNI.h>
#include <fios/fios.h>
#include <psp2/appmgr.h>
#include <psp2/apputil.h>
#include <psp2/kernel/clib.h>
#include <psp2/power.h>
#include <so_util/so_util.h>

#include "config.h"
#include "init.h"
#include "utils/dialog.h"
#include "utils/glutil.h"
#include "utils/logger.h"
#include "utils/settings.h"
#include "utils/utils.h"
#include "reimpl/controls.h"

extern so_module so_mod;
extern so_module libmc_so_mod;

void soloader_init_all() {
    // Launch `app0:configurator.bin` on `-config` init param
    sceAppUtilInit(&(SceAppUtilInitParam){}, &(SceAppUtilBootParam){});
    SceAppUtilAppEventParam eventParam;
    sceClibMemset(&eventParam, 0, sizeof(SceAppUtilAppEventParam));
    sceAppUtilReceiveAppEvent(&eventParam);
    if (eventParam.type == 0x05) {
        char buffer[2048];
        sceAppUtilAppEventParseLiveArea(&eventParam, buffer);
        if (strstr(buffer, "-config"))
            sceAppMgrLoadExec("app0:/configurator.bin", NULL, NULL);
    }

    // Set default overclock values
    scePowerSetArmClockFrequency(444);
    scePowerSetBusClockFrequency(222);
    scePowerSetGpuClockFrequency(222);
    scePowerSetGpuXbarClockFrequency(166);

#ifdef USE_SCELIBC_IO
    if (fios_init(DATA_PATH) == 0)
        l_success("FIOS initialized.");
#endif

    if (!module_loaded("kubridge")) {
        l_fatal("kubridge is not loaded.");
        fatal_error("Error: kubridge.skprx is not installed.");
    }
    l_success("kubridge check passed.");

    if (!file_exists(SO_PATH)) {
        fatal_error(
            "Looks like you haven't installed the data files for this "
            "port, or they are in an incorrect location. Please make "
            "sure that you have %s file exactly at that path.",
            SO_PATH);
    }

    if (!file_exists(SO_PATH_0)) {
        fatal_error(
            "Looks like you haven't installed the data files for this "
            "port, or they are in an incorrect location. Please make "
            "sure that you have %s file exactly at that path.",
            SO_PATH_0);
    }

    if (so_file_load(&libmc_so_mod, SO_PATH_0, LOAD_ADDRESS) < 0) {
        l_fatal("libSO could not be loaded.");
        fatal_error("Error: could not load %s.", SO_PATH_0);
    }

    so_relocate(&libmc_so_mod);
    l_success("libSO relocated.");

    resolve_imports(&libmc_so_mod);
    l_success("libSO imports resolved.");

    so_flush_caches(&libmc_so_mod);
    l_success("libSO caches flushed.");

    so_initialize(&libmc_so_mod);
    l_success("libSO initialized.");

    if (so_file_load(&so_mod, SO_PATH, LOAD_ADDRESS+0x1000000) < 0) {
        l_fatal("SO could not be loaded.");
        fatal_error("Error: could not load %s.", SO_PATH);
    }

    settings_load();
    l_success("Settings loaded.");
    so_relocate(&so_mod);
    l_success("SO relocated.");

    resolve_imports(&so_mod);
    l_success("SO imports resolved.");

    so_patch();
    l_success("SO patched.");

    so_flush_caches(&so_mod);
    l_success("SO caches flushed.");

    so_initialize(&so_mod);
    l_success("SO initialized.");

    gl_preload();
    l_success("OpenGL preloaded.");

    jni_init();
    l_success("FalsoJNI initialized.");

    controls_init();
    l_success("Controls initialized.");
}
