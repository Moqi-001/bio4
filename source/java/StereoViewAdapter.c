#include <stddef.h>

#include <so_common/so_logger.h>
#include <so_jni/so_jni.h>

#include "Bio4PreActivity.h"
#include "java.h"

typedef struct StereoViewAdapter {
    uint32_t reserved;
} StereoViewAdapter;

jclass  StereoViewAdapter_jclass  = NULL;
jobject StereoViewAdapter_jobject = NULL;

static StereoViewAdapter *sInstance = NULL;

// static
static StereoViewAdapter *getInstance() {
    return JniCallNewLocalRef(StereoViewAdapter_jobject);
}

// "setViewMode","(Z)V"
static void setViewMode(StereoViewAdapter *thiz, so_jargs *args) {  // (jboolean)
    jboolean isFlg = SO_JARG(args, jboolean);

    so_logd("[StereoViewAdapter] setViewMode(%u)", isFlg);
    Bio4PreActivity_changeStereoHard(isFlg);
}

static so_jsymbol class_symbols[] = {
    // 实例方法
    SO_JSYMBOL_METHOD("getInstance", "()Ljp/co/capcom/android/bio4_LGUplus0119/StereoViewAdapter;", &getInstance),
    SO_JSYMBOL_METHOD("setViewMode", "(Z)V", &setViewMode),
};

static so_jclass_meta class_meta = {
    .name          = "jp/co/capcom/android/bio4_LGUplus0119/StereoViewAdapter",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = sizeof(StereoViewAdapter),
};

void StereoViewAdapter_class_init() {
    StereoViewAdapter_jclass = so_jclass_create(&class_meta);
    so_jni_register_class_take(StereoViewAdapter_jclass);
    StereoViewAdapter_jobject = so_jclass_new_instance(StereoViewAdapter_jclass);
    sInstance                 = (StereoViewAdapter *)so_jobject_get_instance(StereoViewAdapter_jobject);
}
