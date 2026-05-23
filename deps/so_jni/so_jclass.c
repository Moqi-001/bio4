#include <stdlib.h>
#include <string.h>

#include <so_common/so_logger.h>

#include "so_jni.h"
#include "so_jni_internal.h"

typedef struct so_jclass {
    uint32_t        magic;        // 魔数
    jclass          super_class;  // 父类
    so_jclass_meta *meta;         // 元数据
    uint32_t       *hashtab;      // 哈希表
} so_jclass;

#define SO_JCLASS_MAGIC 0x0157AA91  // "so_jclass".hashCode()

#define SO_JCLASS_CAST(_c) \
    ((so_jclass *)((_c) && ((so_jclass *)(_c))->magic == SO_JCLASS_MAGIC ? (_c) : NULL))

#define SO_JCLASS_FROM_OBJ(_o)                     \
    ({                                             \
        void *_inst = so_jobject_get_instance(_o); \
        SO_JCLASS_CAST(_inst);                     \
    })

static uint32_t *_jclass_create_hashtab(so_jsymbol *symbols, jsize num_symbols) {
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
        uint32_t hash       = so_jni_hash(symbols[i].name, symbols[i].signature);
        uint32_t bucket_idx = hash % nbucket;

        // 头插法
        chains[i]           = buckets[bucket_idx];
        buckets[bucket_idx] = i;
    }

    return hashtab;
}

