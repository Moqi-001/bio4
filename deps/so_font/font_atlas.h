#ifndef SO_FONT_ATLAS_H
#define SO_FONT_ATLAS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include <freetype2/ft2build.h>
#include FT_FREETYPE_H

#include "font_htab.h"
#include "so_font.h"

typedef struct font_atlas_page {
    struct font_atlas_page *prev;
    int                     width;
    int                     height;
    unsigned char          *data;
} font_atlas_page;

typedef struct font_glyph {
    uint32_t       codepoint;
    unsigned char *bitmap_buffer;
    int            bitmap_pitch;
    int            bitmap_width;
    int            bitmap_height;
    int            bitmap_left;
    int            bitmap_top;
    int            advance_x;
    int            advance_y;
} font_glyph;

typedef struct font_atlas {
    int              font_size;
    int              font_height;
    int              font_ascender;
    int              font_descender;
    font_atlas_page *pages;
    int              page_width;
    int              page_height;
    int              pen_x;
    int              pen_y;
    int              y_space;
    font_htab       *htab;
} font_atlas;

font_atlas       *font_atlas_new(int font_size, int atlas_w, int atlas_h);
void              font_atlas_free(font_atlas *atlas);
const font_glyph *font_atlas_insert(font_atlas *atlas, uint32_t codepoint, const FT_GlyphSlot glyph);
const font_glyph *font_atlas_find(font_atlas *atlas, uint32_t codepoint);

#ifdef __cplusplus
}
#endif

#endif
