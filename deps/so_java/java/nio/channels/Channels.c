#include <stddef.h>

#include <so_java/java/lang/Class.h>
#include <so_java/java/nio/channels/Channels.h>

static JClass sClass = NULL;

static JClassMeta class_meta = {
    .name          = "java/nio/channels/Channels",
    .super_name    = NULL,
    .symbols       = NULL,
    .num_symbols   = 0,
    .instance_size = 0,
    .dtor          = NULL,
};

void setup_java_nio_channels_Channels() {
    sClass = JavaCreateClass(&class_meta);
}
