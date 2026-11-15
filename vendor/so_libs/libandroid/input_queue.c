#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <sys/fcntl.h>
#include <sys/unistd.h>

#include <so_common/so_logger.h>

#include "../libc/bionic_fcntl.h"
#include "../libc/bionic_unistd.h"

#include "input_queue.h"
#include "looper.h"

#define INPUT_QUEUE_CAPACITY 64

// 事件队列 + 唤醒管道（与 AOSP InputQueue 同一套机制）：
// looper 用 epoll 监控唤醒管道的读端，生产者投递事件后往写端写 1 字节，
// 读端随即可读 → epoll 唤醒 glue 线程 → pollAll 返回 LOOPER_ID_INPUT。
struct AInputQueue {
    AInputEvent    *events[INPUT_QUEUE_CAPACITY];
    int             head;
    int             count;
    pthread_mutex_t lock;

    ALooper *looper;
    int      sendWaitPipeFd;     // 读端，注册进 looper
    int      receiveWaitPipeFd;  // 写端，投递事件时写 1 字节唤醒
};

AInputQueue *AInputQueue_create() {
    AInputQueue *queue = (AInputQueue *)calloc(1, sizeof(AInputQueue));
    if (!queue) {
        so_loge("AInputQueue_create: alloc failed!");
        return NULL;
    }

    pthread_mutex_init(&queue->lock, NULL);

    int fds[2];
    if (pipe(fds) != 0) {
        so_loge("AInputQueue_create: pipe failed (errno=%d)", errno);
        pthread_mutex_destroy(&queue->lock);
        free(queue);
        return NULL;
    }
    queue->sendWaitPipeFd    = fds[0];
    queue->receiveWaitPipeFd = fds[1];

    fcntl_soloader(queue->sendWaitPipeFd, F_SETFL, O_NONBLOCK);
    fcntl_soloader(queue->receiveWaitPipeFd, F_SETFL, O_NONBLOCK);

    so_logd("AInputQueue_create(): %p (read=%d write=%d)", queue, queue->sendWaitPipeFd, queue->receiveWaitPipeFd);
    return queue;
}

void AInputQueue_destroy(AInputQueue *queue) {
    if (!queue) return;

    close_soloader(queue->sendWaitPipeFd);
    close_soloader(queue->receiveWaitPipeFd);
    pthread_mutex_destroy(&queue->lock);
    free(queue);
    so_logd("AInputQueue_destroy(%p)", queue);
}

// 清空唤醒管道里累积的字节（对应事件已被消费）
static void input_queue_drain(AInputQueue *queue) {
    char    buffer[16];
    ssize_t len;
    do {
        len = read_soloader(queue->sendWaitPipeFd, buffer, sizeof(buffer));
    } while (len > 0);
}

// 供宿主输入线程投递事件；写唤醒管道让 epoll 报读端可读
void AInputQueue_push(AInputQueue *queue, AInputEvent *event) {
    if (!queue || !event) return;

    pthread_mutex_lock(&queue->lock);
    if (queue->count >= INPUT_QUEUE_CAPACITY) {
        pthread_mutex_unlock(&queue->lock);
        so_logw("AInputQueue_push: queue full, event dropped!");
        return;
    }
    queue->events[(queue->head + queue->count) % INPUT_QUEUE_CAPACITY] = event;
    queue->count++;
    pthread_mutex_unlock(&queue->lock);

    char w   = 'w';
    ssize_t wr = write_soloader(queue->receiveWaitPipeFd, &w, 1);
    so_logd("[INPUTQ] push fd=%d write=%d errno=%d", queue->receiveWaitPipeFd, (int)wr, errno);
    if (wr != 1 && errno != EAGAIN) {
        so_logw("AInputQueue_push: wake write failed (errno=%d)", errno);
    }
}

void AInputQueue_attachLooper(AInputQueue *queue, ALooper *looper,
                              int ident, ALooper_callbackFunc callback, void *data) {
    queue->looper = looper;
    ALooper_addFd(looper, queue->sendWaitPipeFd, ident, ALOOPER_EVENT_INPUT, callback, data);
    so_logd("AInputQueue_attachLooper(%p,%p,%d): fd=%d", queue, looper, ident, queue->sendWaitPipeFd);
}

void AInputQueue_detachLooper(AInputQueue *queue) {
    if (queue->looper) {
        ALooper_removeFd(queue->looper, queue->sendWaitPipeFd);
        queue->looper = NULL;
    }
    so_logd("AInputQueue_detachLooper(%p)", queue);
}

int32_t AInputQueue_hasEvents(AInputQueue *queue) {
    pthread_mutex_lock(&queue->lock);
    int32_t has = queue->count > 0;
    pthread_mutex_unlock(&queue->lock);
    return has;
}

int32_t AInputQueue_getEvent(AInputQueue *queue, AInputEvent **outEvent) {
    pthread_mutex_lock(&queue->lock);
    if (queue->count <= 0) {
        pthread_mutex_unlock(&queue->lock);
        return -1;
    }
    so_logd("[INPUTQ] getEvent: delivering type=%d", ((int32_t *)queue->events[queue->head])[0]);
    *outEvent = queue->events[queue->head];
    queue->head = (queue->head + 1) % INPUT_QUEUE_CAPACITY;
    queue->count--;
    int empty = (queue->count == 0);
    pthread_mutex_unlock(&queue->lock);

    // 事件已取空：清掉唤醒字节，避免读端保持可读导致 pollAll 空转
    if (empty) {
        input_queue_drain(queue);
    }
    return 0;
}

int32_t AInputQueue_preDispatchEvent(AInputQueue *queue, AInputEvent *event) {
    // 无 IME/预分发过滤：事件直接交给游戏处理
    return 0;
}

void AInputQueue_finishEvent(AInputQueue *queue, AInputEvent *event, int handled) {
    // 事件对象由宿主分配并持有，处理完成后仅归还（宿主侧释放）
    so_logd("AInputQueue_finishEvent(%p,%p,%d)", queue, event, handled);
}
