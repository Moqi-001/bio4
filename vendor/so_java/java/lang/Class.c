#include <stdlib.h>
#include <string.h>

#include <so_common/so_logger.h>

#include "Class.h"
#include "Object.h"
#include "types.h"

typedef struct JavaClassImpl JavaClassImpl;
typedef struct JavaClassReg  JavaClassReg;

struct JavaClassReg {
    JavaClassImpl *clazz;
    JavaClassReg  *prev;
    JavaClassReg  *next;
};

struct JavaClassImpl {
    uint32_t       magic;    // 魔数
    JavaClassImpl *super;    // 父类
    uint32_t      *hashtab;  // 哈希表
    JClassMeta    *meta;     // 元数据
    JavaClassReg  *reg;      // 注册句柄
};

#define JAVA_CLASS_MAGIC    0x43A23456  // "JClass".hashCode()
#define JAVA_CLASS_CAST(_o) JAVA_BASE_CAST(JavaClassImpl *, _o, JAVA_CLASS_MAGIC)

JavaClassReg *_java_class_registrations = NULL;

static uint32_t _java_hash(const char *name, const char *sig) {
    uint32_t       h = 0, g;
    const uint8_t *p = (const uint8_t *)name;
    while (*p) {
        h = (h << 4) + *p++;
        if ((g = (h & 0xf0000000)) != 0) {
            h ^= g >> 24;
        }
        h &= 0x0fffffff;
    }
    p = (const uint8_t *)sig;
    while (*p) {
        h = (h << 4) + *p++;
        if ((g = (h & 0xf0000000)) != 0) {
            h ^= g >> 24;
        }
        h &= 0x0fffffff;
    }
    return h;
}

static uint32_t *_class_create_hashtab(JSymbol *symbols, JInt num_symbols) {
    if (!symbols || num_symbols <= 0) {
        return NULL;
    }

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
    for (size_t i = 0; i < nbucket; i++) {
        buckets[i] = nchain;
    }

    // 遍历符号表
    for (JInt i = 0; i < num_symbols; i++) {
        uint32_t hash       = _java_hash(symbols[i].name, symbols[i].signature);
        uint32_t bucket_idx = hash % nbucket;

        // 头插法
        chains[i]           = buckets[bucket_idx];
        buckets[bucket_idx] = i;
    }

    return hashtab;
}

static JSymbol *_class_find_symbol(const JavaClassImpl *cls, uint32_t hash, const char *name, const char *sig, int flags) {
    if (!cls || !cls->hashtab) {
        return NULL;
    }

    JClassMeta *meta    = cls->meta;
    uint32_t   *hashtab = cls->hashtab;
    if (!meta || !hashtab || !meta->symbols) {
        return NULL;
    }

    uint32_t  nbucket = hashtab[0];
    uint32_t  nchain  = hashtab[1];
    uint32_t *buckets = hashtab + 2;
    uint32_t *chains  = buckets + nbucket;

    for (uint32_t i = buckets[hash % nbucket]; i < nchain; i = chains[i]) {
        if (meta->symbols[i].flags == flags) {
            if (strcmp(meta->symbols[i].name, name) == 0 && strcmp(meta->symbols[i].signature, sig) == 0) {
                return &meta->symbols[i];
            }
        }
    }

    return NULL;
}

static JInt _class_register(JavaClassImpl *cls, JBoolean adopt) {
    if (cls->reg) {  // 已注册过
        return JAVA_OK;
    }

    JavaClassReg *reg = (JavaClassReg *)calloc(1, sizeof(JavaClassReg));
    if (!reg) {
        return JAVA_ERR;
    }

    reg->clazz = adopt ? cls : Object_retain(cls);

    if (!_java_class_registrations) {
        _java_class_registrations = reg;
    } else {
        _java_class_registrations->prev = reg;
        reg->next                       = _java_class_registrations;
        _java_class_registrations       = reg;
    }

    cls->reg = reg;

    return JAVA_OK;
}

