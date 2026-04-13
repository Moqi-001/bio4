// #include <string.h>
// #include <stdlib.h>
#include <stdio.h>

#include <kubridge.h>
#include <so_util/so_util.h>
#include "config.h"

#include "reimpl/libGgee/ggee.h"
#include "reimpl/libKD/kd.h"
#include "reimpl/libLD/ld.h"
#include "utils/logger.h"

#define _DWORD uintptr_t
extern so_module so_mod;

LdModule *bootstrap_mod                                      = NULL;
int (*bootstrap_start)(int argc, char *argv[], char *envp[]) = NULL;
int bootstrap_start_soloader(int argc, char *argv[], char *envp[])
{
    return 0;
}

char* _ZN8JavaCall23callStaticCharPtrMethodEPKcS1_z()
{
    debugPrintf("_ZN8JavaCall23callStaticCharPtrMethodEPKcS1_z");
    return strdup(getPackagePath(0,0));
}

void so_patch(void) {
    // Sample hook
    hook_addr((uintptr_t)so_symbol(&so_mod, "_ZN8JavaCall23callStaticCharPtrMethodEPKcS1_z"), (uintptr_t)&_ZN8JavaCall23callStaticCharPtrMethodEPKcS1_z);

    // int * UserConfirm = (int *) so_symbol(&so_mod, "_UserConfirm");
    // * UserConfirm = 1;

    // char ** GVfs = (char **) so_symbol(&so_mod, "GVfs");
    // *GVfs = malloc(60);
    // //**GVfs = 0;
    // memset(*GVfs,0,60);
    // debugPrintf("set GVfs over");
}
