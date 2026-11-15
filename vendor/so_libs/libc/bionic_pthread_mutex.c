#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#include <so_common/so_logger.h>

#include "bionic_pthread.h"
#include "bionic_pthread_internal.h"

#define BIONIC_MUTEXATTR_TYPE_MASK   0x000f
#define BIONIC_MUTEXATTR_SHARED_MASK 0x0010

static void bionic_mutexattr_to_mutexattr(const pthread_mutexattr_t_bionic *bionic_attr, pthread_mutexattr_t *attr) {
    int type;
    int pshared;

    if ((*bionic_attr & BIONIC_MUTEXATTR_SHARED_MASK) != 0) {
        pshared = PTHREAD_PROCESS_SHARED;
    } else {
        pshared = PTHREAD_PROCESS_PRIVATE;
    }

    switch (*bionic_attr & BIONIC_MUTEXATTR_TYPE_MASK) {
        case BIONIC_PTHREAD_MUTEX_RECURSIVE:
            type = PTHREAD_MUTEX_RECURSIVE;
            break;
        case BIONIC_PTHREAD_MUTEX_ERRORCHECK:
            type = PTHREAD_MUTEX_ERRORCHECK;
            break;
        case BIONIC_PTHREAD_MUTEX_NORMAL:
        default:
            type = PTHREAD_MUTEX_NORMAL;
            break;
    }

    pthread_mutexattr_settype(attr, type);
    pthread_mutexattr_setpshared(attr, pshared);
}

static pthread_mutex_t *mutex_init(pthread_mutex_t_bionic *mutex, const pthread_mutexattr_t_bionic *attr) {
    if (!mutex) {
        return NULL;
    }

    pthread_mutex_t *real_mutex = malloc(sizeof(pthread_mutex_t));
    if (!real_mutex) {
        so_loge("pthread_mutex_init(%p,%p): not enough space!", mutex, attr);
        return NULL;
    }
    *real_mutex = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;

    int                 result;
    pthread_mutexattr_t real_attr;
    pthread_mutexattr_init(&real_attr);

    if (attr) {
        bionic_mutexattr_to_mutexattr(attr, &real_attr);
        result = pthread_mutex_init(real_mutex, &real_attr);
    } else {
        int type;
        if (mutex->value == BIONIC__PTHREAD_MUTEX_INIT_VALUE) {
            type = PTHREAD_MUTEX_NORMAL;
        } else if (mutex->value == BIONIC__PTHREAD_RECURSIVE_MUTEX_INIT_VALUE) {
            type = PTHREAD_MUTEX_RECURSIVE;
        } else if (mutex->value == BIONIC__PTHREAD_ERRORCHECK_MUTEX_INIT_VALUE) {
            type = PTHREAD_MUTEX_ERRORCHECK;
        }
        pthread_mutexattr_settype(&real_attr, type);
        result = pthread_mutex_init(real_mutex, &real_attr);
    }

    pthread_mutexattr_destroy(&real_attr);

    if (result != 0) {
        so_loge("pthread_mutex_init(%p,%p): error=%d", mutex, attr, result);
        free(real_mutex);
        return NULL;
    }

    int id = pthr_add_cache(mutex, real_mutex);
    if (id < 0) {
        so_loge("pthread_mutex_init(%p,%p): failed to add cache!", mutex, attr);
        pthread_mutex_destroy(real_mutex);
        free(real_mutex);
        return NULL;
    }

    *(int *)mutex = id;
    return real_mutex;
}

pthread_mutex_t *bionic_mutex_to_mutex(pthread_mutex_t_bionic *mutex) {
    if (!mutex) {
        return NULL;
    }

    int              id         = *(int *)mutex;
    pthread_mutex_t *real_mutex = (pthread_mutex_t *)pthr_find_cache(mutex, id);
    if (real_mutex) {
        // so_logd("bionic_mutex_to_mutex: mutex %p has cached %p", mutex, real_mutex);
        return real_mutex;
    }

    real_mutex = mutex_init(mutex, NULL);
    // so_logd("bionic_mutex_to_mutex: mutex %p new cache %p", mutex, real_mutex);
    return real_mutex;
}

int pthread_mutexattr_init_soloader(pthread_mutexattr_t_bionic *attr) {
    if (!attr) {
        return EINVAL;
    }

    *attr = BIONIC_PTHREAD_MUTEX_DEFAULT;
    // so_logd("pthread_mutexattr_init(%p): 0", attr);
    return 0;
}

int pthread_mutexattr_destroy_soloader(pthread_mutexattr_t_bionic *attr) {
    if (!attr) {
        return EINVAL;
    }

    *attr = -1;
    // so_logd("pthread_mutexattr_destroy(%p): 0", attr);
    return 0;
}

int pthread_mutexattr_settype_soloader(pthread_mutexattr_t_bionic *attr, int type) {
    if (attr && type >= BIONIC_PTHREAD_MUTEX_NORMAL && type <= BIONIC_PTHREAD_MUTEX_ERRORCHECK) {
        *attr = (*attr & ~BIONIC_MUTEXATTR_TYPE_MASK) | type;
        // so_logd("pthread_mutexattr_settype(%p,%d): 0", attr, type);
        return 0;
    }
    return EINVAL;
}

