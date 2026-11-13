#ifndef BIONIC_PTHREAD_H
#define BIONIC_PTHREAD_H

#include <pthread.h>

typedef struct {
    int volatile value;
} pthread_mutex_t_bionic;

#define BIONIC__PTHREAD_MUTEX_INIT_VALUE            0
#define BIONIC__PTHREAD_RECURSIVE_MUTEX_INIT_VALUE  0x4000
#define BIONIC__PTHREAD_ERRORCHECK_MUTEX_INIT_VALUE 0x8000

#define BIONIC_PTHREAD_MUTEX_INITIALIZER            {BIONIC__PTHREAD_MUTEX_INIT_VALUE}
#define BIONIC_PTHREAD_RECURSIVE_MUTEX_INITIALIZER  {BIONIC__PTHREAD_RECURSIVE_MUTEX_INIT_VALUE}
#define BIONIC_PTHREAD_ERRORCHECK_MUTEX_INITIALIZER {BIONIC__PTHREAD_ERRORCHECK_MUTEX_INIT_VALUE}

enum {
    BIONIC_PTHREAD_MUTEX_NORMAL     = 0,
    BIONIC_PTHREAD_MUTEX_RECURSIVE  = 1,
    BIONIC_PTHREAD_MUTEX_ERRORCHECK = 2,

    BIONIC_PTHREAD_MUTEX_ERRORCHECK_NP = BIONIC_PTHREAD_MUTEX_ERRORCHECK,
    BIONIC_PTHREAD_MUTEX_RECURSIVE_NP  = BIONIC_PTHREAD_MUTEX_RECURSIVE,

    BIONIC_PTHREAD_MUTEX_DEFAULT = BIONIC_PTHREAD_MUTEX_NORMAL
};

typedef struct {
    int volatile value;
} pthread_cond_t_bionic;

typedef struct {
    uint32_t flags;
    void    *stack_base;
    size_t   stack_size;
    size_t   guard_size;
    int32_t  sched_policy;
    int32_t  sched_priority;
} pthread_attr_t_bionic;

typedef unsigned pthread_mutexattr_t_bionic;
typedef int      pthread_condattr_t_bionic;

typedef int  pthread_key_t_bionic;
typedef long pthread_t_bionic;

typedef volatile int pthread_once_t_bionic;

/*
 * Defines
 */
#define BIONIC_PTHREAD_COND_INITIALIZER {0}

#define BIONIC_PTHREAD_STACK_MIN (2 * PAGE_SIZE)

#define BIONIC_PTHREAD_CREATE_DETACHED 0x00000001
#define BIONIC_PTHREAD_CREATE_JOINABLE 0x00000000

#define BIONIC_PTHREAD_ONCE_INIT 0

#define BIONIC_PTHREAD_PROCESS_PRIVATE 0
#define BIONIC_PTHREAD_PROCESS_SHARED  1

#define BIONIC_PTHREAD_SCOPE_SYSTEM  0
#define BIONIC_PTHREAD_SCOPE_PROCESS 1

/*
 * Prototypes
 */
