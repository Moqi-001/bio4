#include <stdlib.h>
#include <string.h>

#include <so_common/so_logger.h>

#include "so_jni.h"
#include "so_jni_internal.h"

#define SO_JNI_IMPL_PRIVATE
#include "so_jni_impl.h"

// JavaVM function table
static const struct JNIInvokeInterface _jni_invoke_interface = {
    .DestroyJavaVM               = DestroyJavaVM,
    .AttachCurrentThread         = AttachCurrentThread,
    .DetachCurrentThread         = DetachCurrentThread,
    .GetEnv                      = GetEnv,
    .AttachCurrentThreadAsDaemon = AttachCurrentThreadAsDaemon,
};

// JNIEnv function table
static const struct JNINativeInterface _jni_native_interface = {
    .GetVersion                    = GetVersion,
    .DefineClass                   = DefineClass,
    .FindClass                     = FindClass,
    .FromReflectedMethod           = FromReflectedMethod,
    .FromReflectedField            = FromReflectedField,
    .ToReflectedMethod             = ToReflectedMethod,
    .GetSuperclass                 = GetSuperclass,
    .IsAssignableFrom              = IsAssignableFrom,
    .ToReflectedField              = ToReflectedField,
    .Throw                         = Throw,
    .ThrowNew                      = ThrowNew,
    .ExceptionOccurred             = ExceptionOccurred,
    .ExceptionDescribe             = ExceptionDescribe,
    .ExceptionClear                = ExceptionClear,
    .FatalError                    = FatalError,
    .PushLocalFrame                = PushLocalFrame,
    .PopLocalFrame                 = PopLocalFrame,
    .NewGlobalRef                  = NewGlobalRef,
    .DeleteGlobalRef               = DeleteGlobalRef,
    .DeleteLocalRef                = DeleteLocalRef,
    .IsSameObject                  = IsSameObject,
    .NewLocalRef                   = NewLocalRef,
    .EnsureLocalCapacity           = EnsureLocalCapacity,
    .AllocObject                   = AllocObject,
    .NewObject                     = NewObject,
    .NewObjectV                    = NewObjectV,
    .NewObjectA                    = NewObjectA,
    .GetObjectClass                = GetObjectClass,
    .IsInstanceOf                  = IsInstanceOf,
    .GetMethodID                   = GetMethodID,
    .CallObjectMethod              = CallObjectMethod,
    .CallObjectMethodV             = CallObjectMethodV,
    .CallObjectMethodA             = CallObjectMethodA,
    .CallBooleanMethod             = CallBooleanMethod,
    .CallBooleanMethodV            = CallBooleanMethodV,
    .CallBooleanMethodA            = CallBooleanMethodA,
    .CallByteMethod                = CallByteMethod,
    .CallByteMethodV               = CallByteMethodV,
    .CallByteMethodA               = CallByteMethodA,
    .CallCharMethod                = CallCharMethod,
    .CallCharMethodV               = CallCharMethodV,
    .CallCharMethodA               = CallCharMethodA,
    .CallShortMethod               = CallShortMethod,
    .CallShortMethodV              = CallShortMethodV,
    .CallShortMethodA              = CallShortMethodA,
    .CallIntMethod                 = CallIntMethod,
    .CallIntMethodV                = CallIntMethodV,
    .CallIntMethodA                = CallIntMethodA,
    .CallLongMethod                = CallLongMethod,
    .CallLongMethodV               = CallLongMethodV,
    .CallLongMethodA               = CallLongMethodA,
    .CallFloatMethod               = CallFloatMethod,
    .CallFloatMethodV              = CallFloatMethodV,
    .CallFloatMethodA              = CallFloatMethodA,
    .CallDoubleMethod              = CallDoubleMethod,
    .CallDoubleMethodV             = CallDoubleMethodV,
    .CallDoubleMethodA             = CallDoubleMethodA,
    .CallVoidMethod                = CallVoidMethod,
    .CallVoidMethodV               = CallVoidMethodV,
    .CallVoidMethodA               = CallVoidMethodA,
    .CallNonvirtualObjectMethod    = CallNonvirtualObjectMethod,
    .CallNonvirtualObjectMethodV   = CallNonvirtualObjectMethodV,
    .CallNonvirtualObjectMethodA   = CallNonvirtualObjectMethodA,
    .CallNonvirtualBooleanMethod   = CallNonvirtualBooleanMethod,
    .CallNonvirtualBooleanMethodV  = CallNonvirtualBooleanMethodV,
    .CallNonvirtualBooleanMethodA  = CallNonvirtualBooleanMethodA,
    .CallNonvirtualByteMethod      = CallNonvirtualByteMethod,
    .CallNonvirtualByteMethodV     = CallNonvirtualByteMethodV,
    .CallNonvirtualByteMethodA     = CallNonvirtualByteMethodA,
    .CallNonvirtualCharMethod      = CallNonvirtualCharMethod,
    .CallNonvirtualCharMethodV     = CallNonvirtualCharMethodV,
    .CallNonvirtualCharMethodA     = CallNonvirtualCharMethodA,
    .CallNonvirtualShortMethod     = CallNonvirtualShortMethod,
    .CallNonvirtualShortMethodV    = CallNonvirtualShortMethodV,
    .CallNonvirtualShortMethodA    = CallNonvirtualShortMethodA,
    .CallNonvirtualIntMethod       = CallNonvirtualIntMethod,
    .CallNonvirtualIntMethodV      = CallNonvirtualIntMethodV,
    .CallNonvirtualIntMethodA      = CallNonvirtualIntMethodA,
    .CallNonvirtualLongMethod      = CallNonvirtualLongMethod,
    .CallNonvirtualLongMethodV     = CallNonvirtualLongMethodV,
    .CallNonvirtualLongMethodA     = CallNonvirtualLongMethodA,
    .CallNonvirtualFloatMethod     = CallNonvirtualFloatMethod,
    .CallNonvirtualFloatMethodV    = CallNonvirtualFloatMethodV,
    .CallNonvirtualFloatMethodA    = CallNonvirtualFloatMethodA,
    .CallNonvirtualDoubleMethod    = CallNonvirtualDoubleMethod,
    .CallNonvirtualDoubleMethodV   = CallNonvirtualDoubleMethodV,
    .CallNonvirtualDoubleMethodA   = CallNonvirtualDoubleMethodA,
    .CallNonvirtualVoidMethod      = CallNonvirtualVoidMethod,
    .CallNonvirtualVoidMethodV     = CallNonvirtualVoidMethodV,
    .CallNonvirtualVoidMethodA     = CallNonvirtualVoidMethodA,
    .GetFieldID                    = GetFieldID,
    .GetObjectField                = GetObjectField,
    .GetBooleanField               = GetBooleanField,
    .GetByteField                  = GetByteField,
    .GetCharField                  = GetCharField,
    .GetShortField                 = GetShortField,
    .GetIntField                   = GetIntField,
    .GetLongField                  = GetLongField,
    .GetFloatField                 = GetFloatField,
    .GetDoubleField                = GetDoubleField,
    .SetObjectField                = SetObjectField,
    .SetBooleanField               = SetBooleanField,
    .SetByteField                  = SetByteField,
    .SetCharField                  = SetCharField,
    .SetShortField                 = SetShortField,
    .SetIntField                   = SetIntField,
    .SetLongField                  = SetLongField,
    .SetFloatField                 = SetFloatField,
    .SetDoubleField                = SetDoubleField,
    .GetStaticMethodID             = GetStaticMethodID,
    .CallStaticObjectMethod        = CallStaticObjectMethod,
    .CallStaticObjectMethodV       = CallStaticObjectMethodV,
    .CallStaticObjectMethodA       = CallStaticObjectMethodA,
    .CallStaticBooleanMethod       = CallStaticBooleanMethod,
    .CallStaticBooleanMethodV      = CallStaticBooleanMethodV,
    .CallStaticBooleanMethodA      = CallStaticBooleanMethodA,
    .CallStaticByteMethod          = CallStaticByteMethod,
    .CallStaticByteMethodV         = CallStaticByteMethodV,
    .CallStaticByteMethodA         = CallStaticByteMethodA,
    .CallStaticCharMethod          = CallStaticCharMethod,
    .CallStaticCharMethodV         = CallStaticCharMethodV,
    .CallStaticCharMethodA         = CallStaticCharMethodA,
    .CallStaticShortMethod         = CallStaticShortMethod,
    .CallStaticShortMethodV        = CallStaticShortMethodV,
    .CallStaticShortMethodA        = CallStaticShortMethodA,
    .CallStaticIntMethod           = CallStaticIntMethod,
    .CallStaticIntMethodV          = CallStaticIntMethodV,
    .CallStaticIntMethodA          = CallStaticIntMethodA,
    .CallStaticLongMethod          = CallStaticLongMethod,
    .CallStaticLongMethodV         = CallStaticLongMethodV,
    .CallStaticLongMethodA         = CallStaticLongMethodA,
    .CallStaticFloatMethod         = CallStaticFloatMethod,
    .CallStaticFloatMethodV        = CallStaticFloatMethodV,
    .CallStaticFloatMethodA        = CallStaticFloatMethodA,
    .CallStaticDoubleMethod        = CallStaticDoubleMethod,
    .CallStaticDoubleMethodV       = CallStaticDoubleMethodV,
    .CallStaticDoubleMethodA       = CallStaticDoubleMethodA,
    .CallStaticVoidMethod          = CallStaticVoidMethod,
    .CallStaticVoidMethodV         = CallStaticVoidMethodV,
    .CallStaticVoidMethodA         = CallStaticVoidMethodA,
    .GetStaticFieldID              = GetStaticFieldID,
    .GetStaticObjectField          = GetStaticObjectField,
    .GetStaticBooleanField         = GetStaticBooleanField,
    .GetStaticByteField            = GetStaticByteField,
    .GetStaticCharField            = GetStaticCharField,
    .GetStaticShortField           = GetStaticShortField,
    .GetStaticIntField             = GetStaticIntField,
    .GetStaticLongField            = GetStaticLongField,
    .GetStaticFloatField           = GetStaticFloatField,
    .GetStaticDoubleField          = GetStaticDoubleField,
    .SetStaticObjectField          = SetStaticObjectField,
    .SetStaticBooleanField         = SetStaticBooleanField,
    .SetStaticByteField            = SetStaticByteField,
    .SetStaticCharField            = SetStaticCharField,
    .SetStaticShortField           = SetStaticShortField,
    .SetStaticIntField             = SetStaticIntField,
    .SetStaticLongField            = SetStaticLongField,
    .SetStaticFloatField           = SetStaticFloatField,
    .SetStaticDoubleField          = SetStaticDoubleField,
    .NewString                     = NewString,
    .GetStringLength               = GetStringLength,
    .GetStringChars                = GetStringChars,
    .ReleaseStringChars            = ReleaseStringChars,
    .NewStringUTF                  = NewStringUTF,
    .GetStringUTFLength            = GetStringUTFLength,
    .GetStringUTFChars             = GetStringUTFChars,
    .ReleaseStringUTFChars         = ReleaseStringUTFChars,
    .GetArrayLength                = GetArrayLength,
    .NewObjectArray                = NewObjectArray,
    .GetObjectArrayElement         = GetObjectArrayElement,
    .SetObjectArrayElement         = SetObjectArrayElement,
    .NewBooleanArray               = NewBooleanArray,
    .NewByteArray                  = NewByteArray,
    .NewCharArray                  = NewCharArray,
    .NewShortArray                 = NewShortArray,
    .NewIntArray                   = NewIntArray,
    .NewLongArray                  = NewLongArray,
    .NewFloatArray                 = NewFloatArray,
    .NewDoubleArray                = NewDoubleArray,
    .GetBooleanArrayElements       = GetBooleanArrayElements,
    .GetByteArrayElements          = GetByteArrayElements,
    .GetCharArrayElements          = GetCharArrayElements,
    .GetShortArrayElements         = GetShortArrayElements,
    .GetIntArrayElements           = GetIntArrayElements,
    .GetLongArrayElements          = GetLongArrayElements,
    .GetFloatArrayElements         = GetFloatArrayElements,
    .GetDoubleArrayElements        = GetDoubleArrayElements,
    .ReleaseBooleanArrayElements   = ReleaseBooleanArrayElements,
    .ReleaseByteArrayElements      = ReleaseByteArrayElements,
    .ReleaseCharArrayElements      = ReleaseCharArrayElements,
    .ReleaseShortArrayElements     = ReleaseShortArrayElements,
    .ReleaseIntArrayElements       = ReleaseIntArrayElements,
    .ReleaseLongArrayElements      = ReleaseLongArrayElements,
    .ReleaseFloatArrayElements     = ReleaseFloatArrayElements,
    .ReleaseDoubleArrayElements    = ReleaseDoubleArrayElements,
    .GetBooleanArrayRegion         = GetBooleanArrayRegion,
    .GetByteArrayRegion            = GetByteArrayRegion,
    .GetCharArrayRegion            = GetCharArrayRegion,
    .GetShortArrayRegion           = GetShortArrayRegion,
    .GetIntArrayRegion             = GetIntArrayRegion,
    .GetLongArrayRegion            = GetLongArrayRegion,
    .GetFloatArrayRegion           = GetFloatArrayRegion,
    .GetDoubleArrayRegion          = GetDoubleArrayRegion,
    .SetBooleanArrayRegion         = SetBooleanArrayRegion,
    .SetByteArrayRegion            = SetByteArrayRegion,
    .SetCharArrayRegion            = SetCharArrayRegion,
    .SetShortArrayRegion           = SetShortArrayRegion,
    .SetIntArrayRegion             = SetIntArrayRegion,
    .SetLongArrayRegion            = SetLongArrayRegion,
    .SetFloatArrayRegion           = SetFloatArrayRegion,
    .SetDoubleArrayRegion          = SetDoubleArrayRegion,
    .RegisterNatives               = RegisterNatives,
    .UnregisterNatives             = UnregisterNatives,
    .MonitorEnter                  = MonitorEnter,
    .MonitorExit                   = MonitorExit,
    .GetJavaVM                     = GetJavaVM,
    .GetStringRegion               = GetStringRegion,
    .GetStringUTFRegion            = GetStringUTFRegion,
    .GetPrimitiveArrayCritical     = GetPrimitiveArrayCritical,
    .ReleasePrimitiveArrayCritical = ReleasePrimitiveArrayCritical,
    .GetStringCritical             = GetStringCritical,
    .ReleaseStringCritical         = ReleaseStringCritical,
    .NewWeakGlobalRef              = NewWeakGlobalRef,
    .DeleteWeakGlobalRef           = DeleteWeakGlobalRef,
    .ExceptionCheck                = ExceptionCheck,
    .NewDirectByteBuffer           = NewDirectByteBuffer,
    .GetDirectBufferAddress        = GetDirectBufferAddress,
    .GetDirectBufferCapacity       = GetDirectBufferCapacity,
    .GetObjectRefType              = GetObjectRefType,
};

