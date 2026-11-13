#include <stdatomic.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>

#include <psp2/kernel/threadmgr.h>
#include <so_common/so_logger.h>

#include "bionic_pthread.h"
#include "bionic_pthread_internal.h"

int pthread_create_soloader(pthread_t_bionic *thread, const pthread_attr_t_bionic *attr, void *(*start)(void *), void *arg) {
    int       result;
    pthread_t real_thread;
    size_t    stack_size = 0;

    if (!attr) {
        stack_size = 0;
        result     = pthread_create(&real_thread, NULL, start, arg);
    } else {
        stack_size = attr->stack_size;
        pthread_attr_t a;
        pthread_attr_init(&a);
        bionic_attr_to_attr(attr, &a);
        result = pthread_create(&real_thread, &a, start, arg);
        pthread_attr_destroy(&a);
    }

    if (result != 0) {
        so_loge("pthread_create(%p,%p,%p,%p): %d (stacksize=%u)", thread, attr, start, arg, result, stack_size);
        return result;
    }

    *(pthread_t *)thread = real_thread;
    so_logd("pthread_create(%p,%p,%p,%p): %d (stacksize=%u)", thread, attr, start, arg, result, stack_size);
    return result;
}

void pthread_exit_soloader(void *retval) {
    pthread_exit(retval);
    // so_logd("pthread_join(%p)", retval);
}

int pthread_join_soloader(pthread_t_bionic thread, void **ret_val) {
    pthread_t real_thread = (pthread_t)thread;

    int result = pthread_join(real_thread, ret_val);
    // so_logd("pthread_join(%ld,%p): %d", thread, ret_val, result);
    return result;
}

int pthread_detach_soloader(pthread_t_bionic thread) {
    pthread_t real_thread = (pthread_t)thread;

    int result = pthread_detach(thread);
    // so_logd("pthread_detach(%ld): %d", thread, result);
    return result;
}

pthread_t_bionic pthread_self_soloader() {
    pthread_t_bionic thread = (pthread_t_bionic)pthread_self();
    // so_logd("pthread_self(): %ld", thread);
    return thread;
}

int pthread_equal_soloader(pthread_t_bionic one, pthread_t_bionic two) {
    pthread_t real_one = (pthread_t)one;
    pthread_t real_two = (pthread_t)two;

    int result = pthread_equal(real_one, real_two);
    // so_logd("pthread_equal(%ld,%p): %d", one, two, result);
    return result;
}

int pthread_setschedparam_soloader(pthread_t_bionic thread, int policy, const struct sched_param *param) {
    pthread_t real_thread = (pthread_t)thread;

    int result = pthread_setschedparam(real_thread, policy, param);
    // so_logd("pthread_setschedparam(%ld,%d,%p): %d", thread, policy, param, result);
    return result;
}

int pthread_getschedparam_soloader(pthread_t_bionic thread, int *policy, struct sched_param *param) {
    pthread_t real_thread = (pthread_t)thread;

    int result = pthread_getschedparam(real_thread, policy, param);
    // so_logd("pthread_getschedparam(%ld,%p,%p): %d", thread, policy, param, result);
    return result;
}

// --------------------------------------------------------------------------------------------------

int pthread_key_create_soloader(pthread_key_t_bionic *key, void (*destructor_function)(void *)) {
    if (!key) {
        return -1;
    }

    pthread_key_t real_key;

    int result = pthread_key_create(&real_key, destructor_function);
    if (result != 0) {
        so_loge("pthread_key_create(%p,%p): error=%d", key, destructor_function, result);
        *key = -1;
        return result;
    }

    *(pthread_key_t *)key = real_key;
    // so_logd("pthread_key_create(%p,%p): %d", key, destructor_function, result);
    return result;
}

int pthread_key_delete_soloader(pthread_key_t_bionic key) {
    pthread_key_t real_key = (pthread_key_t)key;

    int result = pthread_key_delete(real_key);
    if (result != 0) {
        so_loge("pthread_key_delete(%d): error=%d", key, result);
        return result;
    }

    // so_logd("pthread_key_delete(%d): %d", key, result);
    return result;
}

