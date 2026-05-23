#ifndef SO_COMMON_SHARED_BUFFER_H
#define SO_COMMON_SHARED_BUFFER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>
#include <stdint.h>

void   *so_shared_buffer_alloc(size_t size);
void   *so_shared_buffer_alloc_with_data(size_t size, const void *data);
void    so_shared_buffer_retain(void *data);
int32_t so_shared_buffer_release(void *data);
int32_t so_shared_buffer_release_keep(void *data);   // 保留数据
void    so_shared_buffer_release_force(void *data);  // 强制释放
void   *so_shared_buffer_attempt_edit(void *data);
void   *so_shared_buffer_edit(void *data);
void   *so_shared_buffer_edit_resize(void *data, size_t new_size);
void   *so_shared_buffer_reset(void *data, size_t new_size);
size_t  so_shared_buffer_size(void *data);

#ifdef __cplusplus
}
#endif

#endif