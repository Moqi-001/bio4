#include <pthread.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <so_common/so_logger.h>

#include "AudioTrack.h"
#include "Errors.h"
#include "SharedBuffer.h"

void AAudioTrack_construct(AAudioTrack *thiz) {
    so_logd("[Android] AAudioTrack_construct(%p)", thiz);
    thiz->mStatus             = NO_ERROR;
    thiz->mFrameCount         = 0;
    thiz->mFlags              = AUDIO_OUTPUT_FLAG_NONE;
    thiz->mCbf                = NULL;
    thiz->mUserData           = NULL;
    thiz->mNotificationFrames = 0;
    thiz->mSessionId          = 0;
}

void AAudioTrack_destruct(AAudioTrack *thiz) {
    so_logd("[Android] AAudioTrack_destruct(%p)", thiz);
}

status_t AAudioTrack_initCheck(AAudioTrack *thiz) {
    so_logd("[Android] AAudioTrack_initCheck(%p): %d", thiz, thiz->mStatus);
    return thiz->mStatus;
}

void AAudioTrack_start(AAudioTrack *thiz) {
    so_logd("[Android] AAudioTrack_start(%p)", thiz);
}

void AAudioTrack_pause(AAudioTrack *thiz) {
    so_logd("[Android] AAudioTrack_pause(%p)", thiz);
}

void AAudioTrack_stop(AAudioTrack *thiz) {
    so_logd("[Android] AAudioTrack_stop(%p)", thiz);
}

status_t AAudioTrack_setVolume(AAudioTrack *thiz, float left, float right) {
    so_logd("[Android] AAudioTrack_setVolume(%p, %f, %f)", thiz, left, right);
    return NO_ERROR;
}

/**
 * @param streamType 音频流类型
 * @param sampleRate 采样率
 * @param format 音频格式
 * @param channelMask 声道掩码
 * @param frameCount 帧数
 * @param flags 输出标志
 * @param callback 回调函数
 * @param user 用户数据
 * @param notificationFrames 通知帧数
 * @return status_t 状态码
 */
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
                         int                  sessionId) {
    int result = NO_ERROR;
    so_logd("[Android] AAudioTrack_set(%p,%d,%u,0x%x,%u,%d,0x%x,%p,%p,%d,%p,%u,%d): %d",
            thiz,
            streamType,
            sampleRate,
            format,
            channelMask,
            frameCount,
            flags,
            cbf,
            user,
            notificationFrames,
            sharedBuffer,  // IMemory
            threadCanCallJava,
            sessionId,
            result);

    return NO_ERROR;
}

ssize_t AAudioTrack_write(AAudioTrack *thiz, const void *buffer, size_t userSize) {
    int result = NO_ERROR;
    so_logd("[Android] AAudioTrack_write(%p,%p,%u): %d", thiz, buffer, userSize, result);
    return NO_ERROR;

    /*
    if (thiz->mTransfer != TRANSFER_SYNC || thiz->mIsTimed) {
        return INVALID_OPERATION;
    }

    if ((ssize_t)userSize < 0 || (buffer == NULL && userSize != 0)) {
        // Sanity-check: user is most-likely passing an error code, and it would
        // make the return value ambiguous (actualSize vs error).
        so_logd("AudioTrack::write(buffer=%p, size=%u (%d)", buffer, userSize, userSize);
        return BAD_VALUE;
    }

    size_t written = 0;
    Buffer audioBuffer;

    while (userSize >= thiz->mFrameSize) {
        audioBuffer.frameCount = userSize / thiz->mFrameSize;

        status_t err = obtainBuffer(&audioBuffer, &ClientProxy::kForever);
        if (err < 0) {
            if (written > 0) {
                break;
            }
            return (ssize_t)err;
        }

        size_t toWrite;
        if (thiz->mFormat == AUDIO_FORMAT_PCM_8_BIT && !(thiz->mFlags & AUDIO_OUTPUT_FLAG_DIRECT)) {
            // Divide capacity by 2 to take expansion into account
            toWrite = audioBuffer.size >> 1;
            memcpy_to_i16_from_u8(audioBuffer.i16, (const uint8_t *)buffer, toWrite);
        } else {
            toWrite = audioBuffer.size;
            memcpy(audioBuffer.i8, buffer, toWrite);
        }
        buffer = ((const char *)buffer) + toWrite;
        userSize -= toWrite;
        written += toWrite;

        releaseBuffer(&audioBuffer);
    }

    return written;
    */
    return 0;
}
