#include <SDL2/SDL_mixer.h>
#include <so_linker/so_linker.h>

static so_symbol libSDL2_mixer_symbols[] = {
    SO_SYMBOL_FUNC("Mix_HookMusic", &Mix_HookMusic),
    SO_SYMBOL_FUNC("Mix_QuerySpec", &Mix_QuerySpec),
    SO_SYMBOL_FUNC("Mix_FreeMusic", &Mix_FreeMusic),
    SO_SYMBOL_FUNC("Mix_HaltMusic", &Mix_HaltMusic),
    SO_SYMBOL_FUNC("Mix_HookMusicFinished", &Mix_HookMusicFinished),
    SO_SYMBOL_FUNC("Mix_LoadMUS_RW", &Mix_LoadMUS_RW),
    SO_SYMBOL_FUNC("Mix_Pause", &Mix_Pause),
    SO_SYMBOL_FUNC("Mix_PauseMusic", &Mix_PauseMusic),
    SO_SYMBOL_FUNC("Mix_PlayMusic", &Mix_PlayMusic),
    SO_SYMBOL_FUNC("Mix_Resume", &Mix_Resume),
    SO_SYMBOL_FUNC("Mix_ResumeMusic", &Mix_ResumeMusic),
    SO_SYMBOL_FUNC("Mix_VolumeMusic", &Mix_VolumeMusic),
    SO_SYMBOL_FUNC("Mix_AllocateChannels", &Mix_AllocateChannels),
    SO_SYMBOL_FUNC("Mix_ChannelFinished", &Mix_ChannelFinished),
    SO_SYMBOL_FUNC("Mix_FreeChunk", &Mix_FreeChunk),
    SO_SYMBOL_FUNC("Mix_HaltChannel", &Mix_HaltChannel),
    SO_SYMBOL_FUNC("Mix_LoadWAV_RW", &Mix_LoadWAV_RW),
    SO_SYMBOL_FUNC("Mix_PlayChannelTimed", &Mix_PlayChannelTimed),
    SO_SYMBOL_FUNC("Mix_SetPanning", &Mix_SetPanning),
    SO_SYMBOL_FUNC("Mix_Volume", &Mix_Volume),
    SO_SYMBOL_FUNC("Mix_Init", &Mix_Init),
    SO_SYMBOL_FUNC("Mix_OpenAudio", &Mix_OpenAudio),
    SO_SYMBOL_FUNC("Mix_Quit", &Mix_Quit),
};

so_library libSDL2_mixer_so = {
    .soname      = "libSDL2_mixer.so",
    .symbols     = libSDL2_mixer_symbols,
    .num_symbols = sizeof(libSDL2_mixer_symbols) / sizeof(libSDL2_mixer_symbols[0]),
    .prepare     = NULL,
};
