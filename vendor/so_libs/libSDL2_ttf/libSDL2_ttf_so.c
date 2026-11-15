#include <SDL2/SDL_ttf.h>
#include <so_linker/so_linker.h>

static so_symbol libSDL2_ttf_symbols[] = {
    SO_SYMBOL_FUNC("TTF_CloseFont", &TTF_CloseFont),
    SO_SYMBOL_FUNC("TTF_OpenFontRW", &TTF_OpenFontRW),
    SO_SYMBOL_FUNC("TTF_RenderUTF8_Blended", &TTF_RenderUTF8_Blended),
    SO_SYMBOL_FUNC("TTF_RenderUTF8_Shaded", &TTF_RenderUTF8_Shaded),
    SO_SYMBOL_FUNC("TTF_RenderUTF8_Solid", &TTF_RenderUTF8_Solid),
    SO_SYMBOL_FUNC("TTF_Init", &TTF_Init),
    SO_SYMBOL_FUNC("TTF_Quit", &TTF_Quit),
};

so_library libSDL2_ttf_so = {
    .soname      = "libSDL2_ttf.so",
    .symbols     = libSDL2_ttf_symbols,
    .num_symbols = sizeof(libSDL2_ttf_symbols) / sizeof(libSDL2_ttf_symbols[0]),
    .prepare     = NULL,
};
