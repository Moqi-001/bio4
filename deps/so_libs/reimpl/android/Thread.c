#include <stddef.h>
#include <stdint.h>

#include <so_common/so_logger.h>

#include "Thread.h"

struct Thread {
    uint32_t reserved;
};

// Thread 构造函数
void _ZN7android6ThreadC2Eb(Thread *thiz, int canCallJava) {
    so_logd("[Android][Thread] construct(%p, %d): unsupport!", thiz, canCallJava);
}

// Thread 析构函数
void _ZN7android6ThreadD0Ev(Thread *thiz) {
    so_logd("[Android][Thread] destructDeleting(%p): unsupport!", thiz);
}

void _ZN7android6ThreadD1Ev(Thread *thiz) {
    so_logd("[Android][Thread] destructComplete(%p): unsupport!", thiz);
}

void _ZN7android6ThreadD2Ev(Thread *thiz) {
    so_logd("[Android][Thread] destructBase(%p): unsupport!", thiz);
}

void _ZTv0_n12_N7android6ThreadD0Ev(Thread *thiz) {
    so_logd("[Android][Thread] destructDeleting_thunk(%p): unsupport!", thiz);
}

void _ZTv0_n12_N7android6ThreadD1Ev(Thread *thiz) {
    so_logd("[Android][Thread] destructComplete_thunk(%p): unsupport!", thiz);
}

int _ZN7android6Thread10readyToRunEv(Thread *thiz) {
    so_logd("[Android][Thread] readyToRun(%p): unsupport!", thiz);
    return 1;
}

void _ZN7android6Thread11requestExitEv(Thread *thiz) {
    so_logd("[Android][Thread] requestExit(%p): unsupport!", thiz);
}

int _ZN7android6Thread3runEPKcij(Thread *thiz, const char *name, int priority, size_t stack) {
    so_logd("[Android][Thread] run(%p, %s, %d, %zu): unsupport!",
            thiz, name ? name : "NULL", priority, stack);
    return 1;
}
