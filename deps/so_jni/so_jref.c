#include <stdlib.h>

#include <so_common/so_shared_buffer.h>

#include "so_jni_internal.h"

static so_jframe *_so_jframe_global = NULL;
static so_jframe *_so_jframe_local  = NULL;

static so_jref *_jref_new(jobject object, jobjectRefType type, so_jframe *frame) {
    so_jref *ref = (so_jref *)so_shared_buffer_alloc(sizeof(so_jref));
    if (!ref) return NULL;

    ref->prev  = NULL;
    ref->next  = NULL;
    ref->type  = type;
    ref->frame = frame;
    ref->obj   = object;

    return ref;
}
static void _jref_retain(so_jref *ref) {
    return so_shared_buffer_retain(ref);
}

static int32_t _jref_release(so_jref *ref) {
    return so_shared_buffer_release(ref);
}

static void _jref_release_force(so_jref *ref) {
    so_shared_buffer_release_force(ref);
}

// 将 obj 添加到帧，不增加引用，object 需要是唯一引用才能成功
jint JniAddLocalRef(jobject object) {
    if (!object || !_so_jframe_local) return JNI_ERR;

    // 检查是否已经在当前帧里
    so_jref *ref = JniGetObjectRef(object);
    if (ref && ref->frame == _so_jframe_local) {
        // 已存在无需重复添加，直接返回成功
        return JNI_OK;
    }

    // 唯一引用才可添加
    jobject edit_obj = JniAttemptEditObject(object);
    if (!edit_obj) {
        return JNI_ERR;
    }

    // 创建新 ref
    so_jref *new_ref = _jref_new(edit_obj, JNILocalRefType, _so_jframe_local);
    if (!new_ref) {
        return JNI_ERR;
    }

    // 设置 obj 的 ref
    JniSetObjectRef(edit_obj, new_ref);

    // 添加 ref 到帧
    if (!_so_jframe_local->refs) {
        _so_jframe_local->refs = new_ref;
    } else {
        _so_jframe_local->refs->prev = new_ref;
        new_ref->next                = _so_jframe_local->refs;
        _so_jframe_local->refs       = new_ref;
    }

    return JNI_OK;
}

jobject JniNewLocalRef(jobject object) {
    if (!object || !_so_jframe_local) return NULL;

    // 检查是否已经在当前帧里
    so_jref *ref = JniGetObjectRef(object);
    if (ref && ref->frame == _so_jframe_local) {
        // 已存在无需重复添加，直接引用并返回原对象
        _jref_retain(ref);
        return object;
    }

    // 获取新引用
    jobject new_obj = JniRetainObject(object);
    if (!new_obj) {
        return NULL;
    }
    // 编辑新引用
    jobject edit_obj = JniEditObject(new_obj);
    if (!edit_obj) {
        JniReleaseObject(new_obj);
        return NULL;
    }

    // 创建新 ref
    so_jref *new_ref = _jref_new(edit_obj, JNILocalRefType, _so_jframe_local);
    if (!new_ref) {
        return NULL;
    }

    // 设置 obj 的 ref
    JniSetObjectRef(edit_obj, new_ref);

    // 添加 ref 到帧
    if (!_so_jframe_local->refs) {
        _so_jframe_local->refs = new_ref;
    } else {
        _so_jframe_local->refs->prev = new_ref;
        new_ref->next                = _so_jframe_local->refs;
        _so_jframe_local->refs       = new_ref;
    }

    return edit_obj;
}

void JniDeleteLocalRef(jobject object) {
    if (!object) return;

    so_jref *ref = JniGetObjectRef(object);
    if (!ref || ref->frame != _so_jframe_local) return;

    so_jref *ref_prev = ref->prev;
    so_jref *ref_next = ref->next;

    int32_t prev = _jref_release(ref);
    if (prev == 1) {
        // ref 被销毁了，要从 frame 里移除
        if (ref_prev) {
            ref_prev->next = ref_next;
        }
        if (ref_next) {
            ref_next->prev = ref_prev;
        }
        if (_so_jframe_local->refs == ref) {
            // 更新 frame 的 refs
            _so_jframe_local->refs = ref_next;
        }

        // 释放 object 引用
        JniSetObjectRef(object, NULL);
        JniReleaseObject(object);
    }
}

