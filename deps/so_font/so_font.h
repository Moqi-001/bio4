#ifndef SO_FONT_H
#define SO_FONT_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct so_font so_font;

so_font       *so_font_load_mem(void *buffer, size_t buffer_size, int copy);
so_font       *so_font_load(const char *filepath);
void           so_font_unload(so_font *font);
unsigned char *so_font_create_bitmap(so_font *font, const char *text, int font_size, int linespace, int *out_width, int *out_height);
size_t         so_font_get_font_height(so_font *font, size_t font_size);

#ifdef __cplusplus
}
#endif

#endif