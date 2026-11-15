// android/content/ContentResolver：
// 游戏（如 NativeActivity 引擎）通过 Activity.getContentResolver() 取得，
// 再传给 Settings$Secure.getString() 查询 ANDROID_ID。引擎本身不消费
// resolver 的任何方法，一个占位对象即可。
#include <stdlib.h>
#include <string.h>

#include <so_common/so_logger.h>
#include <so_java/java/lang/Class.h>
#include <so_java/java/lang/Object.h>

#include "ContentResolver.h"

struct _ContentResolver {
    int mHandle;
};

static JClass sClass = NULL;

static void ContentResolver_dummy() {
}

static JSymbol class_symbols[] = {
    JAVA_SYMBOL_STATIC_METHOD("dummy", "()V", &ContentResolver_dummy),
};

static JClassMeta class_meta = {
    .name          = "android/content/ContentResolver",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = sizeof(struct _ContentResolver),
    .dtor          = NULL,
};

ContentResolver ContentResolver_new() {
    return Object_new(sClass);
}

void setup_android_content_ContentResolver(void) {
    sClass = JavaCreateClass(&class_meta);
}
