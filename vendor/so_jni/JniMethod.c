#include <stdlib.h>

#include "so_jni_internal.h"

static jint parse_signature(JniMethod *method) {
    const char *sig = method->signature;

    if (!sig || sig[0] != '(') {
        return JNI_ERR;
    }

    jint        capacity = 32;
    jint        count    = 0;
    const char *p        = sig + 1;

    JniArgParam *params = (JniArgParam *)malloc(capacity * sizeof(JniArgParam));
    if (!params) {
        return JNI_ERR;
    }

    while (*p && *p != ')') {
        JniArgParam *param = &params[count];
        switch (*p) {
            case 'Z':
                param->type = JNI_TYPE_BOOLEAN;
                count++;
                p++;
                break;
            case 'B':
                param->type = JNI_TYPE_BYTE;
                count++;
                p++;
                break;
            case 'C':
                param->type = JNI_TYPE_CHAR;
                count++;
                p++;
                break;
            case 'S':
                param->type = JNI_TYPE_SHORT;
                count++;
                p++;
                break;
            case 'I':
                param->type = JNI_TYPE_INT;
                count++;
                p++;
                break;
            case 'J':
                param->type = JNI_TYPE_LONG;
                count++;
                p++;
                break;
            case 'F':
                param->type = JNI_TYPE_FLOAT;
                count++;
                p++;
                break;
            case 'D':
                param->type = JNI_TYPE_DOUBLE;
                count++;
                p++;
                break;
            case 'L':
                param->type = JNI_TYPE_OBJECT;
                count++;
                p++;
                while (*p && *p != ';') {
                    p++;
                }
                if (*p == ';') {
                    p++;
                }
                break;
            case '[':
                param->type = JNI_TYPE_ARRAY;
                count++;
                p++;
                while (*p && *p == '[') {
                    p++;
                }
                if (*p == 'L') {
                    p++;
                    while (*p && *p != ';') {
                        p++;
                    }
                    if (*p == ';') {
                        p++;
                    }
                }
                break;
            default:
                p++;
                break;
        }

        if (count >= capacity) {
            capacity *= 2;
            JniArgParam *tmp_params = (JniArgParam *)realloc(params, capacity * sizeof(JniArgParam));
            if (!tmp_params) {
                free(params);
                return JNI_ERR;
            }
            params = tmp_params;
        }
    }

    if (*p++ != ')') {
        free(params);
        return JNI_ERR;
    }

    if (count == 0) {
        free(params);
        params = NULL;
    } else if (count < capacity) {
        JniArgParam *tmp_params = (JniArgParam *)realloc(params, count * sizeof(JniArgParam));
        if (!tmp_params) {
            free(params);
            return JNI_ERR;
        }
        params = tmp_params;
    }

    JniType ret_type;
    switch (*p) {
        case 'V':
            ret_type = JNI_TYPE_VOID;
            break;
        case 'Z':
            ret_type = JNI_TYPE_BOOLEAN;
            break;
        case 'B':
            ret_type = JNI_TYPE_BYTE;
            break;
        case 'C':
            ret_type = JNI_TYPE_CHAR;
            break;
        case 'S':
            ret_type = JNI_TYPE_SHORT;
            break;
        case 'I':
            ret_type = JNI_TYPE_INT;
            break;
        case 'J':
            ret_type = JNI_TYPE_LONG;
            break;
        case 'F':
            ret_type = JNI_TYPE_FLOAT;
            break;
        case 'D':
            ret_type = JNI_TYPE_DOUBLE;
            break;
        case 'L':
            ret_type = JNI_TYPE_OBJECT;
            break;
        case '[':
            ret_type = JNI_TYPE_ARRAY;
            break;
        default:
            ret_type = JNI_TYPE_UNKNOWN;
            break;
    }

    if (ret_type == JNI_TYPE_UNKNOWN) {
        free(params);
        return JNI_ERR;
    }

    method->retType   = ret_type;
    method->argParams = params;
    method->argCount  = count;

    return JNI_OK;
}

