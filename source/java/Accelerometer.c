#include <string.h>

#include <so_common/so_logger.h>
#include <so_jni/so_jni.h>

#include "Bio4PreActivity.h"

struct _Accelerometer {
    jfloat m_InputGrav;
};
typedef struct _Accelerometer *Accelerometer;

static JClass        sClass    = NULL;
static Accelerometer sInstance = NULL;

static Accelerometer getInstance() {
    JObject result = Object_retain(sInstance);
    so_logd("[Accelerometer] getInstance(): %p", result);
    return result;
}

// 设置摇晃阈值
static void SetInputGrav(Accelerometer thiz, JFloat i_grav) {
    thiz->m_InputGrav = 250.0f * i_grav;
    so_logd("[Accelerometer] SetInputGrav(%f)", i_grav);
}

void Accelerometer_onSensorChanged(JFloat currX, JFloat currY, JFloat currZ,
                                   JFloat prevX, JFloat prevY, JFloat prevZ) {
    const JFloat G = 9.80665f;
    currX *= G;
    currY *= G;
    currZ *= G;
    prevX *= G;
    prevY *= G;
    prevZ *= G;
    JFloat dx  = prevX - currX;
    JFloat dy  = prevY - currY;
    JFloat dz  = prevZ - currZ;
    JFloat len = (dx * dx) + (dy * dy) + (dz * dz);

    if (len >= sInstance->m_InputGrav) {
        Bio4PreActivity_native_onShake();
    }
}

static JSymbol class_symbols[] = {
    JAVA_SYMBOL_STATIC_METHOD("getInstance", "()Ljp/co/capcom/android/bio4_LGUplus0119/Accelerometer;", &getInstance),

    JAVA_SYMBOL_METHOD("SetInputGrav", "(F)V", &SetInputGrav),
};

static JClassMeta class_meta = {
    .name          = "jp/co/capcom/android/bio4_LGUplus0119/Accelerometer",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = sizeof(Accelerometer),
};

void setup_Accelerometer() {
    sClass                 = JavaCreateClass(&class_meta);
    sInstance              = Object_new(sClass);
    sInstance->m_InputGrav = 100.0f;
}
