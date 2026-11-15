#ifndef JAVA_LANG_CLASS_H
#define JAVA_LANG_CLASS_H

#include "types.h"

enum {
    JAVA_SYMBOL_FLAGS_UNDEF  = 0,       // 未定义
    JAVA_SYMBOL_FLAGS_STATIC = 1 << 0,  // static
    JAVA_SYMBOL_FLAGS_NATIVE = 1 << 1,  // native（仅方法）
    JAVA_SYMBOL_FLAGS_METHOD = 1 << 2,  // 方法
    JAVA_SYMBOL_FLAGS_FIELD  = 1 << 3,  // 字段
};

typedef struct JSymbol {
    char     *name;       // 符号名
    char     *signature;  // 符号签名
    uintptr_t addr;       // 符号地址
    uint32_t  flags;      // 符号标志
    JMethod   method;     // 方法句柄
} JSymbol;

typedef struct JClassMeta {
    char    *name;           // 类名
    char    *super_name;     // 父类名
    JSymbol *symbols;        // 符号数组
    JInt     num_symbols;    // 符号数量
    JInt     instance_size;  // 实例数据大小
    JDtor    dtor;           // 析构函数
} JClassMeta;

// 对应 JNI 的 JNINativeMethod，内存布局完全一致，可直接与 jni 层互转
typedef struct JNativeMethod {
    const char *name;       // 方法名
    const char *signature;  // 方法签名
    void       *fnPtr;      // native 函数指针
} JNativeMethod;

JClass JavaCreateClass(JClassMeta *meta);
JClass JavaFindClass(const char *name);
void   JavaReleaseAllClasses(void);

const JSymbol *Class_findMethodSymbol(const JClass clazz, const char *name, const char *sig, JBoolean is_static);
const JSymbol *Class_findFieldSymbol(const JClass clazz, const char *name, const char *sig, JBoolean is_static);
const JSymbol *Class_findNativeSymbol(const JClass clazz, const char *name, const char *sig);
JClass         Class_getSuper(JClass clazz);
JBoolean       Class_isAssignableFrom(JClass thiz, JClass from);  // JNI 里这个参数应该是搞反了，这里调整了，使用时要注意参数顺序
JInt           Class_registerNatives(JClass clazz, const JNativeMethod *methods, JInt num_methods);
JInt           Class_unregisterNatives(JClass clazz);

JClassMeta *Class_getMeta(JClass clazz);
const char *Class_getName(JClass clazz);
JInt        Class_getInstanceSize(JClass clazz);
JDtor       Class_getDestructor(JClass clazz);

// ============================================
// JAVA 符号表定义宏 (简化 JSymbol 结构体初始化，免去手动填写标志位等)
// ============================================
#define JAVA_SYMBOL_STATIC_METHOD(name, sig, p_func) \
    {name, sig, (uintptr_t)(p_func), (JAVA_SYMBOL_FLAGS_STATIC | JAVA_SYMBOL_FLAGS_METHOD), NULL}
#define JAVA_SYMBOL_STATIC_FIELD(name, sig, offset) \
    {name, sig, (uintptr_t)(offset), (JAVA_SYMBOL_FLAGS_STATIC | JAVA_SYMBOL_FLAGS_FIELD), NULL}
#define JAVA_SYMBOL_METHOD(name, sig, p_func) \
    {name, sig, (uintptr_t)(p_func), JAVA_SYMBOL_FLAGS_METHOD, NULL}
#define JAVA_SYMBOL_FIELD(name, sig, offset) \
    {name, sig, (uintptr_t)(offset), JAVA_SYMBOL_FLAGS_FIELD, NULL}
#define JAVA_SYMBOL_NATIVE(name, sig, pp_func) \
    {name, sig, (uintptr_t)(pp_func), JAVA_SYMBOL_FLAGS_NATIVE | JAVA_SYMBOL_FLAGS_METHOD, NULL}
#define JAVA_SYMBOL_STATIC_NATIVE(name, sig, pp_func) \
    {name, sig, (uintptr_t)(pp_func), JAVA_SYMBOL_FLAGS_STATIC | JAVA_SYMBOL_FLAGS_NATIVE | JAVA_SYMBOL_FLAGS_METHOD, NULL}

#endif