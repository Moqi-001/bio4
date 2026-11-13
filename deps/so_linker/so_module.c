#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <psp2/io/fcntl.h>
#include <so_common/so_logger.h>

#include "so_internal.h"
#include "so_linker.h"

#define SO_SYM_LOOKUP_SELF  0x1
#define SO_SYM_LOOKUP_OTHER 0x2
#define SO_SYM_LOOKUP_ALL   (SO_SYM_LOOKUP_SELF | SO_SYM_LOOKUP_OTHER)

uint32_t so_sysv_hash(const char *name) {
    if (!name) {
        return 0;
    }

    uint32_t       h = 0, g;
    const uint8_t *p = (const uint8_t *)name;
    while (*p) {
        h = (h << 4) + *p++;
        if ((g = (h & 0xf0000000)) != 0) {
            h ^= g >> 24;
        }
        h &= 0x0fffffff;
    }
    return h;
}

uint32_t so_gnu_hash(const char *name) {
    if (!name) {
        return 0;
    }

    uint32_t h = 5381;
    while (*name) {
        h = (h << 5) + h + (uint8_t)*name++;
    }
    return h;
}

static void *_find_export_symbol_sysv(so_module *mod, const char *sym_name, const Elf32_Sym **out_sym) {
    if (!mod || !mod->hash || !mod->dynsym || !mod->dynstr || !sym_name) {
        return NULL;
    }

    uint32_t  sym_hash = so_sysv_hash(sym_name);
    uint32_t  nbucket  = mod->hash[0];
    uint32_t  nchain   = mod->hash[1];
    uint32_t *buckets  = mod->hash + 2;
    uint32_t *chains   = buckets + nbucket;

    for (uint32_t i = buckets[sym_hash % nbucket]; i != 0 && i < nchain; i = chains[i]) {
        const Elf32_Sym *sym = &mod->dynsym[i];
        if (sym->st_shndx != SHN_UNDEF && strcmp(mod->dynstr + sym->st_name, sym_name) == 0) {
            if (out_sym) {
                *out_sym = sym;
            }
            if (sym->st_shndx == SHN_ABS) {
                return (void *)sym->st_value;
            }
            return (void *)(sym->st_value + mod->load_bias);
        }
    }

    return NULL;
}

static void *_find_export_symbol_gnu(so_module *mod, const char *sym_name, const Elf32_Sym **out_sym) {
    if (!mod || !mod->gnu_hash || !mod->dynsym || !mod->dynstr || !sym_name) {
        return NULL;
    }

    uint32_t  sym_hash    = so_gnu_hash(sym_name);
    uint32_t *gh          = mod->gnu_hash;
    uint32_t  nbucket     = gh[0];
    uint32_t  symoffset   = gh[1];
    uint32_t  bloom_size  = gh[2];
    uint32_t  bloom_shift = gh[3];
    uint32_t *bloom       = gh + 4;
    uint32_t *buckets     = bloom + bloom_size;
    uint32_t *chains      = buckets + nbucket;

    uint32_t bloom_word = bloom[(sym_hash / 32) % bloom_size];
    uint32_t bloom_mask = (1u << (sym_hash % 32)) | (1u << ((sym_hash >> bloom_shift) % 32));
    if ((bloom_word & bloom_mask) != bloom_mask) {
        return NULL;
    }

    uint32_t i = buckets[sym_hash % nbucket];
    if (i < symoffset) {
        return NULL;
    }

    for (; i < mod->num_dynsym; i++) {
        uint32_t chain = chains[i - symoffset];
        if ((chain | 1) == (sym_hash | 1)) {
            const Elf32_Sym *sym = &mod->dynsym[i];
            if (sym->st_shndx != SHN_UNDEF && strcmp(mod->dynstr + sym->st_name, sym_name) == 0) {
                if (out_sym) {
                    *out_sym = sym;
                }
                if (sym->st_shndx == SHN_ABS) {
                    return (void *)sym->st_value;
                }
                return (void *)(sym->st_value + mod->load_bias);
            }
        }
        if (chain & 1) {
            break;
        }
    }

    return NULL;
}

static void *_find_export_symbol_linear(so_module *mod, const char *sym_name, const Elf32_Sym **out_sym) {
    if (!mod || !mod->dynsym || !mod->dynstr || !mod->num_dynsym || !sym_name) {
        return NULL;
    }

    // 逐个比对 dynsym（索引 0 固定为 STN_UNDEF）
    for (size_t i = 1; i < mod->num_dynsym; i++) {
        const Elf32_Sym *sym = &mod->dynsym[i];
        if (sym->st_shndx != SHN_UNDEF && strcmp(mod->dynstr + sym->st_name, sym_name) == 0) {
            if (out_sym) {
                *out_sym = sym;
            }
            if (sym->st_shndx == SHN_ABS) {
                return (void *)sym->st_value;
            }
            return (void *)(sym->st_value + mod->load_bias);
        }
    }

    return NULL;
}

