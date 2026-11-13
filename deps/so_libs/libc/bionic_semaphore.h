#ifndef BIONIC_SEMAPHORE_H
#define BIONIC_SEMAPHORE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/time.h>

typedef struct {
    volatile unsigned int count;
} sem_t_bionic;

#define BIONIC_SEM_FAILED NULL

int           sem_init_soloader(sem_t_bionic *sem, int pshared, unsigned int value);
int           sem_destroy_soloader(sem_t_bionic *sem);
int           sem_post_soloader(sem_t_bionic *sem);
int           sem_wait_soloader(sem_t_bionic *sem);
int           sem_trywait_soloader(sem_t_bionic *sem);
int           sem_timedwait_soloader(sem_t_bionic *sem, const struct timespec *abs_timeout);
int           sem_getvalue_soloader(sem_t_bionic *sem, int *sval);
sem_t_bionic *sem_open_soloader(const char *name, int oflag, ...);
int           sem_close_soloader(sem_t_bionic *sem);
int           sem_unlink_soloader(const char *name);

#ifdef __cplusplus
};
#endif

#endif