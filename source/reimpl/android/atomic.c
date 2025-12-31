#include <stdatomic.h>
#include <stdint.h>

int32_t android_atomic_inc(volatile int32_t *addr) {
    return atomic_fetch_add_explicit(
        (volatile atomic_int_least32_t *)addr,
        1,
        memory_order_acq_rel);
}

int32_t android_atomic_dec(volatile int32_t *addr) {
    return atomic_fetch_sub_explicit(
        (volatile atomic_int_least32_t *)addr,
        1,
        memory_order_acq_rel);
}