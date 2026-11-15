#include <stdatomic.h>
#include <stdint.h>

// 原子增加（返回旧值）
int32_t android_atomic_add(int32_t increment, volatile int32_t *ptr) {
    atomic_int_least32_t *atomic_ptr = (atomic_int_least32_t *)ptr;
    return atomic_fetch_add_explicit(atomic_ptr, increment, memory_order_relaxed);
}

// 原子递增（返回旧值）
int32_t android_atomic_inc(int32_t *addr) {
    atomic_int_least32_t *atomic_addr = (atomic_int_least32_t *)addr;
    return atomic_fetch_add_explicit(atomic_addr, 1, memory_order_relaxed);
}

// 原子递减（返回旧值）
int32_t android_atomic_dec(int32_t *addr) {
    atomic_int_least32_t *atomic_addr = (atomic_int_least32_t *)addr;
    return atomic_fetch_sub_explicit(atomic_addr, 1, memory_order_acq_rel);
}

// 原子按位或（返回旧值）
int32_t android_atomic_or(int32_t value, int32_t *addr) {
    atomic_int_least32_t *atomic_addr = (atomic_int_least32_t *)addr;
    return atomic_fetch_or_explicit(atomic_addr, value, memory_order_relaxed);
}

// 原子按位与（返回旧值）
int32_t android_atomic_and(int32_t value, int32_t *addr) {
    atomic_int_least32_t *atomic_addr = (atomic_int_least32_t *)addr;
    return atomic_fetch_and_explicit(atomic_addr, value, memory_order_relaxed);
}

// 原子读取
int32_t android_atomic_acquire_load(volatile const int32_t *ptr) {
    atomic_int_least32_t *atomic_addr = (atomic_int_least32_t *)ptr;
    return atomic_load_explicit(atomic_addr, memory_order_acquire);
}

// 原子读取
int32_t android_atomic_release_load(volatile const int32_t *ptr) {
    atomic_int_least32_t *atomic_addr = (atomic_int_least32_t *)ptr;
    atomic_thread_fence(memory_order_release);
    return atomic_load_explicit(atomic_addr, memory_order_relaxed);
}

void android_atomic_acquire_store(int32_t value, volatile int32_t *ptr) {
    atomic_int_least32_t *atomic_addr = (atomic_int_least32_t *)ptr;
    atomic_store_explicit(atomic_addr, value, memory_order_relaxed);
    atomic_thread_fence(memory_order_acquire);
}

void android_atomic_release_store(int32_t value, volatile int32_t *ptr) {
    atomic_int_least32_t *atomic_addr = (atomic_int_least32_t *)ptr;
    atomic_store_explicit(atomic_addr, value, memory_order_release);
}

int android_atomic_cas(int32_t old_value, int32_t new_value, volatile int32_t *ptr) {
    atomic_int_least32_t *atomic_addr = (atomic_int_least32_t *)ptr;
    int32_t               expected    = old_value;
    int                   success     = atomic_compare_exchange_strong_explicit(
        atomic_addr, &expected, new_value, memory_order_relaxed, memory_order_relaxed);
    return success ? 0 : 1;
}

int android_atomic_acquire_cas(int32_t old_value, int32_t new_value, volatile int32_t *ptr) {
    atomic_int_least32_t *atomic_addr = (atomic_int_least32_t *)ptr;
    int32_t               expected    = old_value;
    int                   success     = atomic_compare_exchange_strong_explicit(
        atomic_addr, &expected, new_value, memory_order_acquire, memory_order_relaxed);
    return success ? 0 : 1;
}

int android_atomic_release_cas(int32_t old_value, int32_t new_value, volatile int32_t *ptr) {
    atomic_int_least32_t *atomic_addr = (atomic_int_least32_t *)ptr;
    int32_t               expected    = old_value;
    int                   success     = atomic_compare_exchange_strong_explicit(
        atomic_addr, &expected, new_value, memory_order_release, memory_order_relaxed);
    return success ? 0 : 1;
}
