/***************************************************************
 * copyright (c) 2011, gaocheng
 * all rights reserved.
 *
 * file name   : vesa.c
 * version     : 1.0
 * description : 显卡支持的所有显示模式
 * author      : gaocheng
 * date        : 2011-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include "vesa.h"
#include "vesa_bios.h"

/***************************************************************
 global variable declare
 ***************************************************************/

/***************************************************************
 * description :
 ***************************************************************/
struct pseudo_struct {
    os_u16 reserve;
    os_u16 limit;
    os_u32 base;
};

/* gdtr */
LOCALD const struct pseudo_struct gdt_pseudo = { 0, 0xffff, GDT_ADDR };
/* idtr */
LOCALD const struct pseudo_struct idt_pseudo = { 0, 0xffff, IDT_ADDR };
/* real mode */
LOCALD const struct pseudo_struct bios_pseudo = { 0, 0x3ff, 0 };
LOCALD struct vesa_ljmp_32_struct { os_u32 offset; os_u32 sel; } vesa_ljmp_32_ram;
LOCALD struct vesa_ljmp_16_struct { os_u16 offset; os_u16 sel; } vesa_ljmp_16_ram;
LOCALD os_u32 vesa_gdtr_addr = 0;
LOCALD os_u32 vesa_idtr_addr = 0;
LOCALD os_u32 vesa_page_dir_addr = 0;

LOCALD struct VbeInfoBlock vbe_info_block = { 0 };

/***************************************************************
 function declare
 ***************************************************************/

/***************************************************************
 * description : 获取显卡支持的所有模式信息
 * history     :
 ***************************************************************/
