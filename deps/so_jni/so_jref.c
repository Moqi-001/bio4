#include <stdlib.h>

#include <so_common/so_shared_buffer.h>

#include "so_jni.h"
#include "so_jni_internal.h"

struct so_jref {
    jobjectRefType type;   // 引用类型
    jobject        obj;    // JNI 对象
    so_jframe     *frame;  // 所属帧
    so_jref       *prev;
    so_jref       *next;
};

struct so_jframe {
    so_jref   *refs;      // 局部引用链表
    jint       capacity;  // 局部引用容量
    jint       count;     // 当前已用数量
    so_jframe *prev;
};

static so_jframe *_so_jframe_global = NULL;
static so_jframe *_so_jframe_local  = NULL;

static so_jref *so_jref_new(jobject object, jobjectRefType type, so_jframe *frame) {
    so_jref *ref = (so_jref *)so_shared_buffer_alloc(sizeof(so_jref));
    if (!ref) return NULL;

    ref->prev  = NULL;
    ref->next  = NULL;
    ref->type  = type;
    ref->frame = frame;
    ref->obj   = object;

    return ref;
}

static jint so_jref_retain(so_jref *ref) {
    if (!ref) return JNI_EINVAL;

    so_shared_buffer_retain(ref);
    return JNI_OK;
}

static int32_t so_jref_release(so_jref *ref) {
    int32_t prev = so_shared_buffer_release(ref);
    return prev;
}

static void so_jref_release_force(so_jref *ref) {
    so_shared_buffer_release_force(ref);
}

jobjectRefType so_jref_get_type(so_jref *ref) {
    if (!ref) return JNIInvalidRefType;

    return ref->type;
}

jint so_jref_take_local(jobject object) {
    if (!object || !_so_jframe_local) return JNI_ERR;

    // 检查是否已经在本地帧里
    so_jref *ref = so_jobject_get_ref(object);
    if (ref && ref->frame == _so_jframe_local) {
        // 已存在无需重复添加，直接返回成功
        return JNI_OK;
    }

    // 唯一引用才可注册
    jobject edit_obj = so_jobject_attempt_edit(object);
    if (!edit_obj) {
        return JNI_ERR;
    }

    // 创建新 ref
    so_jref *new_ref = so_jref_new(edit_obj, JNILocalRefType, _so_jframe_local);
    if (!new_ref) {
        return JNI_ERR;
    }

    so_jobject_set_ref(edit_obj, new_ref);

    // 添加到本地帧
    if (!_so_jframe_local->refs) {
        _so_jframe_local->refs = new_ref;
    } else {
        _so_jframe_local->refs->prev = new_ref;
        new_ref->next                = _so_jframe_local->refs;
        _so_jframe_local->refs       = new_ref;
    }

    return JNI_OK;
}

jobject so_jref_new_local(jobject object) {
    if (!object || !_so_jframe_local) return NULL;

    // 检查是否已经在本地帧里
    so_jref *ref = so_jobject_get_ref(object);
    if (ref && ref->frame == _so_jframe_local) {
        // 已存在无需重复添加，直接引用并返回原对象
        so_jref_retain(ref);
        return object;
    }

    // 获取新引用
    jobject new_obj = so_jobject_retain(object);
    if (!new_obj) {
        return NULL;
    }
    // 编辑新引用
    jobject edit_obj = so_jobject_edit(new_obj);
    if (!edit_obj) {
        so_jobject_release(new_obj);
        return NULL;
    }

    // 创建新 ref
    so_jref *new_ref = so_jref_new(edit_obj, JNILocalRefType, _so_jframe_local);
    if (!new_ref) {
        return NULL;
    }

    so_jobject_set_ref(edit_obj, new_ref);

    // 添加到本地帧
    if (!_so_jframe_local->refs) {
        _so_jframe_local->refs = new_ref;
    } else {
        _so_jframe_local->refs->prev = new_ref;
        new_ref->next                = _so_jframe_local->refs;
        _so_jframe_local->refs       = new_ref;
    }

    return edit_obj;
}

