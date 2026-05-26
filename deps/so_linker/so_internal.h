#ifndef SO_LINKER_INTERNAL_H
#define SO_LINKER_INTERNAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#include "so_linker.h"

#define PAGE_SIZE 4096

#define ALIGN_UP(x, align)   (((x) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(x, align) ((x) & ~((align) - 1))

typedef struct so_context     so_context;
typedef struct so_module      so_module;
typedef struct so_module_node so_module_node;

enum {
    SO_MODULE_STATUS_NONE        = 0,
    SO_MODULE_STATUS_RELOCATED   = 1,
    SO_MODULE_STATUS_INITIALIZED = 2,
    SO_MODULE_STATUS_FINALIZED   = 3
};

struct so_module {
    int   ref_count;  // 引用计数
    int   status;     // 模块状态
    void *cookie;     // 容器标记（不透明指针，由 context 管理）

    char *soname;  // 共享对象名称
    char *sopath;  // 完整文件路径

    void  *load_base;  // 内存块基址
    size_t load_size;  // 内存块大小

    // 入口点
    void *entry_point;  // 入口点地址

    // 动态段
    Elf32_Dyn *dynamic;      // .dynamic 动态段
    size_t     num_dynamic;  // .dynamic 动态段条目数

    // 符号表与字符串表
    Elf32_Sym *dynsym;       // .dynsym 动态符号表
    size_t     num_dynsym;   // .dynsym 动态符号表条目数
    char      *dynstr;       // .dynstr 动态字符串表
    size_t     dynstr_size;  // .dynsym 动态符号表条大小
    uint32_t  *hash;         // .hash   符号哈希表（SysV 哈希，用于加速查找）

    // 动态重定位（用于变量/数据）
    Elf32_Rel  *rel_dyn;       // .rel.dyn 重定位表（REL 格式）
    size_t      num_rel_dyn;   // .rel.dyn 重定位条目数
    Elf32_Rela *rela_dyn;      // .rela.dyn 重定位表（RELA 格式）
    size_t      num_rela_dyn;  // .rela.dyn 重定位条目数

    // PLT 重定位（用于函数调用）
    Elf32_Rel  *rel_plt;       // .rel.plt 重定位表（REL 格式）
    size_t      num_rel_plt;   // .rel.plt 重定位条目数
    Elf32_Rela *rela_plt;      // .rela.plt 重定位表（RELA 格式）
    size_t      num_rela_plt;  // .rela.plt 重定位条目数

    // 初始化函数
    void (*init)(void);         // .init 初始化函数
    void (**init_array)(void);  // .init_array 初始化函数数组
    size_t num_init_array;      // .init_array 初始化函数数量

    // 终止函数
    void (*fini)(void);         // .fini 终止函数
    void (**fini_array)(void);  // .fini_array 终止函数数组
    size_t num_fini_array;      // .fini_array 终止函数数量
};

struct so_context {
    // 模块管理
    so_module_node *modules;      // 模块链表头
    size_t          num_modules;  // 模块数量

    // TLS 管理（未实现）
    uint32_t *tls_free_slots;  // 空闲 TLS 槽位链表
    size_t    tls_free_count;  // 空闲槽位数量
    uint32_t  tls_next_slot;   // 下一个新槽位（当无空闲时）
};

struct so_module_node {
    so_module      *mod;
    so_context     *context;
    so_module_node *next;
    so_module_node *prev;
};

so_context *so_linker_get_context();
int         so_linker_register_module(so_module *mod);
int         so_linker_unregister_module(so_module *mod);

so_context *so_context_create();
void        so_context_destroy(so_context *context);
so_module  *so_context_find_module(so_context *context, const char *soname);
int         so_context_add_module(so_context *context, so_module *mod);
int         so_context_remove_module(so_context *context, so_module *mod);

#ifdef __cplusplus
}
#endif

#endif