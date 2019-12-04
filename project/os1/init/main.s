    .file    "0.cpp"
headoffset=0x800
systemstack=0x9000
page0=0x10000
page1=0x11000
page2=0x12000
page3=0x13000
gdt=0x110000
#this program must begin with _main if i want to use bloodsheld.
#so this _main function will replace head.
#and the real _main function i want to rename.
    .def    ___main;    .scl    2;    .type    32;    .endef
    .text
    .align 2
    .p2align 4,,15 #this will be remove by-fomit-frame-pointer
                   #which is cann't be used with -O2.
    .extern __Z9real_mainv
.globl _main,LC01 #-----------------
    .def    _main;    .scl    2;    .type    32;    .endef
       #.org 0x800 #use by this.-------------------------
_main:
    jmp passdata_head
    LC01:
    .ascii "devcpp\0"
   gdt_pseudo:
    .word 0xffff
    .long gdt
  passdata_head:
    cli
    #init gdt
    xorl %eax,%eax
    movl $gdt,%ebp
    #32 bit code
    movl $0x0000ffff,%eax
    movl %eax,%ds:0x08(%ebp)
    movl $0x00cf9a00,%eax
    movl %eax,%ds:0x0c(%ebp)
    #32 bit data
    movl $0x0000ffff,%eax
    movl %eax,%ds:0x10(%ebp)
    movl $0x00cf9200,%eax
    movl %eax,%ds:0x14(%ebp)
    #16 bit code
    movl $0x0000ffff,%eax
    movl %eax,%ds:0x18(%ebp)
    movl $0x00009800,%eax
    movl %eax,%ds:0x1c(%ebp)
    #16 bit data
    movl $0x0000ffff,%eax
    movl %eax,%ds:0x20(%ebp)
    movl $0x00009200,%eax
    movl %eax,%ds:0x24(%ebp)

	movl $gdt_pseudo,%ebp
    lgdt (%ebp)

    ljmp $0x8,$fresh_regs
  fresh_regs:
    movl $16,%eax
    movl %eax,%ds
    movl %eax,%es
    movl %eax,%fs
    movl %eax,%gs
    movl %eax,%ss

    movl $0x0b8000+(80*10+2)*2,%edi
    mov $0x02,%ah #,01h ;color is blue.
    mov $'t',%al
    mov %ax,%gs:(%edi)
    #####
    #;----
    #movl $0x0b8000+(80*11+10)*2,%edi #not $0b8000H+(...)
    #mov $0x01,%ah #,01h ;color is blue.
    #mov $'d',%al
    #mov %ax,(%edi)
    movl $0x1000000,%esp
    jmp __Z12genuine_mainv #call and jmp is different.
#ab: jmp ab
    #######

