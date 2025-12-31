#include <stdlib.h>
#include <string.h>

#include <kubridge.h>
#include <so_util/so_util.h>

#include "config.h"
#include "ld.h"
#include "ld_mem.h"
#include "patch.h"
#include "utils/logger.h"

uintptr_t (*ldModuleGetSymbol)(LdModule *in_mod, const char *sym_name, uint32_t sym_hash, int type,
                               const char *vna_name, uint32_t vna_hash, LdModule **out_mod);

uintptr_t (*ldModuleLookupSymbol)(LdModule *mod, const char *sym_name, uint32_t sym_hash, int visibility,
                                  const char *vna_name, uint32_t vna_hash);

void ldSetErrorCode(LdContext *context, int code, const char *msg) {
    if (context) {
        context->error_code = code;
        if (msg)
            strncpy(context->error_msg, msg, 255);
        else
            context->error_msg[0] = '\0';
        l_debug("ldSetErrorCode: %s", context->error_msg);
    }
}

const char *ldGetErrorMessage(LdContext *context) {
    if (context) {
        l_debug("ldGetErrorMessage: %s", context->error_msg);
        return (const char *)context->error_msg;
    }
    return NULL;
}

void *ldModuleGetEntryPoint(LdModule *mod) {
    l_debug("ldModuleGetEntryPoint(%p)", mod);
    if (mod && mod->ehdr && mod->ehdr->e_entry) {
        void *entry_point = (char *)mod->load_base + mod->load_offset + mod->ehdr->e_entry;
        l_debug("mod_name=\"%s\", entry_point=%p", mod->mod_name, entry_point);
        if (strcmp(mod->mod_name, "bootstrap.exe") == 0) {
            bootstrap_mod   = mod;
            bootstrap_start = entry_point;
            entry_point     = (void *)&bootstrap_start_soloader;

            // hook
            // __ldModuleHookAddr(mod, , );
        }
        return entry_point;
    }

    return NULL;
}

uint32_t ldModuleSymHash(const uint8_t *name) {
    uint32_t h = 0, g;
    while (*name) {
        h = (h << 4) + *name++;
        if ((g = (h & 0xf0000000)) != 0)
            h ^= (g >> 24);
        h &= 0x0fffffff;
    }
    return h;
}

int ldModuleAllocAndLoadImage(LdModule *mod, const char *path, const char *err_msg) {
    LdLoadMemContext *mem_context;
    int               failed_count;
    char             *file_mem;
    uintptr_t         load_addr;

    if (!mod)
        return -1;

    if (!mod->load_base) {
        l_debug("ldModuleAllocAndLoadImage(%p): name=%s, load_base is empty", mod, mod->mod_name);
        return 0;
    }

    l_debug("ldModuleAllocAndLoadImage(%p): name=%s", mod, mod->mod_name);

    load_addr        = LOAD_ADDRESS;
    mem_context      = NULL;
    failed_count     = 0;
    file_mem         = (char *)mod->load_base;
    mod->load_base   = NULL;
    mod->load_size   = 0;
    mod->load_offset = 0;

LOAD:
    for (int i = 0; i < mod->ehdr->e_phnum; i++) {
        Elf32_Phdr *phdr = &mod->phdr[i];
        if (phdr->p_type == PT_LOAD) {
            if (!mem_context) {
                mem_context = ldLoadMemCreateContext(load_addr);
                if (!mem_context)
                    goto FAILED;
                mod->load_base = (void *)mem_context->load_addr;
            }

            if (phdr->p_memsz > 0) {
                int             flags = ((phdr->p_flags & PF_X) == PF_X) ? SCE_KERNEL_MEMBLOCK_TYPE_USER_RX : SCE_KERNEL_MEMBLOCK_TYPE_USER_RW;
                uintptr_t       addr  = mem_context->load_addr + phdr->p_vaddr;
                LdLoadMemEntry *entry = ldLoadMemCreateEntry(addr, flags, phdr->p_memsz, phdr->p_align);
                if (!entry)
                    goto FAILED;
                ldLoadMemAddEntry(mem_context, entry);

                kuKernelCpuUnrestrictedMemcpy((void *)addr, (void *)((uintptr_t)file_mem + phdr->p_offset), phdr->p_filesz);

                if (phdr->p_memsz > phdr->p_filesz) {
                    size_t zero_size = phdr->p_memsz - phdr->p_filesz;
                    char  *zero      = malloc(zero_size);
                    if (zero) {
                        memset(zero, 0, zero_size);
                        kuKernelCpuUnrestrictedMemcpy((void *)(addr + phdr->p_filesz), zero, zero_size);
                        free(zero);
                    }
                }
            }

            if (mod->load_size < phdr->p_vaddr + phdr->p_memsz)
                mod->load_size = phdr->p_vaddr + phdr->p_memsz;
        }
    }

    free(file_mem);
    l_debug("ldModuleAllocAndLoadImage success!");
    return 0;

FAILED:
    if (mem_context) {
        ldLoadMemDestroyContext(mem_context);
        mem_context = NULL;
    }

    if (failed_count < 50) {  // 如果失败次数<50，将映射地址递增，重新尝试
        l_debug("ldModuleAllocAndLoadImage(%p) failed on addr 0x%x, will retry!", mod, load_addr);
        failed_count++;
        load_addr += 0x01000000;
        goto LOAD;
    }

    free(file_mem);
    l_debug("ldModuleAllocAndLoadImage(%p) failed!", mod);
    return -1;
}

