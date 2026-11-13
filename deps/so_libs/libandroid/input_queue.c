
#include "input_queue.h"

struct AInputQueue {
    /* data */
};

AInputQueue *AInputQueue_create() {

}


void AInputQueue_attachLooper(AInputQueue *queue, ALooper *looper,
                              int ident, ALooper_callbackFunc callback, void *data) {
}

void AInputQueue_detachLooper(AInputQueue *queue) {
}

int32_t AInputQueue_hasEvents(AInputQueue *queue) {
    return 0;
}

int32_t AInputQueue_getEvent(AInputQueue *queue, AInputEvent **outEvent) {
    return -1;
}

int32_t AInputQueue_preDispatchEvent(AInputQueue *queue, AInputEvent *event) {
    return -1;
}

void AInputQueue_finishEvent(AInputQueue *queue, AInputEvent *event, int handled) {
}
