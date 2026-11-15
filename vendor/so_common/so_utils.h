#ifndef SO_COMMON_UTILS_H
#define SO_COMMON_UTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdatomic.h>
#include <stdint.h>

#include <so_common/so_logger.h>

static inline int32_t so_atomic_inc(int32_t *addr) {
    atomic_int_least32_t *atomic_addr = (atomic_int_least32_t *)addr;
    return atomic_fetch_add_explicit(atomic_addr, 1, memory_order_relaxed);
}

static inline int32_t so_atomic_dec(int32_t *addr) {
    atomic_int_least32_t *atomic_addr = (atomic_int_least32_t *)addr;
    return atomic_fetch_sub_explicit(atomic_addr, 1, memory_order_acq_rel);
}

/*
 * 未实现函数桩
 *
 * 如果符号表里直接写一个共享空桩的条目，调用时无从知道是哪一个函数被用到；
 * 用这两个宏为每个未实现函数生成一个具名桩：函数名加 _soloader 后缀，
 * 被调用时输出一条错误日志，并返回调用方指定的值。
 *
 *   SO_UNIMPLEMENTED(foo, 0)               → 定义 int foo_soloader(void)
 *   SO_SYMBOL_FUNC("foo", &foo_soloader)
 *
 * 需要让日志打印游戏导入的原始符号名（mangled）时，用 LABELED 形式显式给标签：
 *   SO_UNIMPLEMENTED_LABELED(android_IBinder_pingBinder, "_ZN7android7IBinder14pingBinderEv", 0)
 *   SO_SYMBOL_FUNC("_ZN7android7IBinder14pingBinderEv", &android_IBinder_pingBinder_soloader)
 *
 * 返回值按调用方语义给：成功/失败标记给 0 或 -1，句柄给 -1 或 NULL(0)。
 *
 * 注意：
 *   - 这是调试初期的占位手段：未实现的符号先全量套这个宏，跑起来看日志就能分清
 *     两类——真的被游戏调用的（必须补实现），和只为满足链接、从未被调用的（留桩即可）。
 *     未实现符号往往很多，逐个补实现在初期不现实，普查一遍才知道该补哪些。
 *   - 某个符号的日志一直刷，说明它在热路径上，要补真实实现，而不是把日志关掉。
 *   - 已有 xxx_soloader 真实实现的函数不要再套这个宏（名字会撞）。
 */
#define SO_UNIMPLEMENTED(name, ret) SO_UNIMPLEMENTED_LABELED(name, #name, ret)

#define SO_UNIMPLEMENTED_LABELED(id, label, ret)                              \
    int id##_soloader(void) {                                                 \
        so_loge("%s() not implemented, returning %d", label, (int)(ret));     \
        return (int)(ret);                                                    \
    }

#ifdef __cplusplus
}
#endif

#endif