LOCALC struct segment_offset BIOS_CODE get_VbeInfoBlock(struct VbeInfoBlock *vib)
{
    os_u32 biosr_addr = (pointer) &bios_pseudo.limit;
    os_u32 *os_gdt_addr = (os_u32 *)(GDT_ADDR);
    lock_t eflag;
    os_u32 esp_save = 0;
    os_u16 ss_save = 0;
    struct segment_offset start_addr = {VESA_REAL_DATA_OFF, VESA_REAL_DATA_SEG};

    /* 修改16位描述符 */
    *(os_gdt_addr + GDT_CODE16_INDEX * 2 + 1) = 0x00009a00 | (VESA_REAL_CODE_64K_NO);

    vesa_ljmp_32_ram.sel = 0;
    vesa_ljmp_32_ram.sel = GDT_CODE16_INDEX * 8;
    vesa_ljmp_32_ram.offset = 0;

    vesa_ljmp_16_ram.sel = VESA_REAL_CODE_SEG;
    vesa_ljmp_16_ram.offset = 0;

    vesa_gdtr_addr = (pointer) &gdt_pseudo.limit;
    vesa_idtr_addr = (pointer) &idt_pseudo.limit;

    vesa_page_dir_addr = PAGE_DIR_ADDR;

    /* 保存标志寄存器 */
    save_eflag(eflag);
    //coms_write(0x8f, 0x00);
    cli();

    /* All the code that is executed must be in a single page
       and the linear addresses in that page must be identity mapped to physical addresses. */
    cassert(virt_is_abs(IDT_ADDR));
    cassert(virt_is_abs(GDT_ADDR));
    do {
        os_u32 sp;
        __asm__ __volatile__("movl %%esp,%%eax\n\t"
                             "movl %%eax,%0\n\t"
                             :"=m"(sp)::"eax");
        cassert(virt_is_abs(sp));
    } while (0);
    if (current_task_handle()) {
        cassert(virt_is_abs(get_task_ldt(current_task_handle())));
    }

    /* check any global variable to make sure linear address == physical address */
    cassert(virt_is_abs((pointer) &gdt_pseudo));

    do {
        os_u16 gs, ds;
        __asm__ __volatile__("movw %%ds,%%ax\n\t"
                             "movw %%ax,%0\n\t"
                             "movw %%gs,%%ax\n\t"
                             "movw %%ax,%1\n\t"
                             :"=m"(ds),"=m"(gs)::"eax");
        cassert(ds == gs); /* make sure gs is a backup of ds */
    } while (0);

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
                         "movl %%cr0,%%eax\n\t"
                         "andl $0x7fffffff,%%eax\n\t"
                         "movl %%eax,%%cr0\n\t"
                         "movl %2,%%eax\n\t"
                         "lidt %%gs:(%%eax)\n\t"
                         "ljmp $"asm_str(GDT_CODE32_INDEX * 8)",$1f\n\t"
                         "1:\n\t"
                         //"ljmp $"asm_str(GDT_CODE16_INDEX * 8)",$2f-"asm_str(OS_IMAGE_ADDR)"\n\t"
                         "movl $2f,%%eax\n\t"
                         /* do not exceed limit */
                         "movw %%ax,%3\n\t"
                         "ljmp *(%3)\n\t"
                         "2:\n\t"
                         "movl %%cr0,%%eax\n\t"
                         ".byte 0x66,0x67\n\t"
                         "andl $0xfffffffe,%%eax\n\t"
                         "movl %%eax,%%cr0\n\t"
                         //".byte 0x66,0x67\n\t"
                         //"ljmp $"asm_str(VESA_REAL_CODE_SEG)",$3f-"asm_str(OS_IMAGE_ADDR)"\n\t"
                         ".byte 0x66\n\t"
                         "movl $3f,%%eax\n\t"
                         ".byte 0x67\n\t"
                         "movl %%eax,%4\n\t"
                         ".byte 0x67\n\t"
                         "ljmp *(%4)\n\t"
                         "3:\n\t"
                         ".byte 0x66\n\t"
                         "movl $"asm_str(VESA_REAL_MODE_STACK)",%%eax\n\t"
                         "movw %%ax,%%ss\n\t"
                         ".byte 0x66\n\t"
                         "movl $0xfffc,%%esp\n\t"
                         /* bios function */
                         ".byte 0x66,0x67\n\t"
                         "movl $"asm_str(VESA_REAL_DATA_OFF)",%%edi\n\t"
                         /* 该指令会使用eax寄存器 */
                         ".byte 0x66\n\t"
                         "movl $"asm_str(VESA_REAL_DATA_SEG)",%%eax\n\t"
                         "movw %%ax,%%es\n\t"
                         /* 该指令必须在后面 */
                         ".byte 0x66,0x67\n\t"
                         "movl $0x4f00,%%eax\n\t"
                         "int $0x10\n\t"
                         /* 使用了新的堆栈, 必须使用全局变量 */
                         ".byte 0x66,0x67\n\t"
                         "movl %%gs:%5,%%eax\n\t" /* ds is freshed, use gs */
                         ".byte 0x66,0x67\n\t"
                         "lgdt %%gs:(%%eax)\n\t" /* pointer of pointer */
                         ".byte 0x66\n\t"
                         "movl %%cr0,%%eax\n\t"
                         ".byte 0x66\n\t"
                         "orl $1,%%eax\n\t"
                         ".byte 0x66\n\t"
                         "movl %%eax,%%cr0\n\t"
                         ".byte 0x66\n\t"
                         "ljmp $"asm_str(GDT_CODE32_INDEX * 8)",$4f\n\t"
                         "4:\n\t"
                         "movw $"asm_str(GDT_DATA32_INDEX * 8)",%%ax\n\t"
                         "movw %%ax,%%ds\n\t"
                         "movw %%ax,%%es\n\t"
                         "movw %%ax,%%fs\n\t"
                         "movw %%ax,%%gs\n\t"
                         "movw %%ax,%%ss\n\t"
                         "movl %1,%%esp\n\t"
                         "movl %6,%%eax\n\t"
                         "lidt %%gs:(%%eax)\n\t"
                         "movl %7,%%eax\n\t"
                         "movl %%eax,%%cr3\n\t"
                         "movl %%cr0,%%eax\n\t"
                         "orl $0x80000000,%%eax\n\t"
                         "movl %%eax,%%cr0\n\t"
                         "ljmp $"asm_str(GDT_CODE32_INDEX * 8)",$5f\n\t"
                         "5:\n\t"
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
                         :
                         :"m"(ss_save),"m"(esp_save),"m"(biosr_addr),"m"(vesa_ljmp_32_ram),"m"(vesa_ljmp_16_ram),"m"(vesa_gdtr_addr),"m"(vesa_idtr_addr),"m"(vesa_page_dir_addr)
                         :"eax");

    mem_cpy(vib, trans_20addr(VESA_REAL_DATA_SEG, VESA_REAL_DATA_OFF), sizeof(struct VbeInfoBlock));

    /* 恢复标志寄存器 */
    restore_eflag(eflag);

    /* 修改16位描述符 */
    *(os_gdt_addr + GDT_CODE16_INDEX * 2 + 1) = 0x00009a00;

    return start_addr;
}
LOCALC os_void BIOS_CODE get_VbeInfoBlock_end(os_void) { cassert(OS_FALSE); }

