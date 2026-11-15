#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <so_common/so_logger.h>

#include "so_jni_internal.h"

#define MAX_REF_CAPACITY  0X40000u
#define INIT_REF_CAPACITY 512

#define LOCAL_REF_BIAS  0X10000000u
#define GLOBAL_REF_BIAS 0X20000000u
#define WEAK_REF_BIAS   0X30000000u

typedef struct JniRefSlot {
    JObject obj;  //  Java 对象指针
} JniRefSlot;

typedef struct JniRefFrame {
    uint32_t            start;     // 帧索引起始
    uint32_t            capacity;  // 帧引用容量
    struct JniRefFrame *prev;      // 前一帧
} JniRefFrame;

typedef struct JniLocalContext {
    JniRefSlot  *slots;     // 槽位数组
    jint         capacity;  // 槽位容量
    jint         count;     // 槽位已使用数量
    JniRefFrame *frame;     // 帧栈
} JniLocalContext;

typedef struct JniGlobalContext {
    JniRefSlot     *slots;      // 槽位数组
    jint            capacity;   // 槽位容量
    jint            count;      // 槽位已使用数量
    jint            free_head;  // 空闲链表头（索引）
    jint           *next_free;  // 下一个空闲位置数组
    pthread_mutex_t lock;       // 线程锁
} JniGlobalContext;

static __thread JniLocalContext _so_local_ctx  = {0};
static JniGlobalContext         _so_global_ctx = {0};
static JniGlobalContext         _so_weak_ctx   = {0};

jobjectRefType JniGetObjectRefType(jobject object) {
    if (!object) {
        return JNIInvalidRefType;
    }

    uintptr_t addr = (uintptr_t)object;
    if (addr >= LOCAL_REF_BIAS && addr < LOCAL_REF_BIAS + MAX_REF_CAPACITY) {
        return JNILocalRefType;
    }
    if (addr >= GLOBAL_REF_BIAS && addr < GLOBAL_REF_BIAS + MAX_REF_CAPACITY) {
        return JNIGlobalRefType;
    }
    if (addr >= WEAK_REF_BIAS && addr < WEAK_REF_BIAS + MAX_REF_CAPACITY) {
        return JNIWeakGlobalRefType;
    }

    return JNIInvalidRefType;
}

JObject JniObjectToJava(jobject object) {
    if (!object) {
        return NULL;
    }

    uintptr_t addr = (uintptr_t)object;

    if (addr >= LOCAL_REF_BIAS && addr < LOCAL_REF_BIAS + MAX_REF_CAPACITY) {
        JniLocalContext *ctx   = &_so_local_ctx;
        jint             index = addr - LOCAL_REF_BIAS;
        if (index >= ctx->count) {
            return NULL;
        }
        return ctx->slots[index].obj;
    }
    if (addr >= GLOBAL_REF_BIAS && addr < GLOBAL_REF_BIAS + MAX_REF_CAPACITY) {
        JniGlobalContext *ctx   = &_so_global_ctx;
        jint              index = addr - GLOBAL_REF_BIAS;
        if (index >= ctx->capacity) {
            return NULL;
        }
        return ctx->slots[index].obj;
    }
    if (addr >= WEAK_REF_BIAS && addr < WEAK_REF_BIAS + MAX_REF_CAPACITY) {
        JniGlobalContext *ctx   = &_so_weak_ctx;
        jint              index = addr - WEAK_REF_BIAS;
        if (index >= ctx->capacity) {
            return NULL;
        }
        return ctx->slots[index].obj;
    }

    return NULL;
}

jobject JniNewLocalRefFromJava(JObject obj) {
    if (!obj) {
        return NULL;
    }

    JniLocalContext *ctx = &_so_local_ctx;
    if (!ctx->slots && JniInitLocalRefContext() != 0) {
        so_loge("[JNI] JniNewLocalRefFromJava(%p): slots is NULL!", obj);
        return NULL;
    }

    JniRefFrame *frame = ctx->frame;
    if (frame && ctx->count >= frame->start + frame->capacity) {
        so_loge("[JNI] JniNewLocalRefFromJava(%p): frame capacity limit exceeded!", obj);
        return NULL;  // 帧容量已满
    }

    if (ctx->count >= ctx->capacity) {  // 需要扩容
        if (ctx->capacity >= MAX_REF_CAPACITY) {
            so_loge("[JNI] JniNewLocalRefFromJava(%p): capacity limit exceeded!", obj);
            return NULL;  // 总容量已达超最大值
        }

        jint old_capacity = ctx->capacity;
        jint new_capacity = old_capacity * 2;
        if (new_capacity > MAX_REF_CAPACITY) {
            new_capacity = MAX_REF_CAPACITY;
        }

        JniRefSlot *new_slots = realloc(ctx->slots, new_capacity * sizeof(JniRefSlot));
        if (!new_slots) {
            return NULL;
        }

        ctx->slots    = new_slots;
        ctx->capacity = new_capacity;
    }

    jint index            = ctx->count;
    ctx->slots[index].obj = Object_retain(obj);
    ctx->count++;

    return (jobject)(index + LOCAL_REF_BIAS);
}

