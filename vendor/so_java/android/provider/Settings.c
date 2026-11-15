/*
 * android/provider/Settings$Secure：
 * 游戏（如 NativeActivity 引擎）用它查询 ANDROID_ID 作设备标识：
 *   Settings$Secure.getString(resolver, "android_id") → 16 位十六进制串。
 * 安卓上该值随设备首次启动生成，这里用固定值，仅保证格式可解析
 * （游戏会把它解析成 64 位整数）。
 */
#include <stdlib.h>
#include <string.h>

#include <so_common/so_logger.h>
#include <so_java/java/lang/Class.h>
#include <so_java/java/lang/Object.h>
#include <so_java/java/lang/String.h>

#include "Settings.h"

#define ANDROID_ID "0123456789abcdef"

static JSymbol secure_symbols[] = {
    JAVA_SYMBOL_STATIC_METHOD("getString", "(Landroid/content/ContentResolver;Ljava/lang/String;)Ljava/lang/String;", &Settings_Secure_getString),
};

static JClassMeta secure_meta = {
    .name          = "android/provider/Settings$Secure",
    .super_name    = NULL,
    .symbols       = secure_symbols,
    .num_symbols   = sizeof(secure_symbols) / sizeof(secure_symbols[0]),
    .instance_size = 0,  // 纯静态方法，不需要实例
    .dtor          = NULL,
};

JString Settings_Secure_getString(ContentResolver resolver, JString key) {
    return String_newUTF(ANDROID_ID);
}

void setup_android_provider_Settings(void) {
    JavaCreateClass(&secure_meta);
}
