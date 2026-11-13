#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <so_common/so_logger.h>
#include <sys/unistd.h>

#include "looper.h"

#include "../libc/sys/bionic_epoll.h"
#include "../libcutils/atomic.h"
#include "../libutils/Timers.h"

typedef struct Request {
    int                  fd;
    int                  ident;
    ALooper_callbackFunc callback;
    void                *data;
    struct Request      *prev;
    struct Request      *next;
} Request;

typedef struct Response {
    int     events;
    Request request;
} Response;

struct ALooper {
    int32_t         refCount;
    bool            allowNonCallbacks;
    int             wakeReadPipeFd;
    int             wakeWritePipeFd;
    int             epollFd;
    Request        *requestHead;
    Request        *requestTail;
    Response       *responses;
    size_t          responseCount;
    size_t          responseIndex;
    pthread_mutex_t lock;
};

// Hint for number of file descriptors to be associated with the epoll instance.
#define EPOLL_SIZE_HINT 8

// Maximum number of file descriptors for which to retrieve poll events each iteration.
#define EPOLL_MAX_EVENTS 16

static pthread_once_t gTLSOnce = PTHREAD_ONCE_INIT;
static pthread_key_t  gTLSKey  = 0;

static void looper_clear_responses(ALooper *looper) {
    if (looper->responses) {
        free(looper->responses);
        looper->responses = NULL;
    }

    looper->responseCount = 0;
    looper->responseIndex = 0;
}

static void looper_clear_requests(ALooper *looper) {
    Request *request = looper->requestHead;
    while (request) {
        Request *next = request->next;
        free(request);
        request = next;
    }

    looper->requestHead = NULL;
    looper->requestTail = NULL;
}

static Request *looper_find_request(ALooper *looper, int fd) {
    Request *request = looper->requestHead;
    while (request) {
        if (request->fd == fd) {
            return request;
        }
        request = request->next;
    }

    return NULL;
}

static void looper_ctor(ALooper *looper, bool allowNonCallbacks) {
    int wakeFds[2];
    int result = pipe(wakeFds);
    if (result != 0) {
        so_logf("Could not create wake pipe!");
        return;
    }

    looper->refCount          = 1;
    looper->allowNonCallbacks = allowNonCallbacks;
    looper->wakeReadPipeFd    = wakeFds[0];
    looper->wakeWritePipeFd   = wakeFds[1];

    result = fcntl(looper->wakeReadPipeFd, F_SETFL, O_NONBLOCK);
    if (result != 0) {
        so_logf("Could not make wake read pipe non-blocking!");
        return;
    }

    result = fcntl(looper->wakeWritePipeFd, F_SETFL, O_NONBLOCK);
    if (result != 0) {
        so_logf("Could not make wake write pipe non-blocking!");
        return;
    }

    looper->epollFd = epoll_create_soloader(EPOLL_SIZE_HINT);
    if (looper->epollFd < 0) {
        so_logf("Could not create epoll instance!");
        return;
    }

    struct epoll_event_bionic event;
    memset(&event, 0, sizeof(struct epoll_event_bionic));  // zero out unused members of data field union
    event.events  = EPOLLIN;
    event.data.fd = looper->wakeReadPipeFd;
    result        = epoll_ctl_soloader(looper->epollFd, EPOLL_CTL_ADD, looper->wakeReadPipeFd, &event);
    if (result != 0) {
        so_logf("Could not add wake read pipe to epoll instance!");
        return;
    }

    looper->requestHead = NULL;
    looper->requestTail = NULL;

    looper->responses     = NULL;
    looper->responseCount = 0;
    looper->responseIndex = 0;

    pthread_mutex_init(&looper->lock, NULL);
}

static void looper_dtor(void *ptr) {
    ALooper *looper = (ALooper *)ptr;
    if (!looper) return;

    close(looper->wakeReadPipeFd);
    close(looper->wakeWritePipeFd);
    close(looper->epollFd);

    looper_clear_responses(looper);
    looper_clear_requests(looper);
    pthread_mutex_destroy(&looper->lock);
}

static void looper_init_key() {
    pthread_key_create(&gTLSKey, looper_dtor);
}

