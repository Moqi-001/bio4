#include <so_common/so_logger.h>
#include <so_common/so_utils.h>
#include <so_linker/so_linker.h>

#include "RefBase.h"
#include "SharedBuffer.h"
#include "String8.h"
#include "Thread.h"
#include "Timers.h"

int AZipFileRO_fake(void) {
    so_logd("[Error] Android ZipFileRO function called: unsupport!");
    return 0;
}

// ==================== libutils.so ====================
// Android 基础工具库
static so_symbol libutils_symbols[] = {
    // ===== RefBase =====
    SO_SYMBOL_FUNC("_ZN7android7RefBaseC2Ev", &ARefBase_ctor),
    SO_SYMBOL_FUNC("_ZN7android7RefBaseD2Ev", &ARefBase_dtor),
    SO_SYMBOL_FUNC("_ZN7android7RefBase10onFirstRefEv", &ARefBase_onFirstRef),
    SO_SYMBOL_FUNC("_ZN7android7RefBase15onLastStrongRefEPKv", &ARefBase_onLastStrongRef),
    SO_SYMBOL_FUNC("_ZN7android7RefBase13onLastWeakRefEPKv", &ARefBase_onLastWeakRef),
    SO_SYMBOL_FUNC("_ZN7android7RefBase20onIncStrongAttemptedEjPKv", &ARefBase_onIncStrongAttempted),
    SO_SYMBOL_FUNC("_ZNK7android7RefBase9incStrongEPKv", &ARefBase_incStrong),
    SO_SYMBOL_FUNC("_ZNK7android7RefBase9decStrongEPKv", &ARefBase_decStrong),

    // ===== SharedBuffer =====
    SO_SYMBOL_FUNC("_ZNK7android12SharedBuffer7acquireEv", &ASharedBuffer_acquire),
    SO_SYMBOL_FUNC("_ZN7android12SharedBuffer5allocEj", &ASharedBuffer_alloc),
    SO_SYMBOL_FUNC("_ZN7android12SharedBuffer7deallocEPKS0_", &ASharedBuffer_dealloc),
    SO_SYMBOL_FUNC("_ZNK7android12SharedBuffer11attemptEditEv", &ASharedBuffer_attemptEdit),
    SO_SYMBOL_FUNC("_ZNK7android12SharedBuffer10editResizeEj", &ASharedBuffer_editResize),
    SO_SYMBOL_FUNC("_ZNK7android12SharedBuffer7releaseEj", &ASharedBuffer_release),

    // ===== String8 =====
    SO_SYMBOL_FUNC("_ZN7android7String8C1Ev", &AString8_ctor),
    SO_SYMBOL_FUNC("_ZN7android7String8C1EPKc", &AString8_ctorWithCstr),
    SO_SYMBOL_FUNC("_ZN7android7String8C1ERKS0_", &AString8_ctorWithOther),
    SO_SYMBOL_FUNC("_ZN7android7String8D1Ev", &AString8_dtor),
    SO_SYMBOL_FUNC("_ZN7android7String85setToEPKc", &AString8_setToCstr),
    SO_SYMBOL_FUNC("_ZN7android7String86appendEPKc", &AString8_appendCstr),
    SO_SYMBOL_FUNC("_ZN7android7String86appendERKS0_", &AString8_append),
    SO_SYMBOL_FUNC("_ZN7android7String810appendPathEPKc", &AString8_appendPathCstr),
    SO_SYMBOL_FUNC("_ZNK7android7String810getPathDirEv", &AString8_getPathDir),
    SO_SYMBOL_FUNC("_ZNK7android7String88walkPathEPS0_", &AString8_walkPath),

    // ===== Thread =====
    SO_SYMBOL_FUNC("_ZN7android6ThreadC2Eb", &AThread_ctor),
    SO_SYMBOL_FUNC("_ZN7android6ThreadD0Ev", &AThread_dtorDeleting),
    SO_SYMBOL_FUNC("_ZN7android6ThreadD1Ev", &AThread_dtorComplete),
    SO_SYMBOL_FUNC("_ZN7android6ThreadD2Ev", &AThread_dtorBase),
    SO_SYMBOL_FUNC("_ZTv0_n12_N7android6ThreadD0Ev", &AThread_dtorDeletingThunk),
    SO_SYMBOL_FUNC("_ZTv0_n12_N7android6ThreadD1Ev", &AThread_dtorCompleteThunk),
    SO_SYMBOL_FUNC("_ZN7android6Thread10readyToRunEv", &AThread_readyToRun),
    SO_SYMBOL_FUNC("_ZN7android6Thread11requestExitEv", &AThread_requestExit),
    SO_SYMBOL_FUNC("_ZN7android6Thread3runEPKcij", &AThread_run),

    // ===== Timers =====
    SO_SYMBOL_FUNC("systemTime", &systemTime),

    // ===== ZipFileRO =====
    SO_SYMBOL_FUNC("_ZN7android9ZipFileRO4openEPKc", &AZipFileRO_fake),
    SO_SYMBOL_FUNC("_ZNK7android9ZipFileRO15findEntryByNameEPKc", &AZipFileRO_fake),
    SO_SYMBOL_FUNC("_ZNK7android9ZipFileRO15uncompressEntryEPvS1_", &AZipFileRO_fake),
    SO_SYMBOL_FUNC("_ZNK7android9ZipFileRO12getEntryInfoEPvPiPlS3_S3_S3_S3_", &AZipFileRO_fake),
    SO_SYMBOL_FUNC("_ZNK7android9ZipFileRO12getEntryInfoEPvPiPjS3_PlS4_S4_", &AZipFileRO_fake),
    SO_SYMBOL_FUNC("_ZNK7android9ZipFileRO12getEntryInfoEPvPiPjS3_PxPlS5_", &AZipFileRO_fake),
};

so_library libutils_so = {
    .soname      = "libutils.so",
    .symbols     = libutils_symbols,
    .num_symbols = sizeof(libutils_symbols) / sizeof(libutils_symbols[0]),
    .prepare     = NULL,
};