static jint compute_layout(JniMethod *method) {
    jboolean     is_static = method->isStatic;
    JniArgParam *params    = method->argParams;
    jsize        count     = method->argCount;

    jint int_idx = is_static ? 0 : 1;  // R0~R3 索引 (非静态函数 R0 为隐藏参数 this)
#ifdef __ARM_PCS_VFP
    jint float_idx = 0;  // S0~S3 索引
#endif
    jint stack_off = 0;  // 栈偏移，从偏移8开始，前8字节要预留

    for (jint i = 0; i < count; i++) {
        JniArgParam *param = &params[i];
        JniType      type  = param->type;

#ifdef __ARM_PCS_VFP
        if (JNI_TYPE_IS_FLOAT(type)) {  // 浮点参数
            if (float_idx < 4) {
                // 寄存器 S0~S3
                if (JNI_TYPE_IS_WIDE(type)) {
                    int aligned_idx = float_idx;
                    if (aligned_idx & 1) {
                        aligned_idx++;
                    }
                    if (aligned_idx <= 2) {
                        float_idx       = aligned_idx + 2;
                        param->stackOff = -1;
                        continue;
                    }
                } else {
                    float_idx++;
                    param->stackOff = -1;
                    continue;
                }
            }
        } else {  // 整数/指针参数
#endif
            if (int_idx < 4) {
                // 寄存器 R0~R3
                if (JNI_TYPE_IS_WIDE(type)) {
                    int aligned_idx = int_idx;
                    if (aligned_idx & 1) {
                        aligned_idx++;
                    }
                    if (aligned_idx <= 2) {
                        int_idx         = aligned_idx + 2;
                        param->stackOff = -1;
                        continue;
                    }
                } else {
                    int_idx++;
                    param->stackOff = -1;
                    continue;
                }
            }
#ifdef __ARM_PCS_VFP
        }
#endif

        // 寄存器放不下，参数压栈
        if (JNI_TYPE_IS_WIDE(type)) {
            stack_off       = (stack_off + 7) & ~7;  // 8字节对齐
            param->stackOff = stack_off;
            stack_off += 8;
        } else {
            param->stackOff = stack_off;
            stack_off += 4;
        }
    }

    method->stackBytes = (stack_off + 7) & ~7;  // 8字节对齐

    return JNI_OK;
}

JniMethod *JniCreateMethod(JSymbol *symbol) {
    JniMethod *method = malloc(sizeof(JniMethod));
    if (!method) {
        return NULL;
    }

    method->name      = symbol->name;
    method->signature = symbol->signature;
    method->fnPtr     = (void *)symbol->addr;
    method->isStatic  = ((symbol->flags & JAVA_SYMBOL_FLAGS_STATIC) == JAVA_SYMBOL_FLAGS_STATIC);

    // 解析签名
    if (parse_signature(method) != JNI_OK) {
        free(method);
        return NULL;
    }

    // 计算布局
    if (compute_layout(method) != JNI_OK) {
        free(method->argParams);
        free(method);
        return NULL;
    }

    return method;
}

jmethodID JniGetMethodID(const jclass clazz, const char *name, const char *sig) {
    JSymbol *symbol = (JSymbol *)Class_findMethodSymbol(JniObjectToJava(clazz), name, sig, JNI_FALSE);
    if (!symbol) {
        return NULL;
    }

    if (symbol->method) {
        return (jmethodID)symbol->method;
    }

    JniMethod *method = JniCreateMethod(symbol);
    if (!method) {
        return NULL;
    }

    symbol->method = (JMethod)method;

    return (jmethodID)symbol->method;
}

jmethodID JniGetStaticMethodID(const jclass clazz, const char *name, const char *sig) {
    JSymbol *symbol = (JSymbol *)Class_findMethodSymbol(JniObjectToJava(clazz), name, sig, JNI_TRUE);
    if (!symbol) {
        return NULL;
    }

    if (symbol->method) {
        return (jmethodID)symbol->method;
    }

    JniMethod *method = JniCreateMethod(symbol);
    if (!method) {
        return NULL;
    }

    symbol->method = (JMethod)method;

    return (jmethodID)symbol->method;
}