static inline ALooper *looper_for_thread() {
    pthread_once(&gTLSOnce, looper_init_key);
    return pthread_getspecific(gTLSKey);
}

ALooper *ALooper_forThread() {
    ALooper *looper = looper_for_thread();
    so_logd("ALooper_forThread(): %p", looper);
    return looper;
}

ALooper *ALooper_prepare(int opts) {
    bool allowNonCallbacks = ((opts & ALOOPER_PREPARE_ALLOW_NON_CALLBACKS) != 0);

    ALooper *looper = looper_for_thread();
    if (!looper) {
        looper = (ALooper *)malloc(sizeof(ALooper));
        if (!looper) {
            so_logf("ALooper_prepare(%d): Could not alloc memory for looper!", opts);
            return NULL;
        }
        looper_ctor(looper, allowNonCallbacks);
        pthread_setspecific(gTLSKey, looper);
    }

    if (looper->allowNonCallbacks != allowNonCallbacks) {
        so_logw(
            "Looper already prepared for this thread with a different value for the "
            "ALOOPER_PREPARE_ALLOW_NON_CALLBACKS option.");
    }

    so_logd("ALooper_prepare(%d): %p", opts, looper);
    return looper;
}

void ALooper_acquire(ALooper *looper) {
    android_atomic_inc(&looper->refCount);
    so_logd("ALooper_acquire(%p)", looper);
}

void ALooper_release(ALooper *looper) {
    if (android_atomic_dec(&looper->refCount) == 1) {
        looper_dtor(looper);
        free(looper);
        pthread_setspecific(gTLSKey, NULL);
    }
    so_logd("ALooper_release(%p)", looper);
}

static int looper_poll_inner(ALooper *looper, int timeoutMillis) {
#if DEBUG_POLL_AND_WAKE
    so_logd("%p ~ pollOnce - waiting: timeoutMillis=%d", looper, timeoutMillis);
#endif

    // Poll.
    int result = ALOOPER_POLL_WAKE;
    looper_clear_responses(looper);

    struct epoll_event_bionic events[EPOLL_MAX_EVENTS];
    int                       event_count = epoll_wait_soloader(looper->epollFd, events, EPOLL_MAX_EVENTS, timeoutMillis);

    // Acquire lock.
    pthread_mutex_lock(&looper->lock);

    // Check for poll error.
    if (event_count < 0) {
        if (errno == EINTR) {
            goto Done;
        }
        so_logw("Poll failed with an unexpected error, errno=%d", errno);
        result = ALOOPER_POLL_ERROR;
        goto Done;
    }

    // Check for poll timeout.
    if (event_count == 0) {
#if DEBUG_POLL_AND_WAKE
        so_logd("%p ~ pollOnce - timeout", looper);
#endif
        result = ALOOPER_POLL_TIMEOUT;
        goto Done;
    }

    // Handle all events.
#if DEBUG_POLL_AND_WAKE
    so_logd("%p ~ pollOnce - handling events from %d fds", looper, event_count);
#endif

    looper->responses = malloc(event_count * sizeof(Response));
    if (!looper->responses) {
        so_logf("Could not alloc memory for responses!");
        result = ALOOPER_POLL_ERROR;
        goto Done;
    }

    for (int i = 0; i < event_count; i++) {
        int      fd          = events[i].data.fd;
        uint32_t epollEvents = events[i].events;
        if (fd == looper->wakeReadPipeFd) {
            // awoken
            if (epollEvents & EPOLLIN) {
                char buffer[16];
                int  read_len;
                do {
                    read_len = read(looper->wakeReadPipeFd, buffer, sizeof(buffer));
                } while ((read_len == -1 && errno == EINTR) || read_len == sizeof(buffer));
            } else {
                so_logw("Ignoring unexpected epoll events 0x%x on wake read pipe.", epollEvents);
            }
        } else {
            Request *request = looper_find_request(looper, fd);
            if (request) {
                int events = 0;
                if (epollEvents & EPOLLIN) events |= ALOOPER_EVENT_INPUT;
                if (epollEvents & EPOLLOUT) events |= ALOOPER_EVENT_OUTPUT;
                if (epollEvents & EPOLLERR) events |= ALOOPER_EVENT_ERROR;
                if (epollEvents & EPOLLHUP) events |= ALOOPER_EVENT_HANGUP;

                Response *response = &looper->responses[looper->responseCount++];
                response->events   = events;
                response->request  = *request;
            } else {
                so_logw(
                    "Ignoring unexpected epoll events 0x%x on fd %d that is "
                    "no longer registered.",
                    epollEvents, fd);
            }
        }
    }

Done:;
    // Release lock.
    pthread_mutex_unlock(&looper->lock);

    // Invoke all response callbacks.
    for (size_t i = 0; i < looper->responseCount; i++) {
        Response *response = &looper->responses[i];
        if (response->request.ident == ALOOPER_POLL_CALLBACK) {
            int                  fd       = response->request.fd;
            int                  events   = response->events;
            void                *data     = response->request.data;
            ALooper_callbackFunc callback = response->request.callback;
#if DEBUG_POLL_AND_WAKE || DEBUG_CALLBACKS
            so_logd("%p ~ pollOnce - invoking fd event callback %p: fd=%d, events=0x%x, data=%p",
                    looper, callback, fd, events, data);
#endif
            int callbackResult = callback(fd, events, data);
            if (callbackResult == 0) {
                ALooper_removeFd(looper, fd);
            }
            result = ALOOPER_POLL_CALLBACK;
        }
    }

    return result;
}

