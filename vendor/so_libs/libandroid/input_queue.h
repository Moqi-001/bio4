#ifndef _ANDROID_INPUT_QUEUE_H
#define _ANDROID_INPUT_QUEUE_H

#include "input.h"

// 宿主侧扩展：NativeActivity host 使用的队列创建/投递接口
AInputQueue *AInputQueue_create();
void         AInputQueue_destroy(AInputQueue *queue);
void         AInputQueue_push(AInputQueue *queue, AInputEvent *event);

#endif