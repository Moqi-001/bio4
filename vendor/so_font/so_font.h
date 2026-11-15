#ifndef SO_FONT_H
#define SO_FONT_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

typedef struct so_font so_font;

typedef struct so_bitmap {
    unsigned char *pixels;   /* 8位灰度像素 */
    int            width;    /* 位图宽度 */
    int            height;   /* 位图高度 */
    int            pitch;    /* 位图行字节数 */
    int            origin_x; /* 原点坐标x */
    int            origin_y; /* 原点坐标y */
} so_bitmap;

so_font   *so_font_load_mem(void *buffer, size_t buffer_size, int take);
so_font   *so_font_load(const char *filepath);
void       so_font_unload(so_font *font);
so_bitmap *so_font_create_text_bitmap(so_font *font, const char *text, int font_size, int linespace);
void       so_font_free_bitmap(so_bitmap *bmp);
size_t     so_font_get_font_height(so_font *font, size_t font_size);

#ifdef __cplusplus
}
#endif

#endif