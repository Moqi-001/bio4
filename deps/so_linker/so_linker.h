#ifndef SO_LINKER_H
#define SO_LINKER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "elf.h"

// 单次 Hook 最多覆盖的原指令字节数
#define SO_HOOK_INSTR_MAX_LEN 32

typedef struct so_hook_ref {
    void   *target;                             // 目标函数地址（Thumb 保留 bit0）
    void   *orig_func;                          // 调用原函数（Thumb 已置位 bit0）
    uint8_t orig_instr[SO_HOOK_INSTR_MAX_LEN];  // 原指令备份（so_unhook 还原用）
    size_t  instr_len;                          // 原指令字节数
} so_hook_ref;

typedef struct so_symbol {
    char     *name;  // 符号名
    uintptr_t addr;  // 符号地址
    size_t    size;  // 符号大小
    uint8_t   info;  // 符号信息
} so_symbol;

// 声明一个函数类型的符号表条目
#define SO_SYMBOL_FUNC(name, p_func) \
    {name, (uintptr_t)(p_func), 0, ELF32_ST_INFO(STB_GLOBAL, STT_FUNC)}
// 声明一个变量类型的符号表条目
#define SO_SYMBOL_OBJECT(name, p_obj, size) \
    {name, (uintptr_t)(p_obj), size, ELF32_ST_INFO(STB_GLOBAL, STT_OBJECT)}

typedef struct so_library {
    const char *soname;
    so_symbol  *symbols;
    size_t      num_symbols;
    int (*prepare)(struct so_library *library);
} so_library;

typedef struct so_module so_module;

int  so_linker_init();
void so_linker_fini();

so_module *so_linker_find_module(const char *soname);

#define PROT_NONE  0x0
#define PROT_READ  0x1
#define PROT_WRITE 0x2
#define PROT_EXEC  0x4

int   so_cacheflush(void *addr, size_t length);
void *so_mmap(size_t length);
void  so_munmap(void *addr);
int   so_mprotect(void *addr, size_t length, int prot);

int so_hook(void *target, void *repl, so_hook_ref *p_hook);    // Thumb 函数需传入 (实际地址 | 1)
int so_hook_got(void *slot, void *repl, so_hook_ref *p_hook);  // 只改 slot 内的值，不修改其指向的函数
int so_unhook(so_hook_ref *p_hook);

int so_inject_data(void *target, const void *data, size_t len);

// hook 调用原函数
#define SO_CONTINUE(type, p_hook, ...) \
    (((type (*)())((p_hook)->orig_func))(__VA_ARGS__))

so_module *so_module_create(const so_library *library);    // 从 so_library 创建模块
so_module *so_module_load(const char *filename);           // 从文件加载模块
so_module *so_module_load_mem(void *buffer, size_t size);  // 从内存加载模块
void       so_module_unload(so_module *mod);               // 卸载模块
int        so_module_relocate(so_module *mod);             // 模块重定位
int        so_module_init(so_module *mod);                 // 模块初始化
void       so_module_fini(so_module *mod);                 // 模块反初始化
int        so_module_flush_caches(so_module *mod);         // 模块刷新缓存

so_module *so_module_open(const char *filename);           // 从文件打开模块 (load + relocate + init)
so_module *so_module_open_mem(void *buffer, size_t size);  // 从内存打开模块 (load + relocate + init)
void       so_module_close(so_module *mod);                // 关闭模块 (unload + fini)

int  so_module_retain(so_module *mod);
void so_module_release(so_module *mod);

void *so_module_find_export(so_module *mod, const char *sym_name);
void *so_module_find_import(so_module *mod, const char *sym_name);

int so_module_hook_address(so_module *mod, ptrdiff_t vaddr, void *repl, so_hook_ref *p_hook);  // Thumb 函数需传入"地址 | 1"
int so_module_hook_export(so_module *mod, const char *sym_name, void *repl, so_hook_ref *p_hook);
int so_module_hook_import(so_module *mod, const char *sym_name, void *repl, so_hook_ref *p_hook);

int so_module_inject_data(so_module *mod, ptrdiff_t vaddr, const void *data, size_t len);

const char *so_module_get_name(so_module *mod);
void       *so_module_get_entry_point(so_module *mod);
const void *so_module_get_load_base(so_module *mod);
size_t      so_module_get_load_size(so_module *mod);
intptr_t    so_module_get_load_bias(so_module *mod);

#ifdef __cplusplus
}
#endif

#endif
