#include <stddef.h>

#include <so_common/so_shared_buffer.h>

#include "so_jni.h"
#include "so_jni_internal.h"

typedef struct so_jinstance {
    so_jdtor_t dtor;
} so_jinstance;

#define dataFromInstance(_inst) ((void *)((_inst) + 1))
#define instanceFromData(_data) ((so_jinstance *)(_data) - 1)

void *so_jinstance_new(jsize size, so_jdtor_t dtor) {
    so_jinstance *inst = so_shared_buffer_alloc(sizeof(so_jinstance) + size);
    if (!inst) {
        return NULL;
    }

    inst->dtor = dtor;

    return dataFromInstance(inst);
}

jint so_jinstance_retain(void *instance) {
    if (!instance) return JNI_EINVAL;

    so_shared_buffer_retain(instanceFromData(instance));
    return JNI_OK;
}

int32_t so_jinstance_release(void *instance) {
    if (!instance) return -1;

    so_jinstance *inst = instanceFromData(instance);
    int32_t       prev = so_shared_buffer_release_keep(inst);
    // 判断是否唯一引用
    if (prev == 1) {
        // 调用析构函数
        if (inst->dtor) {
            inst->dtor(instance);
        }
        // 真正释放
        so_shared_buffer_release_force(inst);
    }

    return prev;
}

void so_jinstance_release_force(void *instance) {
    if (!instance) return;

    so_jinstance *inst = instanceFromData(instance);
    if (inst->dtor) {
        inst->dtor(instance);
    }
    so_shared_buffer_release_force(inst);
}
