#ifndef JNI_CALL_ASM_H
#define JNI_CALL_ASM_H

// 类型标志（位）
#define JNI_RET_NONE       0
#define JNI_RET_INT        1
#define JNI_RET_FLOAT      2
#define JNI_RET_WIDE       4
#define JNI_RET_WIDE_INT   (JNI_RET_INT | JNI_RET_WIDE)
#define JNI_RET_WIDE_FLOAT (JNI_RET_FLOAT | JNI_RET_WIDE)

#ifdef __ARM_PCS_VFP
#define OFFSET_INT_REGS    0
#define OFFSET_FLOAT_REGS  16
#define OFFSET_STACK_BYTES 32
#define OFFSET_STACK_ARGS  36
#define OFFSET_FN_PTR      40
#define OFFSET_RET_KIND    44
#else
#define OFFSET_INT_REGS    0
#define OFFSET_STACK_BYTES 16
#define OFFSET_STACK_ARGS  20
#define OFFSET_FN_PTR      24
#define OFFSET_RET_KIND    28
#endif

#endif