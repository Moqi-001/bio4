#include <pthread.h>

#include <so_common/so_logger.h>

#include "bionic_pthread.h"

int pthread_rwlockattr_init_soloader(pthread_rwlockattr_t_bionic *attr) {
    if (!attr) return EINVAL;

    pthread_rwlockattr_t *real_attr = (pthread_rwlockattr_t *)attr;

    so_logd("pthread_rwlockattr_init(%p): *attr=0x%x", attr, *attr);
    int result = pthread_rwlockattr_init(real_attr);
    so_logd("pthread_rwlockattr_init(%p): *attr=0x%x, result=%d", attr, *attr, result);
    return result;
}

int pthread_rwlockattr_destroy_soloader(pthread_rwlockattr_t_bionic *attr) {
    if (!attr) return EINVAL;

    pthread_rwlockattr_t *real_attr = (pthread_rwlockattr_t *)attr;

    so_logd("pthread_rwlockattr_destroy(%p): *attr=0x%x", attr, *attr);
    int result = pthread_rwlockattr_destroy(real_attr);
    so_logd("pthread_rwlockattr_destroy(%p): *attr=0x%x, result=%d", attr, *attr, result);
    return result;
}

int pthread_rwlockattr_setpshared_soloader(pthread_rwlockattr_t_bionic *attr, int pshared) {
    if (!attr) return EINVAL;
    so_logd("pthread_rwlockattr_setpshared(%p,%d): *attr=0x%x", attr, pshared, *attr);

    pthread_rwlockattr_t *real_attr = (pthread_rwlockattr_t *)attr;

    int result = pthread_rwlockattr_setpshared(real_attr, pshared);
    return result;
}

int pthread_rwlockattr_getpshared_soloader(pthread_rwlockattr_t_bionic *attr, int *pshared) {
    if (!attr || pshared) return EINVAL;
    so_logd("pthread_rwlockattr_getpshared(%p,%p): *attr=0x%x", attr, pshared, *attr);

    pthread_rwlockattr_t *real_attr = (pthread_rwlockattr_t *)attr;

    int result = pthread_rwlockattr_getpshared(real_attr, pshared);
    return result;
}

int pthread_rwlock_init_soloader(pthread_rwlock_t_bionic *rwlock, const pthread_rwlockattr_t_bionic *attr) {
    if (!rwlock) return EINVAL;
    so_logd("pthread_rwlock_init(%p,%p): *attr=0x%x", rwlock, attr, attr ? *attr : 0);

    pthread_rwlock_t     *real_lock = (pthread_rwlock_t *)rwlock;
    pthread_rwlockattr_t *real_attr = (pthread_rwlockattr_t *)attr;

    int result = pthread_rwlock_init(real_lock, real_attr);
    return result;
}

int pthread_rwlock_destroy_soloader(pthread_rwlock_t_bionic *rwlock) {
    pthread_rwlock_t *real_lock = (pthread_rwlock_t *)rwlock;

    int result = pthread_rwlock_destroy(real_lock);
    return result;
}

int pthread_rwlock_rdlock_soloader(pthread_rwlock_t_bionic *rwlock) {
    pthread_rwlock_t *real_lock = (pthread_rwlock_t *)rwlock;

    int result = pthread_rwlock_rdlock(real_lock);
    return result;
}

int pthread_rwlock_tryrdlock_soloader(pthread_rwlock_t_bionic *rwlock) {
    pthread_rwlock_t *real_lock = (pthread_rwlock_t *)rwlock;

    int result = pthread_rwlock_tryrdlock(real_lock);
    return result;
}

int pthread_rwlock_timedrdlock_soloader(pthread_rwlock_t_bionic *rwlock, const struct timespec *abs_timeout) {
    pthread_rwlock_t *real_lock = (pthread_rwlock_t *)rwlock;

    int result = pthread_rwlock_timedrdlock(real_lock, abs_timeout);
    return result;
}

int pthread_rwlock_wrlock_soloader(pthread_rwlock_t_bionic *rwlock) {
    pthread_rwlock_t *real_lock = (pthread_rwlock_t *)rwlock;

    int result = pthread_rwlock_wrlock(real_lock);
    return result;
}

int pthread_rwlock_trywrlock_soloader(pthread_rwlock_t_bionic *rwlock) {
    pthread_rwlock_t *real_lock = (pthread_rwlock_t *)rwlock;

    int result = pthread_rwlock_trywrlock(real_lock);
    return result;
}

int pthread_rwlock_timedwrlock_soloader(pthread_rwlock_t_bionic *rwlock, const struct timespec *abs_timeout) {
    pthread_rwlock_t *real_lock = (pthread_rwlock_t *)rwlock;

    int result = pthread_rwlock_timedwrlock(real_lock, abs_timeout);
    return result;
}

int pthread_rwlock_unlock_soloader(pthread_rwlock_t_bionic *rwlock) {
    pthread_rwlock_t *real_lock = (pthread_rwlock_t *)rwlock;

    int result = pthread_rwlock_unlock(real_lock);
    return result;
}
