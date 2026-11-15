#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <math.h>

#include <so_common/so_logger.h>
#include <so_common/so_utils.h>
#include <so_linker/so_linker.h>

extern int __builtin_isinf(double);

// ==================== libm.so ====================
static so_symbol libm_symbols[] = {
    // ===== Math (double) =====
    SO_SYMBOL_FUNC("acos", &acos),
    SO_SYMBOL_FUNC("asin", &asin),
    SO_SYMBOL_FUNC("atan", &atan),
    SO_SYMBOL_FUNC("atan2", &atan2),
    SO_SYMBOL_FUNC("ceil", &ceil),
    SO_SYMBOL_FUNC("cos", &cos),
    SO_SYMBOL_FUNC("exp", &exp),
    SO_SYMBOL_FUNC("exp2", &exp2),
    SO_SYMBOL_FUNC("floor", &floor),
    SO_SYMBOL_FUNC("fmod", &fmod),
    SO_SYMBOL_FUNC("frexp", &frexp),
    SO_SYMBOL_FUNC("ldexp", &ldexp),
    SO_SYMBOL_FUNC("log", &log),
    SO_SYMBOL_FUNC("log10", &log10),
    SO_SYMBOL_FUNC("modf", &modf),
    SO_SYMBOL_FUNC("pow", &pow),
    SO_SYMBOL_FUNC("rint", &rint),
    SO_SYMBOL_FUNC("round", &round),
    SO_SYMBOL_FUNC("scalbn", &scalbn),
    SO_SYMBOL_FUNC("sin", &sin),
    SO_SYMBOL_FUNC("sincos", &sincos),
    SO_SYMBOL_FUNC("sinh", &sinh),
    SO_SYMBOL_FUNC("sqrt", &sqrt),
    SO_SYMBOL_FUNC("tan", &tan),
    SO_SYMBOL_FUNC("tanh", &tanh),
    SO_SYMBOL_FUNC("trunc", &trunc),

    // ===== Math (float) =====
    SO_SYMBOL_FUNC("acosf", &acosf),
    SO_SYMBOL_FUNC("asinf", &asinf),
    SO_SYMBOL_FUNC("atanf", &atanf),
    SO_SYMBOL_FUNC("atan2f", &atan2f),
    SO_SYMBOL_FUNC("ceilf", &ceilf),
    SO_SYMBOL_FUNC("cosf", &cosf),
    SO_SYMBOL_FUNC("expf", &expf),
    SO_SYMBOL_FUNC("exp2f", &exp2f),
    SO_SYMBOL_FUNC("floorf", &floorf),
    SO_SYMBOL_FUNC("fmodf", &fmodf),
    SO_SYMBOL_FUNC("logf", &logf),
    SO_SYMBOL_FUNC("log10f", &log10f),
    SO_SYMBOL_FUNC("modff", &modff),
    SO_SYMBOL_FUNC("powf", &powf),
    SO_SYMBOL_FUNC("rintf", &rintf),
    SO_SYMBOL_FUNC("roundf", &roundf),
    SO_SYMBOL_FUNC("scalbnf", &scalbnf),
    SO_SYMBOL_FUNC("sinf", &sinf),
    SO_SYMBOL_FUNC("sincosf", &sincosf),
    SO_SYMBOL_FUNC("sinhf", &sinhf),
    SO_SYMBOL_FUNC("sqrtf", &sqrtf),
    SO_SYMBOL_FUNC("tanf", &tanf),
    SO_SYMBOL_FUNC("truncf", &truncf),

    // ===== Math Extensions =====
    SO_SYMBOL_FUNC("__isinf", &__builtin_isinf),
};

so_library libm_so = {
    .soname      = "libm.so",
    .symbols     = libm_symbols,
    .num_symbols = sizeof(libm_symbols) / sizeof(libm_symbols[0]),
    .prepare     = NULL,
};
