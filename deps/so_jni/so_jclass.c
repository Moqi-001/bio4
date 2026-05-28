#include <stdlib.h>
#include <string.h>

#include <so_common/so_logger.h>

#include "java/JavaInstance.h"
#include "so_jni_internal.h"

typedef struct JniClass {
    uint32_t    magic;        // 魔数
    jclass      super_class;  // 父类
    uint32_t   *hashtab;      // 哈希表
    jclassMeta *meta;         // 元数据
    jclassNode *node;         // 节点
} JniClass;

#define JNI_CLASS_MAGIC 0x791400F3  // "JniClass".hashCode()
#define JNI_CLASS_CAST(_c) \
    ((JniClass *)((_c) && ((JniClass *)(_c))->magic == JNI_CLASS_MAGIC ? (_c) : NULL))
#define JNI_CLASS_FROM_OBJ(_o)                  \
    ({                                          \
        void *_inst = JniGetObjectInstance(_o); \
        JNI_CLASS_CAST(_inst);                  \
    })

static jclassNode *_so_jclass_head = NULL;

/********************* internal *********************/
static uint32_t _jni_hash(const char *name, const char *sig) {
    uint32_t       h = 0, g;
    const uint8_t *p = (const uint8_t *)name;
    while (*p) {
        h = (h << 4) + *p++;
        if ((g = (h & 0xf0000000)) != 0)
            h ^= g >> 24;
        h &= 0x0fffffff;
    }
    p = (const uint8_t *)sig;
    while (*p) {
        h = (h << 4) + *p++;
        if ((g = (h & 0xf0000000)) != 0)
            h ^= g >> 24;
        h &= 0x0fffffff;
    }
    return h;
}

static uint32_t *_jclass_create_hashtab(jsymbol *symbols, jsize num_symbols) {
    if (!symbols || num_symbols <= 0) return NULL;

    size_t    nbucket      = num_symbols;                                // 哈希桶数量 = 符号数量
    size_t    nchain       = num_symbols;                                // 哈希链数量 = 符号数量
    size_t    hashtab_size = (2 + nbucket + nchain) * sizeof(uint32_t);  // 哈希表大小 = 头部(2个uint32_t) + 桶数组 + 链数组
    uint32_t *hashtab      = (uint32_t *)malloc(hashtab_size);
    if (!hashtab) {
        return NULL;
    }

    // 设置哈希表头部
    hashtab[0] = nbucket;  // 桶数量
    hashtab[1] = nchain;   // 链数量

    uint32_t *buckets = hashtab + 2;        // 桶数组
    uint32_t *chains  = buckets + nbucket;  // 链数组

    // 把 buckets 全部初始化为 nchain（无效索引）（elf采用0位置为无效索引，但我们这里0是有效的，所以采用nchain做无效索引。读取时需要注意判断，不要访问了越界的数据）
    for (size_t i = 0; i < nchain; i++) {
        buckets[i] = nchain;
    }

    // 遍历符号表
    for (jsize i = 0; i < num_symbols; i++) {
        uint32_t hash       = _jni_hash(symbols[i].name, symbols[i].signature);
        uint32_t bucket_idx = hash % nbucket;

        // 头插法
        chains[i]           = buckets[bucket_idx];
        buckets[bucket_idx] = i;
    }

    return hashtab;
}

static jsymbol *_jclass_find_symbol(const JniClass *cls, uint32_t sym_hash, const char *sym_name, const char *sym_sig, int sym_flags) {
    if (!cls) return NULL;

    jclassMeta *meta    = cls->meta;
    uint32_t   *hashtab = cls->hashtab;
    if (!meta || !hashtab || !meta->symbols) {
        return NULL;
    }

    uint32_t  nbucket = hashtab[0];
    uint32_t  nchain  = hashtab[1];
    uint32_t *buckets = hashtab + 2;
    uint32_t *chains  = buckets + nbucket;

    for (uint32_t i = buckets[sym_hash % nbucket]; i < nchain; i = chains[i]) {
        if (meta->symbols[i].flags == sym_flags) {
            if (strcmp(meta->symbols[i].name, sym_name) == 0 && strcmp(meta->symbols[i].signature, sym_sig) == 0) {
                return &meta->symbols[i];
            }
        }
    }

    return NULL;
}

