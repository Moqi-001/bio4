#ifndef ANDROID_THREAD_H
#define ANDROID_THREAD_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Thread Thread;

void _ZN7android6ThreadC2Eb(Thread *thiz, int canCallJava);
void _ZN7android6ThreadD0Ev(Thread *thiz);
void _ZN7android6ThreadD1Ev(Thread *thiz);
void _ZN7android6ThreadD2Ev(Thread *thiz);
void _ZTv0_n12_N7android6ThreadD0Ev(Thread *thiz);
void _ZTv0_n12_N7android6ThreadD1Ev(Thread *thiz);
int  _ZN7android6Thread10readyToRunEv(Thread *thiz);
void _ZN7android6Thread11requestExitEv(Thread *thiz);
int  _ZN7android6Thread3runEPKcij(Thread *thiz, const char *name, int priority, size_t stack);

#ifdef __cplusplus
}
#endif

#endif