JavaVM _java_vm = (JavaVM)&_jni_invoke_interface;
JNIEnv _jni_env = (JNIEnv)&_jni_native_interface;

static so_jclass_node *_so_jclass_head = NULL;

jint so_jni_init(void) {
    if (so_jref_global_init() != JNI_OK) {
        return -1;
    }

    return 0;
}

void so_jni_fini(void) {
    if (_so_jclass_head) {
        so_jclass_node *node = _so_jclass_head;
        while (node) {
            so_jclass_node *next = node->next;
            // so_jclass_unregister_natives(node->clazz);
            so_jobject_release(node->clazz);
            free(node);
            node = next;
        }
        _so_jclass_head = NULL;
    }

    so_jref_global_fini();
}

jclass so_jni_find_class(const char *name) {
    if (!name) return NULL;

    so_jclass_node *node = _so_jclass_head;
    while (node) {
        const char *cls_name = so_jclass_get_name(node->clazz);
        if (cls_name && strcmp(cls_name, name) == 0) {
            // so_logd("[JNI] so_jni_find_class('%s'): %p", name, node->clazz);
            return node->clazz;
        }
        node = node->next;
    }

    // so_loge("[JNI] so_jni_find_class('%s'): not found!", name);
    return NULL;
}

static jint _jni_register_class(jclass clazz, jint retain) {
    if (!clazz) return JNI_EINVAL;

    so_jclass_node *node = (so_jclass_node *)calloc(1, sizeof(so_jclass_node));
    if (!node) {
        return JNI_ENOMEM;
    }

    if (retain) {
        node->clazz = so_jobject_retain(clazz);
    } else {
        node->clazz = clazz;
    }

    if (!_so_jclass_head) {
        _so_jclass_head = node;
    } else {
        _so_jclass_head->prev = node;
        node->next            = _so_jclass_head;
        _so_jclass_head       = node;
    }

    return JNI_OK;
}

