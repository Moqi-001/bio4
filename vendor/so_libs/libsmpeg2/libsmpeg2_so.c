#include <SDL2/SDL.h>
#include <smpeg2/smpeg.h>
#include <so_linker/so_linker.h>

static so_symbol libsmpeg2_symbols[] = {
    SO_SYMBOL_FUNC("SMPEG_actualSpec", &SMPEG_actualSpec),
    SO_SYMBOL_FUNC("SMPEG_delete", &SMPEG_delete),
    SO_SYMBOL_FUNC("SMPEG_enableaudio", &SMPEG_enableaudio),
    SO_SYMBOL_FUNC("SMPEG_enablevideo", &SMPEG_enablevideo),
    SO_SYMBOL_FUNC("SMPEG_new_rwops", &SMPEG_new_rwops),
    SO_SYMBOL_FUNC("SMPEG_pause", &SMPEG_pause),
    SO_SYMBOL_FUNC("SMPEG_play", &SMPEG_play),
    SO_SYMBOL_FUNC("SMPEG_playAudioSDL", &SMPEG_playAudioSDL),
    SO_SYMBOL_FUNC("SMPEG_setdisplay", &SMPEG_setdisplay),
    SO_SYMBOL_FUNC("SMPEG_setvolume", &SMPEG_setvolume),
    SO_SYMBOL_FUNC("SMPEG_status", &SMPEG_status),
    SO_SYMBOL_FUNC("SMPEG_stop", &SMPEG_stop),
};

so_library libsmpeg2_so = {
    .soname      = "libsmpeg2.so",
    .symbols     = libsmpeg2_symbols,
    .num_symbols = sizeof(libsmpeg2_symbols) / sizeof(libsmpeg2_symbols[0]),
    .prepare     = NULL,
};