// 按目标模块自己的哈希表选择查找方式：导入方与依赖方的哈希风格可能不同
static void *_find_export_symbol(so_module *mod, const char *sym_name, const Elf32_Sym **out_sym) {
    if (!mod) {
        return NULL;
    }
    if (mod->hash) {
        return _find_export_symbol_sysv(mod, sym_name, out_sym);
    }
    if (mod->gnu_hash) {
        return _find_export_symbol_gnu(mod, sym_name, out_sym);
    }

    // 两种哈希都没有：退回线性遍历（模块加载时已告警）
    return _find_export_symbol_linear(mod, sym_name, out_sym);
}

static void *_find_export_symbol_ex(so_module *mod, const char *sym_name, int flags,
                                    so_module **out_mod, const Elf32_Sym **out_sym) {
    if (!mod || !sym_name) {
        return NULL;
    }

    void *sym_addr = NULL;

    // 查找自身
    if (flags & SO_SYM_LOOKUP_SELF) {
        sym_addr = _find_export_symbol(mod, sym_name, out_sym);
        if (sym_addr) {
            if (out_mod) {
                *out_mod = mod;
            }
            return sym_addr;
        }
    }

    // 查找外部
    if (flags & SO_SYM_LOOKUP_OTHER) {
        so_context *context = so_linker_get_context();
        if (!context) {
            return NULL;
        }

        // 先按 DT_NEEDED 的声明顺序解析，与 Android 链接器的全局作用域顺序一致
        for (size_t i = 0; i < mod->num_needed; i++) {
            so_module *dep = so_linker_find_module(mod->needed[i]);
            if (!dep || dep == mod) {
                continue;
            }

            sym_addr = _find_export_symbol(dep, sym_name, out_sym);
            if (sym_addr) {
                if (out_mod) {
                    *out_mod = dep;
                }
                return sym_addr;
            }
        }

        // 依赖模块里没有命中时，退回模块注册顺序
        so_module_node *node = context->modules_head;
        while (node) {
            if (node->mod != mod) {
                sym_addr = _find_export_symbol(node->mod, sym_name, out_sym);
                if (sym_addr) {
                    if (out_mod) {
                        *out_mod = node->mod;
                    }
                    return sym_addr;
                }
            }
            node = node->next;
        }
    }

    return NULL;
}

static void *_find_import_slot(so_module *mod, const char *sym_name) {
    if (!mod || !sym_name) {
        return NULL;
    }

    if (mod->rel_plt) {
        for (uint32_t i = 0; i < mod->num_rel_plt; i++) {
            Elf32_Rel *rel     = &mod->rel_plt[i];
            uint32_t   sym_idx = ELF32_R_SYM(rel->r_info);
            if (sym_idx == 0) {
                continue;
            }

            Elf32_Sym *sym = &mod->dynsym[sym_idx];
            if (sym->st_shndx == SHN_UNDEF && strcmp(mod->dynstr + sym->st_name, sym_name) == 0) {
                return (void *)(mod->load_bias + rel->r_offset);
            }
        }
    } else if (mod->rela_plt) {
        for (uint32_t i = 0; i < mod->num_rela_plt; i++) {
            Elf32_Rela *rela    = &mod->rela_plt[i];
            uint32_t    sym_idx = ELF32_R_SYM(rela->r_info);
            if (sym_idx == 0) {
                continue;
            }

            Elf32_Sym *sym = &mod->dynsym[sym_idx];
            if (sym->st_shndx == SHN_UNDEF && strcmp(mod->dynstr + sym->st_name, sym_name) == 0) {
                return (void *)(mod->load_bias + rela->r_offset);
            }
        }
    }

    return NULL;
}

static int _create_symtab_strtab(so_symbol *symbols, size_t num_symbols,
                                 Elf32_Sym **out_symtab, size_t *out_num_symtab,
                                 char **out_strtab) {
    if (!symbols || num_symbols == 0) {
        return -1;
    }

    // 分配符号表（索引 0 预留为 STN_UNDEF）
    size_t     num_symtab = num_symbols + 1;
    Elf32_Sym *symtab     = calloc(num_symtab, sizeof(Elf32_Sym));
    if (!symtab) {
        return -1;
    }

    // 计算字符串表总大小（包含索引 0 的空字符串）
    size_t strtab_size = 1;
    for (size_t i = 0; i < num_symbols; i++) {
        if (!symbols[i].name) {
            free(symtab);
            return -1;
        }
        strtab_size += strlen(symbols[i].name) + 1;
    }

    // 分配字符串表
    char *strtab = calloc(1, strtab_size);
    if (!strtab) {
        free(symtab);
        return -1;
    }

    char *p = strtab + 1;  // 跳过索引 0（空字符串），符号名从偏移 1 开始
    for (size_t i = 0; i < num_symbols; i++) {
        size_t sym_idx = i + 1;  // 符号表索引从 1 开始

        symtab[sym_idx].st_name  = (Elf32_Word)(p - strtab);     // 符号名称在字符串表中的偏移
        symtab[sym_idx].st_value = (Elf32_Addr)symbols[i].addr;  // 符号地址
        symtab[sym_idx].st_size  = (Elf32_Word)symbols[i].size;  // 符号大小
        symtab[sym_idx].st_info  = symbols[i].info;              // 符号信息
        symtab[sym_idx].st_other = STV_DEFAULT;                  // 默认可见性
        symtab[sym_idx].st_shndx = SHN_ABS;                      // 绝对地址

        const char *src = symbols[i].name;
        while ((*p++ = *src++));
    }

    *out_symtab     = symtab;
    *out_num_symtab = num_symtab;
    *out_strtab     = strtab;
    return 0;
}

