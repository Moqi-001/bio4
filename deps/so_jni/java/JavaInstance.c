#include <stddef.h>
#include <string.h>

#include <so_common/so_shared_buffer.h>

#include "../jni.h"
#include "JavaInstance.h"

typedef struct _JavaInstance {
    JavaDtor_t dtor;
} _JavaInstance;

#define dataFromInstance(_inst) ((void *)((_inst) + 1))
#define instanceFromData(_data) ((_JavaInstance *)(_data) - 1)

JavaInstance JavaInstanceNew(jsize size, JavaDtor_t dtor) {
    _JavaInstance *inst = (_JavaInstance *)so_shared_buffer_alloc(sizeof(_JavaInstance) + size);
    if (!inst) {
        return NULL;
    }

    inst->dtor = dtor;

    JavaInstance instance = (JavaInstance)dataFromInstance(inst);

    return instance;
}

jint JavaInstanceRetain(JavaInstance instance) {
    if (!instance) return JNI_EINVAL;

    _JavaInstance *inst = (_JavaInstance *)instanceFromData(instance);
    so_shared_buffer_retain(inst);

    return JNI_OK;
}

int32_t JavaInstanceRelease(JavaInstance instance) {
    if (!instance) return -1;

    _JavaInstance *inst = instanceFromData(instance);
    int32_t        prev = so_shared_buffer_release_keep(inst);
    if (prev == 1) {
        if (inst->dtor) {
            inst->dtor(dataFromInstance(inst));
        }

        so_shared_buffer_release_force(inst);
    }

    return prev;
}

void JavaInstanceReleaseForce(JavaInstance instance) {
    if (!instance) return;

    _JavaInstance *inst = instanceFromData(instance);
    if (inst->dtor) {
        inst->dtor(dataFromInstance(inst));
    }

    so_shared_buffer_release_force(inst);
}
