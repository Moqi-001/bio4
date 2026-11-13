#include <pthread.h>
#include <string.h>

#include <so_common/so_logger.h>

#include "bionic_pthread.h"
#include "bionic_pthread_internal.h"

#define SCHED_NORMAL 0
#define SCHED_OTHER  0
#define SCHED_FIFO   1
#define SCHED_RR     2

#define PAGE_SIZE   4096
#define MINSIGSTKSZ 2048
#define SIGSTKSZ    8192

#define DEFAULT_THREAD_STACK_SIZE ((1 * 1024 * 1024) - SIGSTKSZ)

void bionic_attr_to_attr(const pthread_attr_t_bionic *bionic_attr, pthread_attr_t *attr) {
    int                detachstate;
    int                schedpolicy = bionic_attr->sched_policy;
    struct sched_param schedparam  = {.sched_priority = bionic_attr->sched_priority};
    size_t             stacksize   = bionic_attr->stack_size;

    if (bionic_attr->flags & BIONIC_PTHREAD_ATTR_FLAG_DETACHED) {
        detachstate = PTHREAD_CREATE_DETACHED;
    } else {
        detachstate = PTHREAD_CREATE_JOINABLE;
    }
    if (stacksize == 0) {
        stacksize = DEFAULT_THREAD_STACK_SIZE;
    }

    pthread_attr_setdetachstate(attr, detachstate);
    pthread_attr_setschedpolicy(attr, schedpolicy);
    pthread_attr_setschedparam(attr, &schedparam);
    if (bionic_attr->stack_base) {
        pthread_attr_setstack(attr, bionic_attr->stack_base, stacksize);
    } else {
        pthread_attr_setstacksize(attr, stacksize);
    }
}

int pthread_attr_init_soloader(pthread_attr_t_bionic *attr) {
    attr->flags          = 0;
    attr->stack_base     = NULL;
    attr->stack_size     = DEFAULT_THREAD_STACK_SIZE;
    attr->guard_size     = PAGE_SIZE;
    attr->sched_policy   = SCHED_NORMAL;
    attr->sched_priority = 0;

    // so_logd("pthread_attr_init(%p): 0", attr);
    return 0;
}

int pthread_attr_destroy_soloader(pthread_attr_t_bionic *attr) {
    memset(attr, 0x42, sizeof(pthread_attr_t_bionic));
    // so_logd("pthread_attr_destroy(%p): 0", attr);
    return 0;
}

int pthread_attr_setdetachstate_soloader(pthread_attr_t_bionic *attr, int state) {
    if (state == BIONIC_PTHREAD_CREATE_DETACHED) {
        attr->flags |= BIONIC_PTHREAD_ATTR_FLAG_DETACHED;
    } else if (state == BIONIC_PTHREAD_CREATE_JOINABLE) {
        attr->flags &= ~BIONIC_PTHREAD_ATTR_FLAG_DETACHED;
    } else {
        so_loge("pthread_attr_setdetachstate(%p,0x%x): EINVAL!", attr, state);
        return EINVAL;
    }

    // so_logd("pthread_attr_setdetachstate(%p,0x%x): 0", attr, state);
    return 0;
}

int pthread_attr_getdetachstate_soloader(pthread_attr_t_bionic const *attr, int *state) {
    *state = (attr->flags & BIONIC_PTHREAD_ATTR_FLAG_DETACHED) ? BIONIC_PTHREAD_CREATE_DETACHED : BIONIC_PTHREAD_CREATE_JOINABLE;
    // so_logd("pthread_attr_getdetachstate(%p,0x%x): state=%d, result=0", attr, state, *state);
    return 0;
}

int pthread_attr_setschedpolicy_soloader(pthread_attr_t_bionic *attr, int policy) {
    attr->sched_policy = policy;
    // so_logd("pthread_attr_setschedpolicy(%p,%d): 0", attr, policy);
    return 0;
}

int pthread_attr_getschedpolicy_soloader(pthread_attr_t_bionic const *attr, int *policy) {
    *policy = attr->sched_policy;
    // so_logd("pthread_attr_getschedpolicy(%p,%p): policy=%d, result=0", attr, policy, *policy);
    return 0;
}

int pthread_attr_setschedparam_soloader(pthread_attr_t_bionic *attr, struct sched_param const *param) {
    attr->sched_priority = param->sched_priority;
    // so_logd("pthread_attr_setschedparam(%p,%p): sched_priority=%d, result=0", attr, param, param->sched_priority);
    return 0;
}

int pthread_attr_getschedparam_soloader(pthread_attr_t_bionic const *attr, struct sched_param *param) {
    param->sched_priority = attr->sched_priority;
    // so_logd("pthread_attr_getschedparam(%p,%p): sched_priority=%d, result=0", attr, param, param->sched_priority);
    return 0;
}

