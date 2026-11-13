#ifndef BIONIC_PTHREAD_INTERNAL_H
#define BIONIC_PTHREAD_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bionic_pthread.h"

/* Has the thread been detached by a pthread_join or pthread_detach call? */
#define BIONIC_PTHREAD_ATTR_FLAG_DETACHED 0x00000001

/* Was the thread's stack allocated by the user rather than by us? */
#define BIONIC_PTHREAD_ATTR_FLAG_USER_STACK 0x00000002

/* Has the thread been joined by another thread? */
#define BIONIC_PTHREAD_ATTR_FLAG_JOINED 0x00000004

/* Has the thread already exited but not been joined? */
#define BIONIC_PTHREAD_ATTR_FLAG_ZOMBIE 0x00000008

// 有些 Bionic 的 pthread mutex 没有调用 pthread_mutex_init 就直接使用 pthread_mutex_lock，
// 而 vita 的 pthread mutex 使用动态内存，所以必需进行初始化，所以我们需要缓存
int   pthr_add_cache(void *key, void *value);
void *pthr_remove_cache(void *key, int id);
void *pthr_find_cache(void *key, int id);

pthread_mutex_t *bionic_mutex_to_mutex(pthread_mutex_t_bionic *mutex);
pthread_cond_t  *bionic_cond_to_cond(pthread_cond_t_bionic *cond);

void bionic_attr_to_attr(const pthread_attr_t_bionic *bionic_attr, pthread_attr_t *attr);

#ifdef __cplusplus
}
#endif

#endif