void ldModuleDestroyImage(LdModule *mod) {
    if (mod && mod->load_base) {
        l_debug("ldModuleDestroyImage(%p): name=%s", mod, mod->mod_name);
        ldLoadMemDestroyContext(ldLoadMemGetContextFromData(mod->load_base));
        mod->load_base = NULL;
        mod->load_size = 0;
    }
}

int ldModuleProtectImage(LdModule *mod) {
    return 0;
}

static int ldModuleRelocateCore2(LdModule *mod, Elf32_Rel *rel_tab, size_t rel_num) {
    int         error_code;
    char        error_msg[256];
    uintptr_t   load_base = (uintptr_t)mod->load_base + mod->load_offset;
    const char *sym_name  = NULL;
    uint32_t    sym_hash  = 0;
    uintptr_t   sym_addr  = 0;
    char       *vna_name  = NULL;
    uint32_t    vna_hash  = 0;

    // l_debug("ldModuleRelocateCore2(%p,%p,%u): %s", mod, rel_tab, rel_num, mod->mod_name);

    int i;
    for (i = 0; i < rel_num; i++) {
        Elf32_Rel *rel        = &rel_tab[i];
        uint32_t   rel_offset = rel->r_offset;
        uint16_t   rel_sym    = ELF32_R_SYM(rel->r_info);
        uint8_t    rel_type   = ELF32_R_TYPE(rel->r_info);
        LdModule  *sym_mod    = NULL;

        sym_name = NULL;
        sym_hash = 0;
        vna_name = NULL;
        vna_hash = 0;

        sym_name = mod->strtab + mod->symtab[rel_sym].st_name;
        sym_hash = ldModuleSymHash(sym_name);

        if (mod->versym) {
            Elf32_Verneed *verneed = mod->verneed;
            Elf32_Versym   ver     = mod->versym[rel_sym];
            if (ver > 1 && verneed) {
                for (int j = 0; j < mod->needed_num; j++) {
                    Elf32_Vernaux *aux = (Elf32_Vernaux *)((uintptr_t)verneed + verneed->vn_aux);
                    for (int k = 0; k < verneed->vn_cnt; k++) {
                        if (aux->vna_other == ver) {
                            vna_name = mod->strtab + aux->vna_name;
                            vna_hash = aux->vna_hash;
                            if (vna_name)
                                break;
                        }
                        aux = (Elf32_Vernaux *)((uintptr_t)aux + aux->vna_next);
                    }
                    verneed = (Elf32_Verneed *)((uintptr_t)verneed + verneed->vn_next);
                }
            }
        }

        if (rel_sym) {
            if (rel_type == R_ARM_COPY)
                sym_addr = ldModuleGetSymbol(mod, sym_name, sym_hash, 2, vna_name, vna_hash, &sym_mod);
            else
                sym_addr = ldModuleGetSymbol(mod->context->loaded_mods->mod, sym_name, sym_hash, 1, vna_name, vna_hash, &sym_mod);

            // printf("ldModuleRelocateCore2: sym_name=\"%s\", sym_addr=0x%x\n", sym_name, sym_addr);

            if (!sym_addr) {
                if (ELF32_ST_BIND(mod->symtab[rel_sym].st_info) == STB_WEAK) {
                    l_debug("Weak symbol '%s' not found, skipping", sym_name);
                    continue;
                } else {
                    if (vna_name)
                        snprintf(error_msg, 256, "%s@%s", sym_name, vna_name);
                    else
                        strncpy(error_msg, sym_name, 255);
                    error_code = 0xE030100;
                    ldSetErrorCode(mod->context, error_code, error_msg);
                    l_debug("ldModuleRelocateCore2(%p): %s failed!", mod, mod->mod_name);
                    return -1;
                }
            }
        }

        uintptr_t  val;
        size_t     len;
        uintptr_t *ptr = (uintptr_t *)(load_base + rel_offset);

        // printf("ldModuleRelocateCore2(%p): ptr=%p, *ptr=0x%x\n", mod, ptr, *ptr);

        switch (rel_type) {
            case R_ARM_NONE:  // 0
                // printf("case R_ARM_NONE\n");
                break;
            case R_ARM_ABS32:  // 2
                val = *ptr + sym_addr;
                len = sizeof(uintptr_t);
                // printf("case R_ARM_ABS32: val=0x%x\n", val);
                kuKernelCpuUnrestrictedMemcpy(ptr, &val, len);
                break;
            case R_ARM_REL32:  // 3
                val = *ptr + (sym_addr - rel_offset);
                len = sizeof(uintptr_t);
                // printf("case R_ARM_REL32: val=0x%x\n", val);
                kuKernelCpuUnrestrictedMemcpy(ptr, &val, len);
                break;
            case R_ARM_TLS_DTPMOD32:  // 17
                val = sym_addr;
                len = sizeof(uintptr_t);
                // printf("case R_ARM_TLS_DTPMOD32: val=0x%x\n", val);
                kuKernelCpuUnrestrictedMemcpy(ptr, &val, len);
                break;
            case R_ARM_TLS_DTPOFF32:  // 18
                val = *ptr + (sym_addr - load_base);
                len = sizeof(uintptr_t);
                // printf("case R_ARM_TLS_DTPOFF32: val=0x%x\n", val);
                kuKernelCpuUnrestrictedMemcpy(ptr, &val, len);
                break;
            case R_ARM_COPY:  // 20
                val = sym_addr;
                len = mod->symtab[rel_sym].st_size;
                // printf("case R_ARM_COPY: val=0x%x\n", val);
                kuKernelCpuUnrestrictedMemcpy(ptr, &val, len);
                break;
            case R_ARM_GLOB_DAT:   // 21
            case R_ARM_JUMP_SLOT:  // 22
                val = sym_addr;
                len = sizeof(uintptr_t);
                // printf("case %s: val=0x%x\n", rel_type == R_ARM_GLOB_DAT ? "R_ARM_GLOB_DAT" : "R_ARM_JUMP_SLOT", val);
                kuKernelCpuUnrestrictedMemcpy(ptr, &val, len);
                break;
            case R_ARM_RELATIVE:  // 23
                val = load_base + *ptr;
                len = sizeof(uintptr_t);
                // printf("case R_ARM_RELATIVE: val=0x%x\n", val);
                kuKernelCpuUnrestrictedMemcpy(ptr, &val, len);
                break;
            default:
                // printf("Unknown relocation type\n");
                snprintf(error_msg, 256, "Unknown relocation type in file: %s", mod->mod_name);
                error_code = 0xE030200;
                ldSetErrorCode(mod->context, error_code, error_msg);
                return -1;
        }

        // printf("ldModuleRelocate \"%s\" success!\n", sym_name);
    }

    // l_debug("ldModuleRelocateCore2(%p): %s success!", mod, mod->mod_name);
    return 0;
}