/***************************************************************
 * description : 获取vesa显示模式信息
 * history     :
 ***************************************************************/
LOCALC os_void BIOS_CODE get_vesa_mode_info(os_u16 mode, struct ModeInfoBlock *mib)
{
    os_u32 biosr_addr = (pointer) &bios_pseudo.limit;
    os_u32 *os_gdt_addr = (os_u32 *)(GDT_ADDR);
    lock_t eflag;
    os_u32 esp_save = 0;
    os_u16 ss_save = 0;

    /* 修改16位描述符 */
    *(os_gdt_addr + GDT_CODE16_INDEX * 2 + 1) = 0x00009a00 | (VESA_REAL_CODE_64K_NO);

    vesa_ljmp_32_ram.sel = 0;
    vesa_ljmp_32_ram.sel = GDT_CODE16_INDEX * 8;
    vesa_ljmp_32_ram.offset = 0;

    vesa_ljmp_16_ram.sel = VESA_REAL_CODE_SEG;
    vesa_ljmp_16_ram.offset = 0;

    vesa_gdtr_addr = (pointer) &gdt_pseudo.limit;
    vesa_idtr_addr = (pointer) &idt_pseudo.limit;

    vesa_page_dir_addr = PAGE_DIR_ADDR;

    /* 保存标志寄存器 */
    save_eflag(eflag);

    //coms_write(0x8f, 0x00);

    cli();

    /* All the code that is executed must be in a single page
       and the linear addresses in that page must be identity mapped to physical addresses. */
    cassert(virt_is_abs(IDT_ADDR));
    cassert(virt_is_abs(GDT_ADDR));
    do {
        os_u32 sp;
        __asm__ __volatile__("movl %%esp,%%eax\n\t"
                             "movl %%eax,%0\n\t"
                             :"=m"(sp)::"eax");
        cassert(virt_is_abs(sp));
    } while (0);
    if (current_task_handle()) {
        cassert(virt_is_abs(get_task_ldt(current_task_handle())));
    }

    do {
        os_u16 gs, ds;
        __asm__ __volatile__("movw %%ds,%%ax\n\t"
                             "movw %%ax,%0\n\t"
                             "movw %%gs,%%ax\n\t"
                             "movw %%ax,%1\n\t"
                             :"=m"(ds),"=m"(gs)::"eax");
        cassert(ds == gs); /* make sure gs is a backup of ds */
    } while (0);

    /* check any global variable to make sure linear address == physical address */
    cassert(virt_is_abs((pointer) &gdt_pseudo));

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
                         /* 设置cx */
                         "movw %8,%%cx\n\t"
                         "movl %%cr0,%%eax\n\t"
                         "andl $0x7fffffff,%%eax\n\t"
                         "movl %%eax,%%cr0\n\t"
                         "movl %2,%%eax\n\t"
                         "lidt %%gs:(%%eax)\n\t"
                         "ljmp $"asm_str(GDT_CODE32_INDEX * 8)",$1f\n\t"
                         "1:\n\t"
                         //"ljmp $"asm_str(GDT_CODE16_INDEX * 8)",$2f-"asm_str(OS_IMAGE_ADDR)"\n\t"
                         "movl $2f,%%eax\n\t"
                         /* do not exceed limit */
                         "movw %%ax,%3\n\t"
                         "ljmp *(%3)\n\t"
                         "2:\n\t"
                         "movl %%cr0,%%eax\n\t"
                         ".byte 0x66,0x67\n\t"
                         "andl $0xfffffffe,%%eax\n\t"
                         "movl %%eax,%%cr0\n\t"
                         //".byte 0x66,0x67\n\t"
                         //"ljmp $"asm_str(VESA_REAL_CODE_SEG)",$3f-"asm_str(OS_IMAGE_ADDR)"\n\t"
                         ".byte 0x66\n\t"
                         "movl $3f,%%eax\n\t"
                         ".byte 0x67\n\t"
                         "movl %%eax,%4\n\t"
                         ".byte 0x67\n\t"
                         "ljmp *(%4)\n\t"
                         "3:\n\t"
                         ".byte 0x66\n\t"
                         "movl $"asm_str(VESA_REAL_MODE_STACK)",%%eax\n\t"
                         "movw %%ax,%%ss\n\t"
                         ".byte 0x66\n\t"
                         "movl $0xfffc,%%esp\n\t"
                         /* bios function, 获取当前显示模式下的信息 */
                         ".byte 0x66,0x67\n\t"
                         "movl $"asm_str(VESA_REAL_DATA_OFF)",%%edi\n\t"
                         /* 该指令会使用eax寄存器 */
                         ".byte 0x66\n\t"
                         "movl $"asm_str(VESA_REAL_DATA_SEG)",%%eax\n\t"
                         "movw %%ax,%%es\n\t"
                         /* 该指令必须在后面 */
                         ".byte 0x66,0x67\n\t"
                         "movl $0x4f01,%%eax\n\t"
                         /* cx has been set */
                         //".byte 0x66,0x67\n\t"
                         //"movl $"asm_str(RESOLUTION_1024_768_MODE)",%%ecx\n\t"
                         "int $0x10\n\t"
                         /* 使用了新的堆栈, 必须使用全局变量 */
                         ".byte 0x66,0x67\n\t"
                         "movl %%gs:%5,%%eax\n\t" /* ds is freshed, use gs */
                         ".byte 0x66,0x67\n\t"
                         "lgdt %%gs:(%%eax)\n\t" /* pointer of pointer */
                         ".byte 0x66\n\t"
                         "movl %%cr0,%%eax\n\t"
                         ".byte 0x66\n\t"
                         "orl $1,%%eax\n\t"
                         ".byte 0x66\n\t"
                         "movl %%eax,%%cr0\n\t"
                         ".byte 0x66\n\t"
                         "ljmp $"asm_str(GDT_CODE32_INDEX * 8)",$4f\n\t"
                         "4:\n\t"
                         "movw $"asm_str(GDT_DATA32_INDEX * 8)",%%ax\n\t"
                         "movw %%ax,%%ds\n\t"
                         "movw %%ax,%%es\n\t"
                         "movw %%ax,%%fs\n\t"
                         "movw %%ax,%%gs\n\t"
                         "movw %%ax,%%ss\n\t"
                         "movl %1,%%esp\n\t"
                         "movl %6,%%eax\n\t"
                         "lidt %%gs:(%%eax)\n\t"
                         "movl %7,%%eax\n\t"
                         "movl %%eax,%%cr3\n\t"
                         "movl %%cr0,%%eax\n\t"
                         "orl $0x80000000,%%eax\n\t"
                         "movl %%eax,%%cr0\n\t"
                         "ljmp $"asm_str(GDT_CODE32_INDEX * 8)",$5f\n\t"
                         "5:\n\t"
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
                         :
                         :"m"(ss_save),"m"(esp_save),"m"(biosr_addr),"m"(vesa_ljmp_32_ram),"m"(vesa_ljmp_16_ram),"m"(vesa_gdtr_addr),"m"(vesa_idtr_addr),"m"(vesa_page_dir_addr),"m"(mode)
                         :"eax");

    mem_cpy(mib, trans_20addr(VESA_REAL_DATA_SEG, VESA_REAL_DATA_OFF), sizeof(struct ModeInfoBlock));

    /* 恢复标志寄存器 */
    restore_eflag(eflag);

    /* 修改16位描述符 */
    *(os_gdt_addr + GDT_CODE16_INDEX * 2 + 1) = 0x00009a00;
}
LOCALC os_void BIOS_CODE get_vesa_mode_info_end(os_void) { cassert(OS_FALSE); }

