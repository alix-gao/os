#################################################################
# copyright (c) 2018, gaocheng all rights reserved.
#
# file name  : lapic.s
# version    : 1.0
# description:
# author     : gaocheng
# date       : 2018-04-22
#################################################################

    .file   "lapic.c"

    .text
    .align 4

    .extern _main
.globl _lapic_timer_int,_lapic_cmci_int,_lapic_lint0_int,_lapic_lint1_int,_lapic_error_int,_lapic_pmc_int,_lapic_ts_int

_lapic_timer_int: #int 0x30, irq 0.
	jmp __end_of_lapic_0
    # prolog for call stack
    pushl %ebp
    movl %esp,%ebp
  __end_of_lapic_0:
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
    call _wrap_lapic_int
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

_lapic_cmci_int: #int 0x31, irq 1.
	jmp __end_of_lapic_1
    # prolog for call stack
    pushl %ebp
    movl %esp,%ebp
  __end_of_lapic_1:
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
    call _wrap_lapic_int
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

_lapic_lint0_int: #int 0x32, irq 2.
	jmp __end_of_lapic_2
    # prolog for call stack
    pushl %ebp
    movl %esp,%ebp
  __end_of_lapic_2:
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
    call _wrap_lapic_int
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

_lapic_lint1_int: #int 0x33, irq 3.
	jmp __end_of_lapic_3
    # prolog for call stack
    pushl %ebp
    movl %esp,%ebp
  __end_of_lapic_3:
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
    call _wrap_lapic_int
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

_lapic_error_int: #int 0x34, irq 4.
	jmp __end_of_lapic_4
    # prolog for call stack
    pushl %ebp
    movl %esp,%ebp
  __end_of_lapic_4:
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
    call _wrap_lapic_int
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

_lapic_pmc_int: #int 0x35, irq 5.
	jmp __end_of_lapic_5
    # prolog for call stack
    pushl %ebp
    movl %esp,%ebp
  __end_of_lapic_5:
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
    call _wrap_lapic_int
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

_lapic_ts_int: #int 0x36, irq 6.
	jmp __end_of_lapic_6
    # prolog for call stack
    pushl %ebp
    movl %esp,%ebp
  __end_of_lapic_6:
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
    call _wrap_lapic_int
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
