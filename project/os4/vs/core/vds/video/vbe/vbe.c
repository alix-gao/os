/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : vbe.c
 * version     : 1.0
 * description : Volume 3: System Programming Guide
 *               CHAPTER 17 MIXING 16-BIT AND 32-BIT CODE
 *               you will found some discription of vbe.pdf is fault and not clear.
 * author      : gaocheng
 * date        : 2011-01-01
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include <vbs.h>

#include "vbe.h"

/***************************************************************
 global variable declare
 ***************************************************************/
LOCALD os_u16 vbe_bios_data_sel = 0;
LOCALD os_u16 vbe_a0000_sel = 0;
LOCALD os_u16 vbe_b0000_sel = 0;
LOCALD os_u16 vbe_b8000_sel = 0;
LOCALD os_u16 vbe_c0000_sel = 0;
LOCALD os_u16 vbe_bios_code_sel = 0;
LOCALD os_u16 vbe_bios_ss_sel = 0;

/* 16位代码模式 */
struct ljmp_16_struct {os_u16 offset; os_u16 sel;};
struct ljmp_16_struct vbe_init_ljmp_16_ram;
struct ljmp_16_struct vbe_entry_ljmp_16_ram;

/* 32位代码模式 */
LOCALD struct ljmp_32_struct { os_u32 offset; os_u32 sel; } vbe_ljmp_32_ram;

LOCALD os_u32 vbe_esp;
LOCALD os_u16 vbe_ss;

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description :
 * history     : 2009-04-22
 ***************************************************************/
LOCALC os_void *lookup_pmid(os_u8 *addr)
{
    os_u32 i;

    for (i = 0; i < PM_BUFF_LEN - 4; i++) {
        if (('P' == *(addr + 0))
         && ('M' == *(addr + 1))
         && ('I' == *(addr + 2))
         && ('D' == *(addr + 3))) {
            return addr;
        }
        addr++;
    }
    return OS_NULL;
}

/* 1m以下静态内存地址 */
#define KSM_1M_ADDR (RM_CODE_STACK_SEG_NO * 0x10000)
#define KSM_1M_LEN (RM_DATA_SEG_NO * 0x10000)

/***************************************************************
 * description : 分配底端内存, 每次分配1个段
 * history     :
 ***************************************************************/
LOCALC os_u32 alloc_ksm_1m(os_void)
{
    GLOBALDIF const os_u32 section_addr = KSM_1M_ADDR;
    os_u32 addr;

    if (KSM_1M_LEN > section_addr) {
        addr = section_addr + 0x1000;
        return addr;
    }

    return 0;
}

LOCALC os_void vbe_fun_low(os_void) { cassert(OS_FALSE); }

/***************************************************************
 * description :
 * history     : 2009-04-22
 ***************************************************************/
