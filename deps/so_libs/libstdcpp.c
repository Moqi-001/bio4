#include <so_linker/so_linker.h>
#include <so_common/so_logger.h>
#include <so_common/so_utils.h>

// ===== C++ ABI exception handling =====
extern void *__cxa_allocate_exception;
extern void *__cxa_begin_catch;
extern void *__cxa_begin_cleanup;
extern void *__cxa_call_unexpected;
extern void *__cxa_end_catch;
extern void *__cxa_free_exception;
extern void *__cxa_rethrow;
extern void *__cxa_throw;
extern void *__cxa_type_match;

// ===== C++ ABI static initialization =====
extern void *__cxa_guard_acquire;
extern void *__cxa_guard_release;

// ===== C++ ABI pure virtual call handler =====
extern void *__cxa_pure_virtual;

// ===== C++ exception personality =====
extern void *__gxx_personality_v0;

// ===== Compiler runtime helpers (libgcc) =====
extern void *__gnu_ldivmod_helper;

// ===== C++ memory allocation =====
extern void *_Znwj;   // operator new (unsigned int)
extern void *_ZdlPv;  // operator delete (void*)
extern void *_Znaj;   // operator new[] (unsigned int)
extern void *_ZdaPv;  // operator delete[] (void*)

// ===== C++ RTTI =====
extern void *_ZTISt8bad_cast;      // typeinfo for std::bad_cast
extern void *_ZTISt9exception;     // typeinfo for std::exception
extern void *_ZNSt8bad_castD1Ev;   // std::bad_cast::~bad_cast()
extern void *_ZNSt9exceptionD2Ev;  // std::exception::~exception()

// ===== C++ RTTI vtables =====
extern void *_ZTVN10__cxxabiv117__class_type_infoE;      // vtable for __class_type_info
extern void *_ZTVN10__cxxabiv120__si_class_type_infoE;   // vtable for __si_class_type_info
extern void *_ZTVN10__cxxabiv121__vmi_class_type_infoE;  // vtable for __vmi_class_type_info

// ===== C++ runtime =====
extern void *_ZSt9terminatev;           // std::terminate()
extern void *_ZSt17__throw_bad_allocv;  // std::__throw_bad_alloc()

// ==================== libstdc++.so ====================
static so_symbol libstdcpp_symbols[] = {
    // ===== C++ ABI exception handling =====
    SO_SYMBOL_FUNC("__cxa_allocate_exception", &__cxa_allocate_exception),
    SO_SYMBOL_FUNC("__cxa_begin_catch", &__cxa_begin_catch),
    SO_SYMBOL_FUNC("__cxa_begin_cleanup", &so_ret_0),
    SO_SYMBOL_FUNC("__cxa_call_unexpected", &__cxa_call_unexpected),
    SO_SYMBOL_FUNC("__cxa_end_catch", &__cxa_end_catch),
    SO_SYMBOL_FUNC("__cxa_free_exception", &__cxa_free_exception),
    SO_SYMBOL_FUNC("__cxa_rethrow", &__cxa_rethrow),
    SO_SYMBOL_FUNC("__cxa_throw", &__cxa_throw),
    SO_SYMBOL_FUNC("__cxa_type_match", &so_ret_0),

    // ===== C++ ABI static initialization =====
    SO_SYMBOL_FUNC("__cxa_guard_acquire", &__cxa_guard_acquire),
    SO_SYMBOL_FUNC("__cxa_guard_release", &__cxa_guard_release),

    // ===== C++ ABI pure virtual call handler =====
    SO_SYMBOL_FUNC("__cxa_pure_virtual", &__cxa_pure_virtual),

    // ===== C++ exception personality =====
    SO_SYMBOL_FUNC("__gxx_personality_v0", &__gxx_personality_v0),

    // ===== Compiler runtime helpers =====
    SO_SYMBOL_FUNC("__gnu_ldivmod_helper", &__gnu_ldivmod_helper),

    // ===== C++ memory allocation =====
    SO_SYMBOL_FUNC("_Znwj", &_Znwj),
    SO_SYMBOL_FUNC("_ZdlPv", &_ZdlPv),
    SO_SYMBOL_FUNC("_Znaj", &_Znaj),
    SO_SYMBOL_FUNC("_ZdaPv", &_ZdaPv),

    // ===== C++ RTTI =====
    SO_SYMBOL_OBJ("_ZTISt8bad_cast", &_ZTISt8bad_cast, 8),
    SO_SYMBOL_OBJ("_ZTISt9exception", &_ZTISt9exception, 8),
    SO_SYMBOL_FUNC("_ZNSt8bad_castD1Ev", &_ZNSt8bad_castD1Ev),
    SO_SYMBOL_FUNC("_ZNSt9exceptionD2Ev", &_ZNSt9exceptionD2Ev),

    // ===== C++ RTTI vtables =====
    SO_SYMBOL_OBJ("_ZTVN10__cxxabiv117__class_type_infoE", &_ZTVN10__cxxabiv117__class_type_infoE, 4),
    SO_SYMBOL_OBJ("_ZTVN10__cxxabiv120__si_class_type_infoE", &_ZTVN10__cxxabiv120__si_class_type_infoE, 4),
    SO_SYMBOL_OBJ("_ZTVN10__cxxabiv121__vmi_class_type_infoE", &_ZTVN10__cxxabiv121__vmi_class_type_infoE, 4),

    // ===== C++ runtime =====
    SO_SYMBOL_FUNC("_ZSt9terminatev", &_ZSt9terminatev),
    SO_SYMBOL_FUNC("_ZSt17__throw_bad_allocv", &_ZSt17__throw_bad_allocv),
};

so_library so_library_stdcpp = {
    .soname      = "libstdc++.so",
    .symbols     = libstdcpp_symbols,
    .num_symbols = sizeof(libstdcpp_symbols) / sizeof(libstdcpp_symbols[0]),
    .prepare     = NULL,
};
