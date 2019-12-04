#################################################################
# copyright (c) 2009, gaocheng all rights reserved.
#
# file name  : crt.s
# version    : 1.0
# description: c run-time
# author     : gaocheng
# date       : 2009-04-22
#################################################################

    .file   "main.c"
    .def    ___main;    .scl    2;  .type   32; .endef
    #.text
    .section .boot,"x"

    # macro define
    stack_pointer = 0x1000000

    .extern _main

    #.org 0x1000000

.globl _main
    .def    _main; .scl    2;  .type   32; .endef
_main:

    cli
    jmp code_area
    nop

    # os info
    data_area:
    .ascii "cos32\0"

  code_area:
    movw $32, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %fs
    movw %ax, %gs
    movw %ax, %ss

    # set stack
    movl $stack_pointer, %esp

    # goto c function
    jmp _os_main

