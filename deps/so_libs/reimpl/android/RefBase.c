#include <so_common/so_logger.h>

#include "RefBase.h"

struct RefBase {
    int ref_count;
};

// RefBase 构造函数
void _ZN7android7RefBaseC2Ev(RefBase *thiz) {
    so_logd("[Android][RefBase] construct(%p): unsupport!", thiz);
}

// RefBase 析构函数
void _ZN7android7RefBaseD2Ev(RefBase *thiz) {
    so_logd("[Android][RefBase] destruct(%p): unsupport!", thiz);
}

void _ZN7android7RefBase10onFirstRefEv(RefBase *thiz) {
    so_logd("[Android][RefBase] onFirstRef(%p): unsupport!", thiz);
}

void _ZN7android7RefBase15onLastStrongRefEPKv(RefBase *thiz, const void *id) {
    so_logd("[Android][RefBase] onLastStrongRef(%p, %p): unsupport!", thiz, id);
}

void _ZN7android7RefBase13onLastWeakRefEPKv(RefBase *thiz, const void *id) {
    so_logd("[Android][RefBase] onLastWeakRef(%p, %p): unsupport!", thiz, id);
}

int _ZN7android7RefBase20onIncStrongAttemptedEjPKv(RefBase *thiz, unsigned int flags, const void *id) {
    so_logd("[Android][RefBase] onIncStrongAttempted(%p, 0x%x, %p): unsupport!", thiz, flags, id);
    return 1;
}

void _ZNK7android7RefBase9incStrongEPKv(RefBase *thiz, const void *id) {
    thiz->ref_count++;
    so_logd("[Android][RefBase] incStrong(%p, %p): unsupport!", thiz, id);
}

void _ZNK7android7RefBase9decStrongEPKv(RefBase *thiz, const void *id) {
    thiz->ref_count--;
    so_logd("[Android][RefBase] decStrong(%p, %p): unsupport!", thiz, id);
}
