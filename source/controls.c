#include <math.h>
#include <stdint.h>

#include <psp2/ctrl.h>
#include <psp2/kernel/clib.h>
#include <psp2/motion.h>
#include <psp2/touch.h>

#include "android/keycodes.h"
#include "java/Accelerometer.h"
#include "java/Bio4PreActivity.h"

#include "config.h"
#include "patch.h"

typedef struct {
    uint32_t native;
    uint32_t mapped;
    int      repeat_count;
} ButtonRemap;

typedef struct {
    int   id;
    float currX;
    float currY;
    float prevX;
    float prevY;
} UITouchInfo;

typedef struct {
    uint32_t    button;
    int         map_x;
    int         map_y;
    UITouchInfo info;
} ButtonToTouch;

typedef struct {
    int   isSetup;
    float currX;
    float currY;
    float currZ;
    float prevX;
    float prevY;
    float prevZ;
} SensorInfo;

#define JOYSTICK_CENTER      127.0f
#define JOYSTICK_DEADZONE    24.0f
#define JOYSTICK_IS_MOVED(v) (((v) > JOYSTICK_CENTER + JOYSTICK_DEADZONE) || \
                              ((v) < JOYSTICK_CENTER - JOYSTICK_DEADZONE))

#define JOYSTICK_SCREEN_CENTER_X   152.0f
#define JOYSTICK_SCREEN_CENTER_Y   400.0f
#define JOYSTICK_SCREEN_MAX_RADIUS 70.0f

typedef enum {
    BTN_IDX_UP = 0,
    BTN_IDX_DOWN,
    BTN_IDX_LEFT,
    BTN_IDX_RIGHT,
    BTN_IDX_CROSS,
    BTN_IDX_CIRCLE,
    BTN_IDX_SQUARE,
    BTN_IDX_TRIANGLE,
    BTN_IDX_L1,
    BTN_IDX_R1,
    BTN_IDX_START,
    BTN_IDX_SELECT,
    BTN_IDX_MAX,
} ButtonIndex;

