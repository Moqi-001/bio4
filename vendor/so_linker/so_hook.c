#include <stdint.h>
#include <string.h>

#include <so_common/so_logger.h>

#include "so_internal.h"
#include "so_linker.h"

/*
 * 跳板 (trampoline) 式 Hook
 * =========================
 *
 * 旧实现把「恢复原指令 -> 调原函数 -> 重新写入跳转」放进 SO_CONTINUE，每次调用
 * 都要两次 memcpy + 两次 cacheflush；而且在这期间目标函数被临时改写，多线程
 * 或递归调用会执行到半条指令。这里改为标准跳板方案：
 *
 *   目标函数 (src)                               跳板 (T, RWX)
 *   +------------------+                        +---------------------------+
 *   | LDR PC, [PC,#-4] | --+                    | 搬过来的原指令（已重定位）| <- 入口
 *   | .word repl|   |                    +---------------------------+
 *   +------------------+   |                    | NOP 补齐（Thumb 2 字节）  |
 *   | 原指令 #3        |   |                    +---------------------------+
 *   | 原指令 #4        |   |                    | 跳回 src + code_len       |
 *   +------------------+   |                    +---------------------------+
 *                          +--> repl()   | 字面量槽（PC 相对指令用） |
 *                                   |           +---------------------------+
 *                                   +-- SO_CONTINUE 直接调用 T（一次普通调用）
 *
 * SO_CONTINUE 因此退化为一次纯函数调用：不写代码、不刷 cache、无竞态。
 *
 * 截断点（搬运多少字节）的规则见 _plan_prologue()：
 *   ARM                4 字节定长，覆盖到 >= 8 字节即可。
 *   Thumb (src%4 == 0) 2/4 混合定长，覆盖到 >= 8 字节且必须落在指令边界。
 *   Thumb (src%4 == 2) 同上，但覆盖到 >= 10 字节（要垫 NOP 才能放 LDR.W）。
 *   命中 IT 块         继续搬运到 IT 块结束，否则跳回指令会被 IT 条件化执行。
 *
 * 只要被搬运的指令里出现「用到 PC 且无法重定位」的形式（B/BL/Bcc/ADR/非字面量
 * 形式的 PC 相对访存等），就拒绝本次 Hook 并打印 opcode，绝不生成会崩的跳板。
 */

#define SO_TRAMP_SIZE      64 /* 每个跳板占用的固定字节数 */
#define SO_TRAMP_MAX_COPY  32 /* 单次最多搬运的原指令字节数 */
#define SO_TRAMP_MAX_INSTR 16 /* 单次最多搬运的指令条数 */
#define SO_TRAMP_MAX_LIT   4  /* 单个跳板最多容纳的字面量槽数量 */
#define SO_TRAMP_PAGE_SIZE (4 * 1024)

/* 被搬运指令的重定位种类 */
enum {
    _IR_NONE = 0,  /* 原样搬运 */
    _IR_LIT_T16,   /* Thumb-16  LDR Rt, [PC, #imm8*4]                  (0x48xx) */
    _IR_LIT_T32,   /* Thumb-32  LDR/LDRB/LDRH/LDRSB/LDRSH [PC, #imm12]          */
    _IR_LIT_A32,   /* ARM       LDR Rt, [PC, #+/-imm12]                          */
    _IR_PC_ADD_T16 /* Thumb-16  ADD Rd, PC —— 折叠进上一条字面量后改成 NOP   */
};

typedef struct {
    uint8_t   off;     /* 在跳板中的偏移 */
    uint8_t   size;    /* 指令长度：2 / 4 */
    uint8_t   kind;    /* 重定位种类 */
    uint8_t   slot;    /* 字面量槽索引 */
    uint8_t   fold;    /* 该字面量槽需要加上后续 ADD Rd, PC 的 PC 值 */
    uintptr_t lit;     /* 原字面量地址 */
    uintptr_t fold_pc; /* 被折叠的 ADD Rd, PC 指令地址（仅 fold 时有意义） */
} instr_rec;

/* ------------------------------------------------------------------ */
/* 跳板内存：按页申请的 bump allocator                                  */
/* 跳板内存常驻进程生命周期（每个 hook 64 字节，一页可放 64 个）。       */
/* ------------------------------------------------------------------ */

typedef struct so_tramp_page {
    struct so_tramp_page *next;
    size_t                used;
    uint8_t               data[SO_TRAMP_PAGE_SIZE];
} so_tramp_page;

static so_tramp_page *_tramp_pages = NULL;