static int ldModuleRelocateCore(LdModule *mod) {
    if (!mod)
        return -1;

    if (!mod->dynamic_phdr)
        return 0;

    if (mod->status > LD_MODULE_STATUS_NONE) {
        l_debug("ldModuleRelocateCore(%p): %s is already relocated", mod, mod->mod_name);
        return 0;
    }

    l_debug("ldModuleRelocateCore(%p): %s", mod, mod->mod_name);

    uintptr_t   load_base    = (uintptr_t)mod->load_base + mod->load_offset;
    Elf32_Phdr *dynamic_phdr = mod->dynamic_phdr;
    Elf32_Dyn  *dynamic      = (Elf32_Dyn *)(load_base + dynamic_phdr->p_vaddr);
    size_t      dynamic_num  = dynamic_phdr->p_memsz / sizeof(Elf32_Dyn);
    Elf32_Rel  *rel_tab      = 0;
    size_t      rel_size     = 0;
    uint32_t    plt_type     = 0;
    uintptr_t   plt_tab      = 0;
    size_t      plt_size     = 0;

    for (int i = 0; i < dynamic_num; i++) {
        Elf32_Dyn *dyn = &dynamic[i];
        switch (dyn->d_tag) {
            case DT_REL:
                rel_tab = (Elf32_Rel *)(load_base + dyn->d_un.d_ptr);
                break;
            case DT_RELSZ:
                rel_size = dyn->d_un.d_val;
                break;
            case DT_PLTREL:
                plt_type = dyn->d_un.d_val;
                break;
            case DT_PLTRELSZ:
                plt_size = dyn->d_un.d_val;
                break;
            case DT_JMPREL:
                plt_tab = load_base + dyn->d_un.d_ptr;
                break;
        }
    }

    if (rel_tab && ldModuleRelocateCore2(mod, rel_tab, rel_size / sizeof(Elf32_Rel)) != 0) {
        l_debug("ldModuleRelocateCore(%p): %s failed!", mod, mod->mod_name);
        return -1;
    }

    if (plt_tab && plt_type == DT_REL && ldModuleRelocateCore2(mod, (Elf32_Rel *)plt_tab, plt_size / sizeof(Elf32_Rel)) != 0) {
        l_debug("ldModuleRelocateCore(%p): %s failed!", mod, mod->mod_name);
        return -1;
    }

    l_debug("ldModuleRelocateCore(%p): %s success!", mod, mod->mod_name);
    mod->status = LD_MODULE_STATUS_RELOCATED;
    return 0;
}

