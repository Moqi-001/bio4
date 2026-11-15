#include "JniCall_asm.h"

.text
.arm
.align 2

.extern JniMarshalCallArguments

.global JniForwardMethod
.type JniForwardMethod, %function

JniForwardMethod:
    push {r4, r5, fp, lr} @ 保存 r4,r5,fp,lr
    mov fp, sp            @ 保存 sp

    mov r4, r0 @ r4 = frame
    mov r5, r1 @ r5 = result

@ === 分配栈空间 ===
    @ 获取栈大小
    ldr ip, [r4, #OFFSET_STACK_BYTES] @ ip = stackBytes
    cmp ip, #0                        @ stackBytes ?== 0
    beq 10f                           @ 跳过分配

    @ 分配栈空间
    sub sp, sp, ip                    @ 分配栈空间
    str sp, [r4, #OFFSET_STACK_ARGS]  @ stackArgs = sp

@ === 打包参数 ===
10:
    @ 调用参数打包函数
    mov r0, r4                  @ r0 = args
    blx JniMarshalCallArguments @ 调用函数 JniMarshalCallArguments

    @ 检查返回值
    cmp r0, #0  @ result ?== JNI_OK
    bne 0f      @ 失败跳出

@ === 调用目标函数 ===
    @ 加载整数寄存器参数
    add ip, r4, #OFFSET_INT_REGS @ ip    = intRegs
    ldm ip, {r0-r3}              @ r0-r3 = intRegs[0]-[3]

#ifdef __ARM_PCS_VFP
    @ 加载浮点寄存器参数
    ldr ip, [r4, #OFFSET_FLOAT_REGS] @ ip    = floatRegs
    vldm ip, {s0-s3}                 @ s0-s3 = floatRegs[0]-[3]
#endif

    @ 调用目标函数
    ldr lr, [r4, #OFFSET_FN_PTR] @ lr = fnPtr
    blx lr                       @ 调用函数 fnPtr

@ === 处理返回值 ===
    cmp r5, #0 @ result ?== NULL
    beq 0f

    ldr ip, [r4, #OFFSET_RET_KIND] @ ip = retKind

    cmp ip, #JNI_RET_NONE
    beq 0f

    cmp ip, #JNI_RET_INT
    beq 1f

    cmp ip, #JNI_RET_FLOAT
    beq 2f

    cmp ip, #JNI_RET_WIDE_INT
    beq 3f

    cmp ip, #JNI_RET_WIDE_FLOAT
    beq 4f

0:  @ 恢复并返回
    mov sp, fp           @ 恢复 sp
    pop {r4, r5, fp, pc} @ 恢复 r4,r5,fp 并返回   

#ifdef __ARM_PCS_VFP
1:  @ JNI_RET_INT
    str r0, [r5]
    b 0b

2:  @ JNI_RET_FLOAT
    vstr s0, [r5]
    b 0b

3:  @ JNI_RET_WIDE_INT
    str r0, [r5]
    str r1, [r5, #4]
    b 0b

4:  @ JNI_RET_WIDE_FLOAT
    vstr s0, [r5]
    vstr s1, [r5, #4]
    b 0b

#else
1:  @ JNI_RET_INT
2:  @ JNI_RET_FLOAT
    str r0, [r5]
    b 0b

3:  @ JNI_RET_WIDE_INT
4:  @ JNI_RET_WIDE_FLOAT
    str r0, [r5]
    str r1, [r5, #4]
    b 0b

#endif

.size JniForwardMethod, . - JniForwardMethod