int ALooper_pollOnce(int timeoutMillis, int *outFd, int *outEvents, void **outData) {
    ALooper *looper = looper_for_thread();
    if (looper == NULL) {
        so_loge("ALooper_pollOnce: No looper for this thread!");
        return ALOOPER_POLL_ERROR;
    }

    int result = 0;

    for (;;) {
        while (looper->responseIndex < looper->responseCount) {
            Response *response = &looper->responses[looper->responseIndex++];

            int ident = response->request.ident;
            if (ident >= 0) {
                int   fd     = response->request.fd;
                int   events = response->events;
                void *data   = response->request.data;
#if DEBUG_POLL_AND_WAKE
                so_logd(
                    "%p ~ pollOnce - returning signalled identifier %d: "
                    "fd=%d, events=0x%x, data=%p",
                    looper, ident, fd, events, data);
#endif
                if (outFd != NULL) *outFd = fd;
                if (outEvents != NULL) *outEvents = events;
                if (outData != NULL) *outData = data;
                so_logd("ALooper_pollOnce(%d,%p,%p,%p): %d", timeoutMillis, outFd, outEvents, outData, ident);
                return ident;
            }
        }

        if (result != 0) {
#if DEBUG_POLL_AND_WAKE
            so_logd("%p ~ pollOnce - returning result %d", looper, result);
#endif
            if (outFd != NULL) *outFd = 0;
            if (outEvents != NULL) *outEvents = 0;
            if (outData != NULL) *outData = NULL;
            so_logd("ALooper_pollOnce(%d,%p,%p,%p): %d", timeoutMillis, outFd, outEvents, outData, result);
            return result;
        }

        result = looper_poll_inner(looper, timeoutMillis);
    }
}

int ALooper_pollAll(int timeoutMillis, int *outFd, int *outEvents, void **outData) {
    int result;

    if (timeoutMillis <= 0) {
        do {
            result = ALooper_pollOnce(timeoutMillis, outFd, outEvents, outData);
        } while (result == ALOOPER_POLL_CALLBACK);
    } else {
        nsecs_t endTime = systemTime(SYSTEM_TIME_MONOTONIC) + milliseconds_to_nanoseconds(timeoutMillis);

        for (;;) {
            int result = ALooper_pollOnce(timeoutMillis, outFd, outEvents, outData);
            if (result != ALOOPER_POLL_CALLBACK) {
                break;
            }

            nsecs_t now   = systemTime(SYSTEM_TIME_MONOTONIC);
            timeoutMillis = toMillisecondTimeoutDelay(now, endTime);
            if (timeoutMillis == 0) {
                result = ALOOPER_POLL_TIMEOUT;
                break;
            }
        }
    }

    so_logd("ALooper_pollAll(%d,%p,%p,%p): %d", timeoutMillis, outFd, outEvents, outData, result);
    return result;
}

