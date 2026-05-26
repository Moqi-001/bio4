#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <psp2/io/fcntl.h>
#include <so_common/so_logger.h>

#include "so_internal.h"
#include "so_linker.h"

static uint32_t _sysv_hash(const char *name) {
    uint32_t       h = 0, g;
    const uint8_t *p = (const uint8_t *)name;
    while (*p) {
        h = (h << 4) + *p++;
        if ((g = (h & 0xf0000000)) != 0)
            h ^= g >> 24;
        h &= 0x0fffffff;
    }
    return h;
}

static void *_find_symbol_ex(so_module *mod, const char *sym_name, uint32_t sym_hash, const Elf32_Sym **out_sym) {
    if (!mod || !mod->hash || !mod->dynsym || !mod->dynstr || !sym_name) {
        return NULL;
    }

    const Elf32_Sym *sym     = NULL;
    uint32_t         nbucket = mod->hash[0];
    uint32_t         nchain  = mod->hash[1];
    uint32_t        *buckets = mod->hash + 2;
    uint32_t        *chains  = buckets + nbucket;

    for (uint32_t i = buckets[sym_hash % nbucket]; i != 0 && i < nchain; i = chains[i]) {
        if (mod->dynsym[i].st_shndx != SHN_UNDEF) {
            const char *name = mod->dynstr + mod->dynsym[i].st_name;
            if (strcmp(name, sym_name) == 0) {
                sym = &mod->dynsym[i];
                break;
            }
        }
    }

    if (!sym) {
        return NULL;
    }

    if (out_sym) {
        *out_sym = sym;
    }

    if (ELF32_ST_TYPE(sym->st_info) == STT_TLS) {  // TLS 符号：返回偏移量
        return (void *)sym->st_value;
    }
    if (sym->st_shndx == SHN_ABS) {
        return (void *)sym->st_value;
    }
    if (sym->st_shndx != SHN_UNDEF) {
        return (void *)((uintptr_t)mod->load_base + sym->st_value);
    }

    return NULL;
}

void *so_module_find_symbol_ex(so_module *mod, const char *sym_name, int local_only,
                               so_module **out_mod, const Elf32_Sym **out_sym) {
    if (!mod || !sym_name) {
        return NULL;
    }

    uint32_t sym_hash = _sysv_hash(sym_name);

    // 先查找本地
    void *sym_addr = _find_symbol_ex(mod, sym_name, sym_hash, out_sym);
    if (sym_addr) {
        if (out_mod) {
            *out_mod = mod;
        }
        return sym_addr;
    }

    if (local_only) {
        return NULL;
    }

    // 查找全局
    so_context *context = so_linker_get_context();
    if (!context) {
        return NULL;
    }

    so_module_node *node = context->modules;
    while (node) {
        if (node->mod != mod) {
            sym_addr = _find_symbol_ex(node->mod, sym_name, sym_hash, out_sym);
            if (sym_addr) {
                if (out_mod) {
                    *out_mod = node->mod;
                }
                return sym_addr;
            }
        }
        node = node->next;
    }

    return NULL;
}