/***************************************************************
 * description : move to real mode memory area
 * history     :
 ***************************************************************/
LOCALC os_void move_func_to_rm(pointer base, pointer end)
{
    os_u8 *src = (os_u8 *)(base);
    os_u8 *dest = (os_u8 *)((VESA_REAL_CODE_64K_NO << 16) + (0xffff & (pointer) base));

    cassert((base & 0xffff0000) == (end & 0xffff0000));
    mem_cpy(dest, src, end - base);
}

/***************************************************************
 * description : mib[0] is reserved for standard mode
 *                      0x118 (1024x768, 16.8M (8:8:8))
 *                      which is defined in VBE
 *               mib[1] is reserved for the best mode
 * history     :
 ***************************************************************/
LOCALD struct vesa_mode_info vmi[2] = { 0 };

struct ModeInfoBlock temp;

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
struct vesa_mode_info *choose_vesa_mode(os_u32 choice)
{
    struct segment_offset start_addr;
    os_u16 *mode;

    mem_set(vmi, 0, sizeof(vmi));
    vmi[0].mode_number = vmi[1].mode_number = INVALID_MODE;

    bios_rst_timer_channel0();

    move_func_to_rm((pointer) get_VbeInfoBlock, (pointer) get_VbeInfoBlock_end);

    /* 获取所有模式信息 */
    start_addr = get_VbeInfoBlock(&vbe_info_block);

