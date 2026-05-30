#ifndef SOLOADED_PATCH_H
#define SOLOADED_PATCH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <so_linker/so_linker.h>

int soloader_patch_mce(so_module *mod);
int soloader_patch_bio4(so_module *mod);

#ifdef __cplusplus
};
#endif

#endif