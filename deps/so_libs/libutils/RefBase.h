#ifndef ANDROID_REF_BASE_H
#define ANDROID_REF_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

//! Flags for extendObjectLifetime()
enum {
    OBJECT_LIFETIME_STRONG = 0x0000,
    OBJECT_LIFETIME_WEAK   = 0x0001,
    OBJECT_LIFETIME_MASK   = 0x0001
};

//! Flags for onIncStrongAttempted()
enum {
    FIRST_INC_STRONG = 0x0001
};

typedef struct ARefBase     ARefBase;
typedef struct weakref_type weakref_type;
typedef struct weakref_impl weakref_impl;

typedef struct ARefBase_VTable {
    void (*destructorComplete)(ARefBase *thiz);                                    // +0
    void (*destructorDeleting)(ARefBase *thiz);                                    // +4
    void (*onFirstRef)(ARefBase *thiz);                                            // +8
    void (*onLastStrongRef)(ARefBase *thiz, const void *id);                       // +12
    bool (*onIncStrongAttempted)(ARefBase *thiz, uint32_t flags, const void *id);  // +16
    void (*onLastWeakRef)(ARefBase *thiz, const void *id);                         // +20
} ARefBase_VTable;

struct ARefBase {
    ARefBase_VTable *vptr;  // +0  c++编译器自动生成的虚函数表占位
    weakref_impl    *refs;  // +4
};  // +8

ARefBase *weakref_type_refBase(weakref_type *thiz);
void      weakref_type_incWeak(weakref_type *thiz, const void *id);
void      weakref_type_decWeak(weakref_type *thiz, const void *id);
bool      weakref_type_attemptIncStrong(weakref_type *thiz, const void *id);
bool      weakref_type_attemptIncWeak(weakref_type *thiz, const void *id);
int32_t   weakref_type_getWeakCount(weakref_type *thiz);
void      weakref_type_printRefs(weakref_type *thiz);
void      weakref_type_trackMe(weakref_type *thiz, bool enable, bool retain);

void          ARefBase_ctor(ARefBase *thiz);
void          ARefBase_dtor(ARefBase *thiz);
weakref_type *RefBase_createWeak(ARefBase *thiz, const void *id);
weakref_type *RefBase_getWeakRefs(ARefBase *thiz);
void          ARefBase_extendObjectLifetime(ARefBase *thiz, int32_t mode);
void          ARefBase_onFirstRef(ARefBase *thiz);
void          ARefBase_onLastStrongRef(ARefBase *thiz, const void *id);
bool          ARefBase_onIncStrongAttempted(ARefBase *thiz, unsigned int flags, const void *id);
void          ARefBase_onLastWeakRef(ARefBase *thiz, const void *id);
void          ARefBase_incStrong(ARefBase *thiz, const void *id);
void          ARefBase_decStrong(ARefBase *thiz, const void *id);
void          ARefBase_forceIncStrong(ARefBase *thiz, const void *id);
int32_t       RefBase_getStrongCount(ARefBase *thiz);

#ifdef __cplusplus
}
#endif

#endif