static void *_tramp_alloc(size_t size) {
    size = ALIGN_UP(size, 8);

    for (so_tramp_page *page = _tramp_pages; page; page = page->next) {
        if (page->used + size <= SO_TRAMP_PAGE_SIZE) {
            void *ptr = page->data + page->used;
            page->used += size;
            return ptr;
        }
    }

    so_tramp_page *page = (so_tramp_page *)so_mmap(sizeof(so_tramp_page));
    if (!page) {
        so_loge("[SOLINKER] hook: failed to alloc trampoline page!");
        return NULL;
    }

    page->next   = _tramp_pages;
    page->used   = size;
    _tramp_pages = page;

    return page->data;
}

/* ------------------------------------------------------------------ */
/* 小工具                                                              */
/* ------------------------------------------------------------------ */

static void _put16(void *p, uint16_t v) {
    memcpy(p, &v, sizeof(v));
}

static void _put32(void *p, uint32_t v) {
    memcpy(p, &v, sizeof(v));
}

static uint16_t _get16(const void *p) {
    uint16_t v;
    memcpy(&v, p, sizeof(v));
    return v;
}

static uint32_t _get32(const void *p) {
    uint32_t v;
    memcpy(&v, p, sizeof(v));
    return v;
}

/* Thumb-2 32 位指令判定：11101 / 11110 / 11111 三种前缀 */
static inline int _thumb_is_32(uint16_t hw) {
    return (hw & 0xF800) >= 0xE800;
}

/* 读 PC 时的实际取值：ARM 为当前指令 + 8；Thumb 为 (当前指令 + 4) 向下 4 对齐 */
static inline uintptr_t _arm_pc(uintptr_t addr) {
    return addr + 8;
}

static inline uintptr_t _thumb_pc(uintptr_t addr) {
    return (addr + 4) & ~(uintptr_t)3;
}

/* IT 块长度 = 4 - ctz(mask)；mask 为 0 时是 NOP/hint，不是 IT */
static int _thumb_it_count(uint16_t hw) {
    uint32_t mask = hw & 0x000F;
    int      tz   = 0;

    if (!mask) {
        return 0;
    }
    while (!(mask & 1)) {
        mask >>= 1;
        tz++;
    }
    return 4 - tz;
}

static inline void _log_reject(const char *why, uintptr_t addr, uint32_t op) {
    so_loge("[SOLINKER] hook: refuse to hook 0x%08x (%s, opcode=0x%08x)", addr, why, op);
}

/* ------------------------------------------------------------------ */
/* 指令分析                                                            */
/* 返回 1 = 需要字面量重定位（已填 rec），0 = 可原样搬运，              */
/*     -1 = 用到 PC 或改变控制流，无法安全搬运                         */
/* ------------------------------------------------------------------ */

static int _analyze_arm(uint32_t w, uintptr_t addr, instr_rec *rec) {
    uint32_t cls = (w >> 25) & 0x7;

    switch (cls) {
        case 0: /* 数据处理 / 乘法 / 杂项 */
        case 1:
            if (((w >> 12) & 0xF) == 15) {
                return -1; /* Rd = PC（含 MOV PC / BX） */
            }
            if (((w >> 16) & 0xF) == 15) {
                return -1; /* Rn = PC（ADR） */
            }
            if ((w & 0xF) == 15) {
                return -1; /* Rm = PC（MOV Rd, PC） */
            }
            return 0;

        case 2: /* 单寄存器加载 / 存储 */
        case 3:
            if (((w >> 16) & 0xF) != 15) {
                return 0;
            }
            /* 只支持 LDR Rt, [PC, #+/-imm12]，其它 PC 相对访存一律拒绝 */
            if ((w & 0x0F7F0000) != 0x051F0000) {
                return -1;
            }
            {
                uintptr_t pc  = _arm_pc(addr);
                uint32_t  off = w & 0xFFF;
                rec->kind     = _IR_LIT_A32;
                rec->lit      = (w & (1u << 23)) ? (pc + off) : (pc - off);
            }
            return 1;

        case 4: /* LDM / STM */
            if (((w >> 16) & 0xF) == 15) {
                return -1; /* 以 PC 为基址 */
            }
            if (w & 0x8000) {
                return -1; /* 寄存器列表含 PC */
            }
            return 0;

        case 5: /* B / BL */
            return -1;

        default: /* 协处理器 / SVC */
            if (((w >> 16) & 0xF) == 15) {
                return -1; /* 含 VLDR 字面量形式 */
            }
            return 0;
    }
}