static void _class_unregister(JavaClassImpl *cls) {
    if (!cls->reg) {
        return;
    }

    if (cls->reg->prev) {
        cls->reg->prev->next = cls->reg->next;
    }
    if (cls->reg->next) {
        cls->reg->next->prev = cls->reg->prev;
    }
    if (_java_class_registrations == cls->reg) {
        _java_class_registrations = cls->reg->next;
    }

    free(cls->reg);
    cls->reg = NULL;
    Object_release(cls);
}

static JavaClassImpl *_find_class(const char *name) {
    if (!name) {
        return NULL;
    }

    JavaClassReg *reg = _java_class_registrations;
    while (reg) {
        if (reg->clazz->meta->name && strcmp(reg->clazz->meta->name, name) == 0) {
            return reg->clazz;
        }
        reg = reg->next;
    }

    return NULL;
}

static JInt _class_ctor(JavaClassImpl *cls, JClassMeta *meta) {
    if (!cls || !meta) {
        return JAVA_ERR;
    }

    cls->magic   = JAVA_CLASS_MAGIC;
    cls->meta    = meta;
    cls->reg     = NULL;
    cls->super   = _find_class(meta->super_name);
    cls->hashtab = _class_create_hashtab(meta->symbols, meta->num_symbols);
    if (!cls->hashtab) {
        return JAVA_ERR;
    }

    _class_register(cls, JAVA_TRUE);

    return JAVA_OK;
}

static void _class_dtor(JavaClassImpl *cls) {
    if (!cls) {
        return;
    }

    if (cls->reg) {
        _class_unregister(cls->reg->clazz);
    }

    if (cls->hashtab) {
        free(cls->hashtab);
    }
}

JClass JavaCreateClass(JClassMeta *meta) {
    JavaClassImpl *cls = (JavaClassImpl *)Object_newWithoutClass(sizeof(JavaClassImpl), (JDtor)_class_dtor);
    if (!cls) {
        return NULL;
    }

    if (_class_ctor(cls, meta) != JAVA_OK) {
        Object_release(cls);
        return NULL;
    }

    so_logd("[JAVA] JavaCreateClass('%s'): %p", meta->name, cls);
    return (JClass)cls;
}

JClass JavaFindClass(const char *name) {
    return (JClass)_find_class(name);
}

void JavaReleaseAllClasses(void) {
    if (!_java_class_registrations) {
        return;
    }

    JavaClassReg *reg = _java_class_registrations;
    while (reg) {
        JavaClassReg  *next  = reg->next;
        JavaClassImpl *clazz = reg->clazz;
        free(reg);
        if (clazz) {
            clazz->reg = NULL;
            Object_release(clazz);
        }
        reg = next;
    }
    _java_class_registrations = NULL;
}

const JSymbol *Class_findMethodSymbol(const JClass clazz, const char *name, const char *sig, JBoolean is_static) {
    JavaClassImpl *cls = JAVA_CLASS_CAST(clazz);
    if (!cls || !name || !sig) {
        return NULL;
    }

    uint32_t hash  = _java_hash(name, sig);
    uint32_t flags = JAVA_SYMBOL_FLAGS_METHOD;
    if (is_static) {
        flags |= JAVA_SYMBOL_FLAGS_STATIC;
    }

    return _class_find_symbol(cls, hash, name, sig, flags);
}

const JSymbol *Class_findFieldSymbol(const JClass clazz, const char *name, const char *sig, JBoolean is_static) {
    JavaClassImpl *cls = JAVA_CLASS_CAST(clazz);
    if (!cls || !name || !sig) {
        return NULL;
    }

    uint32_t hash  = _java_hash(name, sig);
    uint32_t flags = JAVA_SYMBOL_FLAGS_FIELD;
    if (is_static) {
        flags |= JAVA_SYMBOL_FLAGS_STATIC;
    }

    return _class_find_symbol(cls, hash, name, sig, flags);
}

const JSymbol *Class_findNativeSymbol(const JClass clazz, const char *name, const char *sig) {
    JavaClassImpl *cls = JAVA_CLASS_CAST(clazz);
    if (!cls || !name || !sig) {
        return NULL;
    }

    uint32_t hash  = _java_hash(name, sig);
    uint32_t flags = JAVA_SYMBOL_FLAGS_NATIVE | JAVA_SYMBOL_FLAGS_METHOD;

    return _class_find_symbol(cls, hash, name, sig, flags);
}

