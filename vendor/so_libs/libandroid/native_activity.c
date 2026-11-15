
#include <so_common/so_logger.h>
#include <stdbool.h>

#include "native_activity.h"

static bool g_activity_finishing = false;

bool native_activity_is_finishing(void) {
    return g_activity_finishing;
}

void ANativeActivity_finish(ANativeActivity *activity) {
    // 宿主标记：游戏请求退出，loader 的生命周期驱动据此收尾
    g_activity_finishing = true;
    so_logd("ANativeActivity_finish(%p)", activity);
}

void ANativeActivity_setWindowFormat(ANativeActivity *activity, int32_t format) {
    // android_NativeActivity_setWindowFormat(activity, format);
    so_logd("ANativeActivity_setWindowFormat(%p,%d)", activity, format);
}

void ANativeActivity_setWindowFlags(ANativeActivity *activity,
                                    uint32_t addFlags, uint32_t removeFlags) {
    // android_NativeActivity_setWindowFlags(activity, addFlags, addFlags | removeFlags);
    so_logd("ANativeActivity_finish(%p,%d,%d)", activity, addFlags, removeFlags);
}

void ANativeActivity_showSoftInput(ANativeActivity *activity, uint32_t flags) {
    so_logd("ANativeActivity_showSoftInput(%p,%d)", activity, flags);
}

void ANativeActivity_hideSoftInput(ANativeActivity *activity, uint32_t flags) {
    // android_NativeActivity_hideSoftInput(activity, flags);
    so_logd("ANativeActivity_finish(%p,%d)", activity, flags);
}