static int _analyze_thumb16(uint16_t hw, uintptr_t addr, instr_rec *rec) {
    if ((hw & 0xF000) == 0xD000) {
        return -1; /* B<c>     */
    }
    if ((hw & 0xF800) == 0xE000) {
        return -1; /* B        */
    }
    if ((hw & 0xF500) == 0xB100) {
        return -1; /* CBZ/CBNZ */
    }
    if ((hw & 0xF800) == 0xA000) {
        return -1; /* ADR      */
    }
    if ((hw & 0xFFF8) == 0x4778) {
        return -1; /* BX PC（ARM/Thumb 切换桩） */
    }

    /* ADD Rd, PC（01000100 D 1111 Rd）：
     * 这是 "ldr rX,[pc,#n] + add rX,pc" 的 PIC 取址惯用法。搬到跳板后 PC 变了，
     * 单条指令无法自洽，必须把 PC 值折叠进上一条字面量槽（见 _plan_prologue）。 */
    if ((hw & 0xFF00) == 0x4400 && (((hw >> 3) & 0xF) == 0xF)) {
        rec->lit = addr; /* 记录指令自身地址，发射时用来算 PC */
        return 2;
    }

    if ((hw & 0xF800) == 0x4800) { /* LDR Rt, [PC, #imm8*4] */
        rec->kind = _IR_LIT_T16;
        rec->lit  = _thumb_pc(addr) + (uintptr_t)(hw & 0xFF) * 4;
        return 1;
    }

    return 0;
}

static int _analyze_thumb32(uint16_t hw1, uint16_t hw2, uintptr_t addr, instr_rec *rec) {
    /* 字面量加载族：1111 1000/1001 xxxx 1111 | Rt imm12，Rn = PC 且 U = 1 */
    if ((hw1 & 0x000F) == 0x000F &&
        ((hw1 & 0xFF00) == 0xF800 || (hw1 & 0xFF00) == 0xF900)) {
        uint16_t op = hw1 & 0x00F0;
        if (op == 0x90 || op == 0xB0 || op == 0xD0) { /* LDRB / LDRH / LDR（含 SB/SH） */
            rec->kind = _IR_LIT_T32;
            rec->lit  = _thumb_pc(addr) + (uintptr_t)(hw2 & 0xFFF);
            return 1;
        }
        return -1; /* STR 字面量形式 */
    }

    /* ADR.W（ADD/SUB 形式） */
    if ((hw1 & 0xFBFF) == 0xF20F || (hw1 & 0xFBFF) == 0xF2AF) {
        return -1;
    }

    /* B<c>.W / B.W / BL / BLX(imm)：都是 11110 前缀且 hw2 最高位为 1 */
    if ((hw1 & 0xF800) == 0xF000 && (hw2 & 0x8000)) {
        return -1;
    }

    /* LDM/STM/表分支/LDRD 类：基址为 PC 或寄存器列表含 PC 时无法搬运 */
    if ((hw1 & 0xFE00) == 0xE800) {
        if ((hw1 & 0x000F) == 0x000F) {
            return -1; /* Rn = PC（含 LDRD/STRD 字面量形式） */
        }
        if (hw2 & 0x8000) {
            return -1; /* 寄存器列表含 PC（含 POP.W {..., PC}） */
        }
    }

    return 0;
}

/* 取字面量加载指令的目标寄存器编号；不是字面量加载时返回 -1 */
static int _lit_rt(const instr_rec *rec, uintptr_t src) {
    const uint8_t *s = (const uint8_t *)(src + rec->off);

    switch (rec->kind) {
        case _IR_LIT_T16:
            return (int)((_get16(s) >> 8) & 0x7);
        case _IR_LIT_T32:
            return (int)((_get16(s + 2) >> 12) & 0xF);
        case _IR_LIT_A32:
            return (int)((_get32(s) >> 12) & 0xF);
        default:
            return -1;
    }
}

/* ------------------------------------------------------------------ */
/* 规划：第一遍只解码、只算长度，确定搬运范围                          */
/* need_tramp = 0 时（永久替换，不需要 SO_CONTINUE）只求指令边界， */
/* 分支与 PC 相对指令都可以直接覆盖，不需要重定位。                     */
/* ------------------------------------------------------------------ */

