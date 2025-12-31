#ifndef ANDROID_CUTILS_ATOMIC_H
#define ANDROID_CUTILS_ATOMIC_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

int32_t android_atomic_inc(volatile int32_t *addr);
int32_t android_atomic_dec(volatile int32_t *addr);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // ANDROID_CUTILS_ATOMIC_H