jint so_jni_register_class(jclass clazz) {
    jint result = _jni_register_class(clazz, 1);
    if (result != JNI_OK) {
        so_loge("[JNI] so_jni_register_class(%p): error! ('%s')", clazz, so_jclass_get_name(clazz));
        return result;
    }

    so_logd("[JNI] so_jni_register_class(%p): ('%s')", clazz, so_jclass_get_name(clazz));
    return JNI_OK;
}

jint so_jni_register_class_take(jclass clazz) {
    jint result = _jni_register_class(clazz, 0);
    if (result != JNI_OK) {
        so_loge("[JNI] so_jni_register_class_take(%p): error! ('%s')", clazz, so_jclass_get_name(clazz));
        return result;
    }

    so_logd("[JNI] so_jni_register_class_take(%p): ('%s')", clazz, so_jclass_get_name(clazz));
    return JNI_OK;
}

jint so_jni_register_class_from_meta(so_jclass_meta *meta) {
    jclass clazz = so_jclass_create(meta);
    if (!clazz) {
        so_loge("[JNI] so_jni_register_class_from_meta(%p): failed to create class! ('%s')", meta, meta->name);
        return JNI_ERR;
    }
    jint result = _jni_register_class(clazz, 0);
    if (result != JNI_OK) {
        so_jobject_release(clazz);
        so_loge("[JNI] so_jni_register_class_from_meta(%p): error! ('%s')", meta, meta->name);
        return result;
    }

    so_logd("[JNI] so_jni_register_class_from_meta(%p): ('%s')", meta, meta->name);
    return JNI_OK;
}

uint32_t so_jni_hash(const char *name, const char *sig) {
    uint32_t       h = 0, g;
    const uint8_t *p = (const uint8_t *)name;
    while (*p) {
        h = (h << 4) + *p++;
        if ((g = (h & 0xf0000000)) != 0)
            h ^= g >> 24;
        h &= 0x0fffffff;
    }
    p = (const uint8_t *)sig;
    while (*p) {
        h = (h << 4) + *p++;
        if ((g = (h & 0xf0000000)) != 0)
            h ^= g >> 24;
        h &= 0x0fffffff;
    }
    return h;
}
