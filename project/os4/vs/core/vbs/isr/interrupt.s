#################################################################
# copyright (c) 2009, gaocheng all rights reserved.
#
# file name  : int.s
# version    : 1.0
# description: 中断响应
# author     : gaocheng
# date       : 2009-04-22
#################################################################

    .file   "int.c"

    .text
    .align 4

    # macro define
    # for example, stack_pointer = 0x500000

    .extern _main
.globl _system_timer_int,_keyboard_int,_irq2_int,_irq3_int
.globl _irq4_int,_irq5_int,_irq6_int,_irq7_int
.globl _real_time_clock_int,_irq9_int,_irq10_int,_irq11_int
.globl _ps2_mouse,_irq13_int,_harddisk_int,_irq15_int

# 中断发生时, 硬件完成如下功能:
# (1) 取得中断类型号
# (2) 标志寄存器压栈
# (3) 禁止外部中断和单部中断(IF和TF, 因此不能单步跟踪)
# (4) 当前线程的下一条指令地址入栈(CS和IP)
# (5) 根据中断类型号从中断向量表处取中断服务程序入口地址
# (6) 执行中断服务例程
_system_timer_int: #int 0x20, irq 0.
	jmp __end_of_prolog_0
    # prolog for call stack
    pushl %ebp
    movl %esp,%ebp
  __end_of_prolog_0:
    #save context
    pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %edi
    pushl %esi
    pushl %ebp
    push %ds
    push %es
    push %fs
    push %gs
    #handle __cdecl function
    pushl $0x0
    call _wrap_int
    addl $0x4,%esp
    #recover context
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popl %ebp
    popl %esi
    popl %edi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    iret

_keyboard_int: #int 0x21, irq 1.
    jmp __end_of_prolog_1
    # prolog for call stack
    pushl %ebp
    movl %esp,%ebp
  __end_of_prolog_1:
    #save context
    pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %edi
    pushl %esi
    pushl %ebp
    push %ds
    push %es
    push %fs
    push %gs
    #handle __cdecl function
    pushl $0x1
    call _wrap_int
    addl $0x4,%esp
    #recover context
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popl %ebp
    popl %esi
    popl %edi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    iret

_irq2_int: #int 0x22, irq 2.
    jmp __end_of_prolog_2
    # prolog for call stack
    pushl %ebp
    movl %esp,%ebp
  __end_of_prolog_2:
    #save context
    pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %edi
    pushl %esi
    pushl %ebp
    push %ds
    push %es
    push %fs
    push %gs
    #handle __cdecl function
    pushl $0x2
    call _wrap_int
    addl $0x4,%esp
    #recover context
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popl %ebp
    popl %esi
    popl %edi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    iret

_irq3_int: #int 0x23, irq 3.
    jmp __end_of_prolog_3
    # prolog for call stack
    pushl %ebp
    movl %esp,%ebp
  __end_of_prolog_3:
    #save context
    pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %edi
    pushl %esi
    pushl %ebp
    push %ds
    push %es
    push %fs
    push %gs
    #handle __cdecl function
    pushl $0x3
    call _wrap_int
    addl $0x4,%esp
    #recover context
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popl %ebp
    popl %esi
    popl %edi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    iret

_irq4_int: #int 0x24, irq 4.
    jmp __end_of_prolog_4
    # prolog for call stack
    pushl %ebp
    movl %esp,%ebp
  __end_of_prolog_4:
    #save context
    pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %edi
    pushl %esi
    pushl %ebp
    push %ds
    push %es
    push %fs
    push %gs
    #handle __cdecl function
    pushl $0x4
    call _wrap_int
    addl $0x4,%esp
    #recover context
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popl %ebp
    popl %esi
    popl %edi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    iret

_irq5_int: #int 0x25, irq 5.
    jmp __end_of_prolog_5
    # prolog for call stack
    pushl %ebp
    movl %esp,%ebp
  __end_of_prolog_5:
    #save context
    pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %edi
    pushl %esi
    pushl %ebp
    push %ds
    push %es
    push %fs
    push %gs
    #handle __cdecl function
    pushl $0x5
    call _wrap_int
    addl $0x4,%esp
    #recover context
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popl %ebp
    popl %esi
    popl %edi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    iret

_irq6_int: #int 0x26, irq 6.
    jmp __end_of_prolog_6
    # prolog for call stack
    pushl %ebp
    movl %esp,%ebp
  __end_of_prolog_6:
    #save context
    pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %edi
    pushl %esi
    pushl %ebp
    push %ds
    push %es
    push %fs
    push %gs
    #handle __cdecl function
    pushl $0x6
    call _wrap_int
    addl $0x4,%esp
    #recover context
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popl %ebp
    popl %esi
    popl %edi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    iret