const jsymbol *JniFindMethod(const jclass clazz, const char *name, const char *sig, jboolean is_static) {
    JniClass *cls = JNI_CLASS_FROM_OBJ(clazz);
    if (!cls || !name || !sig) return NULL;

    uint32_t hash  = _jni_hash(name, sig);
    uint32_t flags = JNI_SYMBOL_FLAGS_METHOD;
    if (is_static) flags |= JNI_SYMBOL_FLAGS_STATIC;

    return _jclass_find_symbol(cls, hash, name, sig, flags);
}

const jsymbol *JniFindField(const jclass clazz, const char *name, const char *sig, jboolean is_static) {
    JniClass *cls = JNI_CLASS_FROM_OBJ(clazz);
    if (!cls || !name || !sig) return NULL;

    uint32_t hash  = _jni_hash(name, sig);
    uint32_t flags = JNI_SYMBOL_FLAGS_FIELD;
    if (is_static) flags |= JNI_SYMBOL_FLAGS_STATIC;

    return _jclass_find_symbol(cls, hash, name, sig, flags);
}

const jsymbol *JniFindNative(const jclass clazz, const char *name, const char *sig) {
    JniClass *cls = JNI_CLASS_FROM_OBJ(clazz);
    if (!cls || !name || !sig) return NULL;

    uint32_t hash  = _jni_hash(name, sig);
    uint32_t flags = JNI_SYMBOL_FLAGS_NATIVE | JNI_SYMBOL_FLAGS_METHOD;

    return _jclass_find_symbol(cls, hash, name, sig, flags);
}

const char *JniGetClassName(jclass clazz) {
    JniClass *cls = JNI_CLASS_FROM_OBJ(clazz);
    if (!cls || !cls->meta) return NULL;

    return cls->meta->name;
}

jsize JniGetClassInstanceSize(jclass clazz) {
    JniClass *cls = JNI_CLASS_FROM_OBJ(clazz);
    if (!cls || !cls->meta) return 0;

    return cls->meta->instance_size;
}

void JniClearClasses(void) {
    if (_so_jclass_head) {
        jclassNode *node = _so_jclass_head;
        while (node) {
            jclassNode *next = node->next;
            JniDeleteGlobalRef(node->clazz);
            free(node);
            node = next;
        }
        _so_jclass_head = NULL;
    }
}

/********************* exports *********************/
static jint _jclass_register(jclass clazz) {
    JniClass *cls = JNI_CLASS_FROM_OBJ(clazz);
    if (!cls) return JNI_EINVAL;

    if (cls->node) return JNI_OK;

    jclassNode *node = (jclassNode *)calloc(1, sizeof(jclassNode));
    if (!node) {
        return JNI_ENOMEM;
    }

    node->clazz = JniNewGlobalRef(clazz);
    cls->node   = node;

    if (!_so_jclass_head) {
        _so_jclass_head = node;
    } else {
        _so_jclass_head->prev = node;
        node->next            = _so_jclass_head;
        _so_jclass_head       = node;
    }

    return JNI_OK;
}

static void _jclass_unregister(jclass clazz) {
    JniClass *cls = JNI_CLASS_FROM_OBJ(clazz);
    if (!cls) return;

    if (!cls->node) return;

    if (cls->node->prev) {
        cls->node->prev->next = cls->node->next;
    }
    if (cls->node->next) {
        cls->node->next->prev = cls->node->prev;
    }
    if (_so_jclass_head == cls->node) {
        _so_jclass_head = cls->node->next;
    }

    JniDeleteGlobalRef(cls);
    free(cls->node);
    cls->node = NULL;
}

static jint _jclass_ctor(JniClass *cls, jclassMeta *meta) {
    if (!cls || !meta) return JNI_EINVAL;

    cls->magic       = JNI_CLASS_MAGIC;
    cls->meta        = meta;
    cls->node        = NULL;
    cls->super_class = JniFindClass(meta->super_name);
    cls->hashtab     = _jclass_create_hashtab(meta->symbols, meta->num_symbols);
    if (!cls->hashtab) {
        return JNI_ERR;
    }

    return JNI_OK;
}

static void _jclass_dtor(JniClass *cls) {
    if (!cls) return;

    if (cls->node) {
        _jclass_unregister(cls->node->clazz);
    }

    if (cls->hashtab) {
        free(cls->hashtab);
    }
}