int ldModuleRelocate(LdModule *mod) {
    int res;

    if (!mod)
        return -1;

    if (mod->status > LD_MODULE_STATUS_NONE)
        return 0;

    for (int i = 0; i < mod->needed_num; i++) {
        res = ldModuleRelocate(mod->needed_mods[i]);
        if (res != 0 && mod->needed_mods[i]->load_base)
            return res;
    }

    res = ldModuleRelocateCore(mod);
    if (res == 0 && mod->load_base)
        ldModuleProtectImage(mod);

    return res;
}

int ldModuleInitialize(LdModule *mod) {
    if (!mod)
        return -1;

    if (!mod->load_base) {
        l_debug("ldModuleInitialize(%p): %s, load_base is empty", mod, mod->mod_name);
        return 0;
    }

    if (mod->status == LD_MODULE_STATUS_INITIALIZED) {
        l_debug("ldModuleInitialize(%p): %s is already initialized", mod, mod->mod_name);
        return 0;
    }

    l_debug("ldModuleInitialize(%p): name=%s, load_addr=%p, status=%u", mod, mod->mod_name, mod->load_base, mod->status);

    Elf32_Phdr *dynamic_phdr = mod->dynamic_phdr;
    if (!dynamic_phdr)
        return 0;

    // 初始化所有依赖库
    for (int i = 0; i < mod->needed_num; i++) {
        ldModuleInitialize(mod->needed_mods[i]);
    }

    uintptr_t  load_base     = (uintptr_t)mod->load_base + mod->load_offset;
    Elf32_Dyn *dynamic       = (Elf32_Dyn *)(load_base + dynamic_phdr->p_vaddr);
    int        dynamic_num   = dynamic_phdr->p_memsz / sizeof(Elf32_Dyn);
    int (**init_array)(void) = NULL;
    uint32_t init_array_size = 0;
    int (*init_func)(void)   = NULL;

    // 遍历动态段
    for (int i = 0; i < dynamic_num; i++) {
        Elf32_Dyn *dyn = &dynamic[i];
        switch (dyn->d_tag) {
            case DT_INIT_ARRAY:
                init_array = (int (**)(void))(load_base + dyn->d_un.d_ptr);
                break;
            case DT_INIT_ARRAYSZ:
                init_array_size = dyn->d_un.d_val;
                break;
            case DT_INIT:
                init_func = (int (*)(void))(load_base + dyn->d_un.d_ptr);
                break;
        }
    }

    // 执行初始化函数
    if (init_func)
        init_func();

    // 执行初始化函数数组
    if (init_array && init_array_size > 0) {
        int init_func_num = init_array_size / sizeof(void *);
        for (int i = 0; i < init_func_num; i++) {
            init_array[i]();
        }
    }

    mod->status = LD_MODULE_STATUS_INITIALIZED;

    l_debug("ldModuleInitialize(%p) %s success!", mod, mod->mod_name);
    return 0;
}

