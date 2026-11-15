#include <stddef.h>

#include <SDL2/SDL_image.h>
#include <so_linker/so_linker.h>

static so_symbol libSDL2_image_symbols[] = {
    SO_SYMBOL_FUNC("IMG_Load_RW", &IMG_Load_RW),
};

so_library libSDL2_image_so = {
    .soname      = "libSDL2_image.so",
    .symbols     = libSDL2_image_symbols,
    .num_symbols = sizeof(libSDL2_image_symbols) / sizeof(libSDL2_image_symbols[0]),
    .prepare     = NULL,
};
