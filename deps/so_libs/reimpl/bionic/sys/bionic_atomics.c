
int __atomic_cmpxchg_soloader(int old_value, int new_value, volatile int *ptr) {
    /* We must return 0 on success */
    return __sync_val_compare_and_swap(ptr, old_value, new_value) != old_value;
}

int __atomic_swap_soloader(int new_value, volatile int *ptr) {
    int old_value;
    do {
        old_value = *ptr;
    } while (__sync_val_compare_and_swap(ptr, old_value, new_value) != old_value);
    return old_value;
}

int __atomic_inc_soloader(volatile int *ptr) {
    return __sync_fetch_and_add(ptr, 1);
}

int __atomic_dec_soloader(volatile int *ptr) {
    return __sync_fetch_and_sub(ptr, 1);
}