jobject JniNewLocalRef(jobject object) {
    return JniNewLocalRefFromJava(JniObjectToJava(object));
}

void JniDeleteLocalRef(jobject object) {
    JniLocalContext *ctx = &_so_local_ctx;
    if (!ctx->slots) {
        return;
    }

    jint      index = -1;
    uintptr_t addr  = (uintptr_t)object;
    if (addr >= LOCAL_REF_BIAS && addr < LOCAL_REF_BIAS + MAX_REF_CAPACITY) {
        index = addr - LOCAL_REF_BIAS;
    }
    if (index < 0 || index >= ctx->count || !ctx->slots[index].obj) {
        return;
    }

    // 释放引用后仍占着槽位，count 不减，LocalRef 不进行槽位复用
    Object_release(ctx->slots[index].obj);
    ctx->slots[index].obj = NULL;
}

jobject JniNewGlobalRefFromJava(JObject obj) {
    if (!obj) {
        return NULL;
    }

    JniGlobalContext *ctx = &_so_global_ctx;
    if (!ctx->slots) {
        return NULL;
    }

    pthread_mutex_lock(&ctx->lock);

    if (ctx->count >= ctx->capacity) {  // 需扩容
        if (ctx->capacity >= MAX_REF_CAPACITY) {
            pthread_mutex_unlock(&ctx->lock);
            return NULL;
        }
        jint old_capacity = ctx->capacity;
        jint new_capacity = old_capacity * 2;
        if (new_capacity > MAX_REF_CAPACITY) {
            new_capacity = MAX_REF_CAPACITY;
        }

        JniRefSlot *new_slots = realloc(ctx->slots, new_capacity * sizeof(JniRefSlot));
        if (!new_slots) {
            pthread_mutex_unlock(&ctx->lock);
            return NULL;
        }
        memset(new_slots + old_capacity, 0, (new_capacity - old_capacity) * sizeof(JniRefSlot));
        ctx->slots = new_slots;

        jint *new_next_free = realloc(ctx->next_free, new_capacity * sizeof(jint));
        if (!new_next_free) {
            pthread_mutex_unlock(&ctx->lock);
            return NULL;
        }
        for (int i = old_capacity; i < new_capacity; i++) {
            new_next_free[i] = i + 1;
        }

        ctx->capacity  = new_capacity;
        ctx->next_free = new_next_free;
    }

    jint index            = ctx->free_head;
    ctx->slots[index].obj = Object_retain(obj);
    ctx->free_head        = ctx->next_free[ctx->free_head];
    ctx->count++;

    pthread_mutex_unlock(&ctx->lock);

    return (jobject)(index + GLOBAL_REF_BIAS);
}

jobject JniNewGlobalRef(jobject object) {
    return JniNewGlobalRefFromJava(JniObjectToJava(object));
}

void JniDeleteGlobalRef(jobject object) {
    JniGlobalContext *ctx = &_so_global_ctx;
    if (!object || !ctx->slots) {
        return;
    }

    pthread_mutex_lock(&ctx->lock);

    jint      index = -1;
    uintptr_t addr  = (uintptr_t)object;
    if (addr >= GLOBAL_REF_BIAS && addr < GLOBAL_REF_BIAS + MAX_REF_CAPACITY) {
        index = addr - GLOBAL_REF_BIAS;
    }
    if (index < 0 || index >= ctx->capacity || !ctx->slots[index].obj) {
        pthread_mutex_unlock(&ctx->lock);
        return;
    }

    Object_release(ctx->slots[index].obj);
    ctx->slots[index].obj = NULL;

    ctx->next_free[index] = ctx->free_head;
    ctx->free_head        = index;
    ctx->count--;

    pthread_mutex_unlock(&ctx->lock);
}