static int _plan_prologue(uintptr_t src, int is_thumb, int need_tramp,
                          instr_rec *recs, size_t *out_num, size_t *out_code_len) {
    size_t    num        = 0;
    size_t    copied     = 0;
    int       patch_size = is_thumb ? (((src & 2) ? 10 : 8)) : 8;
    int       it_left    = 0;
    uintptr_t cur        = src;

    while (copied < (size_t)patch_size || it_left > 0) {
        instr_rec *rec;
        int        size;
        int        kind;
        int        is_it = 0;

        if (num >= SO_TRAMP_MAX_INSTR) {
            so_loge("[SOLINKER] hook: too many instructions in prologue of 0x%08x", (unsigned)src);
            return -1;
        }

        rec = &recs[num];
        memset(rec, 0, sizeof(*rec));
        rec->off = (uint8_t)copied;

        if (is_thumb) {
            uint16_t hw1 = _get16((const void *)cur);

            if (_thumb_is_32(hw1)) {
                size = 4;

                uint16_t hw2 = _get16((const void *)(cur + 2));
                kind         = need_tramp ? _analyze_thumb32(hw1, hw2, cur, rec) : 0;
                if (kind < 0) {
                    _log_reject("thumb32 uses PC/branch", cur, ((uint32_t)hw2 << 16) | hw1);
                    return -1;
                }
            } else {
                size = 2;
                kind = need_tramp ? _analyze_thumb16(hw1, cur, rec) : 0;
                if (kind < 0) {
                    _log_reject("thumb16 branch/adr", cur, hw1);
                    return -1;
                }
                if ((hw1 & 0xFF00) == 0xBF00 && (hw1 & 0x000F)) {
                    it_left = _thumb_it_count(hw1);
                    is_it   = 1;
                }
            }
        } else {
            size = 4;

            uint32_t w = _get32((const void *)cur);
            kind       = need_tramp ? _analyze_arm(w, cur, rec) : 0;
            if (kind < 0) {
                _log_reject("arm uses PC/branch", cur, w);
                return -1;
            }
        }

        /* ADD Rd, PC：把 PC 值折叠进最近的、同寄存器的字面量加载，本指令改用 NOP */
        if (kind == 2) {
            uint16_t   hw        = _get16((const void *)cur);
            int        rd        = (int)((((hw >> 7) & 1) << 3) | (hw & 0x7));
            instr_rec *fold_into = NULL;

            /*
             * 编译器会把 "ldr rX,[pc,#n]" 和 "add rX,pc" 之间插入其它 ldr，
             * 所以不能要求严格相邻。只允许跨过「其它寄存器的字面量加载」——
             * 这类指令确定不会写 rd，语义上安全；其余情况一律放弃。
             */
            for (size_t j = num; j-- > 0;) {
                if (recs[j].kind == _IR_PC_ADD_T16 || recs[j].fold) {
                    break;
                }
                if (recs[j].kind != _IR_NONE) { /* 字面量加载 */
                    if (_lit_rt(&recs[j], src) == rd) {
                        fold_into = &recs[j];
                        break;
                    }
                    continue; /* 其它寄存器的字面量加载：不写 rd，继续往前找 */
                }
                break; /* 其它指令，无法确定是否写 rd，保守放弃 */
            }

            if (!fold_into) {
                _log_reject("add Rd, pc without matching literal load", cur, hw);
                return -1;
            }
            fold_into->fold    = 1;
            fold_into->fold_pc = cur;
            rec->kind          = _IR_PC_ADD_T16;
        }

        if (copied + (size_t)size > SO_TRAMP_MAX_COPY) {
            so_loge("[SOLINKER] hook: prologue of 0x%08x is too long to hook", (unsigned)src);
            return -1;
        }

        rec->size = (uint8_t)size;
        num++;
        cur += size;
        copied += size;

        if (it_left > 0 && !is_it) {
            it_left--;
        }
    }

    *out_num      = num;
    *out_code_len = copied;
    return 0;
}

/* ------------------------------------------------------------------ */
/* 发射：第二遍已知跳板布局，逐条写出（含字面量槽的绝对地址计算）       */
/* ------------------------------------------------------------------ */

