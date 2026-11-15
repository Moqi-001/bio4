#include <stdarg.h>
#include <string.h>

#include <so_common/so_logger.h>

#include "JniCall_asm.h"
#include "so_jni_internal.h"

typedef union {
    va_list       v;
    const jvalue *a;
} JniArgs;

typedef struct JniCallFrame {
    // 输出参数
    jint intRegs[4];  // R0~R3
#ifdef __ARM_PCS_VFP
    jfloat floatRegs[4];  // S0~S3
#endif
    jsize  stackBytes;  // 栈大小
    jbyte *stackArgs;   // 栈参数
    void  *fnPtr;       // 目标函数指针
    jint   retKind;     // 返回类别

    // 输入参数
    JniMethod *method;
    jobject    object;
    JniArgs    args;
    jboolean   isVa;
} JniCallFrame;

extern void JniForwardMethod(JniCallFrame *args, void *result);

// ==================== 打包参数 ====================
static jint get_ret_kind(JniType type) {
    jint kind = JNI_RET_NONE;

    if (type != JNI_TYPE_VOID) {
        if (JNI_TYPE_IS_FLOAT(type)) {
            kind |= JNI_RET_FLOAT;
        } else {
            kind |= JNI_RET_INT;
        }
        if (JNI_TYPE_IS_WIDE(type)) {
            kind |= JNI_RET_WIDE;
        }
    }

    return kind;
}

jint JniMarshalCallArguments(JniCallFrame *frame) {
    JniMethod   *method     = frame->method;
    JniArgs      args       = frame->args;
    jboolean     is_va      = frame->isVa;
    JniArgParam *arg_params = method->argParams;
    jsize        arg_count  = method->argCount;

    jbyte *stack_args = frame->stackArgs;
    jint  *int_regs   = frame->intRegs;
    jint   int_idx    = 0;
#ifdef __ARM_PCS_VFP
    jfloat *float_regs = frame->floatRegs;
    jint    float_idx  = 0;
#endif

    if (!method->isStatic) {
        int_regs[int_idx++] = (jint)JniObjectToJava(frame->object);
    }

    for (jint i = 0; i < arg_count; i++) {
        JniArgParam *param = &arg_params[i];
        JniType      type  = param->type;

        if (JNI_TYPE_IS_FLOAT(type)) {  // 浮点参数
            if (JNI_TYPE_IS_WIDE(type)) {
                jdouble val = is_va ? va_arg(args.v, jdouble) : args.a[i].d;  // D

                if (param->stackOff == -1) {
#ifdef __ARM_PCS_VFP
                    union {
                        jdouble d;
                        jfloat  f[2];
                    } u;

                    u.d                     = val;
                    float_regs[float_idx++] = u.f[0];  // 低32位
                    float_regs[float_idx++] = u.f[1];  // 高32位
#else
                    union {
                        jdouble d;
                        jint    i[2];
                    } u;

                    u.d                 = val;
                    int_regs[int_idx++] = u.i[0];  // 低32位
                    int_regs[int_idx++] = u.i[1];  // 高32位
#endif
                } else {
                    memcpy(stack_args + param->stackOff, &val, 8);
                }
            } else {
                jfloat val = is_va ? (jfloat)va_arg(args.v, jdouble) : args.a[i].f;  // F

                if (param->stackOff == -1) {
#ifdef __ARM_PCS_VFP
                    float_regs[float_idx++] = val;
#else
                    union {
                        jfloat f;
                        jint   i;
                    } u;

                    u.f                 = val;
                    int_regs[int_idx++] = u.i;
#endif
                } else {
                    memcpy(stack_args + param->stackOff, &val, 4);
                }
            }
        } else {  // 整数/指针参数
            if (JNI_TYPE_IS_WIDE(type)) {
                jlong val = is_va ? va_arg(args.v, jlong) : args.a[i].j;  // J

                if (param->stackOff == -1) {
                    union {
                        jlong l;
                        jint  i[2];
                    } u;

                    u.l                 = val;
                    int_regs[int_idx++] = u.i[0];  // 低32位
                    int_regs[int_idx++] = u.i[1];  // 高32位
                } else {
                    memcpy(stack_args + param->stackOff, &val, 8);
                }
            } else {
                jint val = is_va ? va_arg(args.v, jint) : args.a[i].i;  // I
                if (JNI_TYPE_IS_OBJECT(type)) {
                    val = (jint)JniObjectToJava((jobject)val);
                }

                if (param->stackOff == -1) {
                    int_regs[int_idx++] = val;
                } else {
                    memcpy(stack_args + param->stackOff, &val, 4);
                }
            }
        }
    }

    return JNI_OK;
}

