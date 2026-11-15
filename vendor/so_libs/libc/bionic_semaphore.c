#include <stdint.h>

#include <sys/errno.h>
#include <sys/time.h>

#include <psp2/kernel/threadmgr.h>
#include <so_common/so_logger.h>

#include "bionic_semaphore.h"

int sem_init_soloader(sem_t_bionic *sem, int pshared, unsigned int value) {
    if (!sem) return EINVAL;

    SceUID semaid = sceKernelCreateSema("sema", 0, (int)value, SEM_VALUE_MAX, NULL);
    if (semaid < 0) {
        so_logd("sem_init(%p,%d,%u): error=0x%x", sem, pshared, value, semaid);
        return -1;
    }

    *(SceUID *)sem = semaid;
    // so_logd("sem_init(%p,%d,%u): semaid=0x%x", sem, pshared, value, semaid);
    return 0;
}

int sem_destroy_soloader(sem_t_bionic *sem) {
    if (!sem) return EINVAL;

    SceUID semaid = *(SceUID *)sem;
    int    result = sceKernelDeleteSema(semaid);
    if (result < 0) {
        so_loge("sem_destroy(%p): error=0x%x", sem, result);
        return -1;
    }

    // so_logd("sem_destroy(%p): 0x%x", sem, result);
    return 0;
}

int sem_post_soloader(sem_t_bionic *sem) {
    if (!sem) return EINVAL;

    SceUID semaid = *(SceUID *)sem;
    int    result = sceKernelSignalSema(semaid, 1);
    if (result < 0) {
        so_loge("sem_post(%p): error=0x%x", sem, result);
        return -1;
    }

    // so_logd("sem_post(%p): 0", sem);
    return 0;
}

int sem_wait_soloader(sem_t_bionic *sem) {
    if (!sem) return EINVAL;

    SceUID semaid = *(SceUID *)sem;
    int    result = sceKernelWaitSema(semaid, 1, NULL);
    if (result < 0) {
        so_loge("sem_wait(%p): error=0x%x", sem, result);
        return -1;
    }

    // so_logd("sem_wait(%p): 0", sem);
    return 0;
}

int sem_trywait_soloader(sem_t_bionic *sem) {
    if (!sem) return EINVAL;

    SceUID  semaid  = *(SceUID *)sem;
    SceUInt timeout = 1000;
    int     result  = sceKernelWaitSema(semaid, 1, &timeout);
    if (result < 0) {
        so_loge("sem_trywait(%p): error=0x%x", sem, result);
        return -1;
    }

    // so_logd("sem_trywait(%p): 0", sem);
    return 0;
}

int sem_timedwait_soloader(sem_t_bionic *sem, const struct timespec *abs_timeout) {
    if (!sem || !abs_timeout) return EINVAL;

    SceUID semaid = *(SceUID *)sem;
    int    result;

    if (!abs_timeout) {
        result = sceKernelWaitSema(semaid, 1, NULL);
    } else {
        struct timespec nowtime;
        clock_gettime(CLOCK_REALTIME, &nowtime);

        int64_t now_us = (int64_t)nowtime.tv_sec * 1000000 + nowtime.tv_nsec / 1000;            // us
        int64_t out_us = (int64_t)abs_timeout->tv_sec * 1000000 + abs_timeout->tv_nsec / 1000;  // us
        if (now_us > out_us) {
            so_loge("sem_timedwait(%p,%p): time out!", sem, abs_timeout);
            return ETIMEDOUT;
        }

        SceUInt timeout = out_us - now_us;
        result          = sceKernelWaitSema(semaid, 1, &timeout);
    }

    if (result < 0) {
        so_loge("sem_timedwait(%p,%p): error=0x%x", sem, abs_timeout, result);
        return -1;
    }

    // so_logd("sem_timedwait(%p,%p): 0", sem, abs_timeout);
    return 0;
}

int sem_getvalue_soloader(sem_t_bionic *sem, int *sval) {
    if (!sem || !sval) return EINVAL;

    SceKernelSemaInfo info;
    info.size = sizeof(SceKernelSemaInfo);

    SceUID semaid = *(SceUID *)sem;
    int    result = sceKernelGetSemaInfo(semaid, &info);
    if (result < 0) {
        so_loge("sem_getvalue(%p,%p): error=0x%x", sem, sval, result);
        return -1;
    }

    *sval = info.currentCount;
    // so_logd("sem_getvalue(%p,%p): sval=%d", sem, sval, *sval);
    return 0;
}

sem_t_bionic *sem_open_soloader(const char *name, int oflag, ...) {
    so_loge("sem_open('%s',%d): unimplemented!", name, oflag);
    errno = ENOSYS;
    return BIONIC_SEM_FAILED;
}

int sem_close_soloader(sem_t_bionic *sem) {
    so_loge("sem_close(%p): unimplemented!", sem);
    errno = ENOSYS;
    return -1;
}

int sem_unlink_soloader(const char *name) {
    so_loge("sem_unlink('%s'): unimplemented!", name);
    errno = ENOSYS;
    return -1;
}