#ifdef __cplusplus
extern "C" {
#endif

int pthread_attr_init_soloader(pthread_attr_t_bionic *attr);
int pthread_attr_destroy_soloader(pthread_attr_t_bionic *attr);

int pthread_attr_setdetachstate_soloader(pthread_attr_t_bionic *attr, int state);
int pthread_attr_getdetachstate_soloader(pthread_attr_t_bionic const *attr, int *state);

int pthread_attr_setschedpolicy_soloader(pthread_attr_t_bionic *attr, int policy);
int pthread_attr_getschedpolicy_soloader(pthread_attr_t_bionic const *attr, int *policy);

int pthread_attr_setschedparam_soloader(pthread_attr_t_bionic *attr, struct sched_param const *param);
int pthread_attr_getschedparam_soloader(pthread_attr_t_bionic const *attr, struct sched_param *param);

int pthread_attr_setstacksize_soloader(pthread_attr_t_bionic *attr, size_t stacksize);
int pthread_attr_getstacksize_soloader(pthread_attr_t_bionic const *attr, size_t *stack_size);

int pthread_attr_setstackaddr_soloader(pthread_attr_t_bionic *attr, void *stackaddr);
int pthread_attr_getstackaddr_soloader(pthread_attr_t_bionic const *attr, void **stackaddr);

int pthread_attr_setstack_soloader(pthread_attr_t_bionic *attr, void *stackaddr, size_t stacksize);
int pthread_attr_getstack_soloader(pthread_attr_t_bionic const *attr, void **stackaddr, size_t *stack_size);

int pthread_attr_setguardsize_soloader(pthread_attr_t_bionic *attr, size_t guardsize);
int pthread_attr_getguardsize_soloader(pthread_attr_t_bionic const *attr, size_t *guardsize);

int pthread_attr_setscope_soloader(pthread_attr_t_bionic *attr, int scope);
int pthread_attr_getscope_soloader(pthread_attr_t_bionic const *attr);

int pthread_getattr_np_soloader(pthread_t_bionic thid, pthread_attr_t_bionic *attr);

// --------------------------------------------------------------------------------------

int  pthread_create_soloader(pthread_t_bionic *thread, const pthread_attr_t_bionic *attr, void *(*start)(void *), void *arg);
void pthread_exit_soloader(void *retval);
int  pthread_join_soloader(pthread_t_bionic thread, void **ret_val);
int  pthread_detach_soloader(pthread_t_bionic thread);

pthread_t_bionic pthread_self_soloader();
int              pthread_equal_soloader(pthread_t_bionic t1, pthread_t_bionic t2);

int pthread_getschedparam_soloader(pthread_t_bionic thread, int *policy, struct sched_param *param);
int pthread_setschedparam_soloader(pthread_t_bionic thread, int policy, const struct sched_param *param);

// -----------------------------------------------------------------------------------------------

int pthread_mutexattr_init_soloader(pthread_mutexattr_t_bionic *attr);
int pthread_mutexattr_destroy_soloader(pthread_mutexattr_t_bionic *attr);
int pthread_mutexattr_settype_soloader(pthread_mutexattr_t_bionic *attr, int type);
int pthread_mutexattr_gettype_soloader(const pthread_mutexattr_t_bionic *attr, int *type);
int pthread_mutexattr_setpshared_soloader(pthread_mutexattr_t_bionic *attr, int pshared);
int pthread_mutexattr_getpshared_soloader(pthread_mutexattr_t_bionic *attr, int *pshared);

int pthread_mutex_init_soloader(pthread_mutex_t_bionic *mutex, const pthread_mutexattr_t_bionic *attr);
int pthread_mutex_destroy_soloader(pthread_mutex_t_bionic *mutex);
int pthread_mutex_lock_soloader(pthread_mutex_t_bionic *mutex);
int pthread_mutex_unlock_soloader(pthread_mutex_t_bionic *mutex);
int pthread_mutex_trylock_soloader(pthread_mutex_t_bionic *mutex);
int pthread_mutex_timedlock_soloader(pthread_mutex_t_bionic *mutex, struct timespec *abstime);

/* same as pthread_mutex_lock(), but will wait up to 'msecs' milli-seconds
 * before returning. same return values than pthread_mutex_trylock though, i.e.
 * returns EBUSY if the lock could not be acquired after the timeout
 * expired.
 */
int pthread_mutex_lock_timeout_np_soloader(pthread_mutex_t_bionic *mutex, unsigned msecs);

// -----------------------------------------------------------------------------------------------

int pthread_condattr_init_soloader(pthread_condattr_t_bionic *attr);
int pthread_condattr_destroy_soloader(pthread_condattr_t_bionic *attr);
int pthread_condattr_setpshared_soloader(pthread_condattr_t_bionic *attr, int pshared);
int pthread_condattr_getpshared_soloader(pthread_condattr_t_bionic *attr, int *pshared);

int pthread_cond_init_soloader(pthread_cond_t_bionic *cond, const pthread_condattr_t_bionic *attr);
int pthread_cond_destroy_soloader(pthread_cond_t_bionic *cond);
int pthread_cond_broadcast_soloader(pthread_cond_t_bionic *cond);
int pthread_cond_signal_soloader(pthread_cond_t_bionic *cond);
int pthread_cond_wait_soloader(pthread_cond_t_bionic *cond, pthread_mutex_t_bionic *mutex);
int pthread_cond_timedwait_soloader(pthread_cond_t_bionic  *cond,
                                    pthread_mutex_t_bionic *mutex,
                                    const struct timespec  *abstime);

/* BIONIC: same as pthread_cond_timedwait, except the 'abstime' given refers
 *         to the CLOCK_MONOTONIC clock instead, to avoid any problems when
 *         the wall-clock time is changed brutally
 */
int pthread_cond_timedwait_monotonic_np_soloader(pthread_cond_t_bionic  *cond,
                                                 pthread_mutex_t_bionic *mutex,
                                                 const struct timespec  *abstime);

/* BIONIC: DEPRECATED. same as pthread_cond_timedwait_monotonic_np()
 * unfortunately pthread_cond_timedwait_monotonic has shipped already
 */
int pthread_cond_timedwait_monotonic_soloader(pthread_cond_t_bionic  *cond,
                                              pthread_mutex_t_bionic *mutex,
                                              const struct timespec  *abstime);

#define BIONIC_HAVE_PTHREAD_COND_TIMEDWAIT_MONOTONIC 1

/* BIONIC: same as pthread_cond_timedwait, except the 'reltime' given refers
 *         is relative to the current time.
 */
int pthread_cond_timedwait_relative_np_soloader(pthread_cond_t_bionic  *cond,
                                                pthread_mutex_t_bionic *mutex,
                                                struct timespec        *reltime);

#define BIONIC_HAVE_PTHREAD_COND_TIMEDWAIT_RELATIVE 1

int pthread_cond_timeout_np_soloader(pthread_cond_t_bionic  *cond,
                                     pthread_mutex_t_bionic *mutex,
                                     unsigned                msecs);

// -----------------------------------------------------------------------------------------------

typedef int pthread_rwlockattr_t_bionic;

typedef struct {
    pthread_mutex_t_bionic lock;
    pthread_cond_t_bionic  cond;
    int                    numLocks;
    int                    writerThreadId;
    int                    pendingReaders;
    int                    pendingWriters;
    void                  *reserved[4]; /* for future extensibility */
} pthread_rwlock_t_bionic;

#define BIONIC_PTHREAD_RWLOCK_INITIALIZER                                                           \
    {                                                                                               \
        PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, 0, 0, 0, 0, { NULL, NULL, NULL, NULL } \
    }

int pthread_rwlockattr_init_soloader(pthread_rwlockattr_t_bionic *attr);
int pthread_rwlockattr_destroy_soloader(pthread_rwlockattr_t_bionic *attr);
int pthread_rwlockattr_setpshared_soloader(pthread_rwlockattr_t_bionic *attr, int pshared);
int pthread_rwlockattr_getpshared_soloader(pthread_rwlockattr_t_bionic *attr, int *pshared);

int pthread_rwlock_init_soloader(pthread_rwlock_t_bionic *rwlock, const pthread_rwlockattr_t_bionic *attr);
int pthread_rwlock_destroy_soloader(pthread_rwlock_t_bionic *rwlock);

int pthread_rwlock_rdlock_soloader(pthread_rwlock_t_bionic *rwlock);
int pthread_rwlock_tryrdlock_soloader(pthread_rwlock_t_bionic *rwlock);
int pthread_rwlock_timedrdlock_soloader(pthread_rwlock_t_bionic *rwlock, const struct timespec *abs_timeout);

int pthread_rwlock_wrlock_soloader(pthread_rwlock_t_bionic *rwlock);
int pthread_rwlock_trywrlock_soloader(pthread_rwlock_t_bionic *rwlock);
int pthread_rwlock_timedwrlock_soloader(pthread_rwlock_t_bionic *rwlock, const struct timespec *abs_timeout);

int pthread_rwlock_unlock_soloader(pthread_rwlock_t_bionic *rwlock);

// ---------------------------------------------------------------------

int   pthread_key_create_soloader(pthread_key_t_bionic *key, void (*destructor_function)(void *));
int   pthread_key_delete_soloader(pthread_key_t_bionic key);
int   pthread_setspecific_soloader(pthread_key_t_bionic key, const void *value);
void *pthread_getspecific_soloader(pthread_key_t_bionic key);

int pthread_kill_soloader(pthread_t_bionic thread, int sig);
int pthread_sigmask_soloader(int how, const sigset_t *set, sigset_t *oset);

int pthread_getcpuclockid_soloader(pthread_t_bionic thread, clockid_t *clockid);

int pthread_once_soloader(pthread_once_t_bionic *once_control, void (*init_routine)(void));

int pthread_setname_np_soloader(pthread_t_bionic thread, const char *thname);

int pthread_atfork_soloader(void (*prepare)(void), void (*parent)(void), void (*child)(void));

// ---------------------------------------------------------------------

typedef void (*__pthread_cleanup_func_t_bionic)(void *);

typedef struct __pthread_cleanup_t_bionic {
    struct __pthread_cleanup_t_bionic *__cleanup_prev;
    __pthread_cleanup_func_t_bionic    __cleanup_routine;
    void                              *__cleanup_arg;
} __pthread_cleanup_t_bionic;

void __pthread_cleanup_push_soloader(__pthread_cleanup_t_bionic     *c,
                                     __pthread_cleanup_func_t_bionic routine,
                                     void                           *arg);
void __pthread_cleanup_pop_soloader(__pthread_cleanup_t_bionic *c,
                                    int                         execute);

#ifdef __cplusplus
};
#endif

#endif
