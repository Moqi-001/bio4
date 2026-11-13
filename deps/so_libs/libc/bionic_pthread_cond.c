#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include <so_common/so_logger.h>

#include "bionic_pthread.h"
#include "bionic_pthread_internal.h"

static void bionic_condattr_to_condattr(const pthread_condattr_t_bionic *bionic_attr, pthread_condattr_t *attr) {
    int pshared;
    if (*bionic_attr == BIONIC_PTHREAD_PROCESS_SHARED) {
        pshared = PTHREAD_PROCESS_SHARED;
    } else {
        pshared = PTHREAD_PROCESS_PRIVATE;
    }

    pthread_condattr_setpshared(attr, pshared);
}

static pthread_cond_t *cond_init(pthread_cond_t_bionic *cond, const pthread_condattr_t_bionic *attr) {
    if (!cond) return NULL;

    pthread_cond_t *real_cond = malloc(sizeof(pthread_cond_t));
    if (!real_cond) {
        so_loge("pthread_cond_init(%p,%p): not enough space!", cond, attr);
        return NULL;
    }
    *real_cond = (pthread_cond_t)PTHREAD_COND_INITIALIZER;

    int result;

    if (attr) {
        pthread_condattr_t real_attr;
        pthread_condattr_init(&real_attr);
        bionic_condattr_to_condattr(attr, &real_attr);
        result = pthread_cond_init(real_cond, &real_attr);
        pthread_condattr_destroy(&real_attr);
    } else {
        result = pthread_cond_init(real_cond, NULL);
    }

    if (result != 0) {
        so_loge("pthread_cond_init(%p,%p): error=%d", cond, attr, result);
        free(real_cond);
        return NULL;
    }

    int id = pthr_add_cache(cond, real_cond);
    if (id < 0) {
        so_loge("pthread_cond_init(%p,%p): failed to add cache!", cond, attr);
        pthread_cond_destroy(real_cond);
        free(real_cond);
        return NULL;
    }

    *(int *)cond = id;
    return real_cond;
}

pthread_cond_t *bionic_cond_to_cond(pthread_cond_t_bionic *cond) {
    if (!cond) return NULL;

    int             id        = *(int *)cond;
    pthread_cond_t *real_cond = (pthread_cond_t *)pthr_find_cache(cond, id);
    if (real_cond) {
        // so_logd("bionic_cond_to_cond: cond %p has cached %p", cond, real_cond);
        return real_cond;
    }

    real_cond = cond_init(cond, NULL);
    so_logd("bionic_cond_to_cond: cond %p new cache %p", cond, real_cond);
    return real_cond;
}

int pthread_condattr_init_soloader(pthread_condattr_t_bionic *attr) {
    if (!attr) return EINVAL;

    *attr = BIONIC_PTHREAD_PROCESS_PRIVATE;
    // so_logd("pthread_condattr_init(%p): 0", attr);
    return 0;
}

int pthread_condattr_destroy_soloader(pthread_condattr_t_bionic *attr) {
    if (!attr) return EINVAL;

    *attr = 0xdeada11d;
    // so_logd("pthread_condattr_destroy(%p): 0", attr);
    return 0;
}

int pthread_condattr_setpshared_soloader(pthread_condattr_t_bionic *attr, int pshared) {
    if (!attr) return EINVAL;

    if (pshared != BIONIC_PTHREAD_PROCESS_SHARED && pshared != BIONIC_PTHREAD_PROCESS_PRIVATE)
        return EINVAL;

    *attr = pshared;
    // so_logd("pthread_condattr_setpshared(%p,%d): 0", attr, pshared);
    return 0;
}

int pthread_condattr_getpshared_soloader(pthread_condattr_t_bionic *attr, int *pshared) {
    if (!attr || !pshared) return EINVAL;

    *pshared = *attr;
    // so_logd("pthread_condattr_getpshared(%p,%p): 0", attr, pshared);
    return 0;
}

int pthread_cond_init_soloader(pthread_cond_t_bionic *cond, const pthread_condattr_t_bionic *attr) {
    if (cond_init(cond, attr) == NULL) {
        return -1;
    }

    // so_logd("pthread_cond_init(%p,%p): 0", cond, attr);
    return 0;
}

int pthread_cond_destroy_soloader(pthread_cond_t_bionic *cond) {
    if (!cond) return EINVAL;

    int             id        = *(int *)cond;
    pthread_cond_t *real_cond = (pthread_cond_t *)pthr_remove_cache(cond, id);
    if (!real_cond) return EINVAL;

    int result = pthread_cond_destroy(real_cond);
    free(real_cond);
    cond->value = 0xdeadc04d;

    // so_logd("pthread_cond_destroy(%p): %d", cond, result);
    return result;
}

int pthread_cond_broadcast_soloader(pthread_cond_t_bionic *cond) {
    pthread_cond_t *real_cond = bionic_cond_to_cond(cond);
    if (!real_cond) return EINVAL;

    int result = pthread_cond_broadcast(real_cond);
    // so_logd("pthread_cond_broadcast(%p): %d", cond, result);
    return result;
}

