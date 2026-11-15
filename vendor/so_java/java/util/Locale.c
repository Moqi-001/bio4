// java/util/Locale：
// 游戏（如 NativeActivity 引擎）用 Locale.getDefault().getLanguage() 按
// ISO 639-1 两字母码选择语言。语言/国家取固定英文环境；语言对象全局一份。
#include <stdlib.h>
#include <string.h>

#include <so_common/so_logger.h>
#include <so_java/java/lang/Class.h>
#include <so_java/java/lang/Object.h>
#include <so_java/java/lang/String.h>

#include "Locale.h"

struct _Locale {
    int mHandle;
};

static JClass  sClass    = NULL;
static JObject sDefault  = NULL;

static JString Locale_getLanguage(Locale thiz) {
    return String_newUTF("en");
}

static JString Locale_getCountry(Locale thiz) {
    return String_newUTF("US");
}

static JString Locale_toString(Locale thiz) {
    return String_newUTF("en_US");
}

static JString Locale_getDisplayLanguage(Locale thiz) {
    return String_newUTF("English");
}

static JSymbol class_symbols[] = {
    JAVA_SYMBOL_METHOD("getLanguage", "()Ljava/lang/String;", &Locale_getLanguage),
    JAVA_SYMBOL_METHOD("getCountry", "()Ljava/lang/String;", &Locale_getCountry),
    JAVA_SYMBOL_METHOD("toString", "()Ljava/lang/String;", &Locale_toString),
    JAVA_SYMBOL_METHOD("getDisplayLanguage", "()Ljava/lang/String;", &Locale_getDisplayLanguage),
    JAVA_SYMBOL_STATIC_METHOD("getDefault", "()Ljava/util/Locale;", &Locale_getDefault),
};

static JClassMeta class_meta = {
    .name          = "java/util/Locale",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = sizeof(struct _Locale),
    .dtor          = NULL,
};

Locale Locale_new() {
    return Object_new(sClass);
}

Locale Locale_getDefault() {
    return Object_retain(sDefault);
}

void setup_java_util_Locale(void) {
    sClass   = JavaCreateClass(&class_meta);
    sDefault = Object_new(sClass);
}
