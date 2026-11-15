/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "bitmap.h"

#include <pthread.h>

typedef struct AndroidBitmap {
    AndroidBitmapInfo info;
    void             *pixels;
    pthread_mutex_t   mutex;
} AndroidBitmap;

int AndroidBitmap_getInfo(JNIEnv *env, jobject jbitmap,
                          AndroidBitmapInfo *info) {
    if (NULL == env || NULL == jbitmap) {
        return ANDROID_BITMAP_RESULT_BAD_PARAMETER;
    }

    AndroidBitmap *bm = NULL;
    if (NULL == bm) {
        return ANDROID_BITMAP_RESULT_JNI_EXCEPTION;
    }

    if (info) {
        info->width  = 0;
        info->height = 0;
        info->stride = 0;
        info->flags  = 0;
        info->format = ANDROID_BITMAP_FORMAT_RGBA_8888;
    }
    return ANDROID_BITMAP_RESUT_SUCCESS;
}

int AndroidBitmap_lockPixels(JNIEnv *env, jobject jbitmap, void **addrPtr) {
    if (NULL == env || NULL == jbitmap) {
        return ANDROID_BITMAP_RESULT_BAD_PARAMETER;
    }

    AndroidBitmap *bm = NULL;
    if (NULL == bm) {
        return ANDROID_BITMAP_RESULT_JNI_EXCEPTION;
    }

    pthread_mutex_lock(&bm->mutex);

    if (addrPtr) {
        *addrPtr = bm->pixels;
    }
    return ANDROID_BITMAP_RESUT_SUCCESS;
}

int AndroidBitmap_unlockPixels(JNIEnv *env, jobject jbitmap) {
    if (NULL == env || NULL == jbitmap) {
        return ANDROID_BITMAP_RESULT_BAD_PARAMETER;
    }

    AndroidBitmap *bm = NULL;
    if (NULL == bm) {
        return ANDROID_BITMAP_RESULT_JNI_EXCEPTION;
    }

    pthread_mutex_unlock(&bm->mutex);
    return ANDROID_BITMAP_RESUT_SUCCESS;
}
