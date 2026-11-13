#ifndef ANDROID_MEDIA_AUDIO_TRACK_H
#define ANDROID_MEDIA_AUDIO_TRACK_H

#include <so_java/java/lang/types.h>

typedef struct _AudioTrack *AudioTrack;

void setup_android_media_AudioTrack(void);

void AudioTrack_ctor(AudioTrack thiz, JInt streamType, JInt sampleRateInHz, JInt channelConfig,
                     JInt audioFormat, JInt bufferSizeInBytes, JInt mode);
JInt AudioTrack_write(AudioTrack thiz, JByte *audioData, JInt offsetInBytes, JInt sizeInBytes);
JInt AudioTrack_setStereoVolume(AudioTrack thiz, JFloat leftVolume, JFloat rightVolume);
void AudioTrack_play(AudioTrack thiz);
void AudioTrack_pause(AudioTrack thiz);
void AudioTrack_stop(AudioTrack thiz);
void AudioTrack_flush(AudioTrack thiz);
void AudioTrack_release(AudioTrack thiz);

JInt AudioTrack_getMinBufferSize(JInt sampleRateInHz, JInt channelConfig, JInt audioFormat);

#endif