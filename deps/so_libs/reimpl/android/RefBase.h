#ifndef ANDROID_REF_BASE_H
#define ANDROID_REF_BASE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct RefBase RefBase;

void _ZN7android7RefBaseC2Ev(RefBase *thiz);
void _ZN7android7RefBaseD2Ev(RefBase *thiz);
void _ZN7android7RefBase10onFirstRefEv(RefBase *thiz);
void _ZN7android7RefBase15onLastStrongRefEPKv(RefBase *thiz, const void *id);
void _ZN7android7RefBase13onLastWeakRefEPKv(RefBase *thiz, const void *id);
int  _ZN7android7RefBase20onIncStrongAttemptedEjPKv(RefBase *thiz, unsigned int flags, const void *id);
void _ZNK7android7RefBase9incStrongEPKv(RefBase *thiz, const void *id);
void _ZNK7android7RefBase9decStrongEPKv(RefBase *thiz, const void *id);

#ifdef __cplusplus
}
#endif

#endif