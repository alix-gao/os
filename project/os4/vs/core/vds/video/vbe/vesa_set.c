/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : vesa_bios.c
 * version     : 1.0
 * description : 保护模式下bios调用, only for 1024*768
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>

#include "vesa_bios.h"
#include "vesa.h"

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
LOCALD os_u8 vbe_result;

/***************************************************************
 function declare
 ***************************************************************/

/***************************************************************
 * description : 设置vesa显示模式, a20已经打开, 不再出现地址回绕
 * history     :
 ***************************************************************/
LOCALC os_u8 BIOS_CODE vesa_bios(os_u16 mode)
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

    vesa_page_dir_addr = system_pdbr();

    /* 保存标志寄存器 */
    save_eflag(eflag);

    //coms_write(0x8f, 0x00);

    cli();

    /* the current ds: 0x14
       that means, ldt is used. */
    if (0) {
        os_u32 ds;
        GLOBALDIF os_uint flag = 0;
        __asm__ __volatile__("movl %%ds,%%eax\n\t"
                             "movl %%eax,%0\n\t"
                             :"=m"(ds)::"eax");
        print("current ds : %x\n", ds);
        if (flag) {
            dead();
        } else {
            flag = 1;
        }
    }

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

    /* make sure linear address == physical address, especially memory in task.c */
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
                         /* disable page */
                         "movl %%cr0,%%eax\n\t"
                         "andl $0x7fffffff,%%eax\n\t"
                         "movl %%eax,%%cr0\n\t"
                         /* clear cr3 to flush tlb */
                         "xorl %%eax, %%eax\n\t"
                         "movl %%eax,%%cr3\n\t"
                         /* ljmp to flush */
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
                         ".byte 0x66\n\t"
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
                         "movb $0x4f,%%ah\n\t" /* use vbe */
                         "movb $0x02,%%al\n\t" /* set vesa mode */
                         /* Use linear/flat frame buffer model
                            Use current default refresh rate
                            Clear display memory */
                         ".byte 0x66\n\t"
                         "movl $0x4000,%%ebx\n\t"
                         ".byte 0x67\n\t" /* mode number */
                         "movl %%gs:%8,%%ebx\n\t" /* it is "movw %%gs:%8,%%bx" in rm mode. */
                         /* use default refresh rate, es:di is ignored */
                         "int $0x10\n\t"
                         /* 使用了新的堆栈, 必须使用全局变量 */
                         ".byte 0x67\n\t" /* save result. */
                         "movb %%ah,%%gs:%9\n\t" /* ds is freshed, use gs. */
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
                         :"m"(ss_save),"m"(esp_save),"m"(biosr_addr),"m"(vesa_ljmp_32_ram),"m"(vesa_ljmp_16_ram),"m"(vesa_gdtr_addr),"m"(vesa_idtr_addr),"m"(vesa_page_dir_addr),"m"(mode),"m"(vbe_result)
                         :"eax","ebx");

    /* 恢复标志寄存器 */
    restore_eflag(eflag);

    /* 修改16位描述符 */
    *(os_gdt_addr + GDT_CODE16_INDEX * 2 + 1) = 0x00009a00;

    return vbe_result;
}
LOCALC os_void BIOS_CODE func_end(os_void) { cassert(OS_FALSE); }

/***************************************************************
 * description : move to real mode memory area
 * history     :
 ***************************************************************/
LOCALC os_void move_to_rm(os_void)
{
    os_u8 *src = (os_u8 *)(vesa_bios);
    os_u8 *dest = (os_u8 *)((VESA_REAL_CODE_64K_NO << 16) + (0xffff & (pointer) vesa_bios));

    cassert(((pointer) func_end & 0xffff0000) == ((pointer) vesa_bios & 0xffff0000));
    mem_cpy(dest, src, (pointer) func_end - (pointer) vesa_bios);
}

/***************************************************************
 * description : (8259a, bios)not correlative
 * history     :
 ***************************************************************/
os_ret set_vesa_graphics_mode(struct graphics_mode_info *data, enum graphics_mode mode)
{
    struct vesa_mode_info *vmi;
    os_u8 ret;

    if (OS_NULL != data) {
        vmi = choose_vesa_mode(mode);
        if (INVALID_MODE == vmi->mode_number) {
            return OS_FAIL;
        }

        bios_rst_timer_channel0();

        move_to_rm();

        /* 设置模式 */
        ret = vesa_bios(vmi->mode_number);
        flog("vesa bios result %d\n", ret);
        if (0 != ret) {
            return OS_FAIL;
        }

        init_vesa_paint(&vmi->mib);

        os_set_timer_channel0();

        data->bits_per_pixel = vmi->mib.BitsPerPixel;
        data->x_resolution = vmi->mib.XResolution;
        data->y_resolution = vmi->mib.YResolution;
        data->plane_count = vmi->mib.NumberOfPlanes;
        data->memory_model = vmi->mib.MemoryModel;
        data->PhysBasePtr = vmi->mib.PhysBasePtr;
        return OS_SUCC;
    }
    cassert(OS_FALSE);
    return OS_FAIL;
}