void ALooper_wake(ALooper *looper) {
    int nWrite;
    do {
        nWrite = write(looper->wakeWritePipeFd, "W", 1);
    } while (nWrite == -1 && errno == EINTR);

    if (nWrite != 1) {
        if (errno != EAGAIN) {
            so_logw("Could not write wake signal, errno=%d", errno);
        }
    }

    so_logd("ALooper_wake(%p)", looper);
}

int ALooper_addFd(ALooper *looper, int fd, int ident, int events,
                  ALooper_callbackFunc callback, void *data) {
    if (!callback) {
        if (!looper->allowNonCallbacks) {
            so_loge("ALooper_addFd: Invalid attempt to set NULL callback but not allowed for this looper.");
            return -1;
        }
        if (ident < 0) {
            so_loge("ALooper_addFd: Invalid attempt to set NULL callback with ident < 0.");
            return -1;
        }
    } else {
        ident = ALOOPER_POLL_CALLBACK;
    }

    int epollEvents = 0;
    if (events & ALOOPER_EVENT_INPUT) epollEvents |= EPOLLIN;
    if (events & ALOOPER_EVENT_OUTPUT) epollEvents |= EPOLLOUT;

    struct epoll_event_bionic eventItem;
    memset(&eventItem, 0, sizeof(struct epoll_event_bionic));  // zero out unused members of data field union
    eventItem.events  = epollEvents;
    eventItem.data.fd = fd;

    pthread_mutex_lock(&looper->lock);

    Request *request = looper_find_request(looper, fd);

    if (request) {
        int result = epoll_ctl_soloader(looper->epollFd, EPOLL_CTL_MOD, fd, &eventItem);
        if (result < 0) {
            so_loge("ALooper_addFd: Error adding epoll events for fd %d, errno=%d", fd, errno);
            pthread_mutex_unlock(&looper->lock);
            return -1;
        }

        // 修改条目
        request->fd       = fd;
        request->ident    = ident;
        request->callback = callback;
        request->data     = data;
    } else {
        request = malloc(sizeof(Request));
        if (!request) {
            so_logf("ALooper_addFd: Could not alloc memory for request.");
            return -1;
        }
        request->fd       = fd;
        request->ident    = ident;
        request->callback = callback;
        request->data     = data;
        request->prev     = NULL;
        request->next     = NULL;

        int result = epoll_ctl_soloader(looper->epollFd, EPOLL_CTL_ADD, fd, &eventItem);
        if (result < 0) {
            so_loge("ALooper_addFd: Error adding epoll events for fd %d, errno=%d", fd, errno);
            free(request);
            pthread_mutex_unlock(&looper->lock);
            return -1;
        }

        // 添加条目
        if (!looper->requestHead) {
            looper->requestHead = request;
            looper->requestTail = request;
        } else {
            looper->requestTail->next = request;
            request->prev             = looper->requestTail;
            looper->requestTail       = request;
        }
    }

    so_logd("ALooper_addFd(%p,%d,%d,%d,%p,%p): 1", looper, fd, ident, events, callback, data);
    pthread_mutex_unlock(&looper->lock);
    return 1;
}

int ALooper_removeFd(ALooper *looper, int fd) {
    pthread_mutex_lock(&looper->lock);

    Request *request = looper_find_request(looper, fd);

    if (!request) {
        so_logd("ALooper_removeFd: not found request from fd %d.", fd);
        pthread_mutex_unlock(&looper->lock);
        return 0;
    }

    int result = epoll_ctl_soloader(looper->epollFd, EPOLL_CTL_DEL, fd, NULL);
    if (result < 0) {
        so_loge("ALooper_addFd: Error removing epoll events for fd %d, errno=%d", fd, errno);
        pthread_mutex_unlock(&looper->lock);
        return -1;
    }

    // 删除条目
    if (request->prev) {
        request->prev->next = request->next;
    } else {
        looper->requestHead = request->next;
    }
    if (request->next) {
        request->next->prev = request->prev;
    } else {
        looper->requestTail = request->prev;
    }
    free(request);

    so_logd("ALooper_removeFd(%p,%d): 1", looper, fd);
    pthread_mutex_unlock(&looper->lock);
    return 1;
}