int pthread_mutexattr_gettype_soloader(const pthread_mutexattr_t_bionic *attr, int *type) {
    if (attr) {
        int atype = (*attr & BIONIC_MUTEXATTR_TYPE_MASK);
        if (atype >= BIONIC_PTHREAD_MUTEX_NORMAL && atype <= BIONIC_PTHREAD_MUTEX_ERRORCHECK) {
            *type = atype;
            // so_logd("pthread_mutexattr_gettype(%p,%p): type=0x%x", attr, type, *type);
            return 0;
        }
    }
    return EINVAL;
}

int pthread_mutexattr_setpshared_soloader(pthread_mutexattr_t_bionic *attr, int pshared) {
    if (!attr) {
        return EINVAL;
    }

    switch (pshared) {
        case BIONIC_PTHREAD_PROCESS_PRIVATE:
            *attr &= ~BIONIC_MUTEXATTR_SHARED_MASK;
            break;
        case BIONIC_PTHREAD_PROCESS_SHARED:
            *attr |= BIONIC_MUTEXATTR_SHARED_MASK;
            break;
        default:
            return EINVAL;
    }

    // so_logd("pthread_mutexattr_setpshared(%p,%d): 0", attr, pshared);
    return 0;
}

int pthread_mutexattr_getpshared_soloader(pthread_mutexattr_t_bionic *attr, int *pshared) {
    if (!attr || !pshared) {
        return EINVAL;
    }

    *pshared = (*attr & BIONIC_MUTEXATTR_SHARED_MASK) ? BIONIC_PTHREAD_PROCESS_SHARED
                                                      : BIONIC_PTHREAD_PROCESS_PRIVATE;
    // so_logd("pthread_mutexattr_getpshared(%p,%p): pshared=0x%x", attr, pshared, *pshared);
    return 0;
}

int pthread_mutex_init_soloader(pthread_mutex_t_bionic *mutex, const pthread_mutexattr_t_bionic *attr) {
    if (mutex_init(mutex, attr) == NULL) {
        return -1;
    }

    // so_logd("pthread_mutex_init(%p,%p): 0", mutex, attr);
    return 0;
}

int pthread_mutex_destroy_soloader(pthread_mutex_t_bionic *mutex) {
    if (!mutex) {
        return EINVAL;
    }

    int              id         = *(int *)mutex;
    pthread_mutex_t *real_mutex = (pthread_mutex_t *)pthr_remove_cache(mutex, id);
    if (!real_mutex) {
        return EINVAL;
    }

    int result = pthread_mutex_destroy(real_mutex);
    free(real_mutex);
    mutex->value = 0xdead10cc;

    // so_logd("pthread_mutex_destroy(%p): %d", mutex, result);
    return result;
}

int pthread_mutex_lock_soloader(pthread_mutex_t_bionic *mutex) {
    pthread_mutex_t *real_mutex = bionic_mutex_to_mutex(mutex);
    if (!real_mutex) {
        return EINVAL;
    }

    int result = pthread_mutex_lock(real_mutex);
    // so_logd("pthread_mutex_lock(%p): %d", mutex, result);
    return result;
}

int pthread_mutex_unlock_soloader(pthread_mutex_t_bionic *mutex) {
    pthread_mutex_t *real_mutex = bionic_mutex_to_mutex(mutex);
    if (!real_mutex) {
        return EINVAL;
    }

    int result = pthread_mutex_unlock(real_mutex);
    // so_logd("pthread_mutex_unlock(%p): %d", mutex, result);
    return result;
}

int pthread_mutex_trylock_soloader(pthread_mutex_t_bionic *mutex) {
    pthread_mutex_t *real_mutex = bionic_mutex_to_mutex(mutex);
    if (!real_mutex) {
        return EINVAL;
    }

    int result = pthread_mutex_trylock(real_mutex);
    // so_logd("pthread_mutex_trylock(%p): %d", mutex, result);
    return result;
}

int pthread_mutex_timedlock_soloader(pthread_mutex_t_bionic *mutex, struct timespec *abstime) {
    pthread_mutex_t *real_mutex = bionic_mutex_to_mutex(mutex);
    if (!real_mutex) {
        return EINVAL;
    }

    int result = pthread_mutex_timedlock(real_mutex, abstime);
    // so_logd("pthread_mutex_timedlock(%p): %d", mutex, result);
    return result;
}

int pthread_mutex_lock_timeout_np_soloader(pthread_mutex_t_bionic *mutex, unsigned msecs) {
    pthread_mutex_t *real_mutex = bionic_mutex_to_mutex(mutex);
    if (!real_mutex) {
        return EINVAL;
    }

    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_sec += msecs / 1000;
    ts.tv_nsec += (msecs % 1000) * 1000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec += 1;
        ts.tv_nsec -= 1000000000;
    }

    int result = pthread_mutex_timedlock(real_mutex, &ts);
    // so_logd("pthread_mutex_lock_timeout_np(%p): %d", mutex, result);
    return result;
}