void so_jref_delete_local(jobject object) {
    if (!object) return;

    so_jref *ref = so_jobject_get_ref(object);
    if (!ref || ref->frame != _so_jframe_local) return;

    so_jref *ref_prev = ref->prev;
    so_jref *ref_next = ref->next;

    int32_t prev = so_jref_release(ref);
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
        so_jobject_set_ref(object, NULL);
        so_jobject_release(object);
    }
}

jobject so_jref_new_global(jobject object) {
    if (!object || !_so_jframe_global) return NULL;

    // 检查是否已经在全局帧里
    so_jref *ref = so_jobject_get_ref(object);
    if (ref && ref->frame == _so_jframe_global) {
        // 已存在无需重复添加，直接引用返回原对象
        so_jref_retain(ref);
        return object;
    }

    // 获取新引用
    jobject new_obj = so_jobject_retain(object);
    if (!new_obj) {
        return NULL;
    }
    // 编辑新引用
    jobject edit_obj = so_jobject_edit(new_obj);
    if (!edit_obj) {
        so_jobject_release(new_obj);
        return NULL;
    }

    // 创建新 ref
    so_jref *new_ref = so_jref_new(edit_obj, JNIGlobalRefType, _so_jframe_global);
    if (!new_ref) {
        return NULL;
    }

    so_jobject_set_ref(edit_obj, new_ref);

    // 添加到全局帧
    if (!_so_jframe_global->refs) {
        _so_jframe_global->refs = new_ref;
    } else {
        _so_jframe_global->refs->prev = new_ref;
        new_ref->next                 = _so_jframe_global->refs;
        _so_jframe_global->refs       = new_ref;
    }

    return edit_obj;
}

void so_jref_delete_global(jobject object) {
    if (!object) return;

    so_jref *ref = so_jobject_get_ref(object);
    if (!ref || ref->frame != _so_jframe_global) return;

    so_jref *ref_prev = ref->prev;
    so_jref *ref_next = ref->next;

    int32_t prev = so_jref_release(ref);
    if (prev == 1) {
        // ref 被销毁了，要从 frame 里移除
        if (ref_prev) {
            ref_prev->next = ref_next;
        }
        if (ref_next) {
            ref_next->prev = ref_prev;
        }
        if (_so_jframe_global->refs == ref) {
            // 更新 frame 的 refs
            _so_jframe_global->refs = ref_next;
        }

        // 释放 object 引用
        so_jobject_set_ref(object, NULL);
        so_jobject_release(object);
    }
}

jint so_jref_push_local_frame(jint capacity) {
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

jobject so_jref_pop_local_frame(jobject result) {
    so_jframe *frame = _so_jframe_local;
    if (!frame) {
        return NULL;
    }

    // 需要先切回上一帧，才能将 result 注册到上一帧
    _so_jframe_local  = frame->prev;
    jobject preserved = so_jref_new_local(result);

    // 释放本帧的所有局部引用
    so_jref *ref = frame->refs;
    while (ref) {
        so_jref *next = ref->next;
        so_jobject_set_ref(ref->obj, NULL);
        so_jobject_release(ref->obj);
        so_jref_release_force(ref);  // 弹帧时不管还有多少引用，强制释放 jref （虚拟引用，非真实对象）
        ref = next;
    }

    free(frame);

    return preserved;
}

jint so_jref_global_init(void) {
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

void so_jref_global_fini(void) {
    if (!_so_jframe_global) {
        return;
    }

    so_jframe *frame  = _so_jframe_global;
    _so_jframe_global = NULL;

    // 释放帧内所有引用
    so_jref *ref = frame->refs;
    while (ref) {
        so_jref *next = ref->next;
        so_jobject_release_force(ref->obj);
        so_jref_release_force(ref);
        ref = next;
    }
    free(frame);
}
