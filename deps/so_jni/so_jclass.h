#ifndef SO_JNI_JCLASS_H
#define SO_JNI_JCLASS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "so_jni.h"

enum {
    SO_JSYMBOL_FLAGS_UNDEF  = 0,       // 未定义
    SO_JSYMBOL_FLAGS_STATIC = 1 << 0,  // static
    SO_JSYMBOL_FLAGS_NATIVE = 1 << 1,  // native（仅方法）
    SO_JSYMBOL_FLAGS_METHOD = 1 << 2,  // 方法
    SO_JSYMBOL_FLAGS_FIELD  = 1 << 3,  // 字段
};

typedef struct so_jsymbol {
    char     *name;       // 符号名
    char     *signature;  // 符号签名
    uintptr_t addr;       // 符号地址
    uint32_t  flags;      // 符号标志
} so_jsymbol;

#define SO_JSYMBOL_STATIC_METHOD(name, sig, p_func) \
    {name, sig, (uintptr_t)(p_func), (SO_JSYMBOL_FLAGS_STATIC | SO_JSYMBOL_FLAGS_METHOD)}
#define SO_JSYMBOL_STATIC_FIELD(name, sig, offset) \
    {name, sig, (uintptr_t)(offset), (SO_JSYMBOL_FLAGS_STATIC | SO_JSYMBOL_FLAGS_FIELD)}
#define SO_JSYMBOL_METHOD(name, sig, p_func) \
    {name, sig, (uintptr_t)(p_func), SO_JSYMBOL_FLAGS_METHOD}
#define SO_JSYMBOL_FIELD(name, sig, offset) \
    {name, sig, (uintptr_t)(offset), SO_JSYMBOL_FLAGS_FIELD}
#define SO_JSYMBOL_STATIC_NATIVE(name, sig, pp_func) \
    {name, sig, (uintptr_t)(pp_func), SO_JSYMBOL_FLAGS_STATIC | SO_JSYMBOL_FLAGS_NATIVE | SO_JSYMBOL_FLAGS_METHOD}
#define SO_JSYMBOL_NATIVE(name, sig, pp_func) \
    {name, sig, (uintptr_t)(pp_func), SO_JSYMBOL_FLAGS_NATIVE | SO_JSYMBOL_FLAGS_METHOD}

typedef struct so_jclass_meta {
    char       *name;           // 类名
    char       *super_name;     // 父类名
    so_jsymbol *symbols;        // 符号数组
    jsize       num_symbols;    // 符号数量
    jsize       instance_size;  // 实例数据大小
} so_jclass_meta;

const so_jsymbol *so_jclass_find_method(const jclass clazz, const char *name, const char *sig, jboolean is_static);
const so_jsymbol *so_jclass_find_field(const jclass clazz, const char *name, const char *sig, jboolean is_static);
const so_jsymbol *so_jclass_find_native(const jclass clazz, const char *name, const char *sig);

jclass  so_jclass_create(so_jclass_meta *meta);
jobject so_jclass_new_instance(jclass clazz);
jint    so_jclass_register_natives(jclass clazz, const JNINativeMethod *methods, jint num_nethods);
jint    so_jclass_unregister_natives(jclass clazz);

const char *so_jclass_get_name(jclass clazz);
jsize       so_jclass_get_instance_size(jclass clazz);
jclass      so_jclass_get_super(jclass clazz);
jboolean    so_jclass_is_assignable_from(jclass clazz1, jclass clazz2);

#ifdef __cplusplus
}
#endif

#endif