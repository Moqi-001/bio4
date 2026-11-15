#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include <so_common/so_logger.h>

#include "Errors.h"

#include "Thread.h"

void AThread_ctor(AThread *thiz, int canCallJava) {
    thiz->mCanCallJava = canCallJava;
    thiz->mThread      = (thread_id_t)-1;
    thiz->mStatus      = NO_ERROR;
    thiz->mExitPending = false;
    thiz->mRunning     = false;
    pthread_mutex_init(&thiz->mLock.mutex, NULL);

    so_logd("[NDK][Thread] ctor(%p,%u)", thiz, canCallJava);
}

void AThread_dtorDeleting(AThread *thiz) {
    so_logd("[NDK][Thread] dtorDeleting(%p): unsupport!", thiz);
}

void AThread_dtorComplete(AThread *thiz) {
    so_logd("[NDK][Thread] dtorComplete(%p): unsupport!", thiz);
}

void AThread_dtorBase(AThread *thiz) {
    so_logd("[NDK][Thread] dtorBase(%p): unsupport!", thiz);
}

void AThread_dtorDeletingThunk(AThread *thiz) {
    so_logd("[NDK][Thread] dtorDeletingThunk(%p): unsupport!", thiz);
}

void AThread_dtorCompleteThunk(AThread *thiz) {
    so_logd("[NDK][Thread] dtorCompleteThunk(%p): unsupport!", thiz);
}

int AThread_readyToRun(AThread *thiz) {
    so_logd("[NDK][Thread] readyToRun(%p): %d", thiz, NO_ERROR);
    return NO_ERROR;
}

void AThread_requestExit(AThread *thiz) {
    so_logd("[NDK][Thread] requestExit(%p)", thiz);
    pthread_mutex_lock(&thiz->mLock.mutex);
    thiz->mExitPending = true;
    pthread_mutex_unlock(&thiz->mLock.mutex);
}

static void *_threadLoop(void *user) {
    so_logd("[NDK][Thread] _threadLoop(%p) start", user);

    AThread *thiz = (AThread *)user;

    thiz->mStatus = thiz->vtable->readyToRun(thiz);
    if (thiz->mStatus != NO_ERROR) {
        return (void *)UNKNOWN_ERROR;
    }

    while (1) {
        pthread_mutex_lock(&thiz->mLock.mutex);
        if (thiz->mExitPending) {
            pthread_mutex_unlock(&thiz->mLock.mutex);
            break;
        }
        pthread_mutex_unlock(&thiz->mLock.mutex);

        bool result = thiz->vtable->threadLoop(thiz);
        if (result == false) {
            break;
        }
    }

    so_logd("[NDK][Thread] _threadLoop(%p) exit", user);
    return (void *)0;
}

int AThread_run(AThread *thiz, const char *name, int priority, size_t stack) {
    pthread_attr_t attr;
    pthread_t      thread;
    int            res;

    if (thiz->mRunning) {
        return INVALID_OPERATION;
    }

    pthread_mutex_lock(&thiz->mLock.mutex);

    thiz->mHoldSelf    = thiz;
    thiz->mExitPending = false;
    thiz->mRunning     = true;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if (priority != 0) {
        struct sched_param param;
        param.sched_priority = priority;
        pthread_attr_setschedparam(&attr, &param);
    }
    if (stack != 0) {
        pthread_attr_setstacksize(&attr, stack);
    } else {
        pthread_attr_setstacksize(&attr, 1024 * 1024);
    }

    res = pthread_create(&thread, &attr, _threadLoop, thiz);
    pthread_attr_destroy(&attr);
    if (res != 0) {
        so_loge("[NDK][Thread] run(%p,'%s',0x%x,%u): pthread_create error! (%d)", thiz, name, priority, stack, res);

        thiz->mStatus = UNKNOWN_ERROR;
        thiz->mThread = (thread_id_t)-1;
        pthread_mutex_unlock(&thiz->mLock.mutex);
        return UNKNOWN_ERROR;
    }

    thiz->mThread = (thread_id_t)thread;

#ifdef HAVE_ANDROID_OS
    // thiz->mTid = gettid();
#endif

    pthread_mutex_unlock(&thiz->mLock.mutex);

    so_logd("[NDK][Thread] run(%p,'%s',0x%x,%u): OK!", thiz, name, priority, stack);
    return 0;
}