_irq7_int: #int 0x27, irq 7.
    jmp __end_of_prolog_7
    # prolog for call stack
    pushl %ebp
    movl %esp,%ebp
  __end_of_prolog_7:
    #save context
    pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %edi
    pushl %esi
    pushl %ebp
    push %ds
    push %es
    push %fs
    push %gs
    #handle __cdecl function
    pushl $0x7
    call _wrap_int
    addl $0x4,%esp
    #recover context
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popl %ebp
    popl %esi
    popl %edi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    iret

_real_time_clock_int: #int 0x28, irq 8.
    jmp __end_of_prolog_8
    # prolog for call stack
    pushl %ebp
    movl %esp,%ebp
  __end_of_prolog_8:
    #save context
    pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %edi
    pushl %esi
    pushl %ebp
    push %ds
    push %es
    push %fs
    push %gs
    #handle __cdecl function
    pushl $0x8
    call _wrap_int
    addl $0x4,%esp
    #recover context
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popl %ebp
    popl %esi
    popl %edi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    iret

_irq9_int: #int 0x29, irq 9.
    jmp __end_of_prolog_9
    # prolog for call stack
    pushl %ebp
    movl %esp,%ebp
  __end_of_prolog_9:
    #save context
    pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %edi
    pushl %esi
    pushl %ebp
    push %ds
    push %es
    push %fs
    push %gs
    #handle __cdecl function
    pushl $0x9
    call _wrap_int
    addl $0x4,%esp
    #recover context
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popl %ebp
    popl %esi
    popl %edi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    iret

_irq10_int: #int 0x2a, irq 10.
    jmp __end_of_prolog_10
    # prolog for call stack
    pushl %ebp
    movl %esp,%ebp
  __end_of_prolog_10:
    #save context
    pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %edi
    pushl %esi
    pushl %ebp
    push %ds
    push %es
    push %fs
    push %gs
    #handle __cdecl function
    pushl $0xa
    call _wrap_int
    addl $0x4,%esp
    #recover context
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popl %ebp
    popl %esi
    popl %edi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    iret

_irq11_int: #int 0x2b, irq 11.
    jmp __end_of_prolog_11
    # prolog for call stack
    pushl %ebp
    movl %esp,%ebp
  __end_of_prolog_11:
    #save context
    pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %edi
    pushl %esi
    pushl %ebp
    push %ds
    push %es
    push %fs
    push %gs
    #handle __cdecl function
    pushl $0xb
    call _wrap_int
    addl $0x4,%esp
    #recover context
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popl %ebp
    popl %esi
    popl %edi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    iret

_ps2_mouse: #int 0x2c, irq 12.
    jmp __end_of_prolog_12
    # prolog for call stack
    pushl %ebp
    movl %esp,%ebp
  __end_of_prolog_12:
    #save context
    pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %edi
    pushl %esi
    pushl %ebp
    push %ds
    push %es
    push %fs
    push %gs
    #handle __cdecl function
    pushl $0xc
    call _wrap_int
    addl $0x4,%esp
    #recover context
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popl %ebp
    popl %esi
    popl %edi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    iret

_irq13_int: #int 0x2d, irq 13.
    jmp __end_of_prolog_13
    # prolog for call stack
    pushl %ebp
    movl %esp,%ebp
  __end_of_prolog_13:
    #save context
    pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %edi
    pushl %esi
    pushl %ebp
    push %ds
    push %es
    push %fs
    push %gs
    #handle __cdecl function
    pushl $0xd
    call _wrap_int
    addl $0x4,%esp
    #recover context
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popl %ebp
    popl %esi
    popl %edi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    iret

_harddisk_int: #int 0x2e, irq 14.
    jmp __end_of_prolog_14
    # prolog for call stack
    pushl %ebp
    movl %esp,%ebp
  __end_of_prolog_14:
    #save context
    pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %edi
    pushl %esi
    pushl %ebp
    push %ds
    push %es
    push %fs
    push %gs
    #handle __cdecl function
    pushl $0xe
    call _wrap_int
    addl $0x4,%esp
    #recover context
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popl %ebp
    popl %esi
    popl %edi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    iret

_irq15_int: #int 0x2f, irq 15.
    jmp __end_of_prolog_15
    # prolog for call stack
    pushl %ebp
    movl %esp,%ebp
  __end_of_prolog_15:
    #save context
    pushl %eax
    pushl %ebx
    pushl %ecx
    pushl %edx
    pushl %edi
    pushl %esi
    pushl %ebp
    push %ds
    push %es
    push %fs
    push %gs
    #handle __cdecl function
    pushl $0xf
    call _wrap_int
    addl $0x4,%esp
    #recover context
    pop %gs
    pop %fs
    pop %es
    pop %ds
    popl %ebp
    popl %esi
    popl %edi
    popl %edx
    popl %ecx
    popl %ebx
    popl %eax
    iret