jint JniCallCommonMethod(void *result, jobject object, jmethodID methodID, ...) {
    jint ret;

    va_list args;
    va_start(args, methodID);
    ret = JniCallCommonMethodV(result, object, methodID, args);
    va_end(args);

    return ret;
}

jint JniCallCommonMethodV(void *result, jobject object, jmethodID methodID, va_list args) {
    JniMethod *method = (JniMethod *)methodID;
    if (!method) {
        return JNI_ERR;
    }

    JniCallFrame frame = {
        .method = method,
        .object = object,
        .args.v = args,
        .isVa   = JNI_TRUE,

        .stackBytes = method->stackBytes,
        .stackArgs  = NULL,
        .fnPtr      = method->fnPtr,
        .retKind    = get_ret_kind(method->retType),
    };

    JniForwardMethod(&frame, result);

    // 对象交由jni托管，java端若要保持对对象的持有则需在java端增加引用。
    if (result && JNI_TYPE_IS_OBJECT(method->retType)) {
        JObject obj        = *(JObject *)result;
        *(jobject *)result = JniNewLocalRefFromJava(obj);
        Object_release(obj);
    }

    return JNI_OK;
}

jint JniCallCommonMethodA(void *result, jobject object, jmethodID methodID, const jvalue *args) {
    JniMethod *method = (JniMethod *)methodID;
    if (!method) {
        return JNI_ERR;
    }

    JniCallFrame frame = {
        .method = method,
        .object = object,
        .args.a = args,
        .isVa   = JNI_FALSE,

        .stackBytes = method->stackBytes,
        .stackArgs  = NULL,
        .fnPtr      = method->fnPtr,
        .retKind    = get_ret_kind(method->retType),
    };

    JniForwardMethod(&frame, result);

    if (result && JNI_TYPE_IS_OBJECT(method->retType)) {
        JObject obj        = *(JObject *)result;
        *(jobject *)result = JniNewLocalRefFromJava(obj);
        Object_release(obj);
    }

    return JNI_OK;
}

jint JniCallNonvirtualCommonMethod(void *result, jobject object, jclass clazz, jmethodID methodID, ...) {
    jint ret;

    va_list args;
    va_start(args, methodID);
    ret = JniCallNonvirtualCommonMethodV(result, object, clazz, methodID, args);
    va_end(args);

    return ret;
}

jint JniCallNonvirtualCommonMethodV(void *result, jobject object, jclass clazz, jmethodID methodID, va_list args) {
    JniMethod *method = (JniMethod *)methodID;
    if (!method || method->isStatic) {
        return JNI_ERR;
    }

    jmethodID cls_methodID = JniGetMethodID(clazz, method->name, method->signature);
    if (!cls_methodID) {
        return JNI_ERR;
    }

    return JniCallCommonMethodV(result, object, cls_methodID, args);
}

jint JniCallNonvirtualCommonMethodA(void *result, jobject object, jclass clazz, jmethodID methodID, const jvalue *args) {
    JniMethod *method = (JniMethod *)methodID;
    if (!method || method->isStatic) {
        return JNI_ERR;
    }

    jmethodID cls_methodID = JniGetMethodID(clazz, method->name, method->signature);
    if (!cls_methodID) {
        return JNI_ERR;
    }

    return JniCallCommonMethodA(result, object, cls_methodID, args);
}