static so_jsymbol *_jclass_find_symbol(const so_jclass *cls, uint32_t sym_hash, const char *sym_name, const char *sym_sig, int sym_flags) {
    if (!cls) return NULL;

    so_jclass_meta *meta    = cls->meta;
    uint32_t       *hashtab = cls->hashtab;
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

const so_jsymbol *so_jclass_find_method(const jclass clazz, const char *name, const char *sig, jboolean is_static) {
    so_jclass *cls = SO_JCLASS_FROM_OBJ(clazz);
    if (!cls || !name || !sig) return NULL;

    uint32_t hash  = so_jni_hash(name, sig);
    uint32_t flags = SO_JSYMBOL_FLAGS_METHOD;
    if (is_static) flags |= SO_JSYMBOL_FLAGS_STATIC;

    return _jclass_find_symbol(cls, hash, name, sig, flags);
}

const so_jsymbol *so_jclass_find_field(const jclass clazz, const char *name, const char *sig, jboolean is_static) {
    so_jclass *cls = SO_JCLASS_FROM_OBJ(clazz);
    if (!cls || !name || !sig) return NULL;

    uint32_t hash  = so_jni_hash(name, sig);
    uint32_t flags = SO_JSYMBOL_FLAGS_FIELD;
    if (is_static) flags |= SO_JSYMBOL_FLAGS_STATIC;

    return _jclass_find_symbol(cls, hash, name, sig, flags);
}

const so_jsymbol *so_jclass_find_native(const jclass clazz, const char *name, const char *sig) {
    so_jclass *cls = SO_JCLASS_FROM_OBJ(clazz);
    if (!cls || !name || !sig) return NULL;

    uint32_t hash  = so_jni_hash(name, sig);
    uint32_t flags = SO_JSYMBOL_FLAGS_NATIVE | SO_JSYMBOL_FLAGS_METHOD;

    return _jclass_find_symbol(cls, hash, name, sig, flags);
}

static jint _jclass_ctor(so_jclass *cls, so_jclass_meta *meta) {
    if (!cls || !meta) return JNI_EINVAL;

    cls->magic   = SO_JCLASS_MAGIC;
    cls->meta    = meta;
    cls->hashtab = _jclass_create_hashtab(meta->symbols, meta->num_symbols);
    if (!cls->hashtab) {
        return JNI_ERR;
    }
    cls->super_class = so_jni_find_class(meta->super_name);

    return JNI_OK;
}

static void _jclass_dtor(so_jclass *cls) {
    if (!cls) return;

    if (cls->hashtab) {
        free(cls->hashtab);
    }
}

jclass so_jclass_create(so_jclass_meta *meta) {
    if (!meta || !meta->name) return NULL;

    so_jclass *cls = (so_jclass *)so_jinstance_new(sizeof(so_jclass), (so_jdtor_t)_jclass_dtor);
    if (!cls) {
        so_loge("[JNI] so_jclass_create(%p): failed to alloc a new class! ('%s')", meta, meta->name);
        return NULL;
    }

    if (_jclass_ctor(cls, meta) != JNI_OK) {
        so_loge("[JNI] so_jclass_create(%p): failed to ctor instance! ('%s')", meta, meta->name);
        so_jinstance_release(cls);
        return NULL;
    }

    jclass clazz = so_jobject_new(NULL, cls);
    if (!clazz) {
        so_loge("[JNI] so_jclass_create(%p): failed to alloc a new object! ('%s')", meta, meta->name);
        so_jinstance_release(cls);
        return NULL;
    }

    so_logd("[JNI] so_jclass_create(%p): ('%s')", meta, meta->name);
    return clazz;
}

jobject so_jclass_new_instance(jclass clazz) {
    if (!clazz) return NULL;

    void *instance = so_jinstance_new(so_jclass_get_instance_size(clazz), NULL);
    if (!instance) {
        so_loge("[JNI] so_jclass_new_instance(%p): failed to alloc a new instance! ('%s')", clazz);
        return NULL;
    }

    jclass object = so_jobject_new(clazz, instance);
    if (!object) {
        so_loge("[JNI] so_jclass_create(%p): failed to alloc a new object! ('%s')", clazz);
        so_jinstance_release(instance);
        return NULL;
    }

    return object;
}

jint so_jclass_register_natives(jclass clazz, const JNINativeMethod *methods, jint num_nethods) {
    so_jclass *cls = SO_JCLASS_FROM_OBJ(clazz);
    if (!cls || !methods || num_nethods <= 0) return JNI_EINVAL;

    so_jclass_meta *meta = cls->meta;
    if (!meta || !meta->symbols || meta->num_symbols <= 0) {
        return JNI_ERR;
    }

    so_logd("[JNI] so_jclass_register_natives(%p,%p,%d): ('%s')", clazz, methods, num_nethods, meta->name);

    for (jint i = 0; i < num_nethods; i++) {
        so_jsymbol *sym = (so_jsymbol *)so_jclass_find_native(clazz, methods[i].name, methods[i].signature);
        if (!sym || !sym->addr) {
            so_loge("  natives[%d] register error: name='%s', signature='%s', fnPtr=%p", i, methods[i].name, methods[i].signature, methods[i].fnPtr);
            continue;
        }

        *(void **)sym->addr = methods[i].fnPtr;  // 重定位 native 地址
        so_logd("  natives[%d] registered: name='%s', signature='%s', fnPtr=%p", i, methods[i].name, methods[i].signature, methods[i].fnPtr);
    }

    return JNI_OK;
}

jint so_jclass_unregister_natives(jclass clazz) {
    so_jclass *cls = SO_JCLASS_FROM_OBJ(clazz);
    if (!cls) return JNI_EINVAL;

    so_jclass_meta *meta = cls->meta;
    if (!meta || !meta->symbols || meta->num_symbols <= 0) {
        return JNI_ERR;
    }

    so_logd("[JNI] so_jclass_unregister_natives(%p): ('%s')", clazz, meta->name);

    for (jsize i = 0; i < meta->num_symbols; i++) {
        if (meta->symbols[i].flags == SO_JSYMBOL_FLAGS_NATIVE && meta->symbols[i].addr) {
            *(void **)meta->symbols[i].addr = NULL;  // 置空 native 地址
            so_logd("  natives unregistered: name='%s', signature='%s'", meta->symbols[i].name, meta->symbols[i].signature);
        }
    }

    return JNI_OK;
}

const char *so_jclass_get_name(jclass clazz) {
    so_jclass *cls = SO_JCLASS_FROM_OBJ(clazz);
    if (!cls || !cls->meta) return NULL;

    return cls->meta->name;
}

jsize so_jclass_get_instance_size(jclass clazz) {
    so_jclass *cls = SO_JCLASS_FROM_OBJ(clazz);
    if (!cls || !cls->meta) return 0;

    return cls->meta->instance_size;
}

jclass so_jclass_get_super(jclass clazz) {
    so_jclass *cls = SO_JCLASS_FROM_OBJ(clazz);
    if (!cls) return NULL;

    return cls->super_class;
}

jboolean so_jclass_is_assignable_from(jclass clazz1, jclass clazz2) {
    so_jclass *cls1 = SO_JCLASS_FROM_OBJ(clazz1);
    so_jclass *cls2 = SO_JCLASS_FROM_OBJ(clazz2);
    if (!cls1 || !cls2) return JNI_FALSE;

    if (cls1 == cls2) return JNI_TRUE;

    // 检查 clazz2 是否是 clazz1 的父类
    so_jclass *super1 = SO_JCLASS_FROM_OBJ(cls1->super_class);
    while (super1) {
        if (super1 == cls2) return JNI_TRUE;
        super1 = SO_JCLASS_FROM_OBJ(super1->super_class);
    }

    return JNI_FALSE;
}
