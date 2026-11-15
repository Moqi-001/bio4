#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <so_common/so_logger.h>

#include "../libcutils/atomic.h"
#include "RefBase.h"

// log all reference counting operations
#define PRINT_REFS 1

#define INITIAL_STRONG_VALUE (1 << 28)

struct weakref_impl {
    volatile int32_t mStrong;  // +0
    volatile int32_t mWeak;    // +4
    ARefBase        *mBase;    // +8
    volatile int32_t mFlags;   // +12
};  // +16

static weakref_impl *weakref_impl_new(ARefBase *base) {
    weakref_impl *refs = (weakref_impl *)malloc(sizeof(weakref_impl));
    if (!refs) return NULL;

    refs->mStrong = INITIAL_STRONG_VALUE;
    refs->mWeak   = 0;
    refs->mBase   = base;
    refs->mFlags  = 0;

    return refs;
}

static void weakref_impl_addStrongRef(weakref_impl *thiz, const void *id) {}
static void weakref_impl_removeStrongRef(weakref_impl *thiz, const void *id) {}
static void weakref_impl_renameStrongRefId(weakref_impl *thiz, const void *old_id, const void *new_id) {}
static void weakref_impl_addWeakmBaseRef(weakref_impl *thiz, const void *id) {}
static void weakref_impl_removeWeakRef(weakref_impl *thiz, const void *id) {}
static void weakref_impl_renameWeakRefId(weakref_impl *thiz, const void *old_id, const void *new_id) {}
static void weakref_impl_printRefs(const weakref_impl *thiz) {}
static void weakref_impl_trackMe(weakref_impl *thiz, bool track, bool retain) {}

ARefBase *weakref_type_refBase(weakref_type *thiz) {
    weakref_impl *impl = (weakref_impl *)thiz;
    so_logd("[NDK][weakref_type] refBase(%p): %p", thiz, impl->mBase);
    return impl->mBase;
}

void weakref_type_incWeak(weakref_type *thiz, const void *id) {
    so_logd("[NDK][weakref_type] incWeak(%p,%p)", thiz, id);

    weakref_impl *impl = (weakref_impl *)thiz;
    weakref_impl_addWeakmBaseRef(impl, id);

    const int32_t c = android_atomic_inc(&impl->mWeak);
    if (c < 0) {
        so_loge("incWeak called on %p after last weak ref", thiz);
        return;
    }
}

void weakref_type_decWeak(weakref_type *thiz, const void *id) {
    so_logd("[NDK][weakref_type] decWeak(%p,%p)", thiz, id);

    weakref_impl *impl = (weakref_impl *)thiz;
    weakref_impl_removeWeakRef(impl, id);

    const int32_t c = android_atomic_dec(&impl->mWeak);
    if (c < 1) {
        so_loge("decWeak called on %p too many times", thiz);
        return;
    }

    if (c != 1) return;

    if ((impl->mFlags & OBJECT_LIFETIME_WEAK) == OBJECT_LIFETIME_STRONG) {
        // This is the regular lifetime case. The object is destroyed
        // when the last strong reference goes away. Since weakref_impl
        // outlive the object, it is not destroyed in the dtor, and
        // we'll have to do it here.
        if (impl->mStrong == INITIAL_STRONG_VALUE) {
            // Special case: we never had a strong reference, so we need to
            // destroy the object now.
            free(impl->mBase);
        } else {
            // ALOGV("Freeing refs %p of old ARefBase %p\n", this, thiz->mBase);
            free(impl);
        }
    } else {
        // less common case: lifetime is OBJECT_LIFETIME_{WEAK|FOREVER}
        ARefBase_onLastWeakRef(impl->mBase, id);
        if ((impl->mFlags & OBJECT_LIFETIME_MASK) == OBJECT_LIFETIME_WEAK) {
            // this is the OBJECT_LIFETIME_WEAK case. The last weak-reference
            // is gone, we can destroy the object.
            free(impl->mBase);
        }
    }
}