int pthread_attr_setstacksize_soloader(pthread_attr_t_bionic *attr, size_t stacksize) {
    if (!attr || stacksize < PTHREAD_STACK_MIN) {
        so_loge("pthread_attr_setstacksize(%p,%u): EINVAL!", attr, stacksize);
        return EINVAL;
    }

    attr->stack_size = stacksize;
    so_logd("pthread_attr_setstacksize(%p,%u): 0", attr, stacksize);
    return 0;
}

int pthread_attr_getstacksize_soloader(pthread_attr_t_bionic const *attr, size_t *stacksize) {
    if (!attr || !stacksize) {
        so_loge("pthread_attr_getstacksize(%p,%p): EINVAL!", attr, stacksize);
        return EINVAL;
    }

    *stacksize = attr->stack_size;
    so_logd("pthread_attr_getstacksize(%p,%p): stacksize=%u", attr, stacksize, *stacksize);
    return 0;
}

int pthread_attr_setstackaddr_soloader(pthread_attr_t_bionic *attr, void *stackaddr) {
    if (!attr) {
        so_loge("pthread_attr_setstackaddr(%p,%p): EINVAL!", attr, stackaddr);
        return EINVAL;
    }

    attr->stack_base = stackaddr;
    so_logd("pthread_attr_setstackaddr(%p,%p): 0", attr, stackaddr);
    return 0;
}

int pthread_attr_getstackaddr_soloader(pthread_attr_t_bionic const *attr, void **stackaddr) {
    if (!attr || !stackaddr) {
        so_loge("pthread_attr_getstackaddr(%p,%p): EINVAL!", attr, stackaddr);
        return EINVAL;
    }

    *stackaddr = attr->stack_base;
    so_logd("pthread_attr_getstackaddr(%p,%p): stackaddr=%p", attr, stackaddr, *stackaddr);
    return 0;
}

int pthread_attr_setstack_soloader(pthread_attr_t_bionic *attr, void *stackaddr, size_t stacksize) {
    if (!attr) {
        return EINVAL;
    }

    attr->stack_base = stackaddr;
    attr->stack_size = stacksize;
    so_logd("pthread_attr_setstack(%p,%p,%u): 0", attr, stackaddr, stacksize);
    return 0;
}

int pthread_attr_getstack_soloader(pthread_attr_t_bionic const *attr, void **stackaddr, size_t *stacksize) {
    if (!attr) {
        so_loge("pthread_attr_getstack(%p,%p,%p): EINVAL!", attr, stackaddr, stacksize);
        return EINVAL;
    }

    if (stackaddr) {
        *stackaddr = attr->stack_base;
    }
    if (stacksize) {
        *stacksize = attr->stack_size;
    }
    so_logd("pthread_attr_getstack(%p,%p,%p): stackaddr=%p, stacksize=%u", attr, stackaddr, stacksize, attr->stack_base, attr->stack_size);
    return 0;
}

int pthread_attr_setguardsize_soloader(pthread_attr_t_bionic *attr, size_t guardsize) {
    if (!attr) {
        so_loge("pthread_attr_setguardsize(%p,%u): EINVAL!", attr, guardsize);
        return EINVAL;
    }

    attr->guard_size = guardsize;
    so_logd("pthread_attr_getstack(%p,%u): 0", attr, guardsize);
    return 0;
}

int pthread_attr_getguardsize_soloader(pthread_attr_t_bionic const *attr, size_t *guardsize) {
    if (!attr || !guardsize) {
        so_loge("pthread_attr_getstack(%p,%u): EINVAL!", attr, guardsize);
        return EINVAL;
    }

    *guardsize = attr->guard_size;
    so_logd("pthread_attr_getguardsize(%p,%p): %d", attr, guardsize, *guardsize);
    return 0;
}

int pthread_attr_setscope_soloader(pthread_attr_t_bionic *attr, int scope) {
    if (scope == PTHREAD_SCOPE_SYSTEM) {
        so_loge("pthread_attr_setscope(%p,%d): 0", attr, scope);
        return 0;
    }
    if (scope == PTHREAD_SCOPE_PROCESS) {
        so_loge("pthread_attr_setscope(%p,%d): error=%d", attr, scope, ENOTSUP);
        return ENOTSUP;
    }

    so_loge("pthread_attr_setscope(%p,%d): ENOSYS", attr, scope);
    return ENOSYS;
}

int pthread_attr_getscope_soloader(pthread_attr_t_bionic const *attr) {
    int scope = PTHREAD_SCOPE_SYSTEM;
    so_logd("pthread_attr_getscope(%p): %d", attr, scope);
    return scope;
}

int pthread_getattr_np_soloader(pthread_t_bionic thid, pthread_attr_t_bionic *attr) {
    so_loge("pthread_getattr_np(%p,%p): ENOSYS!", thid, attr);
    return ENOSYS;
}
