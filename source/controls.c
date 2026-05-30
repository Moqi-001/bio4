#include <math.h>
#include <stdint.h>

#include <psp2/ctrl.h>
#include <psp2/kernel/clib.h>
#include <psp2/touch.h>

#include "android/keycodes.h"
#include "java/Bio4PreActivity.h"

#include "config.h"

typedef struct {
    uint32_t native;
    uint32_t mapped;
    int      repeat_count;
} ButtonRemap;

typedef struct {
    uint32_t button;
    uint32_t x;
    uint32_t y;
} ButtonToTouch;

typedef struct UITouchInfo {
    int   nPointerId;
    float nCurrPointX;
    float nCurrPointY;
    float nPrevPointX;
    float nPrevPointY;
} UITouchInfo;

static ButtonRemap remap[] = {
    {SCE_CTRL_UP,       AKEYCODE_DPAD_UP,       0},
    {SCE_CTRL_DOWN,     AKEYCODE_DPAD_DOWN,     0},
    {SCE_CTRL_LEFT,     AKEYCODE_DPAD_LEFT,     0},
    {SCE_CTRL_RIGHT,    AKEYCODE_DPAD_RIGHT,    0},
    {SCE_CTRL_CROSS,    AKEYCODE_BUTTON_B,      0},
    {SCE_CTRL_CIRCLE,   AKEYCODE_BUTTON_A,      0},
    {SCE_CTRL_SQUARE,   AKEYCODE_BUTTON_X,      0},
    {SCE_CTRL_TRIANGLE, AKEYCODE_BUTTON_Y,      0},
    {SCE_CTRL_LTRIGGER, AKEYCODE_BUTTON_L1,     0},
    {SCE_CTRL_RTRIGGER, AKEYCODE_BUTTON_R1,     0},
    {SCE_CTRL_START,    AKEYCODE_BUTTON_START,  0},
    {SCE_CTRL_SELECT,   AKEYCODE_BUTTON_SELECT, 0},
};

#define native_touchDown(pTouchInfo, currX, currY, prevX, prevY) \
    Bio4PreActivity_native_onTouchBegan(&_jni_env, Bio4PreActivity_jobject, pTouchInfo, currX, currY, prevX, prevY)
#define native_touchMove(pTouchInfo, currX, currY, prevX, prevY) \
    Bio4PreActivity_native_onTouchMoved(&_jni_env, Bio4PreActivity_jobject, pTouchInfo, currX, currY, prevX, prevY)
#define native_touchUp(pTouchInfo, currX, currY, prevX, prevY) \
    Bio4PreActivity_native_onTouchEnded(&_jni_env, Bio4PreActivity_jobject, pTouchInfo, currX, currY, prevX, prevY)
#define native_keyDown(keyCode, event) \
    Bio4PreActivity_native_onKeyDown(&_jni_env, Bio4PreActivity_jobject, keyCode, event)
#define native_keyUp(keyCode, event) \
    Bio4PreActivity_native_onKeyDown(&_jni_env, Bio4PreActivity_jobject, keyCode, event)

void controls_poll() {
    static uint32_t    prev_buttons = 0;
    static UITouchInfo touchInfo[2] = {
        {-1, -1, -1, -1, -1},
        {-1, -1, -1, -1, -1},
    };

    SceCtrlData  pad;
    SceTouchData touch;
    sceCtrlPeekBufferPositive(0, &pad, 1);
    sceTouchPeek(SCE_TOUCH_PORT_FRONT, &touch, 1);

    // Button
    if (pad.lx > 162) {
        pad.buttons |= SCE_CTRL_RIGHT;
    } else if (pad.lx < 92) {
        pad.buttons |= SCE_CTRL_LEFT;
    }
    if (pad.ly > 162) {
        pad.buttons |= SCE_CTRL_DOWN;
    } else if (pad.ly < 92) {
        pad.buttons |= SCE_CTRL_UP;
    }
    for (int i = 0; i < sizeof(remap) / sizeof(ButtonRemap); i++) {
        int curr_pressed = ((pad.buttons & remap[i].native) == remap[i].native);
        int prev_pressed = ((prev_buttons & remap[i].native) == remap[i].native);
        if (curr_pressed) {
            if (!prev_pressed)
                native_keyDown(remap[i].mapped, (jobject)remap[i].mapped);
            remap[i].repeat_count++;
        } else {
            if (prev_pressed)
                native_keyUp(remap[i].mapped, (jobject)remap[i].mapped);
            remap[i].repeat_count = 0;
        }
    }
    prev_buttons = pad.buttons;

    // Touch
    for (int i = 0; i < 2; i++) {
        touchInfo[i].nPointerId = i;
        if (i < touch.reportNum) {
            touchInfo[i].nCurrPointX = (float)touch.report[i].x * (float)SCREEN_W / 1920.0f;
            touchInfo[i].nCurrPointY = (float)touch.report[i].y * (float)SCREEN_H / 1088.0f;

            if (touchInfo[i].nPrevPointX == -1 || touchInfo[i].nPrevPointY == -1) {
                native_touchDown(&touchInfo[i], touchInfo[i].nCurrPointX, touchInfo[i].nCurrPointY, touchInfo[i].nPrevPointX, touchInfo[i].nPrevPointY);
            } else if (touchInfo[i].nCurrPointX != touchInfo[i].nPrevPointX || touchInfo[i].nCurrPointY != touchInfo[i].nPrevPointY) {
                native_touchMove(&touchInfo[i], touchInfo[i].nCurrPointX, touchInfo[i].nCurrPointY, touchInfo[i].nPrevPointX, touchInfo[i].nPrevPointY);
            }
        } else {
            if (touchInfo[i].nPrevPointX != -1 || touchInfo[i].nPrevPointY != -1) {
                native_touchUp(&touchInfo[i], touchInfo[i].nCurrPointX, touchInfo[i].nCurrPointY, touchInfo[i].nPrevPointX, touchInfo[i].nPrevPointY);
            }
            touchInfo[i].nCurrPointX = -1;
            touchInfo[i].nCurrPointY = -1;
        }
        touchInfo[i].nPrevPointX = touchInfo[i].nCurrPointX;
        touchInfo[i].nPrevPointY = touchInfo[i].nCurrPointY;
    }
}