static uint32_t *_create_hashtab(const Elf32_Sym *symtab, size_t num_symtab, const char *strtab) {
    size_t    nbucket      = num_symtab - 1;                             // 哈希桶数量 = 实际符号数量（索引 0 是 STN_UNDEF，不计入）
    size_t    nchain       = num_symtab;                                 // 哈希链数量 = 符号表总项数（包括索引 0）
    size_t    hashtab_size = (2 + nbucket + nchain) * sizeof(uint32_t);  // 哈希表大小 = 头部(2个uint32_t) + 桶数组 + 链数组
    uint32_t *hashtab      = (uint32_t *)calloc(1, hashtab_size);
    if (!hashtab) {
        return NULL;
    }

    // 设置哈希表头部
    hashtab[0] = nbucket;  // 桶数量
    hashtab[1] = nchain;   // 链数量

    uint32_t *buckets = hashtab + 2;        // 桶数组
    uint32_t *chains  = buckets + nbucket;  // 链数组

    // 遍历符号表，从索引 1 开始（跳过索引 0 的 STN_UNDEF）
    for (size_t i = 1; i < num_symtab; i++) {
        uint32_t hash       = so_sysv_hash(strtab + symtab[i].st_name);
        uint32_t bucket_idx = hash % nbucket;

        // 头插法
        chains[i]           = buckets[bucket_idx];
        buckets[bucket_idx] = i;
    }

    return hashtab;
}

so_module *so_module_create(const so_library *library) {
    if (!library) {
        return NULL;
    }

    so_module *mod = calloc(1, sizeof(so_module));
    if (!mod) {
        so_logd("[SOLINKER] so_module_create(%p): failed to alloc %u memory for so_module obj!", library, sizeof(so_module));
        return NULL;
    }

    if (library->prepare && library->prepare((so_library *)library) < 0) {
        so_loge("[SOLINKER] so_module_create(%p): failed to prepare! ('%s')", library, library->soname);
        free(mod);
        return NULL;
    }

    if (_create_symtab_strtab(library->symbols, library->num_symbols, &mod->dynsym, &mod->num_dynsym, &mod->dynstr) < 0) {
        so_loge("[SOLINKER] so_module_create(%p): failed to create dynsym and dynstr! ('%s')", library, library->soname);
        free(mod);
        return NULL;
    }

    mod->hash = _create_hashtab(mod->dynsym, mod->num_dynsym, mod->dynstr);
    if (!mod->hash) {
        so_loge("[SOLINKER] so_module_create(%p): failed to create hashtab! ('%s')", library, library->soname);
        free(mod->dynsym);
        free(mod->dynstr);
        free(mod);
        return NULL;
    }

    if (library->soname) {
        mod->soname = strdup(library->soname);
    }

    mod->status = SO_MODULE_STATUS_INITIALIZED;

    so_linker_register_module(mod);

    so_logd("[SOLINKER] so_module_create(%p): success! ('%s')", library, mod->soname);
    return mod;
}

