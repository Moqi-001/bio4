#ifndef ANDROID_THREAD_H
#define ANDROID_THREAD_H

#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include <stddef.h>

typedef int32_t status_t;

typedef void *android_thread_id_t;
typedef int (*android_thread_func_t)(void *);

typedef android_thread_id_t   thread_id_t;
typedef android_thread_func_t thread_func_t;

typedef struct AThread AThread;

typedef struct AThread_VTable {
    // Thread 自身的虚函数
    void (*destructorComplete)(AThread *thiz);                                        // +0
    void (*destructorDeleting)(AThread *thiz);                                        // +4
    int32_t (*run)(AThread *thiz, const char *name, int32_t priority, size_t stack);  // +8
    void (*requestExit)(AThread *thiz);                                               // +12
    int32_t (*readyToRun)(AThread *thiz);                                             // +16
    bool (*threadLoop)(AThread *thiz);                                                // +20  ← 纯虚

    // 从 RefBase 继承的虚函数
    void (*onFirstRef)(AThread *thiz);                                            // +24
    void (*onLastStrongRef)(AThread *thiz, const void *id);                       // +28
    bool (*onIncStrongAttempted)(AThread *thiz, uint32_t flags, const void *id);  // +32
    void (*onLastWeakRef)(AThread *thiz, const void *id);                         // +36
} AThread_VTable;

typedef struct AMutex {
    pthread_mutex_t mutex;
} AMutex;

typedef struct ACondition {
    pthread_cond_t cond;
} ACondition;

struct AThread {
    AThread_VTable *vtable;                  // +0
    bool            mCanCallJava;            // +4
    uint8_t         padding1[3];             // +5
    thread_id_t     mThread;                 // +8
    AMutex          mLock;                   // +12
    ACondition      mThreadExitedCondition;  // +16
    status_t        mStatus;                 // +20
    volatile bool   mExitPending;            // +24
    volatile bool   mRunning;                // +25
    uint8_t         padding2[2];             // +26
    AThread        *mHoldSelf;               // +28
#ifdef HAVE_ANDROID_OS
    pid_t mTid;  // +32
#endif
};

void AThread_ctor(AThread *thiz, int canCallJava);
void AThread_dtorDeleting(AThread *thiz);
void AThread_dtorComplete(AThread *thiz);
void AThread_dtorBase(AThread *thiz);
void AThread_dtorDeletingThunk(AThread *thiz);
void AThread_dtorCompleteThunk(AThread *thiz);
int  AThread_readyToRun(AThread *thiz);
void AThread_requestExit(AThread *thiz);
int  AThread_run(AThread *thiz, const char *name, int priority, size_t stack);

#ifdef __cplusplus
}
#endif

#endif