JClass Class_getSuper(JClass clazz) {
    JavaClassImpl *cls = JAVA_CLASS_CAST(clazz);
    if (!cls) {
        return NULL;
    }

    return (JClass)cls->super;
}

JBoolean Class_isAssignableFrom(JClass _this, JClass _from) {
    JavaClassImpl *thiz = JAVA_CLASS_CAST(_this);
    JavaClassImpl *from = JAVA_CLASS_CAST(_from);
    if (!thiz || !from) {
        return JAVA_FALSE;
    }

    if (thiz == from) {
        return JAVA_TRUE;
    }

    // 判断 from 是否是 thiz 的子类 (子类包含了父类的所有数据，可以赋值给父类)
    JavaClassImpl *from_super = from->super;
    while (from_super) {
        if (thiz == from_super) {
            return JAVA_TRUE;
        }
        from_super = from_super->super;
    }

    return JAVA_FALSE;
}

JInt Class_registerNatives(JClass clazz, const JNativeMethod *methods, JInt num_methods) {
    JavaClassImpl *cls = JAVA_CLASS_CAST(clazz);
    if (!cls || !methods || num_methods <= 0) {
        return JAVA_ERR;
    }

    JClassMeta *meta = cls->meta;
    if (!meta || !meta->symbols || meta->num_symbols <= 0) {
        return JAVA_ERR;
    }

    so_logd("[JAVA][Class] registerNatives(%p,%p,%d): ('%s')", clazz, methods, num_methods, meta->name);

    for (JInt i = 0; i < num_methods; i++) {
        JSymbol *sym = (JSymbol *)Class_findNativeSymbol(clazz, methods[i].name, methods[i].signature);
        if (!sym || !sym->addr) {
            so_loge("  natives register error: name='%s', signature='%s', fnPtr=%p", methods[i].name, methods[i].signature, methods[i].fnPtr);
            continue;
        }

        *(void **)sym->addr = methods[i].fnPtr;  // 重定位 native 地址
        so_logd("   registered: name='%s', signature='%s', fnPtr=%p", methods[i].name, methods[i].signature, methods[i].fnPtr);
    }

    return JAVA_OK;
}

JInt Class_unregisterNatives(JClass clazz) {
    JavaClassImpl *cls = JAVA_CLASS_CAST(clazz);
    if (!cls) {
        return JAVA_ERR;
    }

    JClassMeta *meta = cls->meta;
    if (!meta || !meta->symbols || meta->num_symbols <= 0) {
        return JAVA_ERR;
    }

    so_logd("[JAVA][Class] UnregisterNatives(%p): ('%s')", clazz, meta->name);
    for (JInt i = 0; i < meta->num_symbols; i++) {
        if (meta->symbols[i].flags == JAVA_SYMBOL_FLAGS_NATIVE && meta->symbols[i].addr) {
            *(void **)meta->symbols[i].addr = NULL;  // 置空 native 地址
            so_logd("   unregistered: name='%s', signature='%s'", meta->symbols[i].name, meta->symbols[i].signature);
        }
    }

    return JAVA_OK;
}

JClassMeta *Class_getMeta(JClass clazz) {
    JavaClassImpl *cls = JAVA_CLASS_CAST(clazz);
    if (!cls) {
        return NULL;
    }

    return cls->meta;
}

const char *Class_getName(JClass clazz) {
    JavaClassImpl *cls = JAVA_CLASS_CAST(clazz);
    if (!cls) {
        return NULL;
    }

    return cls->meta->name;
}

JInt Class_getInstanceSize(JClass clazz) {
    JavaClassImpl *cls = JAVA_CLASS_CAST(clazz);
    if (!cls) {
        return 0;
    }

    return cls->meta->instance_size;
}

JDtor Class_getDestructor(JClass clazz) {
    JavaClassImpl *cls = JAVA_CLASS_CAST(clazz);
    if (!cls) {
        return 0;
    }

    return cls->meta->dtor;
}