int pthread_setspecific_soloader(pthread_key_t_bionic key, const void *value) {
    pthread_key_t real_key = (pthread_key_t)key;

    int result = pthread_setspecific(real_key, value);
    if (result != 0) {
        so_loge("pthread_setspecific(%d,%p): error=%d", key, value, result);
        return result;
    }

    // so_logd("pthread_setspecific(%d,%p): %d", key, value, result);
    return result;
}

void *pthread_getspecific_soloader(pthread_key_t_bionic key) {
    pthread_key_t real_key = (pthread_key_t)key;

    void *result = pthread_getspecific(real_key);
    if (result == NULL) {
        so_loge("pthread_getspecific(%d): error=%d", key, result);
        return result;
    }

    // so_logd("pthread_getspecific(%d): %p", key, result);
    return result;
}

// ---------------------------------------------------------------------

int pthread_kill_soloader(pthread_t_bionic thread, int sig) {
    pthread_t real_thread = (pthread_t)thread;

    int result = pthread_kill(real_thread, sig);
    if (result != 0) {
        so_loge("pthread_kill(%ld,%d): error=%d", thread, sig, result);
        return result;
    }

    // so_logd("pthread_kill(%ld,%d): %d", thread, sig, result);
    return result;
}

int pthread_sigmask_soloader(int how, const sigset_t *set, sigset_t *oset) {
    int result = pthread_sigmask(how, set, oset);
    if (result != 0) {
        so_loge("pthread_sigmask(%p,%p,%p): error=%d", how, set, oset, result);
        return result;
    }

    // so_logd("pthread_sigmask(%d,%p,%p): %d", how, set, oset, result);
    return result;
}

int pthread_getcpuclockid_soloader(pthread_t_bionic thread, clockid_t *clockid) {
    if (!clockid) {
        return EINVAL;
    }

    int result = 0;
    *clockid   = CLOCK_REALTIME;
    // so_logd("pthread_atfork(%ld,%p): %d", thread, clockid, result);
    return result;
}

int pthread_once_soloader(pthread_once_t_bionic *once_control, void (*init_routine)(void)) {
    if (!once_control || !init_routine) {
        return EINVAL;
    }

    if (atomic_load(once_control) == 2) {
        return 0;
    }

    int expected = 0;
    if (atomic_compare_exchange_strong(once_control, &expected, 1)) {
        init_routine();
        atomic_store(once_control, 2);
        so_logd("pthread_once(%p,%p): 0", once_control, init_routine);
        return 0;
    }

    while (atomic_load(once_control) != 2) {
        sceKernelDelayThread(1000);
    }

    return 0;
}

int pthread_setname_np_soloader(pthread_t_bionic thread, const char *thname) {
    so_loge("pthread_setname_np(%ld,%s): unimplemented!", thread, thname ? thname : "(null)");
    return ENOSYS;
}

int pthread_atfork_soloader(void (*prepare)(void), void (*parent)(void), void (*child)(void)) {
    int result = pthread_atfork(prepare, parent, child);
    if (result != 0) {
        so_loge("pthread_atfork(%p,%p,%p): error=%d", prepare, parent, child, result);
        return result;
    }

    // so_logd("pthread_atfork(%p,%p,%p): result=%d", prepare, parent, child, result);
    return result;
}

// --------------------------------------------------------------------------------------------------

void __pthread_cleanup_push_soloader(__pthread_cleanup_t_bionic *c, void (*routine)(void *), void *arg) {
    pte_cleanup_t *cleanup = (pte_cleanup_t *)c;  // 结构体成员顺序不同，但大小相同，直接强制转换
    pte_push_cleanup(cleanup, routine, arg);
    // so_logd("__pthread_cleanup_push(%p,%p,%p)", cleanup, routine, arg);
}

void __pthread_cleanup_pop_soloader(__pthread_cleanup_t_bionic *c, int execute) {
    pte_pop_cleanup(execute);
    // so_logd("__pthread_cleanup_pop(%p,%d)", cleanup, execute);
}
