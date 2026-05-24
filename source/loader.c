#include <stdio.h>
#include <string.h>

#include <psp2/kernel/processmgr.h>
#include <psp2/vshbridge.h>
#include <so_common/so_logger.h>
#include <so_jni/so_jni.h>
#include <so_linker/so_linker.h>
#include <vitaGL.h>

#include "config.h"
#include "java.h"
#include "loader.h"
#include "patch.h"
#include "utils/dialog.h"
#include "utils/utils.h"

extern so_library so_library_c;
extern so_library so_library_stdcpp;
extern so_library so_library_m;
extern so_library so_library_EGL;
extern so_library so_library_GLESv1_CM;
extern so_library so_library_log;
extern so_library so_library_dl;
extern so_library so_library_cutils;
extern so_library so_library_dvm;

so_module *mce_mod  = NULL;
so_module *bio4_mod = NULL;

static int check_kubridge() {
    int search_unk[2];
    if (_vshKernelSearchModuleByName("kubridge", search_unk) < 0) {
        fatal_error("Error: kubridge.skprx is not installed.");
        return -1;
    }
    return 0;
}

static int check_opengl() {
    if (!file_exists("ur0:/data/libshacccg.suprx") && !file_exists("ur0:/data/external/libshacccg.suprx")) {
        fatal_error(
            "Error: libshacccg.suprx is not installed. "
            "Google \"ShaRKBR33D\" for quick installation.");
        return -1;
    }
    return 0;
}

static int init_needed_modules() {
    so_linker_register_module_from_library(&so_library_c);
    so_linker_register_module_from_library(&so_library_stdcpp);
    so_linker_register_module_from_library(&so_library_m);
    so_linker_register_module_from_library(&so_library_EGL);
    so_linker_register_module_from_library(&so_library_GLESv1_CM);
    so_linker_register_module_from_library(&so_library_log);
    so_linker_register_module_from_library(&so_library_dl);
    so_linker_register_module_from_library(&so_library_cutils);
    so_linker_register_module_from_library(&so_library_dvm);

    return 0;
}

static int main_thread(SceSize args, void *argp) {
    sceKernelExitThread(0);
    return 0;
}

so_module *soloader_load_library(const char *soname) {
    char path[1024];
    snprintf(path, 1024, "%slib%s.so", SO_LIB_DIR, soname);

    so_module *mod = so_module_open(path);
    if (!mod) {
        fatal_error("Error: could not load %s.", path);
        return NULL;
    }
    so_linker_register_module(mod);

    so_logd("[LOADER] load_library success: soname='%s', load_base=%p", so_module_name(mod), so_module_load_base(mod));
    return mod;
}

int soloader_jni_onload(so_module *mod) {
    jint (*JNI_OnLoad)(JavaVM *, void *) = so_module_find_symbol(mod, "JNI_OnLoad");
    if (JNI_OnLoad) {
        if (JNI_CALL_TYPE_NATIVE(jint, JNI_OnLoad, &_java_vm, NULL) < 0) {
            fatal_error("JNI_OnLoad: error!");
            return -1;
        }
        so_logd("[LOADER] JNI_OnLoad success! ('%s')", so_module_name(mod));
    }
    return 0;
}

int soloader_init() {
    so_logd("[LOADER] soloader_init...");

    check_kubridge();
    check_opengl();
    so_linker_init();
    so_jni_init();
    init_needed_modules();
    soloader_java_init();

    mce_mod = soloader_load_library("mc_eruption_for_android_jni");
    soloader_patch_mce(mce_mod);

    bio4_mod = soloader_load_library("bio4af");
    so_logd("[LOADER] '%s' loaded: load_base=%p", so_module_name(bio4_mod), so_module_load_base(bio4_mod));
    soloader_patch_bio4(bio4_mod);

    vglInitExtended(0, 960, 544, MEMORY_VITAGL_THRESHOLD_MB * 1024 * 1024, SCE_GXM_MULTISAMPLE_4X);

    so_logd("[LOADER] soloader_init success!");

    return 0;
}

int soloader_run() {
    so_logd("[LOADER] soloader_run...");

    soloader_jni_onload(mce_mod);
    soloader_jni_onload(bio4_mod);

    Bio4PreActivity_onCreate(NULL);
    Bio4PreActivity_onStart();

    so_logd("[LOADER] soloader_run over.");

    return 0;
}

void soloader_term() {
    so_logd("[LOADER] soloader_term...");

    so_module_close(bio4_mod);
    so_module_close(mce_mod);

    so_jni_fini();
    so_linker_fini();

    so_logd("[LOADER] soloader_term success!");
}
