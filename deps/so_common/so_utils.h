#ifndef SO_COMMON_UTILS_H
#define SO_COMMON_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

int so_ret_0(void);
int so_ret_1(void);
int so_ret_neg1(void);

int32_t so_atomic_inc(int32_t *addr);
int32_t so_atomic_dec(int32_t *addr);

#ifdef __cplusplus
}
#endif

#endif