static int _emit_trampoline(uint8_t *tramp, uintptr_t src, int is_thumb,
                            const instr_rec *recs, size_t num, size_t code_len,
                            size_t slot_base) {
    uintptr_t slot0    = (uintptr_t)(tramp + slot_base);
    size_t    jump_off = ALIGN_UP(code_len, 4);

    /* 1. 搬运原指令 */
    for (size_t i = 0; i < num; i++) {
        const instr_rec *rec  = &recs[i];
        const uint8_t   *s    = (const uint8_t *)(src + rec->off);
        uint8_t         *d    = tramp + rec->off;
        uintptr_t        slot = slot0 + (uintptr_t)rec->slot * 4;

        switch (rec->kind) {
            case _IR_LIT_A32: {
                uint32_t w     = _get32(s);
                int64_t  delta = (int64_t)slot - (int64_t)_arm_pc((uintptr_t)d);

                if (delta < 0 || delta > 0xFFF) {
                    so_loge("[SOLINKER] hook: arm literal out of range (0x%08x)", (unsigned)src);
                    return -1;
                }
                w &= ~0x00800FFFu; /* 清 U 位与 imm12 */
                w |= 0x00800000u | (uint32_t)delta;
                _put32(d, w);
                break;
            }

            case _IR_LIT_T16: {
                uint16_t hw    = _get16(s);
                int64_t  delta = (int64_t)slot - (int64_t)_thumb_pc((uintptr_t)d);

                if (delta < 0 || (delta & 3) || delta > 0x3FC) {
                    so_loge("[SOLINKER] hook: thumb16 literal out of range (0x%08x)", (unsigned)src);
                    return -1;
                }
                hw = (uint16_t)((hw & 0xFF00) | (uint16_t)(delta >> 2));
                _put16(d, hw);
                break;
            }

            case _IR_LIT_T32: {
                uint16_t hw1   = _get16(s);
                uint16_t hw2   = _get16(s + 2);
                int64_t  delta = (int64_t)slot - (int64_t)_thumb_pc((uintptr_t)d);

                if (delta < 0 || delta > 0xFFF) {
                    so_loge("[SOLINKER] hook: thumb32 literal out of range (0x%08x)", (unsigned)src);
                    return -1;
                }
                hw2 = (uint16_t)((hw2 & 0xF000) | (uint16_t)delta); /* imm12 在 hw2 */
                _put16(d, hw1);
                _put16(d + 2, hw2);
                break;
            }

            case _IR_PC_ADD_T16: /* 已折叠进上一条字面量槽 */
                _put16(d, 0xBF00);
                break;

            default:
                memcpy(d, s, rec->size);
                break;
        }
    }

    /* 2. NOP 补齐，让跳回指令落在 4 字节边界上 */
    if (is_thumb && (code_len & 2)) {
        _put16(tramp + code_len, 0xBF00);
    }

    /* 3. 跳回 src + code_len */
    {
        uint32_t back = (uint32_t)((src + code_len) | (uintptr_t)is_thumb);

        if (is_thumb) {
            _put32(tramp + jump_off, 0xF000F8DF); /* LDR.W PC, [PC] */
            _put32(tramp + jump_off + 4, back);
        } else {
            _put32(tramp + jump_off, 0xE51FF004); /* LDR PC, [PC, #-4] */
            _put32(tramp + jump_off + 4, back);
        }
    }

    /* 4. 字面量槽内容 */
    for (size_t i = 0; i < num; i++) {
        const instr_rec *rec = &recs[i];
        uint32_t         v;

        if (rec->kind == _IR_NONE || rec->kind == _IR_PC_ADD_T16) {
            continue;
        }

        v = _get32((const void *)rec->lit);

        /* 被折叠的 ADD Rd, PC：把原 PC 值加进槽内容 */
        if (rec->fold) {
            v += (uint32_t)_thumb_pc(rec->fold_pc);
        }

        _put32((uint8_t *)slot0 + (uintptr_t)rec->slot * 4, v);
    }

    return 0;
}

/* ------------------------------------------------------------------ */
/* 对外接口                                                            */
/* ------------------------------------------------------------------ */