jobject JniNewGlobalRef(jobject object) {
    if (!object || !_so_jframe_global) return NULL;

    so_jref *ref = JniGetObjectRef(object);
    if (ref && ref->frame == _so_jframe_global) {
        _jref_retain(ref);
        return object;
    }

    jobject new_obj = JniRetainObject(object);
    if (!new_obj) {
        return NULL;
    }
    jobject edit_obj = JniEditObject(new_obj);
    if (!edit_obj) {
        JniReleaseObject(new_obj);
        return NULL;
    }

    so_jref *new_ref = _jref_new(edit_obj, JNIGlobalRefType, _so_jframe_global);
    if (!new_ref) {
        return NULL;
    }

    JniSetObjectRef(edit_obj, new_ref);

    if (!_so_jframe_global->refs) {
        _so_jframe_global->refs = new_ref;
    } else {
        _so_jframe_global->refs->prev = new_ref;
        new_ref->next                 = _so_jframe_global->refs;
        _so_jframe_global->refs       = new_ref;
    }

    return edit_obj;
}

jobject JniNewGlobalRefTake(jobject object) {
    if (JniGetObjectRefType(object) == JNIGlobalRefType) {
        return object;
    }

    jobject result = JniNewGlobalRef(object);
    JniDeleteLocalRef(object);

    return result;
}

void JniDeleteGlobalRef(jobject object) {
    if (!object) return;

    so_jref *ref = JniGetObjectRef(object);
    if (!ref || ref->frame != _so_jframe_global) return;

    so_jref *ref_prev = ref->prev;
    so_jref *ref_next = ref->next;

    int32_t prev = _jref_release(ref);
    if (prev == 1) {
        if (ref_prev) {
            ref_prev->next = ref_next;
        }
        if (ref_next) {
            ref_next->prev = ref_prev;
        }
        if (_so_jframe_global->refs == ref) {
            _so_jframe_global->refs = ref_next;
        }

        JniSetObjectRef(object, NULL);
        JniReleaseObject(object);
    }
}

jint JniPushLocalFrame(jint capacity) {
    so_jframe *frame = (so_jframe *)malloc(sizeof(so_jframe));
    if (!frame) {
        return JNI_ERR;
    }

    frame->refs      = NULL;
    frame->capacity  = capacity;
    frame->count     = 0;
    frame->prev      = _so_jframe_local;
    _so_jframe_local = frame;

    return JNI_OK;
}

jobject JniPopLocalFrame(jobject result) {
    so_jframe *frame = _so_jframe_local;
    if (!frame) {
        return NULL;
    }

    // 需要先切回上一帧，才能将 result 注册到上一帧
    _so_jframe_local  = frame->prev;
    jobject preserved = JniNewLocalRef(result);

    // 释放帧内的所有引用
    so_jref *ref = frame->refs;
    while (ref) {
        so_jref *next = ref->next;
        JniSetObjectRef(ref->obj, NULL);
        JniReleaseObject(ref->obj);
        _jref_release_force(ref);  // 弹帧时不管还有多少引用，强制释放 jref （虚拟引用，非真实对象）
        ref = next;
    }

    free(frame);

    return preserved;
}

jint JniPushGlobalFrame(void) {
    if (_so_jframe_global) {
        return JNI_OK;  // 已经存在，全局帧只需要一个
    }

    so_jframe *frame = malloc(sizeof(so_jframe));
    if (!frame) return JNI_ENOMEM;

    frame->refs       = NULL;
    frame->capacity   = 0;
    frame->count      = 0;
    frame->prev       = NULL;
    _so_jframe_global = frame;

    return JNI_OK;
}

void JniPopGlobalFrame(void) {
    if (!_so_jframe_global) {
        return;
    }

    so_jframe *frame  = _so_jframe_global;
    _so_jframe_global = NULL;

    so_jref *ref = frame->refs;
    while (ref) {
        so_jref *next = ref->next;
        JniForceReleaseObject(ref->obj);
        _jref_release_force(ref);
        ref = next;
    }
    free(frame);
}
