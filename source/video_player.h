#ifndef VIDEO_PLAYER_H
#define VIDEO_PLAYER_H

#ifdef __cplusplus
extern "C" {
#endif

int video_start(const char *filepath, float volume);
int video_is_playing();

#ifdef __cplusplus
};
#endif

#endif