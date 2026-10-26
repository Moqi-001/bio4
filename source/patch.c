#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <psp2/apputil.h>
#include <psp2/system_param.h>
#include <so_common/so_logger.h>
#include <so_common/so_utils.h>
#include <so_jni/so_jni.h>
#include <so_linker/so_linker.h>

#include "java.h"
#include "java/lang/System.h"

typedef struct Bio4_UserData Bio4_UserData;
typedef struct JavaCall      JavaCall;

jstring (*JavaCall_callStaticStringMethod)(JavaCall *thiz, const char *name, const char *sig);

int Bio4_lang_id = 1;
int bio4_window_type = 0;

static int _get_lang_id() {
    int language = 0;
    sceAppUtilSystemParamGetInt(SCE_SYSTEM_PARAM_ID_LANG, &language);

    // 一共支持 6 种语言，特供版的语言应该都是替换了英文，如已知的中韩。
    // 除了 id0 日语 id1 外，其它的语言似乎都未翻译完整，暂不开启。
    switch (language) {
        case SCE_SYSTEM_PARAM_LANG_JAPANESE:  // 日语
            return 0;
        /*
        case SCE_SYSTEM_PARAM_LANG_FRENCH:  // 法语
            return 2;
        case SCE_SYSTEM_PARAM_LANG_SPANISH:  // 西班牙语
            return 3;
        case SCE_SYSTEM_PARAM_LANG_ITALIAN:  // 意大利语
            return 4;
        case SCE_SYSTEM_PARAM_LANG_GERMAN:  // 德语
            return 5;
        */
        case SCE_SYSTEM_PARAM_LANG_KOREAN:      // 韩语
        case SCE_SYSTEM_PARAM_LANG_CHINESE_T:   // 中文(繁体)
        case SCE_SYSTEM_PARAM_LANG_CHINESE_S:   // 中文(简体)
        case SCE_SYSTEM_PARAM_LANG_ENGLISH_US:  // 英语(美国)
        case SCE_SYSTEM_PARAM_LANG_ENGLISH_GB:  // 英语(英国)
        default:
            return 1;  // (默认)
    }
}

char *JavaCall_callStaticCharPtrMethod(JavaCall *thiz, const char *name, const char *sig, ...) {  // 原函数有 bug
    jstring     str    = JavaCall_callStaticStringMethod(thiz, name, sig);
    const char *utf8   = JniGetStringUTFChars(str, NULL);
    size_t      len    = strlen(utf8);
    char       *result = malloc(len + 1);
    strcpy(result, utf8);
    JniReleaseStringUTFChars(str, utf8);
    JniDeleteLocalRef(str);
    return result;
}

jlong PLT_Time_GetMilliSeconds(void *thiz, jclass clazz) {  // 每帧都执行，原代码通过 JNI 查找并调用方法但不保存，hook 优化
    return JSystem_currentTimeMillis();
}

int Bio4_UserData_GetLangId(Bio4_UserData *thiz) {
    return Bio4_lang_id;
}

so_hook_ref GM_CtrlLayout_Set_WindowType_hook_ref;
void        GM_CtrlLayout_Set_WindowType(void *thiz, int type, int btn) {
    if (type == 2 && btn <= 0) {
        bio4_window_type = 1;
    } else {
        bio4_window_type = type;
    }
    SO_CONTINUE(int, &GM_CtrlLayout_Set_WindowType_hook_ref, thiz, type, btn);
}

so_hook_ref GM_CtrlMain_Event_Set_MesEnd_hook_ref;
void        GM_CtrlMain_Event_Set_MesEnd(void *thiz) {
    bio4_window_type = 0;
    SO_CONTINUE(int, &GM_CtrlMain_Event_Set_MesEnd_hook_ref, thiz);
}

int soloader_patch_mce(so_module *mod) {
    return 0;
}

int soloader_patch_bio4(so_module *mod) {
    JavaCall_callStaticStringMethod = so_module_find_symbol(mod, "_ZN8JavaCall22callStaticStringMethodEPKcS1_z");
    so_module_hook_symbol(mod, "_ZN8JavaCall23callStaticCharPtrMethodEPKcS1_z", (void *)&JavaCall_callStaticCharPtrMethod, NULL);
    so_module_hook_symbol(mod, "_ZN8PLT_Time15GetMilliSecondsEv", (void *)&PLT_Time_GetMilliSeconds, NULL);
    so_module_hook_symbol(mod, "_ZNK13Bio4_UserData9GetLangIdEv", (void *)&Bio4_UserData_GetLangId, NULL);

    Bio4_lang_id = _get_lang_id();

    so_module_hook_symbol(mod, "_ZN13GM_CtrlLayout14Set_WindowTypeEll", (void *)&GM_CtrlLayout_Set_WindowType, &GM_CtrlLayout_Set_WindowType_hook_ref);
    so_module_hook_symbol(mod, "_ZN17GM_CtrlMain_Event11_Set_MesEndEv", (void *)&GM_CtrlMain_Event_Set_MesEnd, &GM_CtrlMain_Event_Set_MesEnd_hook_ref);

    return 0;
}