bool weakref_type_attemptIncStrong(weakref_type *thiz, const void *id) {
    so_logd("[NDK][weakref_type] attemptIncStrong(%p,%p)", thiz, id);

    weakref_type_incWeak(thiz, id);

    weakref_impl *impl     = (weakref_impl *)thiz;
    int32_t       curCount = impl->mStrong;

    if (curCount < 0) {
        so_loge("attemptIncStrong called on %p after underflow", thiz);
        return false;
    }

    while (curCount > 0 && curCount != INITIAL_STRONG_VALUE) {
        // we're in the easy/common case of promoting a weak-reference
        // from an existing strong reference.
        if (android_atomic_cmpxchg(curCount, curCount + 1, &impl->mStrong) == 0) {
            break;
        }
        // the strong count has changed on us, we need to re-assert our
        // situation.
        curCount = impl->mStrong;
    }

    if (curCount <= 0 || curCount == INITIAL_STRONG_VALUE) {
        // we're now in the harder case of either:
        // - there never was a strong reference on us
        // - or, all strong references have been released
        if ((impl->mFlags & OBJECT_LIFETIME_WEAK) == OBJECT_LIFETIME_STRONG) {
            // this object has a "normal" life-time, i.e.: it gets destroyed
            // when the last strong reference goes away
            if (curCount <= 0) {
                // the last strong-reference got released, the object cannot
                // be revived.
                weakref_type_decWeak(thiz, id);
                return false;
            }

            // here, curCount == INITIAL_STRONG_VALUE, which means
            // there never was a strong-reference, so we can try to
            // promote this object; we need to do that atomically.
            while (curCount > 0) {
                if (android_atomic_cmpxchg(curCount, curCount + 1,
                                           &impl->mStrong) == 0) {
                    break;
                }
                // the strong count has changed on us, we need to re-assert our
                // situation (e.g.: another thread has inc/decStrong'ed us)
                curCount = impl->mStrong;
            }

            if (curCount <= 0) {
                // promote() failed, some other thread destroyed us in the
                // meantime (i.e.: strong count reached zero).
                weakref_type_decWeak(thiz, id);
                return false;
            }
        } else {
            // this object has an "extended" life-time, i.e.: it can be
            // revived from a weak-reference only.
            // Ask the object's implementation if it agrees to be revived
            if (!ARefBase_onIncStrongAttempted(impl->mBase, FIRST_INC_STRONG, id)) {
                // it didn't so give-up.
                weakref_type_decWeak(thiz, id);
                return false;
            }
            // grab a strong-reference, which is always safe due to the
            // extended life-time.
            curCount = android_atomic_inc(&impl->mStrong);
        }

        // If the strong reference count has already been incremented by
        // someone else, the implementor of onIncStrongAttempted() is holding
        // an unneeded reference.  So call onLastStrongRef() here to remove it.
        // (No, this is not pretty.)  Note that we MUST NOT do this if we
        // are in fact acquiring the first reference.
        if (curCount > 0 && curCount < INITIAL_STRONG_VALUE) {
            ARefBase_onLastStrongRef(impl->mBase, id);
        }
    }

    weakref_impl_addStrongRef(impl, id);

#if PRINT_REFS
    so_logd("attemptIncStrong of %p from %p: cnt=%d", thiz, id, curCount);
#endif

    // now we need to fix-up the count if it was INITIAL_STRONG_VALUE
    // this must be done safely, i.e.: handle the case where several threads
    // were here in attemptIncStrong().
    curCount = impl->mStrong;
    while (curCount >= INITIAL_STRONG_VALUE) {
        if (curCount <= INITIAL_STRONG_VALUE) {
            so_loge("attemptIncStrong in %p underflowed to INITIAL_STRONG_VALUE", thiz);
            return false;
        }
        if (android_atomic_cmpxchg(curCount, curCount - INITIAL_STRONG_VALUE,
                                   &impl->mStrong) == 0) {
            break;
        }
        // the strong-count changed on us, we need to re-assert the situation,
        // for e.g.: it's possible the fix-up happened in another thread.
        curCount = impl->mStrong;
    }

    return true;
}

bool weakref_type_attemptIncWeak(weakref_type *thiz, const void *id) {
    so_logd("[NDK][weakref_type] attemptIncWeak(%p,%p)", thiz, id);

    weakref_impl *const impl     = (weakref_impl *const)thiz;
    int32_t             curCount = impl->mWeak;

    if (curCount < 0) {
        so_loge("attemptIncWeak called on %p after underflow", thiz);
        return false;
    }

    while (curCount > 0) {
        if (android_atomic_cmpxchg(curCount, curCount + 1, &impl->mWeak) == 0) {
            break;
        }
        curCount = impl->mWeak;
    }

    if (curCount > 0) {
        weakref_impl_addWeakmBaseRef(impl, id);
    }

    return curCount > 0;
}

int32_t weakref_type_getWeakCount(weakref_type *thiz) {
    weakref_impl *impl = (weakref_impl *)thiz;
    return impl->mWeak;
}

void weakref_type_printRefs(weakref_type *thiz) {
    weakref_impl *impl = (weakref_impl *)thiz;
    weakref_impl_printRefs(impl);
}

void weakref_type_trackMe(weakref_type *thiz, bool enable, bool retain) {
    weakref_impl *impl = (weakref_impl *)thiz;
    weakref_impl_trackMe(impl, enable, retain);
}

// ---------------------------------------------------------------------------
void ARefBase_ctor(ARefBase *thiz) {
    so_logd("[NDK][RefBase] ctor(%p)", thiz);
    thiz->refs = weakref_impl_new(thiz);
}

void ARefBase_dtor(ARefBase *thiz) {
    so_logd("[NDK][RefBase] dtor(%p)", thiz);

    if (thiz->refs->mStrong == INITIAL_STRONG_VALUE) {
        // we never acquired a strong (and/or weak) reference on this object.
        free(thiz->refs);
    } else {
        // life-time of this object is extended to WEAK or FOREVER, in
        // which case weakref_impl doesn't out-live the object and we
        // can free it now.
        if ((thiz->refs->mFlags & OBJECT_LIFETIME_MASK) != OBJECT_LIFETIME_STRONG) {
            // It's possible that the weak count is not 0 if the object
            // re-acquired a weak reference in its destructor
            if (thiz->refs->mWeak == 0) {
                free(thiz->refs);
            }
        }
    }
    // for debugging purposes, clear this.

    thiz->refs = NULL;
}