so_module *so_module_load_mem(void *buffer, size_t size) {
    if (!buffer || size == 0) {
        return NULL;
    }

    // 检查 ELF 魔数（0x7F 'E' 'L' 'F'）
    if (memcmp(buffer, ELFMAG, SELFMAG) != 0) {
        return NULL;
    }

    // ELF 文件头
    Elf32_Ehdr *ehdr = (Elf32_Ehdr *)buffer;
    // 检查 ELF 类别（32位）
    if (ehdr->e_ident[EI_CLASS] != ELFCLASS32) {
        return NULL;
    }
    // 检查目标架构（ARM）
    if (ehdr->e_machine != EM_ARM) {
        return NULL;
    }
    // 检查文件类型（共享库）
    if (ehdr->e_type != ET_DYN) {
        return NULL;
    }

    so_module *mod = calloc(1, sizeof(so_module));
    if (!mod) {
        so_logd("[SOLINKER] so_module_load_mem(%p,%u): failed to alloc %u memory for so_module obj!", buffer, size, sizeof(so_module));
        return NULL;
    }

    // 程序头（段表）
    Elf32_Phdr *phdr = (Elf32_Phdr *)((uintptr_t)buffer + ehdr->e_phoff);

    // 计算内存大小
    size_t load_start  = 0;
    size_t load_end    = 0;
    int    found_start = 0;
    for (size_t i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD) {
            if (!found_start) {
                load_start  = phdr[i].p_vaddr;
                found_start = 1;
            }
            if (load_end < phdr[i].p_vaddr + phdr[i].p_memsz) {
                load_end = phdr[i].p_vaddr + phdr[i].p_memsz;
            }
        }
    }

    // 分配内存
    size_t load_size = load_end - load_start;
    mod->load_size   = ALIGN_UP(load_size, PAGE_SIZE);
    mod->load_base   = so_mmap(mod->load_size);
    if (!mod->load_base) {
        so_logd("[SOLINKER] so_module_load_mem(%p,%u): failed to alloc %u memory for PT_LOAD!", buffer, size, mod->load_size);
        free(mod);
        return NULL;
    }
    mod->load_bias = (intptr_t)mod->load_base - (intptr_t)load_start;

    // 解析程序头（段表）
    for (size_t i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD) {
            void *dst = (void *)(phdr[i].p_vaddr + mod->load_bias);
            void *src = (void *)((uintptr_t)buffer + phdr[i].p_offset);
            memcpy(dst, src, phdr[i].p_filesz);
            if (phdr[i].p_memsz > phdr[i].p_filesz) {
                memset((void *)((uintptr_t)dst + phdr[i].p_filesz), 0, phdr[i].p_memsz - phdr[i].p_filesz);
            }
        } else if (phdr[i].p_type == PT_DYNAMIC) {
            mod->dynamic     = (Elf32_Dyn *)(phdr[i].p_vaddr + mod->load_bias);
            mod->num_dynamic = phdr[i].p_memsz / sizeof(Elf32_Dyn);
        }
    }

    if (!mod->dynamic) {
        so_loge("[SOLINKER] so_module_load_mem(%p,%u): failed to get dynamic!", buffer, size);
        so_munmap(mod->load_base);
        free(mod);
        return NULL;
    }

    // 保存 ELF 入口点
    if (ehdr->e_entry) {
        mod->entry_point = (void *)(ehdr->e_entry + mod->load_bias);
    }

    // 解析 DYNAMIC 段
    uint32_t soname_offset = 0;
    uint32_t plt_type      = DT_NULL;
    void    *plt_tab       = NULL;
    size_t   plt_size      = 0;

    for (size_t i = 0; i < mod->num_dynamic; i++) {
        Elf32_Dyn *dyn = &mod->dynamic[i];
        switch (dyn->d_tag) {
            case DT_SONAME:
                soname_offset = dyn->d_un.d_val;
                break;
            case DT_NEEDED:
                mod->num_needed++;
                break;
            case DT_SYMTAB:
                mod->dynsym = (Elf32_Sym *)(dyn->d_un.d_ptr + mod->load_bias);
                break;
            case DT_STRTAB:
                mod->dynstr = (char *)(dyn->d_un.d_ptr + mod->load_bias);
                break;
            case DT_STRSZ:
                mod->dynstr_size = dyn->d_un.d_val;
                break;
            case DT_HASH:
                mod->hash       = (uint32_t *)(dyn->d_un.d_ptr + mod->load_bias);
                mod->num_dynsym = mod->hash[1];
                break;
            case DT_GNU_HASH:
                mod->gnu_hash = (uint32_t *)(dyn->d_un.d_ptr + mod->load_bias);
                break;
            case DT_INIT:
                mod->init = (void (*)(void))(dyn->d_un.d_ptr + mod->load_bias);
                break;
            case DT_INIT_ARRAY:
                mod->init_array = (void (**)(void))(dyn->d_un.d_ptr + mod->load_bias);
                break;
            case DT_INIT_ARRAYSZ:
                mod->num_init_array = (dyn->d_un.d_val / sizeof(Elf32_Addr));
                break;
            case DT_FINI:
                mod->fini = (void (*)(void))(dyn->d_un.d_ptr + mod->load_bias);
                break;
            case DT_FINI_ARRAY:
                mod->fini_array = (void (**)(void))(dyn->d_un.d_ptr + mod->load_bias);
                break;
            case DT_FINI_ARRAYSZ:
                mod->num_fini_array = (dyn->d_un.d_val / sizeof(Elf32_Addr));
                break;
            case DT_REL:
                mod->rel_dyn = (Elf32_Rel *)(dyn->d_un.d_ptr + mod->load_bias);
                break;
            case DT_RELSZ:
                mod->num_rel_dyn = (dyn->d_un.d_val / sizeof(Elf32_Rel));
                break;
            case DT_RELA:
                mod->rela_dyn = (Elf32_Rela *)(dyn->d_un.d_ptr + mod->load_bias);
                break;
            case DT_RELR:
                mod->relr_dyn = (Elf32_Addr *)(dyn->d_un.d_ptr + mod->load_bias);
                break;
            case DT_RELRSZ:
                mod->num_relr = dyn->d_un.d_val / sizeof(Elf32_Addr);
                break;
            case DT_RELASZ:
                mod->num_rela_dyn = (dyn->d_un.d_val / sizeof(Elf32_Rela));
                break;
            case DT_PLTREL:
                plt_type = dyn->d_un.d_val;
                break;
            case DT_JMPREL:
                plt_tab = (void *)(dyn->d_un.d_ptr + mod->load_bias);
                break;
            case DT_PLTRELSZ:
                plt_size = dyn->d_un.d_val;
                break;
        }
    }

    // 检查必需的动态段条目
    if (!mod->dynsym || !mod->dynstr || (!mod->rel_dyn && !mod->rela_dyn && !mod->relr_dyn)) {
        so_loge("[SOLINKER] so_module_load_mem(%p,%u): error dynamic!", buffer, size);
        so_munmap(mod->load_base);
        free(mod);
        return NULL;
    }

    // 保存共享对象名称
    mod->soname = strdup(mod->dynstr + soname_offset);

    // 既无 SysV 也无 GNU 哈希：符号查找将退回线性遍历
    if (!mod->hash && !mod->gnu_hash) {
        so_logw("[SOLINKER] so_module_load_mem: '%s' has no hash table, falling back to linear symbol lookup!", mod->soname);
    }

    // GNU 哈希不携带符号数量，用 symtab 到 strtab 的距离推导
    if (!mod->num_dynsym) {
        mod->num_dynsym = (size_t)(mod->dynstr - (char *)mod->dynsym) / sizeof(Elf32_Sym);
    }

    // 收集依赖模块，保留 DT_NEEDED 的声明顺序
    if (mod->num_needed) {
        mod->needed = calloc(mod->num_needed, sizeof(char *));
        if (!mod->needed) {
            so_loge("[SOLINKER] so_module_load_mem(%p,%u): failed to alloc needed array!", buffer, size);
            so_munmap(mod->load_base);
            free(mod->soname);
            free(mod);
            return NULL;
        }

        size_t needed_idx = 0;
        for (size_t i = 0; i < mod->num_dynamic; i++) {
            if (mod->dynamic[i].d_tag == DT_NEEDED) {
                mod->needed[needed_idx++] = mod->dynstr + mod->dynamic[i].d_un.d_val;
            }
        }
    }

    // 保存 PLT 重定位表
    if (plt_type == DT_REL) {
        mod->rel_plt     = plt_tab;
        mod->num_rel_plt = plt_size / sizeof(Elf32_Rel);
    } else if (plt_type == DT_RELA) {
        mod->rela_plt     = plt_tab;
        mod->num_rela_plt = plt_size / sizeof(Elf32_Rela);
    }

    so_logd("[SOLINKER] so_module_load_mem(%p,%u): success! ('%s')", buffer, size, mod->soname);
    return mod;
}