int ldModuleDeinitialize(LdModule *mod) {
    if (!mod)
        return -1;

    l_debug("ldModuleDeinitialize(%p): name=%s, status=%u", mod, mod->mod_name, mod->status);

    if (mod->context->exit_state) {
        mod->status = LD_MODULE_STATUS_DEINITIALIZED;
        return 0;
    }

    if (mod->status != LD_MODULE_STATUS_INITIALIZED) {
        l_debug("ldModuleDeinitialize(%p): %s is not initialized", mod, mod->mod_name);
        return 0;
    }

    if (!mod->dynamic_phdr)
        return 0;

    Elf32_Phdr *dynamic_phdr = mod->dynamic_phdr;
    if (!dynamic_phdr)
        return 0;

    char      *load_base      = (char *)mod->load_base + mod->load_offset;
    Elf32_Dyn *dynamic        = (Elf32_Dyn *)(load_base + dynamic_phdr->p_vaddr);
    int        dynamic_num    = dynamic_phdr->p_memsz / sizeof(Elf32_Dyn);
    void (**fini_array)(void) = NULL;
    size_t fini_array_size    = 0;
    void (*fini_func)(void)   = NULL;

    // 遍历动态段
    for (int i = 0; i < dynamic_num; i++) {
        Elf32_Dyn *dyn = &dynamic[i];
        switch (dyn->d_tag) {
            case DT_FINI_ARRAY:
                fini_array = (void (**)(void))(load_base + dyn->d_un.d_ptr);
                break;
            case DT_FINI_ARRAYSZ:
                fini_array_size = dyn->d_un.d_val;
                break;
            case DT_FINI:
                fini_func = (void (*)(void))(load_base + dyn->d_un.d_ptr);
                break;
        }
    }

    // 执行反初始化函数数组
    if (fini_array) {
        int fini_array_num = fini_array_size / sizeof(void *);
        for (int i = 0; i < fini_array_num; i++) {
            fini_array[i]();
        }
    }

    // 执行反初始化函数
    if (fini_func) {
        fini_func();
    }

    mod->status = LD_MODULE_STATUS_DEINITIALIZED;

    l_debug("ldModuleDeinitialize(%p) %s success!", mod, mod->mod_name);
    return 0;
}

so_hook __ldModuleHookAddr(LdModule *mod, uintptr_t vaddr, uintptr_t dst) {
    if (!mod) {
        so_hook h;
        return h;
    }

    uintptr_t addr = (uintptr_t)mod->load_base + mod->load_offset + vaddr;
    return hook_addr(addr, dst);
}

so_hook __ldModuleHookSymbol(LdModule *mod, const char *symbol, uintptr_t dst) {
    if (!mod) {
        so_hook h;
        return h;
    }

    uint32_t  hash = ldModuleSymHash(symbol);
    uintptr_t addr = ldModuleLookupSymbol(mod, symbol, hash, STV_DEFAULT, NULL, 0);
    return hook_addr(addr, dst);
}