weakref_type *RefBase_createWeak(ARefBase *thiz, const void *id) {
    weakref_type *refs = (weakref_type *)thiz->refs;
    weakref_type_incWeak(refs, id);
    so_logd("[NDK][RefBase] createWeak(%p,%p): %p", thiz, id, refs);
    return refs;
}

weakref_type *RefBase_getWeakRefs(ARefBase *thiz) {
    so_logd("[NDK][RefBase] createWeak(%p): %p", thiz, thiz->refs);
    return (weakref_type *)thiz->refs;
}

void ARefBase_extendObjectLifetime(ARefBase *thiz, int32_t mode) {
    so_logd("[NDK][RefBase] extendObjectLifetime(%p,0x%x)", thiz, mode);
    android_atomic_or(mode, &thiz->refs->mFlags);
}

void ARefBase_onFirstRef(ARefBase *thiz) {
    so_logd("[NDK][RefBase] onFirstRef(%p): ignored!", thiz);
}

void ARefBase_onLastStrongRef(ARefBase *thiz, const void *id) {
    so_logd("[NDK][RefBase] onLastStrongRef(%p,%p): ignored!", thiz, id);
}

bool ARefBase_onIncStrongAttempted(ARefBase *thiz, unsigned int flags, const void *id) {
    bool result = (flags & FIRST_INC_STRONG) ? true : false;
    so_logd("[NDK][RefBase] onIncStrongAttempted(%p,0x%x,%p): %u", thiz, flags, id, result);
    return result;
}

void ARefBase_onLastWeakRef(ARefBase *thiz, const void *id) {
    so_logd("[NDK][RefBase] onLastWeakRef(%p,%p): ignored!", thiz, id);
}

void ARefBase_incStrong(ARefBase *thiz, const void *id) {
    so_logd("[NDK][RefBase] incStrong(%p,%p)", thiz, id);

    weakref_impl *refs = thiz->refs;
    weakref_type_incWeak((weakref_type *)refs, id);
    weakref_impl_addStrongRef(refs, id);

    const int32_t c = android_atomic_inc(&refs->mStrong);
#if PRINT_REFS
    so_logd("[NDK][RefBase] incStrong of %p from %p: cnt=%d", thiz, id, c);
#endif
    if (c <= 0) {
        so_loge("[NDK][RefBase] incStrong() called on %p after last strong ref", refs);
        return;
    }
    if (c != INITIAL_STRONG_VALUE) {
        return;
    }

    android_atomic_add(-INITIAL_STRONG_VALUE, &refs->mStrong);
    ARefBase_onFirstRef(refs->mBase);
}

void ARefBase_decStrong(ARefBase *thiz, const void *id) {
    so_logd("[NDK][RefBase] decStrong(%p,%p)", thiz, id);

    weakref_impl *refs = thiz->refs;
    weakref_impl_removeStrongRef(refs, id);

    const int32_t c = android_atomic_dec(&refs->mStrong);
#if PRINT_REFS
    so_logd("[NDK][RefBase] decStrong of %p from %p: cnt=%d", thiz, id, c);
#endif
    if (c < 1) {
        so_loge("[NDK][RefBase] decStrong() called on %p too many times", refs);
        return;
    }
    if (c == 1) {
        ARefBase_onLastStrongRef(refs->mBase, id);
        if ((refs->mFlags & OBJECT_LIFETIME_MASK) == OBJECT_LIFETIME_STRONG) {
            free(thiz);
        }
    }

    weakref_type_decWeak((weakref_type *)refs, id);
}

void ARefBase_forceIncStrong(ARefBase *thiz, const void *id) {
    so_logd("[NDK][RefBase] forceIncStrong(%p,%p)", thiz, id);

    weakref_impl *refs = thiz->refs;
    weakref_type_incWeak((weakref_type *)thiz->refs, id);
    weakref_impl_addStrongRef(refs, id);

    const int32_t c = android_atomic_inc(&refs->mStrong);
#if PRINT_REFS
    so_logd("[NDK][RefBase] forceIncStrong of %p from %p: cnt=%d", thiz, id, c);
#endif
    if (c <= 0) {
        so_loge("[NDK][RefBase] forceIncStrong() called on %p after ref count underflow", refs);
        return;
    }
    switch (c) {
        case INITIAL_STRONG_VALUE:
            android_atomic_add(-INITIAL_STRONG_VALUE, &refs->mStrong);
            // fall through...
        case 0:
            ARefBase_onLastStrongRef(refs->mBase, id);
    }
}

int32_t RefBase_getStrongCount(ARefBase *thiz) {
    so_logd("[NDK][RefBase] getStrongCount(%p): %d", thiz, thiz->refs->mStrong);
    return thiz->refs->mStrong;
}