int pthread_cond_signal_soloader(pthread_cond_t_bionic *cond) {
    pthread_cond_t *real_cond = bionic_cond_to_cond(cond);
    if (!real_cond) return EINVAL;

    int result = pthread_cond_signal(real_cond);
    // so_logd("pthread_cond_signal(%p): %d", cond, result);
    return result;
}

int pthread_cond_wait_soloader(pthread_cond_t_bionic *cond, pthread_mutex_t_bionic *mutex) {
    pthread_cond_t  *real_cond  = bionic_cond_to_cond(cond);
    pthread_mutex_t *real_mutex = bionic_mutex_to_mutex(mutex);
    if (!real_cond || !real_mutex) return EINVAL;

    int result = pthread_cond_wait(real_cond, real_mutex);
    // so_logd("pthread_cond_wait(%p,%p): %d", cond, mutex, result);
    return result;
}

int pthread_cond_timedwait_soloader(pthread_cond_t_bionic  *cond,
                                    pthread_mutex_t_bionic *mutex,
                                    const struct timespec  *abstime) {
    pthread_cond_t  *real_cond  = bionic_cond_to_cond(cond);
    pthread_mutex_t *real_mutex = bionic_mutex_to_mutex(mutex);
    if (!real_cond || !real_mutex) return EINVAL;

    int result = pthread_cond_timedwait(real_cond, real_mutex, abstime);
    // so_logd("pthread_cond_timedwait(%p,%p,%p): %d", cond, mutex, abstime, result);
    return result;
}

int pthread_cond_timedwait_monotonic_np_soloader(pthread_cond_t_bionic  *cond,
                                                 pthread_mutex_t_bionic *mutex,
                                                 const struct timespec  *abstime) {
    pthread_cond_t  *real_cond  = bionic_cond_to_cond(cond);
    pthread_mutex_t *real_mutex = bionic_mutex_to_mutex(mutex);
    if (!real_cond || !real_mutex) return EINVAL;

    struct timespec realtime;
    struct timespec monotime;

    clock_gettime(CLOCK_REALTIME, &realtime);
    clock_gettime(CLOCK_MONOTONIC, &monotime);

    realtime.tv_sec += (abstime->tv_sec - monotime.tv_sec);
    realtime.tv_nsec += (abstime->tv_nsec - monotime.tv_nsec);
    if (realtime.tv_nsec < 0) {
        realtime.tv_sec--;
        realtime.tv_nsec += 1000000000;
    } else if (realtime.tv_nsec >= 1000000000) {
        realtime.tv_sec++;
        realtime.tv_nsec -= 1000000000;
    }

    int result = pthread_cond_timedwait(real_cond, real_mutex, &realtime);
    // so_logd("pthread_cond_timedwait_monotonic_np(%p,%p,%p): %d", cond, mutex, abstime, result);
    return result;
}

int pthread_cond_timedwait_monotonic_soloader(pthread_cond_t_bionic  *cond,
                                              pthread_mutex_t_bionic *mutex,
                                              const struct timespec  *abstime) {
    return pthread_cond_timedwait_monotonic_np_soloader(cond, mutex, abstime);
}

int pthread_cond_timedwait_relative_np_soloader(pthread_cond_t_bionic  *cond,
                                                pthread_mutex_t_bionic *mutex,
                                                struct timespec        *reltime) {
    pthread_cond_t  *real_cond  = bionic_cond_to_cond(cond);
    pthread_mutex_t *real_mutex = bionic_mutex_to_mutex(mutex);
    if (!real_cond || !real_mutex) return EINVAL;

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += reltime->tv_sec;
    ts.tv_nsec += reltime->tv_nsec;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec += 1;
        ts.tv_nsec -= 1000000000;
    }

    int result = pthread_cond_timedwait(real_cond, real_mutex, &ts);
    // so_logd("pthread_cond_timedwait_relative_np(%p,%p,%p): %d", cond, mutex, reltime, result);
    return result;
}

int pthread_cond_timeout_np_soloader(pthread_cond_t_bionic  *cond,
                                     pthread_mutex_t_bionic *mutex,
                                     unsigned                msecs) {
    pthread_cond_t  *real_cond  = bionic_cond_to_cond(cond);
    pthread_mutex_t *real_mutex = bionic_mutex_to_mutex(mutex);
    if (!real_cond || !real_mutex) return EINVAL;

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += msecs / 1000;
    ts.tv_nsec += (msecs % 1000) * 1000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec += 1;
        ts.tv_nsec -= 1000000000;
    }

    int result = pthread_cond_timedwait(real_cond, real_mutex, &ts);
    // so_logd("pthread_cond_timeout_np(%p,%p,%u): %d", cond, mutex, msecs, result);
    return result;
}