void *so_module_find_symbol(so_module *mod, const char *sym_name) {
    return so_module_find_symbol_ex(mod, sym_name, 1, NULL, NULL);
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

static uint32_t *_jclass_create_hashtab(const Elf32_Sym *symtab, size_t num_symtab, const char *strtab) {
    size_t    nbucket      = num_symtab - 1;                             // 哈希桶数量 = 实际符号数量（索引 0 是 STN_UNDEF，不计入）
    size_t    nchain       = num_symtab;                                 // 哈希链数量 = 符号表总项数（包括索引 0）
    size_t    hashtab_size = (2 + nbucket + nchain) * sizeof(uint32_t);  // 哈希表大小 = 头部(2个uint32_t) + 桶数组 + 链数组
    uint32_t *hashtab      = (uint32_t *)calloc(1, hashtab_size);
    if (!hashtab)
        return NULL;

    // 设置哈希表头部
    hashtab[0] = nbucket;  // 桶数量
    hashtab[1] = nchain;   // 链数量

    uint32_t *buckets = hashtab + 2;        // 桶数组
    uint32_t *chains  = buckets + nbucket;  // 链数组

    // 遍历符号表，从索引 1 开始（跳过索引 0 的 STN_UNDEF）
    for (size_t i = 1; i < num_symtab; i++) {
        uint32_t hash       = _sysv_hash(strtab + symtab[i].st_name);
        uint32_t bucket_idx = hash % nbucket;

        // 头插法
        chains[i]           = buckets[bucket_idx];
        buckets[bucket_idx] = i;
    }

    return hashtab;
}

so_module *so_module_create(const so_library *library) {
    if (!library) return NULL;

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

    mod->hash = _jclass_create_hashtab(mod->dynsym, mod->num_dynsym, mod->dynstr);
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

    so_module_retain(mod);
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
    size_t load_size = 0;
    for (size_t i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD) {
            if (load_size < phdr[i].p_vaddr + phdr[i].p_memsz) {
                load_size = phdr[i].p_vaddr + phdr[i].p_memsz;
            }
        }
    }

    // 分配内存
    mod->load_size = ALIGN_UP(load_size, PAGE_SIZE);
    mod->load_base = so_mmap(mod->load_size);
    if (!mod->load_base) {
        so_logd("[SOLINKER] so_module_load_mem(%p,%u): failed to alloc %u memory for PT_LOAD!", buffer, size, mod->load_size);
        free(mod);
        return NULL;
    }

    // 解析程序头（段表）
    for (size_t i = 0; i < ehdr->e_phnum; i++) {
        if (phdr[i].p_type == PT_LOAD) {
            void *dst = (void *)((uintptr_t)mod->load_base + phdr[i].p_vaddr);
            void *src = (void *)((uintptr_t)buffer + phdr[i].p_offset);
            so_memcpy(dst, src, phdr[i].p_filesz);
            if (phdr[i].p_memsz > phdr[i].p_filesz) {
                so_memset((void *)((uintptr_t)dst + phdr[i].p_filesz), 0, phdr[i].p_memsz - phdr[i].p_filesz);
            }
        } else if (phdr[i].p_type == PT_DYNAMIC) {
            mod->dynamic     = (Elf32_Dyn *)((uintptr_t)mod->load_base + phdr[i].p_vaddr);
            mod->num_dynamic = phdr[i].p_memsz / sizeof(Elf32_Dyn);
        } else if (phdr[i].p_type == PT_TLS) {
            // 待实现
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
        mod->entry_point = (void *)((uintptr_t)mod->load_base + ehdr->e_entry);
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
            case DT_SYMTAB:
                mod->dynsym = (Elf32_Sym *)((uintptr_t)mod->load_base + dyn->d_un.d_ptr);
                break;
            case DT_STRTAB:
                mod->dynstr = (char *)((uintptr_t)mod->load_base + dyn->d_un.d_ptr);
                break;
            case DT_STRSZ:
                mod->dynstr_size = dyn->d_un.d_val;
                break;
            case DT_HASH:
                mod->hash       = (uint32_t *)((uintptr_t)mod->load_base + dyn->d_un.d_ptr);
                mod->num_dynsym = mod->hash[1];
                break;
            case DT_INIT:
                mod->init = (void (*)(void))((uintptr_t)mod->load_base + dyn->d_un.d_ptr);
                break;
            case DT_INIT_ARRAY:
                mod->init_array = (void (**)(void))((uintptr_t)mod->load_base + dyn->d_un.d_ptr);
                break;
            case DT_INIT_ARRAYSZ:
                mod->num_init_array = (dyn->d_un.d_val / sizeof(Elf32_Addr));
                break;
            case DT_FINI:
                mod->fini = (void (*)(void))((uintptr_t)mod->load_base + dyn->d_un.d_ptr);
                break;
            case DT_FINI_ARRAY:
                mod->fini_array = (void (**)(void))((uintptr_t)mod->load_base + dyn->d_un.d_ptr);
                break;
            case DT_FINI_ARRAYSZ:
                mod->num_fini_array = (dyn->d_un.d_val / sizeof(Elf32_Addr));
                break;
            case DT_REL:
                mod->rel_dyn = (Elf32_Rel *)((uintptr_t)mod->load_base + dyn->d_un.d_ptr);
                break;
            case DT_RELSZ:
                mod->num_rel_dyn = (dyn->d_un.d_val / sizeof(Elf32_Rel));
                break;
            case DT_RELA:
                mod->rela_dyn = (Elf32_Rela *)((uintptr_t)mod->load_base + dyn->d_un.d_ptr);
                break;
            case DT_RELASZ:
                mod->num_rela_dyn = (dyn->d_un.d_val / sizeof(Elf32_Rela));
                break;
            case DT_PLTREL:
                plt_type = dyn->d_un.d_val;
                break;
            case DT_JMPREL:
                plt_tab = (void *)((uintptr_t)mod->load_base + dyn->d_un.d_ptr);
                break;
            case DT_PLTRELSZ:
                plt_size = dyn->d_un.d_val;
                break;
        }
    }

    // 检查必需的动态段条目
    if (!mod->dynsym || !mod->dynstr || (!mod->rel_dyn && !mod->rela_dyn)) {
        so_loge("[SOLINKER] so_module_load_mem(%p,%u): error dynamic!", buffer, size);
        so_munmap(mod->load_base);
        free(mod);
        return NULL;
    }

    // 保存共享对象名称
    if (soname_offset) {
        mod->soname = strdup(mod->dynstr + soname_offset);
    }

    // 保存 PLT 重定位表
    if (plt_type == DT_REL) {
        mod->rel_plt     = plt_tab;
        mod->num_rel_plt = plt_size / sizeof(Elf32_Rel);
    } else if (plt_type == DT_RELA) {
        mod->rela_plt     = plt_tab;
        mod->num_rela_plt = plt_size / sizeof(Elf32_Rela);
    }

    so_module_retain(mod);
    so_linker_register_module(mod);

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

    if (!mod->soname) {
        const char *soname = strrchr(filename, '/');
        if (!soname) {
            soname = filename;
        }
        mod->soname = strdup(soname);
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
    const Elf32_Sym *sym      = NULL;  // 目标符号条目
    uintptr_t        sym_addr = 0;     // 目标符号地址
    so_module       *sym_mod  = NULL;  // 目标符号模块
    uint32_t         sym_size = 0;     // 目标符号大小
    const char      *sym_name = NULL;  // 目标符号名称

    // ===== 查找符号 =====
    if (rel_symidx) {  // 需重定位的符号位置若不为0，则需查找目标符号地址
        sym_name = mod->dynstr + mod->dynsym[rel_symidx].st_name;

        if (rel_type == R_ARM_COPY)
            sym_addr = (uintptr_t)so_module_find_symbol_ex(mod, sym_name, 1, &sym_mod, &sym);
        else
            sym_addr = (uintptr_t)so_module_find_symbol_ex(mod, sym_name, 0, &sym_mod, &sym);

        if (!sym_addr) {
            if (ELF32_ST_BIND(mod->dynsym[rel_symidx].st_info) == STB_WEAK) {
                so_logw("[SOLINKER] process_relocation: weak symbol '%s' not found, skipping", sym_name);
                return 0;
            }

            so_loge("[SOLINKER] process_relocation: symbol '%s' not found", sym_name);
            return -1;
        }

        sym_size = sym->st_size;
    }

    // ===== 执行重定位 =====
    uint32_t rel_val;

    switch (rel_type) {
        case R_ARM_NONE:  // 0
            break;
        case R_ARM_ABS32:  // 2 (S + A)
            rel_val = sym_addr + rel_addend;
            so_memcpy(rel_target, &rel_val, sizeof(uint32_t));
            // so_logd("  R_ARM_ABS32: %s -> 0x%x", sym_name, *rel_target);
            break;
        case R_ARM_REL32:  // 3 (S + A - P)
            rel_val = sym_addr + rel_addend - (uintptr_t)rel_target;
            so_memcpy(rel_target, &rel_val, sizeof(uint32_t));
            // so_logd("  R_ARM_REL32: %s -> 0x%x", sym_name, *rel_target);
            break;
        case R_ARM_TLS_DTPMOD32:  // 17 (MODULE_ID)
            rel_val = (uintptr_t)sym_mod;
            so_memcpy(rel_target, &rel_val, sizeof(uint32_t));
            // so_logd("  R_ARM_TLS_DTPMOD32: %s -> 0x%x", sym_name, *rel_target);
            break;
        case R_ARM_TLS_DTPOFF32:  // 18 (B(S) + A)
            rel_val = sym->st_value + rel_addend;
            so_memcpy(rel_target, &rel_val, sizeof(uint32_t));
            // so_logd("  R_ARM_TLS_DTPOFF32: %s -> 0x%x", sym_name, *rel_target);
            break;
        case R_ARM_COPY:  // 20 (S -> T)
            so_memcpy(rel_target, (void *)sym_addr, sym_size);
            // so_logd("  R_ARM_COPY: %s, 0x%x -> %p, size=%zu", sym_name, sym_addr, rel_target, sym_size);
            break;
        case R_ARM_GLOB_DAT:   // 21 (S)
        case R_ARM_JUMP_SLOT:  // 22 (S)
            rel_val = sym_addr;
            so_memcpy(rel_target, &rel_val, sizeof(uint32_t));
            // so_logd("  %s: %s -> 0x%x", rel_type == R_ARM_GLOB_DAT ? "R_ARM_GLOB_DAT" : "R_ARM_JUMP_SLOT", sym_name, *rel_target);
            break;
        case R_ARM_RELATIVE:  // 23 (B + A)
            rel_val = (uintptr_t)mod->load_base + rel_addend;
            so_memcpy(rel_target, &rel_val, sizeof(uint32_t));
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
        uint32_t *rel_target = (uint32_t *)((uintptr_t)mod->load_base + rel_tab[i].r_offset);
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
        uint32_t *rel_target = (uint32_t *)((uintptr_t)mod->load_base + rela_tab[i].r_offset);
        uint32_t  rel_addend = rela_tab[i].r_addend;

        if (_process_relocation(mod, rel_type, rel_symidx, rel_target, rel_addend) < 0) {
            result = -1;
            continue;
        }
    }

    return result;
}

int so_module_relocate(so_module *mod) {
    if (!mod) return -1;

    if (!mod->load_base) return 0;  // 从 so_library 创建的模块不需要重定位

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
    if (!mod) return -1;

    if (!mod->load_base) return 0;  // 从 so_library 创建的模块不需要初始化

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
    if (!mod) return;

    if (!mod->load_base) return;  // 从 so_library 创建的模块不需要反初始化

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
    if (!mod || !mod->load_base) return -1;

    if (so_msync(mod->load_base, mod->load_size) < 0) {
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
        so_module_close(mod);
        return NULL;
    }

    if (so_module_flush_caches(mod) < 0) {
        so_loge("[SOLINKER] so_module_open_mem(%p,%u): failed to flush caches!", buffer, size);
        so_module_close(mod);
        return NULL;
    }

    if (so_module_init(mod) < 0) {
        so_loge("[SOLINKER] so_module_open_mem(%p,%u): failed to initialize!", buffer, size);
        so_module_close(mod);
        return NULL;
    }

    so_logd("[SOLINKER] so_module_open_mem(%p,%u): success! load_base=%p, soname='%s'", buffer, size, mod->load_base, mod->soname);
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
        so_module_close(mod);
        return NULL;
    }

    if (so_module_flush_caches(mod) < 0) {
        so_loge("[SOLINKER] so_module_open(%s): failed to flush caches!", filename);
        so_module_close(mod);
        return NULL;
    }

    if (so_module_init(mod) < 0) {
        so_loge("[SOLINKER] so_module_open(%s): failed to initialize!", filename);
        so_module_close(mod);
        return NULL;
    }

    so_logd("[SOLINKER] so_module_open('%s'): success! load_base=%p, soname='%s'", filename, mod->load_base, mod->soname);
    return mod;
}