so_module *so_module_load(const char *filename) {
    if (!filename) {
        return NULL;
    }

    SceUID fd = sceIoOpen(filename, SCE_O_RDONLY, 0);
    if (fd < 0) {
        so_loge("[SOLINKER] so_module_load('%s'): failed to open file!", filename);
        return NULL;
    }

    int64_t size = sceIoLseek(fd, 0, SCE_SEEK_END);
    if (size <= 0) {
        so_loge("[SOLINKER] so_module_load('%s'): error file size!", filename);
        sceIoClose(fd);
        return NULL;
    }
    sceIoLseek(fd, 0, SCE_SEEK_SET);

    void *buffer = malloc(size);
    if (!buffer) {
        so_loge("[SOLINKER] so_module_load('%s'): failed to alloc memory for read!", filename);
        sceIoClose(fd);
        return NULL;
    }

    sceIoRead(fd, buffer, size);
    sceIoClose(fd);

    so_module *mod = so_module_load_mem(buffer, size);
    free(buffer);

    if (!mod) {
        so_loge("[SOLINKER] so_module_load('%s'): failed!", filename);
        return NULL;
    }

    so_logd("[SOLINKER] so_module_load('%s'): success! ('%s')", filename, mod->soname);
    return mod;
}

void so_module_unload(so_module *mod) {
    if (mod) {
        so_logd("[SOLINKER] so_module_unload(%p): ('%s')", mod, mod->soname);

        if (mod->soname) {
            free(mod->soname);
        }
        if (mod->needed) {
            free(mod->needed);  // 字符串指向 dynstr，无需单独释放
        }
        if (mod->load_base) {
            so_munmap(mod->load_base);
        } else {
            // 从 so_library 创建的模块需要释放一些资源
            if (mod->dynsym) {
                free(mod->dynsym);
            }
            if (mod->dynstr) {
                free(mod->dynstr);
            }
            if (mod->hash) {
                free(mod->hash);
            }
        }
        free(mod);
    }
}

