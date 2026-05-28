#ifndef VIDEO_PLAYER_H
#define VIDEO_PLAYER_H

int  video_player_init();
void video_player_term();

int video_start(const char *filepath, float volume);
int video_is_playing();

#endif