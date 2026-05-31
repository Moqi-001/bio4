#ifndef BIONIC_SYS_SYSTEM_PROPERTIES_H
#define BIONIC_SYS_SYSTEM_PROPERTIES_H

#ifdef __cplusplus
extern "C" {
#endif

int __system_property_get_soloader(const char *name, char *value);

#ifdef __cplusplus
};
#endif

#endif