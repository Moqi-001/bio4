#ifndef BIONIC_SYS_ATOMICS_H
#define BIONIC_SYS_ATOMICS_H

#ifdef __cplusplus
extern "C" {
#endif

int __atomic_cmpxchg_soloader(int old_value, int new_value, volatile int *ptr);
int __atomic_swap_soloader(int new_value, volatile int *ptr);
int __atomic_inc_soloader(volatile int *ptr);
int __atomic_dec_soloader(volatile int *ptr);

#ifdef __cplusplus
};
#endif

#endif