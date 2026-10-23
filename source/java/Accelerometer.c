#include <string.h>

#include <so_common/so_logger.h>
#include <so_jni/so_jni.h>

#include "java.h"

typedef struct Accelerometer {
    jfloat m_InputGrav;
} Accelerometer;

static jclass         Accelerometer_jclass   = NULL;
static jobject        Accelerometer_jobject  = NULL;
static Accelerometer *Accelerometer_instance = NULL;

static jobject getInstance() {
    jobject result = JniNewLocalRef(Accelerometer_jobject);
    so_logd("[Accelerometer] getInstance(): %p", result);
    return result;
}

void Accelerometer_ctor(Accelerometer *thiz) {
    so_logd("[Accelerometer] ctor()");
}

void Accelerometer_onSensorChanged(float currX, float currY, float currZ,
                                   float prevX, float prevY, float prevZ) {
    const float G = 9.80665f;
    currX *= G;
    currY *= G;
    currZ *= G;
    prevX *= G;
    prevY *= G;
    prevZ *= G;
    float dx  = prevX - currX;
    float dy  = prevY - currY;
    float dz  = prevZ - currZ;
    float len = (dx * dx) + (dy * dy) + (dz * dz);

    if (len >= Accelerometer_instance->m_InputGrav) {
        Bio4PreActivity_native_onShake(&_jni_env, Bio4PreActivity_jobject);
    }
}

// 设置摇晃阈值
// signature: SetInputGrav(F)V
void SetInputGrav(Accelerometer *thiz, jargs *args) {
    jfloat i_grav     = JARG(args, jfloat);
    thiz->m_InputGrav = 250.0f * i_grav;

    so_logd("[Accelerometer] SetInputGrav(%f)", i_grav);
}

static jsymbol class_symbols[] = {
    // 静态方法
    JNI_SYMBOL_STATIC_METHOD("getInstance", "()Ljp/co/capcom/android/bio4_LGUplus0119/Accelerometer;", &getInstance),

    // 实例方法
    JNI_SYMBOL_METHOD("SetInputGrav", "(F)V", &SetInputGrav),
};

static jclassMeta class_meta = {
    .name          = "jp/co/capcom/android/bio4_LGUplus0119/Accelerometer",
    .super_name    = NULL,
    .symbols       = class_symbols,
    .num_symbols   = sizeof(class_symbols) / sizeof(class_symbols[0]),
    .instance_size = sizeof(Accelerometer),
};

void Accelerometer_class_init() {
    Accelerometer_jclass                = JniCreateClass(&class_meta);
    Accelerometer_jobject               = JniNewGlobalRefTake(JniAllocObject(Accelerometer_jclass));
    Accelerometer_instance              = JniGetObjectInstance(Accelerometer_jobject);
    Accelerometer_instance->m_InputGrav = 100.0f;
}