jclass JniCreateClass(jclassMeta *meta) {
    if (!meta || !meta->name) return NULL;

    JniClass *cls = (JniClass *)JavaInstanceAlloc(sizeof(JniClass), (JavaDtor_t)_jclass_dtor);
    if (!cls) {
        so_loge("[JNI] JniCreateClass(%p): failed to alloc a new class! ('%s')", meta, meta->name);
        return NULL;
    }

    if (_jclass_ctor(cls, meta) != JNI_OK) {
        so_loge("[JNI] JniCreateClass(%p): failed to ctor instance! ('%s')", meta, meta->name);
        JavaInstanceRelease(cls);
        return NULL;
    }

    jclass clazz = JniCreateObject(NULL, cls);
    if (!clazz) {
        so_loge("[JNI] JniCreateClass(%p): failed to alloc a new object! ('%s')", meta, meta->name);
        JavaInstanceRelease(cls);
        return NULL;
    }

    _jclass_register(clazz); // 注册到全局

    so_logd("[JNI] JniCreateClass(%p): ('%s')", meta, meta->name);
    return clazz;
}

jclass JniFindClass(const char *name) {
    if (!name) return NULL;

    jclassNode *node = _so_jclass_head;
    while (node) {
        const char *cls_name = JniGetClassName(node->clazz);
        if (cls_name && strcmp(cls_name, name) == 0) {
            return JniNewLocalRef(node->clazz);
        }
        node = node->next;
    }

    return NULL;
}

jclass JniGetSuperclass(jclass clazz) {
    JniClass *cls = JNI_CLASS_FROM_OBJ(clazz);
    if (!cls || cls->super_class) return NULL;

    return JniNewLocalRef(cls->super_class);
}

jboolean JniIsAssignableFrom(jclass clazz1, jclass clazz2) {
    JniClass *cls1 = JNI_CLASS_FROM_OBJ(clazz1);
    JniClass *cls2 = JNI_CLASS_FROM_OBJ(clazz2);
    if (!cls1 || !cls2) return JNI_FALSE;

    if (cls1 == cls2) return JNI_TRUE;

    // 检查 clazz2 是否是 clazz1 的父类
    JniClass *super1 = JNI_CLASS_FROM_OBJ(cls1->super_class);
    while (super1) {
        if (super1 == cls2) return JNI_TRUE;
        super1 = JNI_CLASS_FROM_OBJ(super1->super_class);
    }

    return JNI_FALSE;
}

jint JniRegisterNatives(jclass clazz, const JNINativeMethod *methods, jint num_nethods) {
    JniClass *cls = JNI_CLASS_FROM_OBJ(clazz);
    if (!cls || !methods || num_nethods <= 0) return JNI_EINVAL;

    jclassMeta *meta = cls->meta;
    if (!meta || !meta->symbols || meta->num_symbols <= 0) {
        return JNI_ERR;
    }

    so_logd("[JNI] JniRegisterNatives(%p,%p,%d): ('%s')", clazz, methods, num_nethods, meta->name);

    for (jint i = 0; i < num_nethods; i++) {
        jsymbol *sym = (jsymbol *)JniFindNative(clazz, methods[i].name, methods[i].signature);
        if (!sym || !sym->addr) {
            so_loge("  natives[%d] register error: name='%s', signature='%s', fnPtr=%p", i, methods[i].name, methods[i].signature, methods[i].fnPtr);
            continue;
        }

        *(void **)sym->addr = methods[i].fnPtr;  // 重定位 native 地址
        so_logd("  natives[%d] registered: name='%s', signature='%s', fnPtr=%p", i, methods[i].name, methods[i].signature, methods[i].fnPtr);
    }

    return JNI_OK;
}

jint JniUnregisterNatives(jclass clazz) {
    JniClass *cls = JNI_CLASS_FROM_OBJ(clazz);
    if (!cls) return JNI_EINVAL;

    jclassMeta *meta = cls->meta;
    if (!meta || !meta->symbols || meta->num_symbols <= 0) {
        return JNI_ERR;
    }

    so_logd("[JNI] JniUnregisterNatives(%p): ('%s')", clazz, meta->name);

    for (jsize i = 0; i < meta->num_symbols; i++) {
        if (meta->symbols[i].flags == JNI_SYMBOL_FLAGS_NATIVE && meta->symbols[i].addr) {
            *(void **)meta->symbols[i].addr = NULL;  // 置空 native 地址
            so_logd("  natives unregistered: name='%s', signature='%s'", meta->symbols[i].name, meta->symbols[i].signature);
        }
    }

    return JNI_OK;
}