jweak JniNewWeakGlobalRefFromJava(JObject obj) {
    if (!obj) {
        return NULL;
    }

    JniGlobalContext *ctx = &_so_weak_ctx;
    if (!ctx->slots) {
        return NULL;
    }

    pthread_mutex_lock(&ctx->lock);

    if (ctx->count >= ctx->capacity) {  // 需扩容
        if (ctx->capacity >= MAX_REF_CAPACITY) {
            pthread_mutex_unlock(&ctx->lock);
            return NULL;
        }
        jint old_capacity = ctx->capacity;
        jint new_capacity = old_capacity * 2;
        if (new_capacity > MAX_REF_CAPACITY) {
            new_capacity = MAX_REF_CAPACITY;
        }

        JniRefSlot *new_slots = realloc(ctx->slots, new_capacity * sizeof(JniRefSlot));
        if (!new_slots) {
            pthread_mutex_unlock(&ctx->lock);
            return NULL;
        }
        memset(new_slots + old_capacity, 0, (new_capacity - old_capacity) * sizeof(JniRefSlot));
        ctx->slots = new_slots;

        jint *new_next_free = realloc(ctx->next_free, new_capacity * sizeof(jint));
        if (!new_next_free) {
            pthread_mutex_unlock(&ctx->lock);
            return NULL;
        }
        for (int i = old_capacity; i < new_capacity; i++) {
            new_next_free[i] = i + 1;
        }

        ctx->capacity  = new_capacity;
        ctx->next_free = new_next_free;
    }

    jint index = ctx->free_head;
    if (Object_retainWeak(obj, &ctx->slots[index].obj) != JNI_OK) {
        pthread_mutex_unlock(&ctx->lock);
        return NULL;
    }
    ctx->slots[index].obj = obj;
    ctx->free_head        = ctx->next_free[ctx->free_head];
    ctx->count++;

    pthread_mutex_unlock(&ctx->lock);

    return (jweak)(index + WEAK_REF_BIAS);
}

jobject JniNewWeakGlobalRef(jobject object) {
    return JniNewWeakGlobalRefFromJava(JniObjectToJava(object));
}

void JniDeleteWeakGlobalRef(jweak object) {
    JniGlobalContext *ctx = &_so_weak_ctx;
    if (!object || !ctx->slots) {
        return;
    }

    pthread_mutex_lock(&ctx->lock);

    jint      index = -1;
    uintptr_t addr  = (uintptr_t)object;
    if (addr >= WEAK_REF_BIAS && addr < WEAK_REF_BIAS + MAX_REF_CAPACITY) {
        index = addr - WEAK_REF_BIAS;
    }
    if (index < 0 || index >= ctx->capacity || !ctx->slots[index].obj) {
        pthread_mutex_unlock(&ctx->lock);
        return;
    }

    Object_releaseWeak(ctx->slots[index].obj, &ctx->slots[index].obj);
    ctx->slots[index].obj = NULL;

    ctx->next_free[index] = ctx->free_head;
    ctx->free_head        = index;
    ctx->count--;

    pthread_mutex_unlock(&ctx->lock);
}

jint JniPushLocalFrame(jint capacity) {
    JniLocalContext *ctx = &_so_local_ctx;
    if (!ctx->slots) {
        return JNI_ERR;
    }

    if (capacity <= 0) {
        capacity = MAX_REF_CAPACITY - ctx->count;
    } else if (capacity > MAX_REF_CAPACITY - ctx->count) {  // 超出最大容量
        return JNI_ERR;
    }

    JniRefFrame *frame = (JniRefFrame *)malloc(sizeof(JniRefFrame));
    if (!frame) {
        return JNI_ERR;
    }

    frame->start    = ctx->count;
    frame->capacity = capacity;
    frame->prev     = ctx->frame;
    ctx->frame      = frame;

    return JNI_OK;
}

jobject JniPopLocalFrame(jobject result) {
    JniLocalContext *ctx = &_so_local_ctx;
    if (!ctx->slots || !ctx->frame) {
        return NULL;
    }

    JniRefSlot  *slots = ctx->slots;
    JniRefFrame *frame = ctx->frame;
    ctx->frame         = frame->prev;

    // 需对 Java 对象进行引用保护以防止弹帧时被释放
    JObject obj = JniObjectToJava(result);
    if (obj) {
        Object_retain(obj);
    }

    for (jint i = frame->start; i < ctx->count; i++) {
        if (slots[i].obj) {
            Object_release(slots[i].obj);
            slots[i].obj = NULL;
        }
    }

    ctx->count = frame->start;  // 回退 index

    // 切回上一帧且必须在释放当前帧后，才能将 result 注册到上一帧
    if (obj) {
        result = JniNewLocalRefFromJava(obj);
        Object_release(obj);
    } else {
        result = NULL;
    }

    free(frame);  // 释放 frame

    return result;
}

