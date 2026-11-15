#ifndef ANDROID_MEDIA_AUDIO_FORMAT_H
#define ANDROID_MEDIA_AUDIO_FORMAT_H

//---------------------------------------------------------
// Constants
//---------------------------------------------------------

/* Invalid audio data format */
#define ENCODING_INVALID   0
/* Default audio data format */
#define ENCODING_DEFAULT   1
/* These two values must be kept in sync with JNI code for AudioTrack, AudioRecord */
/* Audio data format: PCM 16 bit per sample. Guaranteed to be supported by devices. */
#define ENCODING_PCM_16BIT 2
/* Audio data format: PCM 8 bit per sample. Not guaranteed to be supported by devices. */
#define ENCODING_PCM_8BIT  3

/* Invalid audio channel configuration */
/* @deprecated use CHANNEL_INVALID instead */
#define CHANNEL_CONFIGURATION_INVALID 0
/* Default audio channel configuration */
/* @deprecated use CHANNEL_OUT_DEFAULT or CHANNEL_IN_DEFAULT instead */
#define CHANNEL_CONFIGURATION_DEFAULT 1
/* Mono audio configuration */
/* @deprecated use CHANNEL_OUT_MONO or CHANNEL_IN_MONO instead */
#define CHANNEL_CONFIGURATION_MONO    2
/* Stereo (2 channel) audio configuration */
/* @deprecated use CHANNEL_OUT_STEREO or CHANNEL_IN_STEREO instead */
#define CHANNEL_CONFIGURATION_STEREO  3

/* Invalid audio channel mask */
#define CHANNEL_INVALID 0
/* Default audio channel mask */
#define CHANNEL_OUT_DEFAULT 1

/*
 * Channel mask definitions below are translated to the native values defined in
 * /system/core/include/system/audio.h in the JNI code of AudioTrack.
 */
#define CHANNEL_OUT_FRONT_LEFT            0x4
#define CHANNEL_OUT_FRONT_RIGHT           0x8
#define CHANNEL_OUT_FRONT_CENTER          0x10
#define CHANNEL_OUT_LOW_FREQUENCY         0x20
#define CHANNEL_OUT_BACK_LEFT             0x40
#define CHANNEL_OUT_BACK_RIGHT            0x80
#define CHANNEL_OUT_FRONT_LEFT_OF_CENTER  0x100
#define CHANNEL_OUT_FRONT_RIGHT_OF_CENTER 0x200
#define CHANNEL_OUT_BACK_CENTER           0x400
/* @hide */
#define CHANNEL_OUT_SIDE_LEFT             0x800
/* @hide */
#define CHANNEL_OUT_SIDE_RIGHT            0x1000
/* @hide */
#define CHANNEL_OUT_TOP_CENTER            0x2000
/* @hide */
#define CHANNEL_OUT_TOP_FRONT_LEFT        0x4000
/* @hide */
#define CHANNEL_OUT_TOP_FRONT_CENTER      0x8000
/* @hide */
#define CHANNEL_OUT_TOP_FRONT_RIGHT       0x10000
/* @hide */
#define CHANNEL_OUT_TOP_BACK_LEFT         0x20000
/* @hide */
#define CHANNEL_OUT_TOP_BACK_CENTER       0x40000
/* @hide */
#define CHANNEL_OUT_TOP_BACK_RIGHT        0x80000

#define CHANNEL_OUT_MONO     (CHANNEL_OUT_FRONT_LEFT)
#define CHANNEL_OUT_STEREO   (CHANNEL_OUT_FRONT_LEFT | CHANNEL_OUT_FRONT_RIGHT)
#define CHANNEL_OUT_QUAD     (CHANNEL_OUT_FRONT_LEFT | CHANNEL_OUT_FRONT_RIGHT | \
                              CHANNEL_OUT_BACK_LEFT  | CHANNEL_OUT_BACK_RIGHT)
#define CHANNEL_OUT_SURROUND (CHANNEL_OUT_FRONT_LEFT | CHANNEL_OUT_FRONT_RIGHT | \
                              CHANNEL_OUT_FRONT_CENTER | CHANNEL_OUT_BACK_CENTER)
#define CHANNEL_OUT_5POINT1  (CHANNEL_OUT_FRONT_LEFT | CHANNEL_OUT_FRONT_RIGHT | \
                              CHANNEL_OUT_FRONT_CENTER | CHANNEL_OUT_LOW_FREQUENCY | \
                              CHANNEL_OUT_BACK_LEFT  | CHANNEL_OUT_BACK_RIGHT)
#define CHANNEL_OUT_7POINT1  (CHANNEL_OUT_FRONT_LEFT | CHANNEL_OUT_FRONT_RIGHT | \
                              CHANNEL_OUT_FRONT_CENTER | CHANNEL_OUT_LOW_FREQUENCY | \
                              CHANNEL_OUT_BACK_LEFT  | CHANNEL_OUT_BACK_RIGHT | \
                              CHANNEL_OUT_FRONT_LEFT_OF_CENTER | CHANNEL_OUT_FRONT_RIGHT_OF_CENTER)
/* @hide */
#define CHANNEL_OUT_7POINT1_SURROUND (CHANNEL_OUT_FRONT_LEFT | CHANNEL_OUT_FRONT_CENTER | \
                                      CHANNEL_OUT_FRONT_RIGHT | CHANNEL_OUT_SIDE_LEFT | \
                                      CHANNEL_OUT_SIDE_RIGHT | CHANNEL_OUT_BACK_LEFT | \
                                      CHANNEL_OUT_BACK_RIGHT | CHANNEL_OUT_LOW_FREQUENCY)

#define CHANNEL_IN_DEFAULT          1
#define CHANNEL_IN_LEFT             0x4
#define CHANNEL_IN_RIGHT            0x8
#define CHANNEL_IN_FRONT            0x10
#define CHANNEL_IN_BACK             0x20
#define CHANNEL_IN_LEFT_PROCESSED   0x40
#define CHANNEL_IN_RIGHT_PROCESSED  0x80
#define CHANNEL_IN_FRONT_PROCESSED  0x100
#define CHANNEL_IN_BACK_PROCESSED   0x200
#define CHANNEL_IN_PRESSURE         0x400
#define CHANNEL_IN_X_AXIS           0x800
#define CHANNEL_IN_Y_AXIS           0x1000
#define CHANNEL_IN_Z_AXIS           0x2000
#define CHANNEL_IN_VOICE_UPLINK     0x4000
#define CHANNEL_IN_VOICE_DNLINK     0x8000
#define CHANNEL_IN_MONO             (CHANNEL_IN_FRONT)
#define CHANNEL_IN_STEREO           (CHANNEL_IN_LEFT | CHANNEL_IN_RIGHT)

#endif
