#include <so_common/so_logger.h>

#include "input.h"
#include "input_queue.h"

#include <errno.h>
#include <poll.h>

typedef struct InputEvent {
    int32_t type;
    int32_t deviceId;
    int32_t source;
} InputEvent;

typedef struct KeyEvent {
    InputEvent event;
    int32_t    action;
    int32_t    flags;
    int32_t    keyCode;
    int32_t    scanCode;
    int32_t    metaState;
    int32_t    repeatCount;
    int64_t    downTime;
    int64_t    eventTime;
} KeyEvent;

typedef struct MotionEvent {
    InputEvent event;
    int32_t    action;
    int32_t    flags;
    int32_t    metaState;
    int32_t    edgeFlags;
    int64_t    downTime;
    int64_t    eventTime;
    float      xOffset;
    float      yOffset;
    float      xPrecision;
    float      yPrecision;
    size_t     pointerCount;
    int32_t    *pointerIds;
} MotionEvent;

int32_t AInputEvent_getType(const AInputEvent *event) {
    return ((InputEvent *)event)->type;
}

int32_t AInputEvent_getDeviceId(const AInputEvent *event) {
    return ((InputEvent *)event)->deviceId;
}

int32_t AInputEvent_getSource(const AInputEvent *event) {
    return ((InputEvent *)event)->source;
}

int32_t AKeyEvent_getAction(const AInputEvent *key_event) {
    return ((KeyEvent *)key_event)->action;
}

int32_t AKeyEvent_getFlags(const AInputEvent *key_event) {
    return ((KeyEvent *)key_event)->flags;
}

int32_t AKeyEvent_getKeyCode(const AInputEvent *key_event) {
    return ((KeyEvent *)key_event)->keyCode;
}

int32_t AKeyEvent_getScanCode(const AInputEvent *key_event) {
    return ((KeyEvent *)key_event)->scanCode;
}

int32_t AKeyEvent_getMetaState(const AInputEvent *key_event) {
    return ((KeyEvent *)key_event)->metaState;
}

int32_t AKeyEvent_getRepeatCount(const AInputEvent *key_event) {
    return ((KeyEvent *)key_event)->repeatCount;
}

int64_t AKeyEvent_getDownTime(const AInputEvent *key_event) {
    return ((KeyEvent *)key_event)->downTime;
}

int64_t AKeyEvent_getEventTime(const AInputEvent *key_event) {
    return ((KeyEvent *)key_event)->eventTime;
}

int32_t AMotionEvent_getAction(const AInputEvent *motion_event) {
    return ((MotionEvent *)motion_event)->action;
}

int32_t AMotionEvent_getFlags(const AInputEvent *motion_event) {
    return ((MotionEvent *)motion_event)->flags;
}

int32_t AMotionEvent_getMetaState(const AInputEvent *motion_event) {
    return ((MotionEvent *)motion_event)->metaState;
}

int32_t AMotionEvent_getEdgeFlags(const AInputEvent *motion_event) {
    return ((MotionEvent *)motion_event)->edgeFlags;
}

int64_t AMotionEvent_getDownTime(const AInputEvent *motion_event) {
    return ((MotionEvent *)motion_event)->downTime;
}

int64_t AMotionEvent_getEventTime(const AInputEvent *motion_event) {
    return ((MotionEvent *)motion_event)->eventTime;
}

float AMotionEvent_getXOffset(const AInputEvent *motion_event) {
    return ((MotionEvent *)motion_event)->xOffset;
}

float AMotionEvent_getYOffset(const AInputEvent *motion_event) {
    return ((MotionEvent *)motion_event)->yOffset;
}

float AMotionEvent_getXPrecision(const AInputEvent *motion_event) {
    return ((MotionEvent *)motion_event)->xPrecision;
}

float AMotionEvent_getYPrecision(const AInputEvent *motion_event) {
    return ((MotionEvent *)motion_event)->yPrecision;
}

size_t AMotionEvent_getPointerCount(const AInputEvent *motion_event) {
    return ((MotionEvent *)motion_event)->pointerCount;
}

int32_t AMotionEvent_getPointerId(const AInputEvent *motion_event, size_t pointer_index) {
    return ((MotionEvent *)motion_event)->pointerIds[pointer_index];
}

float AMotionEvent_getRawX(const AInputEvent *motion_event, size_t pointer_index) {
    return 0;
}

float AMotionEvent_getRawY(const AInputEvent *motion_event, size_t pointer_index) {
    return 0;
}

float AMotionEvent_getX(const AInputEvent *motion_event, size_t pointer_index) {
    return 0;
}

float AMotionEvent_getY(const AInputEvent *motion_event, size_t pointer_index) {
    return 0;
}

float AMotionEvent_getPressure(const AInputEvent *motion_event, size_t pointer_index) {
    return 0;
}

float AMotionEvent_getSize(const AInputEvent *motion_event, size_t pointer_index) {
    return 0;
}

float AMotionEvent_getTouchMajor(const AInputEvent *motion_event, size_t pointer_index) {
    return 0;
}

float AMotionEvent_getTouchMinor(const AInputEvent *motion_event, size_t pointer_index) {
    return 0;
}

float AMotionEvent_getToolMajor(const AInputEvent *motion_event, size_t pointer_index) {
    return 0;
}

float AMotionEvent_getToolMinor(const AInputEvent *motion_event, size_t pointer_index) {
    return 0;
}

float AMotionEvent_getOrientation(const AInputEvent *motion_event, size_t pointer_index) {
    return 0;
}

size_t AMotionEvent_getHistorySize(const AInputEvent *motion_event) {
    return 0;
}

int64_t AMotionEvent_getHistoricalEventTime(const AInputEvent *motion_event,
                                            size_t       history_index) {
    return 0;
}

float AMotionEvent_getHistoricalRawX(const AInputEvent *motion_event, size_t pointer_index,
                                     size_t history_index) {
    return 0;
}

float AMotionEvent_getHistoricalRawY(const AInputEvent *motion_event, size_t pointer_index,
                                     size_t history_index) {
    return 0;
}

float AMotionEvent_getHistoricalX(const AInputEvent *motion_event, size_t pointer_index,
                                  size_t history_index) {
    return 0;
}

float AMotionEvent_getHistoricalY(const AInputEvent *motion_event, size_t pointer_index,
                                  size_t history_index) {
    return 0;
}

float AMotionEvent_getHistoricalPressure(const AInputEvent *motion_event, size_t pointer_index,
                                         size_t history_index) {
    return 0;
}

float AMotionEvent_getHistoricalSize(const AInputEvent *motion_event, size_t pointer_index,
                                     size_t history_index) {
    return 0;
}

float AMotionEvent_getHistoricalTouchMajor(const AInputEvent *motion_event, size_t pointer_index,
                                           size_t history_index) {
    return 0;
}

float AMotionEvent_getHistoricalTouchMinor(const AInputEvent *motion_event, size_t pointer_index,
                                           size_t history_index) {
    return 0;
}

float AMotionEvent_getHistoricalToolMajor(const AInputEvent *motion_event, size_t pointer_index,
                                          size_t history_index) {
    return 0;
}

float AMotionEvent_getHistoricalToolMinor(const AInputEvent *motion_event, size_t pointer_index,
                                          size_t history_index) {
    return 0;
}

float AMotionEvent_getHistoricalOrientation(const AInputEvent *motion_event, size_t pointer_index,
                                            size_t history_index) {
    return 0;
}