void so_module_close(so_module *mod) {
    if (mod) {
        so_logd("[SOLINKER] so_module_close(%p): ('%s')", mod, mod->soname);
        so_linker_unregister_module(mod);
        so_module_fini(mod);
        so_module_unload(mod);
    }
}

int so_module_retain(so_module *mod) {
    if (!mod) return -1;

    mod->ref_count++;
    return 0;
}

void so_module_release(so_module *mod) {
    if (!mod) return;

    if (mod->ref_count == 0 || --mod->ref_count == 0) {
        so_module_close(mod);
    }
}

int so_module_hook_offset(so_module *mod, ptrdiff_t offset, void *replacement, so_hook_ref *p_hook) {
    if (!mod || offset < 0 || !replacement) return -1;

    void *target   = (void *)((uintptr_t)mod->load_base + offset);
    void *boundary = (void *)((uintptr_t)mod->load_base + mod->load_size);
    return so_hook_addr_safe(target, boundary, replacement, p_hook);
}

int so_module_hook_symbol(so_module *mod, const char *sym_name, void *replacement, so_hook_ref *p_hook) {
    if (!mod || !sym_name || !replacement) return -1;

    void *target   = so_module_find_symbol(mod, sym_name);
    void *boundary = (void *)((uintptr_t)mod->load_base + mod->load_size);
    return so_hook_addr_safe(target, boundary, replacement, p_hook);
}

int so_module_inject_data_offset(so_module *mod, ptrdiff_t offset, const void *data, size_t len) {
    if (!mod || offset < 0 || !data || len == 0) return -1;

    void *target   = (void *)((uintptr_t)mod->load_base + offset);
    void *boundary = (void *)((uintptr_t)mod->load_base + mod->load_size);
    return so_inject_data_safe(target, boundary, data, len);
}

const char *so_module_name(so_module *mod) {
    return mod ? mod->soname : NULL;
}

void *so_module_entry_point(so_module *mod) {
    return mod ? mod->entry_point : NULL;
}

const void *so_module_load_base(so_module *mod) {
    return mod ? mod->load_base : NULL;
}

size_t so_module_load_size(so_module *mod) {
    return mod ? mod->load_size : 0;
}