static int _process_relocation(so_module *mod, uint8_t rel_type, uint32_t rel_symidx, uint32_t *rel_target, uint32_t rel_addend) {
    const Elf32_Sym *rel_sym  = NULL;
    const Elf32_Sym *sym      = NULL;
    so_module       *sym_mod  = NULL;
    const char      *sym_name = NULL;
    uintptr_t        sym_addr = 0;
    uint32_t         sym_size = 0;

    // ===== 查找符号 =====
    if (rel_symidx) {
        rel_sym  = &mod->dynsym[rel_symidx];
        sym_name = mod->dynstr + rel_sym->st_name;
        sym_addr = (uintptr_t)_find_export_symbol_ex(mod, sym_name, SO_SYM_LOOKUP_ALL, &sym_mod, &sym);

        if (!sym_addr) {
            if (ELF32_ST_BIND(rel_sym->st_info) != STB_WEAK || rel_type == R_ARM_COPY) {
                so_loge("[SOLINKER] process_relocation: symbol '%s' not found", sym_name);
                return -1;
            }

            so_logw("[SOLINKER] process_relocation: weak symbol '%s' not found, skipping", sym_name);
            return 0;
        }
    }

    // ===== 执行重定位 =====
    uint32_t rel_val;

    switch (rel_type) {
        case R_ARM_NONE:  // 0
            break;

        case R_ARM_ABS32:  // 2 (S + A)
            rel_val = sym_addr + rel_addend;
            memcpy(rel_target, &rel_val, sizeof(uint32_t));
            // so_logd("  R_ARM_ABS32: %s -> 0x%x", sym_name, *rel_target);
            break;

        case R_ARM_REL32:  // 3 (S + A - P)
            rel_val = sym_addr + rel_addend - (uintptr_t)rel_target;
            memcpy(rel_target, &rel_val, sizeof(uint32_t));
            // so_logd("  R_ARM_REL32: %s -> 0x%x", sym_name, *rel_target);
            break;

        case R_ARM_COPY:  // 20 (S -> T)  (似乎只有FLAG_EXE才有这种类型?)
            if ((uintptr_t)rel_target != sym_addr) {
                so_loge("  R_ARM_COPY: %s relocation target cannot be resolved!", sym_name);
                return -1;
            }
            sym_addr = (uintptr_t)_find_export_symbol_ex(mod, sym_name, SO_SYM_LOOKUP_OTHER, &sym_mod, &sym);
            if (!sym_addr || rel_sym->st_size < sym->st_size) {
                so_loge("  R_ARM_COPY: %s relocation target cannot be resolved!", sym_name);
                return -1;
            }
            memcpy(rel_target, (void *)sym_addr, sym->st_size);
            // so_logd("  R_ARM_COPY: %s, 0x%x -> %p, size=%u", sym_name, sym_addr, rel_target, sym->st_size);
            break;

        case R_ARM_GLOB_DAT:   // 21 (S)
        case R_ARM_JUMP_SLOT:  // 22 (S)
            rel_val = sym_addr;
            memcpy(rel_target, &rel_val, sizeof(uint32_t));
            // so_logd("  %s: %s -> 0x%x", rel_type == R_ARM_GLOB_DAT ? "R_ARM_GLOB_DAT" : "R_ARM_JUMP_SLOT", sym_name, *rel_target);
            break;

        case R_ARM_RELATIVE:  // 23 (B + A)
            rel_val = mod->load_bias + rel_addend;
            memcpy(rel_target, &rel_val, sizeof(uint32_t));
            // so_logd("  R_ARM_RELATIVE: %s -> 0x%x", sym_name, *rel_target);
            break;

        default:
            so_loge("[SOLINKER] process_relocation: unsupported type %d for symbol '%s'", rel_type, sym_name ? sym_name : "(null)");
            return -1;
    }

    return 0;
}

static int _relocate_rel_table(so_module *mod, Elf32_Rel *rel_tab, uint32_t rel_num) {
    int result = 0;

    for (uint32_t i = 0; i < rel_num; i++) {
        uint8_t   rel_type   = ELF32_R_TYPE(rel_tab[i].r_info);
        uint32_t  rel_symidx = ELF32_R_SYM(rel_tab[i].r_info);
        uint32_t *rel_target = (uint32_t *)(rel_tab[i].r_offset + mod->load_bias);
        uint32_t  rel_addend = *rel_target;

        if (_process_relocation(mod, rel_type, rel_symidx, rel_target, rel_addend) < 0) {
            result = -1;
            continue;
        }
    }

    return result;
}

static int _relocate_rela_table(so_module *mod, Elf32_Rela *rela_tab, uint32_t rela_num) {
    int result = 0;

    for (size_t i = 0; i < rela_num; i++) {
        uint8_t   rel_type   = ELF32_R_TYPE(rela_tab[i].r_info);
        uint32_t  rel_symidx = ELF32_R_SYM(rela_tab[i].r_info);
        uint32_t *rel_target = (uint32_t *)(rela_tab[i].r_offset + mod->load_bias);
        uint32_t  rel_addend = rela_tab[i].r_addend;

        if (_process_relocation(mod, rel_type, rel_symidx, rel_target, rel_addend) < 0) {
            result = -1;
            continue;
        }
    }

    return result;
}

static int _relocate_relr_table(so_module *mod, Elf32_Addr *relr_tab, size_t relr_num) {
    uint32_t *rel_target = NULL;

    // RELR 只承载 R_ARM_RELATIVE：目标位置里保存的就是加数，加上 load_bias 即可
    for (size_t i = 0; i < relr_num; i++) {
        Elf32_Addr entry = relr_tab[i];

        if ((entry & 1) == 0) {
            // 地址条目：定位到该地址，后续位图条目以它为基准
            rel_target = (uint32_t *)(entry + mod->load_bias);
            *rel_target++ += (uint32_t)mod->load_bias;
        } else if (rel_target) {
            // 位图条目：bit1..bit31 依次表示紧随其后的 31 个地址是否重定位
            for (int bit = 0; bit < 31; bit++) {
                if (entry & (2u << bit)) {
                    *rel_target += (uint32_t)mod->load_bias;
                }
                rel_target++;
            }
        }
    }

    return 0;
}