LOCALC os_void use_vesa(os_void)
{
    os_u8 *pm_buff;
    struct pm_info_block *pmib;
    os_u8 *bios_data;
    os_u8 *bios_stack;
    os_u32 *os_gdt_addr = (os_u32 *)(GDT_ADDR);
    os_u16 ss_reg;

    /* 1. Allocate a protected mode buffer large enough to hold the entire BIOS image (32Kb normally). */
    /* 代码段必须使用1m以下, 否则ret时eip高地址将丢失 */
    pm_buff = (os_u8 *) alloc_ksm_1m();
    if (OS_NULL == pm_buff) {
        return;
    }

    /* 2. Copy the BIOS image from the C0000h physical memory region to the protected mode buffer. */
    mem_cpy(pm_buff, 0xc0000, 2 * PM_BUFF_LEN);

    /* 3. Scan the BIOS image for the PMlnfoBlock structure and check that the checksum for the block is 0. */
#if 1
{
    /******* ati stub start *******/
    /* push ax
       push ds
       mov ds, word ptr cs:0x54ea
       mov ax, ds
       mov word ptr ds:0x1bc4, ax
       mov word ptr ds:0x1bd4, ax
       mov word ptr ds:0x1be6, ax
       mov ds, word ptr cs:0x54e2
       mov word ptr ds:0x4aa, ax
       pop ds
       pop ax
       retf */
    GLOBALDIF const os_u32 ati_vesa_bios_init_code[] = {
        0x8e2e1e50, 0x8c54ea1e, 0x1bc4a3d8, 0xa31bd4a3,
        0x8e2e1be6, 0xa354e21e, 0x581f04aa, 0x11e852cb,
        0xc35aef00
    };

    /* cmp ah, 0xa0
       jz .+0xf449
       cmp ah, 0x4f
       jz .+2efa */
    GLOBALDIF const os_u32 ati_vesa_bios_entry_code[] = {
        0x0fa0fc80, 0x80f44984, 0x840f4ffc, 0xff2e2efa,
        0xeb0f6836, 0x80fdfb2e, 0x840fa0fc, 0x33e8f42b,
        0xe80f756e
    };

    /* push word ptr cs:0xf68
       jmp .+0x0005
       cmp al, 0x16
       jb .+0001
       cld
       xor ah, ah
       shl ax, 1
       xchg bx, ax
       jmp word ptr cs:[bx+0x54aa] ;bx is 4 */
    GLOBALDIF const os_u32 ati_vesa_bios_4a29_code[] = {
        0x6836ff2e, 0x2e05eb0f, 0x0f6636ff, 0x0172163c,
        0xe432fcc3, 0x2e93e0d1, 0x54aaa7ff, 0x571ed88b,
        0x52515356
    };

    /* function jmp table */
    GLOBALDIF const os_u32 ati_vesa_bios_54aa_code[] = {
        0x4b104a45, 0x4da14c77, 0x4de24ed8, 0x502d4f53
    };

    /* mov bx, ax
       push es
       push ds
       push di
       push si
       push dx
       push cx
       push bp
       push bx
       push cs
       pop ds
       mov cx, bx
       mov al, bh
       and al, 0x80
       and ch, 0x3f
       cmp cx, 0x0013
       jnbe .+0x0005
       call .+0x01c6 */
    GLOBALDIF const os_u32 ati_vesa_bios_4c77_code[] = {
        0x1e06d88b, 0x51525657, 0x1f0e5355, 0xc78acb8b,
        0xe5808024, 0x13f9833f, 0xc10a0577, 0xe800f5e9,
        0x4ee801c6, 0xff3c8301
    };

    /* push ax
       push bx
       mov bx, 0x0228
       call .+0x8010 */
    GLOBALDIF const os_u32 ati_vesa_bios_4e5f_code[] = {
        0x28bb5350, 0x8010e802, 0x66e85d24, 0x0b00bb07,
        0x0c0805e8, 0x075be801, 0x53c3585b, 0xb8274be8,
        0x035b0100
    };

    GLOBALDIF struct pm_info_block pmib_tmp = {0};

    pmib_tmp.signature[0] = 'P';
    pmib_tmp.signature[1] = 'M';
    pmib_tmp.signature[2] = 'I';
    pmib_tmp.signature[3] = 'D';
    pmib_tmp.entry_point = 0x1b21;
    pmib_tmp.pm_initialize = 0x54f0;
    pmib_tmp.bios_data_sel = 0x0000;
    pmib_tmp.a0000_sel = 0xa000;
    pmib_tmp.b0000_sel = 0xb000;
    pmib_tmp.b8000_sel = 0xb800;
    pmib_tmp.code_seg_sel = 0xc000;
    pmib_tmp.in_protect_mode = 0;
    pmib_tmp.checksum = 0;
#define PMIB_OFFSET 0x54da
    mem_cpy(pm_buff+PMIB_OFFSET, &pmib_tmp, sizeof(pmib_tmp));
    mem_cpy(pm_buff + pmib_tmp.pm_initialize, ati_vesa_bios_init_code, sizeof(ati_vesa_bios_init_code));
    mem_cpy(pm_buff + pmib_tmp.entry_point, ati_vesa_bios_entry_code, sizeof(ati_vesa_bios_entry_code));
    mem_cpy(pm_buff + 0x4a29, ati_vesa_bios_4a29_code, sizeof(ati_vesa_bios_4a29_code));
    mem_cpy(pm_buff + 0x54aa, ati_vesa_bios_54aa_code, sizeof(ati_vesa_bios_54aa_code));
    mem_cpy(pm_buff + 0x4c77, ati_vesa_bios_4c77_code, sizeof(ati_vesa_bios_4c77_code));
    mem_cpy(pm_buff + 0x4e5f, ati_vesa_bios_4e5f_code, sizeof(ati_vesa_bios_4e5f_code));
}
#endif

    pmib = lookup_pmid(pm_buff);
    if (OS_NULL == pmib) {
        return;
    }

    flog("pmib addr = %x\n", pmib);

    /* no check sum */

    /* 4. Allocate an empty (all zeros) block of memory for the BIOS data area emulation block that is at least 600h bytes in length. Create a new data selector that points to the start of this memory block and put the value of that selector into the BIOSDataSel field of the PMlnfoBlock. */
#define VBE_BIOS_DATA_EMUL_LEN 0x600
    bios_data = (os_u8 *)(os_u32) alloc_ksm(VBE_BIOS_DATA_EMUL_LEN);
    cassert(OS_NULL != bios_data);

    bios_data = (os_u8 *) KSM_1M_LEN;
    mem_set(bios_data, 0, VBE_BIOS_DATA_EMUL_LEN);

    vbe_bios_data_sel = alloc_gdt_item();
    if (0 == vbe_bios_data_sel) {
        return;
    }

    modify_16data_descriptor(vbe_bios_data_sel, bios_data, 0xffffffff);

    pmib->bios_data_sel = vbe_bios_data_sel * 8;

    /* 5. Create selectors that point to the A0000h, B0000h and B8000h physical memory locations (with lengths of 64Kb, 64Kb and 32Kb respectively). Put the values of these selectors into the A0000Se1, B0000Se1 and B8000Se1 fields of the PMlnfoBlock respectively. */
    vbe_a0000_sel = alloc_gdt_item();
    if (0 == vbe_a0000_sel) {
        return;
    }

    modify_16data_descriptor(vbe_a0000_sel, 0xa0000, 0xffffffff);

    pmib->a0000_sel = vbe_a0000_sel * 8;

    vbe_b0000_sel = alloc_gdt_item();
    if (0 == vbe_b0000_sel) {
        return;
    }

    modify_16data_descriptor(vbe_b0000_sel, 0xb0000, 0xffffffff);

    pmib->b0000_sel = vbe_b0000_sel * 8;

    vbe_b8000_sel = alloc_gdt_item();
    if (0 == vbe_b8000_sel) {
        return;
    }

    modify_16data_descriptor(vbe_b8000_sel, 0xb8000, 0x7fffffff);

    pmib->b8000_sel = vbe_b8000_sel * 8;

    /* 6. Create a read/write data selector that points to the loaded BIOS image and put the value into the CodeSegSel field of the PMlnfoBlock. */
    vbe_c0000_sel = alloc_gdt_item();
    if (0 == vbe_c0000_sel) {
        return;
    }

    modify_16data_descriptor(vbe_c0000_sel, pm_buff, 0xffffffff);

    pmib->code_seg_sel = vbe_c0000_sel * 8;

    /* 7. Set the InProtectMode field of the PMlnfoBlock to a '1' to indicate to the BIOS code that it is now running in protected mode. */
    pmib->in_protect_mode = 1;

    /* 8. Create a 16-bit code segment selector (execute and read permissions) that points to the start of the protected mode BIOS buffer you have allocated. This selector and the PMlnitialize entry point from the PMlnfoBlock provide a 16:16 or 16:32 (the offset must be extended to 32-bit under a 32-bit OS) far pointer to the protected mode BIOS initialization function. You should call this function first before calling any of the functions via the protected mode entry point. */
    vbe_bios_code_sel = alloc_gdt_item();
    if (0 == vbe_bios_code_sel) {
        return;
    }

    /* bug, execute and read permission */
    modify_16code_read_descriptor(vbe_bios_code_sel, pm_buff, 0xffffffff);

    /* 9. Create a 16-bit data segment selector that points to a region of memory at least 1024 bytes in size that will be used as the protected mode stack for calls to the protected mode BIOS code. This selector will have to be loaded into the SS selector before the call, and the SP register should be cleared to zero (i.e.: SS:0 points to the start of the 16-bit protected mode stack). The 16-bit protected mode stack is necessary so that if the BIOS is using the stack to maintain local variables at mntime, those variables can be correctly referenced via the 16-bit stack pointer. */
#define VBE_BIOS_STACK_LEN 1024
    bios_stack = (os_u8 *)(os_u32) alloc_ksm(VBE_BIOS_STACK_LEN);
    cassert(OS_NULL != bios_stack);

    vbe_bios_ss_sel = alloc_gdt_item();
    if (0 == vbe_bios_ss_sel) {
        return;
    }

    /* 注意16位模式下堆栈的环回效应, 减少4个字节以便从高地址开始 */
    bios_stack = (os_u8 *)((pointer)(bios_stack - 4) & 0xffff0000);

    modify_16data_descriptor(vbe_bios_ss_sel, bios_stack, 0xffff);

    /* 10. Using the selector created in the above step and the protected mode entry point from the PMlnfoBlock, you now have a 16:16 or 16:32 (the offset must be extended to 32- bit under a 32-bit OS) far pointer to the protected mode BIOS, which you can simply call directly. Note that you do need to ensure that the stack is switched to the 16-bit protected mode stack before the protected mode BIOS is called. */

    /* 使用16位堆栈段 */
#if 0
    __asm__ __volatile__("movw %%ax,%%ss\n\t"
                         "xorl %%esp,%%esp\n\t"
                         ::"a"(vbe_bios_ss_sel * 8));
#endif

    /* 调用初始化函数 */
    vbe_init_ljmp_16_ram.sel = vbe_bios_code_sel * 8;
    vbe_init_ljmp_16_ram.offset = pmib->pm_initialize;

    vbe_entry_ljmp_16_ram.sel = vbe_bios_code_sel * 8;
    vbe_entry_ljmp_16_ram.offset = pmib->entry_point;

    vbe_ljmp_32_ram.sel = 0; /* clear */
    vbe_ljmp_32_ram.sel = GDT_CODE16_INDEX * 8;
    vbe_ljmp_32_ram.offset = 0;

    /* 修改16位代码段描述符 */
    *(os_gdt_addr + GDT_CODE16_INDEX * 2 + 1) = 0x00009a00 | (VBE_REAL_CODE_64K_NO);

    ss_reg = vbe_bios_ss_sel * 8;

    __asm__ __volatile__("pushl %%eax\n\t"
                         "pushl %%ebx\n\t"
                         "pushl %%ecx\n\t"
                         "pushl %%edx\n\t"
                         "pushl %%edi\n\t"
                         "pushl %%esi\n\t"
                         "pushl %%ebp\n\t"
                         "push %%ds\n\t"
                         "push %%es\n\t"
                         "push %%fs\n\t"
                         "push %%gs\n\t"
                         /* 保存ss和sp到全局变量中 */
                         "movw %%ss,%0\n\t"
                         "movl %%esp,%1\n\t"
                         "movw %2,%%ss\n\t"
                         "movl $0xfffffffc,%%esp\n\t"
                         "movb $0x4f,%%ah\n\t"
                         /* 不再使用"ljmp $"asm_str(GDT_CODE16_INDEX * 8)",$1f-"asm_str(OS_IMAGE_ADDR)"\n\t"这种方式 */
                         /* 这种方式依赖于指令地址, 例如1f为81xxxxH, 就会出现问题 */
                         /* 因为标号不能在c语言和汇编之间传递, 因此使用内存变量 */
                         "movl $1f,%%eax\n\t"
                         /* do not exceed limit */
                         "movw %%ax,%3\n\t"
                         "ljmp *(%3)\n\t"
                         "1:\n\t"
                         /* 17.4.5. Writing Interface Procedures, use 16bit call for 16bit ret */
                         /* 因为堆栈段已经变化, 因此不能使用堆栈变量 ljmp_struct */
                         /* 目前使用ds寄存器, 但如果前面增加了跳转到1m的功能会导致, 该指令会使用堆栈寄存器 */
                         /* 但实际上可以暂时不跳转到1m内存下的代码中,因为长跳转后的返回会自动回到1m内存代码 */
                         //".byte 0x66\n\t" /* 32位代码段使用 */
                         /* call init */
                         ".byte 0x67\n\t" /* 16位代码段使用 */
                         "lcall *(%4)\n\t"
#if 0
                         /* do not set D11, es:di will be ignored. */
                         ".byte 0x66,0x67\n\t"
                         "movl $0x2000,%%eax\n\t"
                         "movw %%ax,%%es\n\t"
                         ".byte 0x66,0x67\n\t"
                         "movl $0,%%edi\n\t"
#endif
                         /* set mode */
                         "movb $0x4f,%%ah\n\t" /* use vbe */
                         "movb $0x02,%%al\n\t" /* set vesa mode */
#define RESOLUTION_1024_768_LFM 0x4118
                         ".byte 0x66\n\t"
                         "movl $"asm_str(RESOLUTION_1024_768_LFM)",%%ebx\n\t"
#if 0
                         ".byte 0x67\n\t"
                         "lcall *(%5)\n\t"
#endif
                         /* 17.4.1. Code-Segment Pointer Size, use memory under 1m */
                         ".byte 0x66\n\t"
                         ".byte 0x67\n\t"
                         "ljmp $"asm_str(GDT_CODE32_INDEX * 8)",$2f\n\t"
                         "2:\n\t"
                         /* 恢复堆栈 */
                         "movw %0,%%ss\n\t"
                         "movl %1,%%esp\n\t"
                         "pop %%gs\n\t"
                         "pop %%fs\n\t"
                         "pop %%es\n\t"
                         "pop %%ds\n\t"
                         "popl %%ebp\n\t"
                         "popl %%esi\n\t"
                         "popl %%edi\n\t"
                         "popl %%edx\n\t"
                         "popl %%ecx\n\t"
                         "popl %%ebx\n\t"
                         "popl %%eax\n\t"
                         "nop"
                         ::"m"(vbe_ss),"m"(vbe_esp),"m"(ss_reg),"m"(vbe_ljmp_32_ram),"m"(vbe_init_ljmp_16_ram),"m"(vbe_entry_ljmp_16_ram)
                         :"eax");

    /* 修改16位描述符 */
    *(os_gdt_addr + GDT_CODE16_INDEX * 2 + 1) = 0x00009a00;

    /* 设置vbe参数 */

    /* 调用设置显卡模式功能 */

    flog("vbe\n");
}

LOCALC os_void vbe_fun_high(os_void) { cassert(OS_FALSE); }

/***************************************************************
 * description :
 * history     : 2009-04-22
 ***************************************************************/
LOCALC os_void move_veas_1k(os_void)
{
    os_u8 *src = (os_u8 *)(use_vesa);
    os_u8 *dest = (os_u8 *)((VBE_REAL_CODE_64K_NO << 16) + (0xffff & (pointer)use_vesa));

    mem_cpy(dest, src, (pointer)(vbe_fun_high - vbe_fun_low));
}

/***************************************************************
 * description :
 * history     : 2009-04-22
 ***************************************************************/
os_void init_vbe(os_void)
{
    move_veas_1k();

    use_vesa();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_vbe(os_u32 argc, os_u8 *argv[])
{
    lock_t eflag;
    os_u16 save;

    if (0 != argc) {
        return OS_FAIL;
    }

    lock_int(eflag);

    save = close_pic(0xffff);
    init_vbe();
    open_pic(save);

    unlock_int(eflag);

    return OS_SUCC;
}
