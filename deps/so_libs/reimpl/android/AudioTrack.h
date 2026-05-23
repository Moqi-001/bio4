#ifndef ANDROID_AUDIO_TRACK_H
#define ANDROID_AUDIO_TRACK_H

#include "audio.h"
#include "Errors.h"

enum channel_index {
    MONO  = 0,
    LEFT  = 0,
    RIGHT = 1
};

typedef enum {
    TRANSFER_DEFAULT,   // not specified explicitly; determine from the other parameters
    TRANSFER_CALLBACK,  // callback EVENT_MORE_DATA
    TRANSFER_OBTAIN,    // FIXME deprecated: call obtainBuffer() and releaseBuffer()
    TRANSFER_SYNC,      // synchronous write()
    TRANSFER_SHARED,    // shared memory
} transfer_type;

/* Events used by AudioTrack callback function (callback_t).
 * Keep in sync with frameworks/base/media/java/android/media/AudioTrack.java NATIVE_EVENT_*.
 */
enum event_type {
    EVENT_MORE_DATA = 0,        // Request to write more data to buffer.
                                // If this event is delivered but the callback handler
                                // does not want to write more data, the handler must explicitly
                                // ignore the event by setting frameCount to zero.
    EVENT_UNDERRUN = 1,         // Buffer underrun occurred.
    EVENT_LOOP_END = 2,         // Sample loop end was reached; playback restarted from
                                // loop start if loop count was not 0.
    EVENT_MARKER = 3,           // Playback head is at the specified marker position
                                // (See setMarkerPosition()).
    EVENT_NEW_POS = 4,          // Playback head is at a new position
                                // (See setPositionUpdatePeriod()).
    EVENT_BUFFER_END = 5,       // Playback head is at the end of the buffer.
                                // Not currently used by android.media.AudioTrack.
    EVENT_NEW_IAUDIOTRACK = 6,  // IAudioTrack was re-created, either due to re-routing and
                                // voluntary invalidation by mediaserver, or mediaserver crash.
    EVENT_STREAM_END = 7,       // Sent after all the buffers queued in AF and HW are played
                                // back (after stop is called)
    EVENT_NEW_TIMESTAMP = 8,    // Delivered periodically and when there's a significant change
                                // in the mapping from frame position to presentation time.
                                // See AudioTimestamp for the information included with event.
};

typedef void (*callback_t)(int event, void *user, void *info);

typedef struct AAudioTrack AAudioTrack;
struct AAudioTrack {
    status_t             mStatus;
    audio_stream_type_t  mStreamType;
    uint32_t             mSampleRate;
    audio_format_t       mFormat;
    audio_channel_mask_t mChannelMask;
    size_t               mFrameCount;
    audio_output_flags_t mFlags;
    callback_t           mCbf;
    void                *mUserData;
    uint32_t             mNotificationFrames;
    const void          *mSharedBuffer;
    int                  mSessionId;

    transfer_type mTransfer;

    pthread_mutex_t mLock;
    bool            mIsTimed;
};  // +216

void     AAudioTrack_construct(AAudioTrack *thiz);
void     AAudioTrack_destruct(AAudioTrack *thiz);
status_t AAudioTrack_initCheck(AAudioTrack *thiz);
void     AAudioTrack_start(AAudioTrack *thiz);
void     AAudioTrack_pause(AAudioTrack *thiz);
void     AAudioTrack_stop(AAudioTrack *thiz);
status_t AAudioTrack_setVolume(AAudioTrack *thiz, float left, float right);
status_t AAudioTrack_set(AAudioTrack         *thiz,
                         audio_stream_type_t  streamType,
                         uint32_t             sampleRate,
                         audio_format_t       format,
                         uint32_t             channelMask,
                         int                  frameCount,
                         audio_output_flags_t flags,
                         callback_t           cbf,
                         void                *user,
                         int                  notificationFrames,
                         const void          *sharedBuffer,  // IMemory
                         bool                 threadCanCallJava,
                         int                  sessionId);
ssize_t  AAudioTrack_write(AAudioTrack *thiz, const void *buffer, size_t userSize);

#endif