    if (('V' != vbe_info_block.VbeSignature[0])
     || ('E' != vbe_info_block.VbeSignature[1])
     || ('S' != vbe_info_block.VbeSignature[2])
     || ('A' != vbe_info_block.VbeSignature[3])) {
        flog("get vbe info block fail\n");
        goto end;
    }

    flog("vbe info block addr: %x\n", &vbe_info_block);

    /* 绝对地址 */
    mode = (os_u16 *) trans_20addr((os_u16)(vbe_info_block.VideoModePtr >> 16), (os_u16) vbe_info_block.VideoModePtr);
    move_func_to_rm((pointer) get_vesa_mode_info, (pointer) get_vesa_mode_info_end);
    for (; INVALID_MODE != *mode; mode++) {
        get_vesa_mode_info(*mode, &temp);
        if (6 != temp.MemoryModel) {
            /* 只支持direct color模式 */
            continue;
        }
        if (*mode == 0x118) {
            mem_cpy(&vmi[0].mib, &temp, sizeof(struct ModeInfoBlock));
            vmi[0].mode_number = *mode;
            continue;
        }
        if ((temp.XResolution > vmi[1].mib.XResolution)
         || ((temp.XResolution == vmi[1].mib.XResolution)
          && (temp.BitsPerPixel > vmi[1].mib.BitsPerPixel))) {
            mem_cpy(&vmi[1].mib, &temp, sizeof(struct ModeInfoBlock));
            vmi[1].mode_number = *mode;
        }
    }

    flog("current choice %d\n", choice);
    flog("standard vesa info: %x %d %d %d %d\n", vmi[0].mode_number, vmi[0].mib.XResolution, vmi[0].mib.YResolution, vmi[0].mib.BitsPerPixel, vmi[0].mib.MemoryModel);
    flog("%x %d\n", vmi[0].mib.PhysBasePtr, vmi[0].mib.LinBytesPerScanLine);
    flog("the best vesa info: %x %d %d\n", vmi[1].mode_number, vmi[1].mib.XResolution, vmi[1].mib.YResolution);
  end:
    return &vmi[choice];
}

/***************************************************************
 * description : (8259a, bios) not correlative
 * history     :
 ***************************************************************/
os_void show_vesa_mode_info(os_void)
{
    os_uint i;
    struct ModeInfoBlock *t;

    for (i = 0; i < array_size(vmi); i++) {
        t = &vmi[i].mib;
        print("mode 0x%x\n", vmi[i].mode_number);
        print("%d %d %d %d %d\n", t->XResolution, t->YResolution, t->NumberOfPlanes, t->BitsPerPixel, t->MemoryModel);
        print("[%d,%d][%d,%d][%d,%d][%d,%d]\n", t->RedMaskSize, t->RedMaskPos, t->GreenMaskSize, t->GreenMaskPos, t->BlueMaskSize, t->BlueMaskPos, t->ReservedMaskSize, t->ReservedMaskPos);
        print("%d\n", t->LinBytesPerScanLine);
        print("%x\n", t->PhysBasePtr);
    }
}

