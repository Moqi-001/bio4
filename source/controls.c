#include <math.h>
#include <stdint.h>

#include <psp2/ctrl.h>
#include <psp2/kernel/clib.h>
#include <psp2/touch.h>

#include "config.h"
#include "java/Bio4PreActivity.h"

#define AKEYCODE_DPAD_UP       19
#define AKEYCODE_DPAD_DOWN     20
#define AKEYCODE_DPAD_LEFT     21
#define AKEYCODE_DPAD_RIGHT    22
#define AKEYCODE_BUTTON_A      96
#define AKEYCODE_BUTTON_B      97
#define AKEYCODE_BUTTON_X      99
#define AKEYCODE_BUTTON_Y      100
#define AKEYCODE_BUTTON_L1     102
#define AKEYCODE_BUTTON_R1     103
#define AKEYCODE_BUTTON_START  108
#define AKEYCODE_BUTTON_SELECT 109

typedef struct {
    uint32_t sce_button;
    uint32_t android_button;
    int      repeat_count;
} ButtonMapping;

typedef struct {
    uint32_t sce_button;
    uint32_t x;
    uint32_t y;
} ButtonToTouchMapping;

static ButtonMapping mapping[] = {
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

typedef struct UITouchInfo {
    int   nPointerId;
    float nCurrPointX;
    float nCurrPointY;
    float nPrevPointX;
    float nPrevPointY;
} UITouchInfo;

#define navite_touchDown(pTouchInfo, currX, currY, prevX, prevY) \
    Bio4PreActivity_native_onTouchBegan(&_jni_env, Bio4PreActivity_jobject, pTouchInfo, currX, currY, prevX, prevY)
#define navite_touchMove(pTouchInfo, currX, currY, prevX, prevY) \
    Bio4PreActivity_native_onTouchMoved(&_jni_env, Bio4PreActivity_jobject, pTouchInfo, currX, currY, prevX, prevY)
#define navite_touchUp(pTouchInfo, currX, currY, prevX, prevY) \
    Bio4PreActivity_native_onTouchEnded(&_jni_env, Bio4PreActivity_jobject, pTouchInfo, currX, currY, prevX, prevY)
#define navite_keyDown(keyCode, event) \
    Bio4PreActivity_native_onKeyDown(&_jni_env, Bio4PreActivity_jobject, keyCode, event)
#define navite_keyUp(keyCode, event) \
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
    for (int i = 0; i < sizeof(mapping) / sizeof(ButtonMapping); i++) {
        int curr_pressed = ((pad.buttons & mapping[i].sce_button) == mapping[i].sce_button);
        int prev_pressed = ((prev_buttons & mapping[i].sce_button) == mapping[i].sce_button);
        if (curr_pressed) {
            if (!prev_pressed)
                navite_keyDown(mapping[i].android_button, NULL);
            mapping[i].repeat_count++;
        } else {
            if (prev_pressed)
                navite_keyUp(mapping[i].android_button, NULL);
            mapping[i].repeat_count = 0;
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
                navite_touchDown(&touchInfo[i], touchInfo[i].nCurrPointX, touchInfo[i].nCurrPointY, touchInfo[i].nPrevPointX, touchInfo[i].nPrevPointY);
            } else if (touchInfo[i].nCurrPointX != touchInfo[i].nPrevPointX || touchInfo[i].nCurrPointY != touchInfo[i].nPrevPointY) {
                navite_touchMove(&touchInfo[i], touchInfo[i].nCurrPointX, touchInfo[i].nCurrPointY, touchInfo[i].nPrevPointX, touchInfo[i].nPrevPointY);
            }
        } else {
            if (touchInfo[i].nPrevPointX != -1 || touchInfo[i].nPrevPointY != -1) {
                navite_touchUp(&touchInfo[i], touchInfo[i].nCurrPointX, touchInfo[i].nCurrPointY, touchInfo[i].nPrevPointX, touchInfo[i].nPrevPointY);
            }
            touchInfo[i].nCurrPointX = -1;
            touchInfo[i].nCurrPointY = -1;
        }
        touchInfo[i].nPrevPointX = touchInfo[i].nCurrPointX;
        touchInfo[i].nPrevPointY = touchInfo[i].nCurrPointY;
    }
}
