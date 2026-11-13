#include <string.h>

#include <so_common/so_logger.h>

int __system_property_get_soloader(const char *name, char *value) {
    so_logw("__system_property_get(%s, %p): not implemented!", name, value);
    strncpy(value, "psvita", 7);
    return 7;
}