int so_module_relocate(so_module *mod) {
    if (!mod) {
        return -1;
    }

    if (!mod->load_base) {
        return 0;  // 从 so_library 创建的模块不需要重定位
    }

    if (mod->status >= SO_MODULE_STATUS_RELOCATED) {
        so_logd("[SOLINKER] so_module_relocate(%p): is already relocated. ('%s')", mod, mod->soname);
        return 0;
    }

    if (mod->rel_dyn && mod->num_rel_dyn > 0) {
        if (_relocate_rel_table(mod, mod->rel_dyn, mod->num_rel_dyn) < 0) {
            so_loge("[SOLINKER] so_module_relocate(%p): failed to process rel_dyn table! ('%s')", mod, mod->soname);
            return -1;
        }
    }
    if (mod->rela_dyn && mod->num_rela_dyn > 0) {
        if (_relocate_rela_table(mod, mod->rela_dyn, mod->num_rela_dyn) < 0) {
            so_loge("[SOLINKER] so_module_relocate(%p): failed to process rela_dyn table! ('%s')", mod, mod->soname);
            return -1;
        }
    }
    if (mod->relr_dyn && mod->num_relr > 0) {
        if (_relocate_relr_table(mod, mod->relr_dyn, mod->num_relr) < 0) {
            so_loge("[SOLINKER] so_module_relocate(%p): failed to process relr table! ('%s')", mod, mod->soname);
            return -1;
        }
    }

    if (mod->rel_plt && mod->num_rel_plt > 0) {
        if (_relocate_rel_table(mod, mod->rel_plt, mod->num_rel_plt) < 0) {
            so_loge("[SOLINKER] so_module_relocate(%p): failed to process rel_plt table! ('%s')", mod, mod->soname);
            return -1;
        }
    }
    if (mod->rela_plt && mod->num_rela_plt > 0) {
        if (_relocate_rela_table(mod, mod->rela_plt, mod->num_rela_plt) < 0) {
            so_loge("[SOLINKER] so_module_relocate(%p): failed to process rela_plt table! ('%s')", mod, mod->soname);
            return -1;
        }
    }

    mod->status = SO_MODULE_STATUS_RELOCATED;

    so_logd("[SOLINKER] so_module_relocate(%p): success! ('%s')", mod, mod->soname);
    return 0;
}

int so_module_init(so_module *mod) {
    if (!mod) {
        return -1;
    }

    if (!mod->load_base) {
        return 0;  // 从 so_library 创建的模块不需要初始化
    }

    if (mod->status == SO_MODULE_STATUS_INITIALIZED) {
        so_logd("[SOLINKER] so_module_init(%p): is already initialized. ('%s')", mod, mod->soname);
        return 0;
    }

    if (mod->status < SO_MODULE_STATUS_RELOCATED) {
        so_logd("[SOLINKER] so_module_init(%p): is not relocated. ('%s')", mod, mod->soname);
        return 0;
    }

    if (mod->init && (intptr_t)mod->init != -1) {
        mod->init();
    }

    if (mod->init_array) {
        for (uint32_t i = 0; i < mod->num_init_array; i++) {
            if (mod->init_array[i] && (intptr_t)mod->init_array[i] != -1) {
                mod->init_array[i]();
            }
        }
    }

    mod->status = SO_MODULE_STATUS_INITIALIZED;

    so_logd("[SOLINKER] so_module_init(%p): success! ('%s')", mod, mod->soname);
    return 0;
}

void so_module_fini(so_module *mod) {
    if (!mod) {
        return;
    }

    if (!mod->load_base) {
        return;  // 从 so_library 创建的模块不需要反初始化
    }

    if (mod->status == SO_MODULE_STATUS_FINALIZED) {
        so_logd("[SOLINKER] so_module_fini(%p): is already finalized. ('%s')", mod, mod->soname);
        return;
    }

    if (mod->status != SO_MODULE_STATUS_INITIALIZED) {
        so_logd("[SOLINKER] so_module_fini(%p): is not initialized. ('%s')", mod, mod->soname);
        return;
    }

    so_logd("[SOLINKER] so_module_fini(%p): ('%s')", mod, mod->soname);

    if (mod->fini && (intptr_t)mod->fini != -1) {
        mod->fini();
    }

    if (mod->fini_array) {
        for (uint32_t i = 0; i < mod->num_fini_array; i++) {
            if (mod->fini_array[i] && (intptr_t)mod->fini_array[i] != -1) {
                mod->fini_array[i]();
            }
        }
    }

    mod->status = SO_MODULE_STATUS_FINALIZED;
}

int so_module_flush_caches(so_module *mod) {
    if (!mod || !mod->load_base) {
        return -1;
    }

    if (so_cacheflush(mod->load_base, mod->load_size) < 0) {
        so_logd("[SOLINKER] so_module_flush_caches(%p): failed! ('%s')", mod, mod->soname);
        return -1;
    }

    so_logd("[SOLINKER] so_module_flush_caches(%p): success! ('%s')", mod, mod->soname);
    return 0;
}

