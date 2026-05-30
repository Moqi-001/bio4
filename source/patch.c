#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <so_common/so_logger.h>
#include <so_common/so_utils.h>
#include <so_jni/so_jni.h>
#include <so_linker/so_linker.h>

#include "java.h"
#include "java/lang/System.h"

typedef struct JavaCall JavaCall;
jstring (*JavaCall_callStaticStringMethod)(JavaCall *thiz, const char *name, const char *sig);

char *JavaCall_callStaticCharPtrMethod(JavaCall *thiz, const char *name, const char *sig, ...) { // 原函数有 bug
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

int soloader_patch_mce(so_module *mod) {
    return 0;
}

int soloader_patch_bio4(so_module *mod) {
    JavaCall_callStaticStringMethod = so_module_find_symbol(mod, "_ZN8JavaCall22callStaticStringMethodEPKcS1_z");
    so_module_hook_symbol(mod, "_ZN8JavaCall23callStaticCharPtrMethodEPKcS1_z", (void *)&JavaCall_callStaticCharPtrMethod, NULL);
    so_module_hook_symbol(mod, "_ZN8PLT_Time15GetMilliSecondsEv", (void *)&PLT_Time_GetMilliSeconds, NULL);

    return 0;
}
