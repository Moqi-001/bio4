#include <stddef.h>

#include <so_common/so_logger.h>
#include <so_jni/so_jni.h>

#include "Bio4PreActivity.h"

struct _StereoViewAdapter {
    JBoolean isFlg;
};
typedef struct _StereoViewAdapter *StereoViewAdapter;

static JClass            sClass    = NULL;
static StereoViewAdapter sInstance = NULL;

static StereoViewAdapter getInstance() {
    JObject result = Object_retain(sInstance);
    so_logd("[StereoViewAdapter] getInstance(): %p", result);
    return result;
}

static void setViewMode(StereoViewAdapter thiz, JBoolean isFlg) {
    thiz->isFlg = isFlg;
    so_logd("[StereoViewAdapter] setViewMode(%u)", isFlg);
    Bio4PreActivity_changeStereoHard(isFlg);
}

static JSymbol class_symbols[] = {
    JAVA_SYMBOL_STATIC_METHOD("getInstance", "()Ljp/co/capcom/android/bio4_LGUplus0119/StereoViewAdapter;", &getInstance),

    JAVA_SYMBOL_METHOD("setViewMode", "(Z)V", &setViewMode),
};

static JClassMeta class_meta = {
    .name          = "jp/co/capcom/android/bio4_LGUplus0119/StereoViewAdapter",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = sizeof(struct _StereoViewAdapter),
};

void setup_StereoViewAdapter() {
    sClass    = JavaCreateClass(&class_meta);
    sInstance = Object_new(sClass);
}