so_module *so_module_open_mem(void *buffer, size_t size) {
    so_module *mod = so_module_load_mem(buffer, size);
    if (!mod) {
        so_loge("[SOLINKER] so_module_open_mem(%p,%u): failed to load!", buffer, size);
        return NULL;
    }

    if (so_module_relocate(mod) < 0) {
        so_loge("[SOLINKER] so_module_open_mem(%p,%u): failed to relocate!", buffer, size);
        so_module_unload(mod);
        return NULL;
    }

    if (so_module_flush_caches(mod) < 0) {
        so_loge("[SOLINKER] so_module_open_mem(%p,%u): failed to flush caches!", buffer, size);
        so_module_unload(mod);
        return NULL;
    }

    if (so_module_init(mod) < 0) {
        so_loge("[SOLINKER] so_module_open_mem(%p,%u): failed to initialize!", buffer, size);
        so_module_unload(mod);
        return NULL;
    }

    so_logd("[SOLINKER] so_module_open_mem(%p,%u): success! load_base=%p, load_size=%u, load_bias=0x%x, soname='%s'",
            buffer, size, mod->load_base, mod->load_size, mod->load_bias, mod->soname);
    return mod;
}

so_module *so_module_open(const char *filename) {
    so_module *mod = so_module_load(filename);
    if (!mod) {
        so_loge("[SOLINKER] so_module_open(%s): failed to load!", filename);
        return NULL;
    }

    if (so_module_relocate(mod) < 0) {
        so_loge("[SOLINKER] so_module_open(%s): failed to relocate!", filename);
        so_module_unload(mod);
        return NULL;
    }

    if (so_module_flush_caches(mod) < 0) {
        so_loge("[SOLINKER] so_module_open(%s): failed to flush caches!", filename);
        so_module_unload(mod);
        return NULL;
    }

    if (so_module_init(mod) < 0) {
        so_loge("[SOLINKER] so_module_open(%s): failed to initialize!", filename);
        so_module_unload(mod);
        return NULL;
    }

    so_linker_register_module(mod);

    so_logd("[SOLINKER] so_module_open('%s'): success! load_base=%p, load_size=%u, load_bias=0x%x, soname='%s'",
            filename, mod->load_base, mod->load_size, mod->load_bias, mod->soname ? mod->soname : "(null)");
    return mod;
}

void so_module_close(so_module *mod) {
    if (mod) {
        so_logd("[SOLINKER] so_module_close(%p): ('%s')", mod, mod->soname);
        so_linker_unregister_module(mod);
        mod->ref_count = 0;
        so_module_fini(mod);
        so_module_unload(mod);
    }
}

int so_module_retain(so_module *mod) {
    if (!mod) {
        return -1;
    }

    mod->ref_count++;
    return 0;
}

void so_module_release(so_module *mod) {
    if (!mod) {
        return;
    }

    if (mod->ref_count-- == 1) {
        so_module_close(mod);
    }
}

void *so_module_find_export(so_module *mod, const char *sym_name) {
    return _find_export_symbol_ex(mod, sym_name, SO_SYM_LOOKUP_SELF, NULL, NULL);
}

void *so_module_find_import(so_module *mod, const char *sym_name) {
    void *slot = _find_import_slot(mod, sym_name);
    return slot ? *(void **)slot : NULL;
}

int so_module_hook_address(so_module *mod, ptrdiff_t vaddr, void *repl, so_hook_ref *p_hook) {
    if (!mod || vaddr < 0 || !repl) {
        return -1;
    }

    void *target = (void *)(vaddr + mod->load_bias);
    return so_hook(target, repl, p_hook);
}

int so_module_hook_export(so_module *mod, const char *sym_name, void *repl, so_hook_ref *p_hook) {
    if (!mod || !sym_name || !repl) {
        return -1;
    }

    void *target = so_module_find_export(mod, sym_name);
    return so_hook(target, repl, p_hook);
}

int so_module_hook_import(so_module *mod, const char *sym_name, void *repl, so_hook_ref *p_hook) {
    if (!mod || !sym_name || !repl) {
        return -1;
    }

    void *slot = _find_import_slot(mod, sym_name);
    return so_hook_got(slot, repl, p_hook);
}

int so_module_inject_data(so_module *mod, ptrdiff_t vaddr, const void *repl, size_t len) {
    if (!mod || vaddr < 0 || !repl || len == 0) {
        return -1;
    }

    void *target = (void *)(vaddr + mod->load_bias);
    return so_inject_data(target, repl, len);
}

const char *so_module_get_name(so_module *mod) {
    return mod ? mod->soname : NULL;
}

void *so_module_get_entry_point(so_module *mod) {
    return mod ? mod->entry_point : NULL;
}

const void *so_module_get_load_base(so_module *mod) {
    return mod ? mod->load_base : NULL;
}

size_t so_module_get_load_size(so_module *mod) {
    return mod ? mod->load_size : 0;
}

intptr_t so_module_get_load_bias(so_module *mod) {
    return mod ? mod->load_bias : 0;
}
