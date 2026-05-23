#include <stdatomic.h>
#include <stdint.h>

int so_ret_0(void) { return 0; }

int so_ret_1(void) { return 1; }

int so_ret_neg1(void) { return -1; }

int32_t so_atomic_inc(int32_t *addr) {
    return atomic_fetch_add_explicit(
        (atomic_int_least32_t *)addr,
        1,
        memory_order_relaxed);
}

int32_t so_atomic_dec(int32_t *addr) {
    return atomic_fetch_sub_explicit(
        (atomic_int_least32_t *)addr,
        1,
        memory_order_acq_rel);
}