int so_hook(void *target, void *repl, so_hook_ref *p_hook) {
    if (!target || !repl) {
        return -1;
    }

    uintptr_t tgt_addr = (uintptr_t)target & ~(uintptr_t)1;
    int       is_thumb = (uintptr_t)target & 1;
    instr_rec recs[SO_TRAMP_MAX_INSTR];
    size_t    num      = 0;
    size_t    code_len = 0;
    size_t    num_lit  = 0;
    size_t    jump_off;
    size_t    slot_base;
    size_t    total;
    uint8_t  *tramp;

    if (p_hook) {
        p_hook->target    = NULL;
        p_hook->orig_func = NULL;
        p_hook->instr_len = 0;
    }

    if (!is_thumb && (tgt_addr & 3)) {
        so_loge("[SOLINKER] hook: target %p is not 4-byte aligned", (void *)tgt_addr);
        return -1;
    }

    int need_tramp = (p_hook != NULL);

    if (_plan_prologue(tgt_addr, is_thumb, need_tramp, recs, &num, &code_len) < 0) {
        return -1;
    }

    for (size_t i = 0; i < num; i++) {
        if (recs[i].kind != _IR_NONE && recs[i].kind != _IR_PC_ADD_T16) {
            recs[i].slot = (uint8_t)(num_lit++);
        }
    }

    if (need_tramp) {
        jump_off  = ALIGN_UP(code_len, 4);
        slot_base = jump_off + 8;
        total     = slot_base + num_lit * 4;

        if (num_lit > SO_TRAMP_MAX_LIT || total > SO_TRAMP_SIZE) {
            so_loge("[SOLINKER] hook: trampoline of 0x%08x needs %u bytes (max %u)", tgt_addr, total, SO_TRAMP_SIZE);
            return -1;
        }

        tramp = (uint8_t *)_tramp_alloc(SO_TRAMP_SIZE);
        if (!tramp) {
            return -1;
        }
        memset(tramp, 0, SO_TRAMP_SIZE);

        if (_emit_trampoline(tramp, tgt_addr, is_thumb, recs, num, code_len, slot_base) < 0) {
            return -1;
        }
        so_cacheflush(tramp, SO_TRAMP_SIZE);
    } else {
        tramp = NULL; /* 永久替换：不生成跳板 */
    }

    if (p_hook) {
        p_hook->target    = (void *)(tgt_addr | is_thumb);
        p_hook->orig_func = (void *)((uintptr_t)tramp | is_thumb);
        memcpy(p_hook->orig_instr, (void *)tgt_addr, code_len);
        p_hook->instr_len = code_len;
    }

    size_t put_len;
    if (is_thumb) {
        if (tgt_addr & 2) {
            _put16((void *)(tgt_addr + 0), 0xBF00);     /* NOP：把 LDR.W 顶到 4 字节对齐处 */
            _put32((void *)(tgt_addr + 2), 0xF000F8DF); /* LDR.W PC, [PC] */
            _put32((void *)(tgt_addr + 6), (uint32_t)repl);
            put_len = 10;
        } else {
            _put32((void *)(tgt_addr + 0), 0xF000F8DF); /* LDR.W PC, [PC] */
            _put32((void *)(tgt_addr + 4), (uint32_t)repl);
            put_len = 8;
        }

        /* 剩余置 NOP */
        for (size_t i = put_len; i + 2 <= code_len; i += 2) {
            _put16((void *)(tgt_addr + i), 0xBF00); /* NOP */
        }
    } else {
        _put32((void *)(tgt_addr + 0), 0xE51FF004); /* LDR PC, [PC, #-4] */
        _put32((void *)(tgt_addr + 4), (uint32_t)repl);
        put_len = 8;

        /* 剩余置 NOP */
        for (size_t i = put_len; i + 4 <= code_len; i += 4) {
            _put32((void *)(tgt_addr + i), 0xE1A00000); /* MOV r0,r0 */
        }
    }

    so_cacheflush((void *)tgt_addr, code_len);

    so_logd("[SOLINKER] hook: %p->%p, trampoline=%p, %u bytes", target, repl, tramp, code_len);
    return 0;
}

int so_hook_got(void *slot, void *repl, so_hook_ref *p_hook) {
    if (!slot || !repl) {
        return -1;
    }

    size_t instr_len = sizeof(void *);

    if (p_hook) {
        p_hook->target    = slot;
        p_hook->orig_func = *(void **)slot;
        memcpy(p_hook->orig_instr, slot, instr_len);
        p_hook->instr_len = instr_len;
    }

    memcpy(slot, &repl, instr_len);
    so_cacheflush(slot, instr_len);
    return 0;
}

int so_unhook(so_hook_ref *p_hook) {
    if (!p_hook || !p_hook->target) {
        return -1;
    }

    uintptr_t dst = (uintptr_t)p_hook->target & ~(uintptr_t)1;

    memcpy((void *)dst, p_hook->orig_instr, p_hook->instr_len);
    so_cacheflush((void *)dst, p_hook->instr_len);

    p_hook->target    = NULL;
    p_hook->orig_func = NULL;

    return 0;
}

int so_inject_data(void *target, const void *repl, size_t len) {
    if (!target || !repl || len == 0) {
        return -1;
    }

    memcpy(target, repl, len);
    so_cacheflush(target, len);
    return 0;
}
