
#include <so_common/so_logger.h>

#include "native_activity.h"

void ANativeActivity_finish(ANativeActivity *activity) {
    // android_NativeActivity_finish(activity);
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
    // android_NativeActivity_showSoftInput(activity, flags);
    so_logd("ANativeActivity_finish(%p,%d)", activity, flags);
}

void ANativeActivity_hideSoftInput(ANativeActivity *activity, uint32_t flags) {
    // android_NativeActivity_hideSoftInput(activity, flags);
    so_logd("ANativeActivity_finish(%p,%d)", activity, flags);
}