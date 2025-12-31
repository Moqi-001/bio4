#ifndef SOLOADED_LD_H
#define SOLOADED_LD_H

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>

#include <so_util/elf.h>
#include <so_util/so_util.h>

typedef struct
{
    const char *dli_fname;  // 共享库文件路径
    void       *dli_fbase;  // 共享库基地址
    const char *dli_sname;  // 符号名称
    void       *dli_saddr;  // 符号实际地址
} Dl_info;

// 错误码定义
#define LD_ERROR_MEMORY_ALLOC_FAILED 0xE030001
#define LD_ERROR_OPEN_FILE_FAILED    0xE030700
#define LD_ERROR_EXPAND_LIB_FAILED   0xE030400
#define LD_ERROR                     0xE03FE00

typedef enum {
    LD_MODULE_STATUS_NONE          = 0,
    LD_MODULE_STATUS_RELOCATED     = 1,
    LD_MODULE_STATUS_INITIALIZED   = 2,
    LD_MODULE_STATUS_DEINITIALIZED = 3
} LdModuleStatus;

typedef struct LdContext     LdContext;
typedef struct LdModule      LdModule;
typedef struct LdModuleEntry LdModuleEntry;

typedef struct IoVtable {
    FILE *(*fopen)(const char *filename, const char *mode);               // +0
    size_t (*fread)(void *ptr, size_t size, size_t nmemb, FILE *stream);  // +4
    long (*ftell)(FILE *stream);                                          // +8
    int (*fseek)(FILE *stream, long int offset, int whence);              // +12
    int (*fclose)(FILE *stream);                                          // +16
} IoVtable;

// 未确认，先占位
typedef struct DlVtable {
    void (*dlopen)(void *a1);                                                // +0
    char *(*dlerror)(void);                                                  // +4
    int (*dlclose)(void *handle);                                            // +8
    void *(*dlsym)(void *handle, const char *symbol);                        // +12
    void *(*dlvsym)(void *handle, const char *symbol, const char *version);  // +16
    int (*dladdr)(const void *addr, Dl_info *context);                       // +20
} DlVtable;

struct LdModuleEntry {
    LdModule      *mod;
    LdModuleEntry *next;
};

// 加载器上下文
struct LdContext {
    LdModuleEntry   *loaded_mods;        // +0   已加载的模块链表（头部）
    LdModuleEntry  **tail_next_mod_ref;  // +4   已加载的模块链表尾部的next引用
    LdModuleEntry   *pending_mods;       // +8   未加载的模块链表（头部）
    pthread_mutex_t *mutex;              // +12  线程互斥锁
    DlVtable         dl_vtable;          // +16  6个dl虚函数表 （程序内传入了0）
    IoVtable         io_vtable;          // +40  5个io虚函数表
    int32_t          exit_state;         // +60  退出状态（值未知，程序内未见调用）
    char             error_msg[256];     // +64  错误信息
    int32_t          error_code;         // +320 错误代码
};  // +324

struct LdModule {
    uint32_t       ref_count;     // +0   引用计数器
    char          *mod_name;      // +4   动态库文件名
    char          *mod_path;      // +8   动态库文件路径
    uint32_t       flags;         // +12  ？初始化值为0xFFFFEFFF，添加到context后|0x1000，最终的值应为0xFFFFFFFF
    uint32_t       status;        // +16  模块状态 (是否已初始化等)
    LdContext     *context;       // +20  加载器上下文反引用
    LdModule      *parent_mod;    // +24  ？似乎是父module
    uint32_t       needed_num;    // +28  依赖库数量
    LdModule     **needed_mods;   // +32  依赖库模块列表
    uint32_t       unk_36;        // +36
    void          *load_base;     // +40  so库内存块数据指针
    uint32_t       load_size;     // +44  so库内存块数据大小
    uint32_t       load_offset;   // +48
    void          *tls;           // +52
    uint32_t       tls_filesz;    // +56
    uint32_t       tls_memsz;     // +60
    uint32_t       tls_map;       // +64
    Elf32_Ehdr    *ehdr;          // +68
    uint32_t      *hashtab;       // +72  symbol的哈希表
    char          *strtab;        // +76  symbol的字符串表
    Elf32_Sym     *symtab;        // +80  symbol的函数地址表
    Elf32_Phdr    *phdr;          // +84
    Elf32_Phdr    *dynamic_phdr;  // +88
    Elf32_Verdef  *verdef;        // +92
    uint32_t       verdefnum;     // +96
    Elf32_Verneed *verneed;       // +100
    uint32_t       verneednum;    // +104
    Elf32_Versym  *versym;        // +108
    uint32_t       versymnum;     // +112
};  // +116

typedef struct LdBindingCreatext_offsettor {
    uint32_t     dynsym_capacity;  // +0   动态符号总容量
    uint32_t     dynsym_num;       // +4   动态符号当前个数
    uint32_t     dynstr_size;      // +8   动态字符串表大小
    const char **dynstr_array;     // +12  动态字符串指针数组
    void       **dynsym_array;     // +16  动态符号指针数组
    uint32_t     reserved[2];      // +20  (未见使用，应是预留)
} LdBindingCreator;                // +28

const char *ldGetErrorMessage(LdContext *context);
void       *ldModuleGetEntryPoint(LdModule *mod);

int  ldModuleRelocate(LdModule *mod);
int  ldModuleAllocAndLoadImage(LdModule *mod, const char *path, const char *err_msg);
void ldModuleDestroyImage(LdModule *mod);
int  ldModuleInitialize(LdModule *mod);
int  ldModuleDeinitialize(LdModule *mod);

extern uintptr_t (*ldModuleGetSymbol)(LdModule *in_mod, const char *sym_name, uint32_t sym_hash, int type,
                                      const char *vna_name, uint32_t vna_hash, LdModule **out_mod);
extern uintptr_t (*ldModuleLookupSymbol)(LdModule *mod, const char *sym_name, uint32_t sym_hash, int visibility,
                                         const char *vna_name, uint32_t vna_hash);

so_hook __ldModuleHookAddr(LdModule *mod, uintptr_t vaddr, uintptr_t dst);
so_hook __ldModuleHookSymbol(LdModule *mod, const char *symbol, uintptr_t dst);

#endif