jint JniInitLocalRefContext(void) {
    JniLocalContext *ctx = &_so_local_ctx;
    if (ctx->slots) {
        return JNI_OK;
    }

    jint        capacity = INIT_REF_CAPACITY;
    JniRefSlot *slots    = calloc(capacity, sizeof(JniRefSlot));
    if (!slots) {
        return JNI_ERR;
    }

    ctx->slots    = slots;
    ctx->capacity = capacity;
    ctx->count    = 0;
    ctx->frame    = NULL;

    so_logd("[JNI] JniInitLocalRefContext(): success!");
    return JNI_OK;
}

void JniFiniLocalRefContext(void) {
    JniLocalContext *ctx = &_so_local_ctx;

    if (ctx->slots) {
        for (jint i = 0; i < ctx->count; i++) {
            if (ctx->slots[i].obj) {
                Object_release(ctx->slots[i].obj);
                ctx->slots[i].obj = NULL;
            }
        }
        free(ctx->slots);
        ctx->slots = NULL;
    }

    JniRefFrame *frame = ctx->frame;
    while (frame) {
        JniRefFrame *prev = frame->prev;
        free(frame);
        frame = prev;
    }

    ctx->capacity = 0;
    ctx->count    = 0;
    ctx->frame    = NULL;

    so_logd("[JNI] JniFiniLocalRefContext()");
}

jint JniInitGlobalRefContext(void) {
    JniGlobalContext *ctx = &_so_global_ctx;
    if (ctx->slots) {
        return JNI_OK;
    }

    jint        capacity = INIT_REF_CAPACITY;
    JniRefSlot *slots    = calloc(capacity, sizeof(JniRefSlot));
    if (!slots) {
        return JNI_ERR;
    }

    jint *next_free = malloc(capacity * sizeof(jint));
    if (!next_free) {
        free(slots);
        return JNI_ERR;
    }
    for (jint i = 0; i < capacity; i++) {
        next_free[i] = i + 1;
    }

    ctx->slots     = slots;
    ctx->capacity  = capacity;
    ctx->count     = 0;
    ctx->free_head = 0;
    ctx->next_free = next_free;

    pthread_mutex_init(&ctx->lock, NULL);

    so_logd("[JNI] JniInitGlobalRefContext(): success!");
    return JNI_OK;
}

void JniFiniGlobalRefContext(void) {
    JniGlobalContext *ctx = &_so_global_ctx;
    if (ctx->slots) {
        for (jint i = 0; i < ctx->capacity; i++) {
            if (ctx->slots[i].obj) {
                Object_release(ctx->slots[i].obj);
                ctx->slots[i].obj = NULL;
            }
        }
        free(ctx->slots);
        ctx->slots = NULL;
    }

    if (ctx->next_free) {
        free(ctx->next_free);
        ctx->next_free = NULL;
    }

    ctx->capacity  = 0;
    ctx->count     = 0;
    ctx->free_head = 0;

    pthread_mutex_destroy(&ctx->lock);

    so_logd("[JNI] JniFiniGlobalRefContext()");
}

jint JniInitWeakGlobalRefContext(void) {
    JniGlobalContext *ctx = &_so_weak_ctx;
    if (ctx->slots) {
        return JNI_OK;
    }

    jint        capacity = INIT_REF_CAPACITY;
    JniRefSlot *slots    = calloc(capacity, sizeof(JniRefSlot));
    if (!slots) {
        return JNI_ERR;
    }

    jint *next_free = malloc(capacity * sizeof(jint));
    if (!next_free) {
        free(slots);
        return JNI_ERR;
    }
    for (jint i = 0; i < capacity; i++) {
        next_free[i] = i + 1;
    }

    ctx->slots     = slots;
    ctx->capacity  = capacity;
    ctx->count     = 0;
    ctx->free_head = 0;
    ctx->next_free = next_free;

    pthread_mutex_init(&ctx->lock, NULL);

    so_logd("[JNI] JniInitWeakGlobalRefContext(): success!");
    return JNI_OK;
}

void JniFiniWeakGlobalRefContext(void) {
    JniGlobalContext *ctx = &_so_weak_ctx;
    if (ctx->slots) {
        for (jint i = 0; i < ctx->capacity; i++) {
            if (ctx->slots[i].obj) {
                Object_releaseWeak(ctx->slots[i].obj, &ctx->slots[i].obj);
                ctx->slots[i].obj = NULL;
            }
        }
        free(ctx->slots);
        ctx->slots = NULL;
    }

    if (ctx->next_free) {
        free(ctx->next_free);
        ctx->next_free = NULL;
    }

    ctx->capacity  = 0;
    ctx->count     = 0;
    ctx->free_head = 0;

    pthread_mutex_destroy(&ctx->lock);

    so_logd("[JNI] JniFiniWeakGlobalRefContext()");
}
