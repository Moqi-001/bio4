#ifndef SO_LINKER_H
#define SO_LINKER_H

#ifdef __cplusplus
extern "C" {
#endif

#include "elf.h"

typedef struct so_hook_ref {
    void   *target;         // 目标
    int     instr_len;      // 指令长度
    uint8_t old_instr[16];  // 旧指令
    uint8_t new_instr[16];  // 新指令 (实现跳转)
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
#define SO_SYMBOL_OBJ(name, p_obj, size) \
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

void *so_mmap(size_t length);
void  so_munmap(void *addr);
int   so_msync(void *addr, size_t length);
void *so_memcpy(void *dst, const void *src, size_t n);
void *so_memset(void *dst, int c, size_t n);

int so_hook_addr_safe(void *target, void *boundary, void *replacement, so_hook_ref *p_hook);  // Thumb 函数需传入 (实际地址 | 1)
int so_hook_addr(void *target, void *replacement, so_hook_ref *p_hook);                       // Thumb 函数需传入 (实际地址 | 1)
int so_unhook(so_hook_ref *p_hook);

int so_inject_data_safe(void *target, void *boundary, const void *data, size_t len);
int so_inject_data(void *target, const void *data, size_t len);

// 调用原函数
#define SO_CONTINUE(type, p_hook, ...) ({                             \
    void *target_real = (void *)((uintptr_t)(p_hook)->target & ~1);   \
    so_memcpy(target_real, (p_hook)->old_instr, (p_hook)->instr_len); \
    so_msync(target_real, (p_hook)->instr_len);                       \
    type r = ((type (*)())((p_hook)->target))(__VA_ARGS__);           \
    so_memcpy(target_real, (p_hook)->new_instr, (p_hook)->instr_len); \
    so_msync(target_real, (p_hook)->instr_len);                       \
    r;                                                                \
})

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

void *so_module_find_symbol(so_module *mod, const char *sym_name);
void *so_module_find_symbol_ex(so_module *mod, const char *sym_name, int self_only,
                               so_module **out_mod, const Elf32_Sym **out_sym);

int  so_module_retain(so_module *mod);
void so_module_release(so_module *mod);

int so_module_hook_offset(so_module *mod, ptrdiff_t offset, void *replacement, so_hook_ref *p_hook);  // Thumb 函数需传入 (实际偏移 | 1)
int so_module_hook_symbol(so_module *mod, const char *sym_name, void *replacement, so_hook_ref *p_hook);

int so_module_inject_data_offset(so_module *mod, ptrdiff_t offset, const void *data, size_t len);

const char *so_module_name(so_module *mod);
void       *so_module_entry_point(so_module *mod);
const void *so_module_load_base(so_module *mod);
size_t      so_module_load_size(so_module *mod);

#ifdef __cplusplus
}
#endif

#endif
