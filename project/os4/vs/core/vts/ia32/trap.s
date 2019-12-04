#################################################################
# copyright (c) 2009, gaocheng all rights reserved.
#
# file name  : trap.s
# version    : 1.0
# description:
# author     : gaocheng
# date       : 2009-04-22
#################################################################

    .file   "trap.c"

    .text
    .align 4

    # macro define
    # for example, stack_pointer = 0x500000

    .extern _main
.globl _divide_error,_debug,_nmi,_int3,_overflow,_bounds,_invalid_op,_device_not_available
.globl _double_fault,_coprocessor_segment_overrun,_invalid_tss,_segment_not_present,_stack_segment,_general_protection,_page_fault,_reserved
.globl _x87_fpu_floating_point_error,_alignment_check,_machine_check,_simd_floating_point_exception

__trap_wrap:
    jmp __end_of_prolog
    # prolog for call stack
    pushl %ebp
    movl %esp,%ebp
  __end_of_prolog:
    # save context
    xchgl %eax,4(%esp) # save parameter code into eax
    xchgl %ebx,(%esp) # save parameter id into eax
    pushl %ecx
    pushl %edx
    pushl %edi
    pushl %esi
    pushl %ebp
    push %ds
    push %es
    push %fs
    push %gs
    # handle, stdcall
    push %eax # os_u32 code
    push %ebx # os_u32 id
    call _trap_wrap_handle@8
    # recover context
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

_divide_error: #int 0
    pushl $0
    pushl $0
    jmp __trap_wrap

_debug: #int 1
    # Note that the POPF, POPFD, and IRET instructions do not transfer the RF image into the EFLAGS register.
    # debugger software must set this flag in the EFLAGS image on the stack just prior to returning to the interrupted program with the IRETD instruction.
    # 8(%esp):eflag
    # 4(%esp):cs
    # 0(%esp):eip
    xorl $0x00010000,%ss:8(%esp)
    pushl $0
    pushl $1
    jmp __trap_wrap

_nmi: #int 2
    pushl $0
    pushl $2
    jmp __trap_wrap

_int3: #int 3
    pushl $0
    pushl $3
    jmp __trap_wrap

_overflow: #int 4
    pushl $0
    pushl $4
    jmp __trap_wrap

_bounds: #int 5
    pushl $0
    pushl $5
    jmp __trap_wrap

_invalid_op: #int 6
    pushl $0
    pushl $6
    jmp __trap_wrap

_device_not_available: #int 7
    pushl $0
    pushl $7
    jmp __trap_wrap

_double_fault: #int 8
# Note that the error code is not popped when the IRET instruction is executed to return from an exception handler,
# so the handler must remove the error code before executing a return.
    pushl $8
	jmp __trap_wrap

_coprocessor_segment_overrun: #int 9
    pushl $0
    pushl $9
    jmp __trap_wrap

_invalid_tss: #int 10
    pushl $0
    pushl $10
    jmp __trap_wrap

_segment_not_present: #int 11
    pushl $0
    pushl $11
    jmp __trap_wrap

_stack_segment: #int 12
    pushl $0
    pushl $12
    jmp __trap_wrap

_general_protection: #int 13
    pushl $0
    pushl $13
    jmp __trap_wrap

_page_fault: #int 14
    pushl $0
    pushl $14
    jmp __trap_wrap

_reserved: #int 15
    pushl $0
    pushl $15
    jmp __trap_wrap

_x87_fpu_floating_point_error: #int 16
    pushl $0
    pushl $16
    jmp __trap_wrap

_alignment_check: #int 17
    # error code is zero
    pushl $17
    jmp __trap_wrap

_machine_check: #int 18
    pushl $0
    pushl $18
    jmp __trap_wrap

_simd_floating_point_exception: #int 19
    pushl $0
    pushl $19
    jmp __trap_wrap

