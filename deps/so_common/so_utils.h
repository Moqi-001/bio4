#ifndef SO_COMMON_UTILS_H
#define SO_COMMON_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdatomic.h>
#include <stdint.h>

static inline int32_t so_atomic_inc(int32_t *addr) {
    atomic_int_least32_t *atomic_addr = (atomic_int_least32_t *)addr;
    return atomic_fetch_add_explicit(atomic_addr, 1, memory_order_relaxed);
}

static inline int32_t so_atomic_dec(int32_t *addr) {
    atomic_int_least32_t *atomic_addr = (atomic_int_least32_t *)addr;
    return atomic_fetch_sub_explicit(atomic_addr, 1, memory_order_acq_rel);
}

int so_ret_0(void);
int so_ret_1(void);
int so_ret_neg1(void);

#ifdef __cplusplus
}
#endif

#endif