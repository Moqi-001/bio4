#include <so_linker/so_linker.h>
#include <so_common/so_logger.h>
#include <so_common/so_utils.h>

#include "reimpl/android/RefBase.h"
#include "reimpl/android/SharedBuffer.h"
#include "reimpl/android/String8.h"
#include "reimpl/android/Thread.h"
#include "reimpl/android/Timers.h"
#include "reimpl/md5/md5.h"

int AZipFileRO_fake(void) {
    so_logd("[Error] Android ZipFileRO function called: unsupport!");
    return 0;
}

// ==================== libutils.so ====================
// Android 基础工具库
static so_symbol libutils_symbols[] = {
    // ===== RefBase =====
    SO_SYMBOL_FUNC("_ZN7android7RefBaseC2Ev", &_ZN7android7RefBaseC2Ev),
    SO_SYMBOL_FUNC("_ZN7android7RefBaseD2Ev", &_ZN7android7RefBaseD2Ev),
    SO_SYMBOL_FUNC("_ZN7android7RefBase10onFirstRefEv", &_ZN7android7RefBase10onFirstRefEv),
    SO_SYMBOL_FUNC("_ZN7android7RefBase15onLastStrongRefEPKv", &_ZN7android7RefBase15onLastStrongRefEPKv),
    SO_SYMBOL_FUNC("_ZN7android7RefBase13onLastWeakRefEPKv", &_ZN7android7RefBase13onLastWeakRefEPKv),
    SO_SYMBOL_FUNC("_ZN7android7RefBase20onIncStrongAttemptedEjPKv", &_ZN7android7RefBase20onIncStrongAttemptedEjPKv),
    SO_SYMBOL_FUNC("_ZNK7android7RefBase9incStrongEPKv", &_ZNK7android7RefBase9incStrongEPKv),
    SO_SYMBOL_FUNC("_ZNK7android7RefBase9decStrongEPKv", &_ZNK7android7RefBase9decStrongEPKv),

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
    SO_SYMBOL_FUNC("_ZN7android6ThreadC2Eb", &_ZN7android6ThreadC2Eb),
    SO_SYMBOL_FUNC("_ZN7android6ThreadD0Ev", &_ZN7android6ThreadD0Ev),
    SO_SYMBOL_FUNC("_ZN7android6ThreadD1Ev", &_ZN7android6ThreadD1Ev),
    SO_SYMBOL_FUNC("_ZN7android6ThreadD2Ev", &_ZN7android6ThreadD2Ev),
    SO_SYMBOL_FUNC("_ZTv0_n12_N7android6ThreadD0Ev", &_ZTv0_n12_N7android6ThreadD0Ev),
    SO_SYMBOL_FUNC("_ZTv0_n12_N7android6ThreadD1Ev", &_ZTv0_n12_N7android6ThreadD1Ev),
    SO_SYMBOL_FUNC("_ZN7android6Thread10readyToRunEv", &_ZN7android6Thread10readyToRunEv),
    SO_SYMBOL_FUNC("_ZN7android6Thread11requestExitEv", &_ZN7android6Thread11requestExitEv),
    SO_SYMBOL_FUNC("_ZN7android6Thread3runEPKcij", &_ZN7android6Thread3runEPKcij),

    // ===== Timers =====
    SO_SYMBOL_FUNC("systemTime", &systemTime),

    // ===== ZipFileRO =====
    SO_SYMBOL_FUNC("_ZN7android9ZipFileRO4openEPKc", &AZipFileRO_fake),
    SO_SYMBOL_FUNC("_ZNK7android9ZipFileRO15findEntryByNameEPKc", &AZipFileRO_fake),
    SO_SYMBOL_FUNC("_ZNK7android9ZipFileRO15uncompressEntryEPvS1_", &AZipFileRO_fake),
    SO_SYMBOL_FUNC("_ZNK7android9ZipFileRO12getEntryInfoEPvPiPlS3_S3_S3_S3_", &AZipFileRO_fake),
    SO_SYMBOL_FUNC("_ZNK7android9ZipFileRO12getEntryInfoEPvPiPjS3_PlS4_S4_", &AZipFileRO_fake),
    SO_SYMBOL_FUNC("_ZNK7android9ZipFileRO12getEntryInfoEPvPiPjS3_PxPlS5_", &AZipFileRO_fake),

    // ===== MD5 =====
    SO_SYMBOL_FUNC("MD5_Init", &MD5_Init),
    SO_SYMBOL_FUNC("MD5_Update", &MD5_Update),
    SO_SYMBOL_FUNC("MD5_Final", &MD5_Final),
};

so_library so_library_utils = {
    .soname      = "libutils.so",
    .symbols     = libutils_symbols,
    .num_symbols = sizeof(libutils_symbols) / sizeof(libutils_symbols[0]),
    .prepare     = NULL,
};
