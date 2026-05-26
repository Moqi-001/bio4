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

// static, "getInstance", "()Ljp/co/capcom/android/bio4_LGUplus0119/StereoViewAdapter;"
static StereoViewAdapter *getInstance() {
    return JniNewLocalRef(StereoViewAdapter_jobject);
}

// "setViewMode","(Z)V"
static void setViewMode(StereoViewAdapter *thiz, jargs *args) {  // (jboolean)
    jboolean isFlg = JARG(args, jboolean);

    so_logd("[StereoViewAdapter] setViewMode(%u)", isFlg);
    Bio4PreActivity_changeStereoHard(isFlg);
}

static jsymbol class_symbols[] = {
    // 静态方法
    JNI_SYMBOL_STATIC_METHOD("getInstance", "()Ljp/co/capcom/android/bio4_LGUplus0119/StereoViewAdapter;", &getInstance),
    // 实例方法
    JNI_SYMBOL_METHOD("setViewMode", "(Z)V", &setViewMode),
};

static jclassMeta class_meta = {
    .name          = "jp/co/capcom/android/bio4_LGUplus0119/StereoViewAdapter",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = sizeof(StereoViewAdapter),
};

void StereoViewAdapter_class_init() {
    StereoViewAdapter_jclass  = JniCreateClass(&class_meta);
    StereoViewAdapter_jobject = JniNewGlobalRefTake(JniAllocObject(StereoViewAdapter_jclass));
    sInstance                 = (StereoViewAdapter *)JniGetObjectInstance(StereoViewAdapter_jobject);
}