static ButtonRemap btn_remap[] = {
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

static ButtonToTouch btn2touch_main[] = {
    // {SCE_CTRL_UP,       152.0f, 330.0f, {0, -1, -1, -1, -1}},
    // {SCE_CTRL_DOWN,     152.0f, 470.0f, {0, -1, -1, -1, -1}},
    // {SCE_CTRL_LEFT,     82.0f,  400.0f, {0, -1, -1, -1, -1}},
    // {SCE_CTRL_RIGHT,    222.0f, 400.0f, {0, -1, -1, -1, -1}},

    {SCE_CTRL_CROSS,    866.0f, 240.0f, {0, -1, -1, -1, -1}}, // 切换为行走状态
    {SCE_CTRL_CIRCLE,   866.0f, 344.0f, {0, -1, -1, -1, -1}}, // 开枪
    {SCE_CTRL_SQUARE,   776.0f, 332.0f, {0, -1, -1, -1, -1}}, // 开门/拾取
    {SCE_CTRL_TRIANGLE, 814.0f, 450.0f, {0, -1, -1, -1, -1}}, // 菜刀
    {SCE_CTRL_LTRIGGER, 0.0f,   0.0f,   {0, -1, -1, -1, -1}}, // 补充子弹
    {SCE_CTRL_RTRIGGER, 152.0f, 470.0f, {0, -1, -1, -1, -1}}, // 开枪
    {SCE_CTRL_START,    92.0f,  146.0f, {0, -1, -1, -1, -1}}, // 说明
    {SCE_CTRL_SELECT,   62.0f,  48.0f,  {0, -1, -1, -1, -1}}, // 菜单
};

static ButtonToTouch btn2touch_win1[] = {
    {SCE_CTRL_START,  74.0f,  468.0f, {0, -1, -1, -1, -1}}, // 跳过对话
    {SCE_CTRL_CIRCLE, 898.0f, 452.0f, {0, -1, -1, -1, -1}}, // 下个对话
};

static ButtonToTouch btn2touch_win2[] = {
    {SCE_CTRL_CROSS,    868.0f, 450.0f, {0, -1, -1, -1, -1}}, // 取消
    {SCE_CTRL_CIRCLE,   868.0f, 346.0f, {0, -1, -1, -1, -1}}, // 收取
    {SCE_CTRL_TRIANGLE, 868.0f, 242.0f, {0, -1, -1, -1, -1}}, // 使用
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
    Bio4PreActivity_native_keyUp(&_jni_env, Bio4PreActivity_jobject, keyCode, event)

static int joystick_screen_offset(int v) {
    float off = v - JOYSTICK_CENTER;
    if (off > JOYSTICK_DEADZONE) {
        return (off - JOYSTICK_DEADZONE) / (JOYSTICK_CENTER - JOYSTICK_DEADZONE) * JOYSTICK_SCREEN_MAX_RADIUS;
    }
    if (off < -JOYSTICK_DEADZONE) {
        return (off + JOYSTICK_DEADZONE) / (JOYSTICK_CENTER - JOYSTICK_DEADZONE) * JOYSTICK_SCREEN_MAX_RADIUS;
    }

    return 0;
}

void controls_poll() {
    static UITouchInfo touchInfo[2] = {
        {0, -1, -1, -1, -1},
        {1, -1, -1, -1, -1},
    };
    static UITouchInfo joyInfo = {
        0, -1, -1, -1, -1};
    static SensorInfo sensorInfo = {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
    };
    static int      turnKeyPressed = 0;
    static uint32_t old_buttons    = 0;

    SceCtrlData          pad;
    SceTouchData         touch;
    SceMotionSensorState sensor;
    sceCtrlPeekBufferPositive(0, &pad, 1);
    sceTouchPeek(SCE_TOUCH_PORT_FRONT, &touch, 1);
    sceMotionGetSensorState(&sensor, 1);

    int map_port = 0;

    // ButtonToTouch
    ButtonToTouch *btn2touch       = btn2touch_main;
    int            btn2touch_count = sizeof(btn2touch_main) / sizeof(ButtonToTouch);
    if (bio4_window_type) {
        if (bio4_window_type == 1 || bio4_window_type == 3) {
            btn2touch       = btn2touch_win1;
            btn2touch_count = sizeof(btn2touch_win1) / sizeof(ButtonToTouch);
        } else if (bio4_window_type == 2) {
            btn2touch       = btn2touch_win2;
            btn2touch_count = sizeof(btn2touch_win2) / sizeof(ButtonToTouch);
        }
    }

    for (int i = 0; i < btn2touch_count; i++) {
        uint32_t     button      = btn2touch[i].button;
        int          map_x       = btn2touch[i].map_x;
        int          map_y       = btn2touch[i].map_y;
        UITouchInfo *info        = &btn2touch[i].info;
        int          pressed     = ((pad.buttons & button) == button);
        int          old_pressed = ((old_buttons & button) == button);

        if (btn2touch == btn2touch_main) {
            if (turnKeyPressed) {  // 转身触发
                if (btn2touch[i].button == SCE_CTRL_RTRIGGER) {
                    if (turnKeyPressed == 1) {
                        pressed = 1;
                        turnKeyPressed++;
                    } else if (turnKeyPressed == 2) {
                        pressed = 0;
                        turnKeyPressed++;
                    } else if (turnKeyPressed == 3) {
                        pressed = 1;
                        turnKeyPressed++;
                    } else {
                        turnKeyPressed = 0;
                    }
                }
            }
        }

        if (pressed) {
            info->id    = map_port++;
            info->currX = map_x;
            info->currY = map_y;

            if (info->prevX == -1 || info->prevY == -1) {
                if (btn2touch == btn2touch_main) {
                    if (btn2touch[i].button == SCE_CTRL_LTRIGGER) {
                        Bio4PreActivity_native_onShake(&_jni_env, Bio4PreActivity_jobject);
                    } else {
                        if (btn2touch[i].button == SCE_CTRL_RTRIGGER) {
                            if (!turnKeyPressed) turnKeyPressed = 1;
                        }
                        native_touchDown(&info, info->currX, info->currY, info->prevX, info->prevY);
                    }
                } else {
                    native_touchDown(&info, info->currX, info->currY, info->prevX, info->prevY);
                }
            } else {
                if (info->currX != info->prevX || info->currY != info->prevY) {
                    native_touchMove(&info, info->currX, info->currY, info->prevX, info->prevY);
                }
            }

        } else {
            if (info->prevX != -1 || info->prevY != -1) {
                info->id = map_port++;
                native_touchUp(&info, info->currX, info->currY, info->prevX, info->prevY);
            }
            info->currX = -1;
            info->currY = -1;
        }
        info->prevX = info->currX;
        info->prevY = info->currY;
    }

    old_buttons = pad.buttons;

    if (turnKeyPressed) {
        pad.lx = JOYSTICK_CENTER;
        pad.ly = JOYSTICK_CENTER;
    }

    // JoystickToTouch
    if (JOYSTICK_IS_MOVED(pad.lx) || JOYSTICK_IS_MOVED(pad.ly)) {
        joyInfo.id    = map_port++;
        joyInfo.currX = JOYSTICK_SCREEN_CENTER_X + joystick_screen_offset(pad.lx);
        joyInfo.currY = JOYSTICK_SCREEN_CENTER_Y + joystick_screen_offset(pad.ly);

        if (joyInfo.prevX == -1 || joyInfo.prevY == -1) {
            native_touchDown(&joyInfo, joyInfo.currX, joyInfo.currY, joyInfo.prevX, joyInfo.prevY);
        } else {
            native_touchMove(&joyInfo, joyInfo.currX, joyInfo.currY, joyInfo.prevX, joyInfo.prevY);
        }
    } else {
        if (joyInfo.prevX != -1 || joyInfo.prevY != -1) {
            joyInfo.id = map_port++;
            native_touchUp(&joyInfo, joyInfo.currX, joyInfo.currY, joyInfo.prevX, joyInfo.prevY);
        }
    }
    joyInfo.prevX = joyInfo.currX;
    joyInfo.prevY = joyInfo.currY;

    // Touch
    for (int i = 0; i < 2; i++) {
        UITouchInfo *info = &touchInfo[i];
        if (i < touch.reportNum) {
            info->id    = map_port++;
            info->currX = (float)touch.report[i].x * (float)SCREEN_W / 1920.0f;
            info->currY = (float)touch.report[i].y * (float)SCREEN_H / 1088.0f;

            if (info->prevX == -1 || info->prevY == -1) {
                native_touchDown(&info, info->currX, info->currY, info->prevX, info->prevY);
            } else if (info->currX != info->prevX || info->currY != info->prevY) {
                native_touchMove(&info, info->currX, info->currY, info->prevX, info->prevY);
            }
        } else {
            if (info->prevX != -1 || info->prevY != -1) {
                info->id = map_port++;
                native_touchUp(&info, info->currX, info->currY, info->prevX, info->prevY);
            }
            info->currX = -1;
            info->currY = -1;
        }
        info->prevX = info->currX;
        info->prevY = info->currY;
    }

    // Sensor
    sensorInfo.currX = sensor.accelerometer.x;
    sensorInfo.currY = sensor.accelerometer.y;
    sensorInfo.currZ = sensor.accelerometer.z;

    if (!sensorInfo.isSetup) {
        sensorInfo.prevX   = sensorInfo.currX;
        sensorInfo.prevY   = sensorInfo.currY;
        sensorInfo.prevZ   = sensorInfo.currZ;
        sensorInfo.isSetup = 1;
    }

    Accelerometer_onSensorChanged(sensorInfo.currX, sensorInfo.currY, sensorInfo.currZ,
                                  sensorInfo.prevX, sensorInfo.prevY, sensorInfo.prevZ);

    sensorInfo.prevX = sensorInfo.currX;
    sensorInfo.prevY = sensorInfo.currY;
    sensorInfo.prevZ = sensorInfo.currZ;
}
