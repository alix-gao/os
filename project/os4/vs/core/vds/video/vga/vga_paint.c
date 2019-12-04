/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : vga_paint.c
 * version     : 1.0
 * description : (key) vga有四种写模式, 两种读模式。
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include "vga_paint.h"

/***************************************************************
 global variable declare
 ***************************************************************/

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC inline os_u32 argb_convert(enum vga_color color)
{
    GLOBALDIF const os_u32 argb_2_vga[][2] = {
        {VGA_COLOR_BLACK,   0},
        {VGA_COLOR_NAVY,    1},
        {VGA_COLOR_BLUE,    9},
        {VGA_COLOR_GREEN,   2},
        {VGA_COLOR_TEAL,    3},
        {VGA_COLOR_LIME,    10},
        {VGA_COLOR_AQUA,    11},
        {VGA_COLOR_MAROON,  4},
        {VGA_COLOR_PURPLE,  5},
        {VGA_COLOR_OLIVE,   6},
        {VGA_COLOR_GRAY,    8},
        {VGA_COLOR_SILVER,  7},
        {VGA_COLOR_RED,     12},
        {VGA_COLOR_FUCHSIA, 13},
        {VGA_COLOR_YELLOW,  14},
        {VGA_COLOR_WHITE,   15}
    };

    os_u32 i = 0;

    /* 折半 */
    if (0x800000 <= (color & 0xff0000)) {
        i = 7;
    }
    for (; i < 16; i++) {
        if (argb_2_vga[i][0] >= color) {
            return argb_2_vga[i][1];
        }
    }
    /* 默认黑色 */
    return 0;
}

/***************************************************************
 * description : 此函数不可重入
 *               在显卡的掩码被修改后中断, 而中断又修改掩码后返回.
 *               将导致中断前的显卡掩码被修改, 颜色显示异常.
 *               非任务切换保护寄存器级的操作(包括cpu内和cpu外)要关中断.
 *               采用写模式2
 * history     :
 ***************************************************************/
os_void vga_write_pix(screen_csys *p, enum vga_color color)
{
    cassert(OS_NULL != p);

    /* 将RGB模式颜色转换为4色调色板颜色 */
    color = argb_convert(color & 0x00ffffff);

    /* 公式为x/8+y*80, 其中x/8的余数作为掩码, 其余加到偏移量中.
       ebx存放显存地址
       eax用于计算
       edx存放中间结果
       ebx=ebx+x/8+y*8*(2+8)
       下面的计算都是无符号数, 屏幕坐标都是无符号数. */
    __asm__ __volatile__("movl $"asm_str(VGA_MEM_ADDR)",%%ebx\n\t"
                         /* 坐标y置于寄存器eax中 */
                         "movl %1,%%eax\n\t"
                         /* 备份到ecx */
                         "movl %%eax,%%ecx\n\t"
                         /* 乘法操作, 并依次加到ebx中 */
                         "shll $4,%%eax\n\t"
                         "addl %%eax,%%ebx\n\t"
                         "shll $6,%%ecx\n\t"
                         "addl %%ecx,%%ebx\n\t"
                         /* 坐标x置于寄存器eax */
                         "movl %0,%%eax\n\t"
                         /* 获取余数, 置于cl(循环移位必须用cl) */
                         "movl %%eax,%%ecx\n\t"
                         "andb $0x07,%%cl\n\t"
                         /* 逻辑右移, 对应无符号除法 */
                         "shrl $3,%%eax\n\t"
                         /* ebx=ebx+x/8+y*80 */
                         "addl %%eax,%%ebx\n\t"
                         /* 计算掩码, 结果置于ch中 */
                         "movb $0x80,%%ch\n\t"
                         "shrb %%cl,%%ch\n\t"
                         /* 关中断 */
                         "pushfl\n\t"
                         "cli\n\t"
                         /* 设置掩码 */
                         "movw $0x3ce,%%dx\n\t"
                         "movb $8,%%al\n\t"
                         "outb %%al,%%dx\n\t"
                         "movw $0x3cf,%%dx\n\t"
                         /* 掩码位于ch中 */
                         "movb %%ch,%%al\n\t"
                         "outb %%al,%%dx\n\t"
                         /* 读颜色 */
                         "movb (%%ebx),%%cl\n\t"
                         /* 写颜色 */
                         "movl %2,%%ecx\n\t"
                         "movb %%cl,(%%ebx)\n\t"
                         /* 恢复标志寄存器 */
                         "popfl"
                         :
                         :"m"(p->x),"m"(p->y),"m"(color)
                         :"eax","ebx","ecx");
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC inline enum vga_color vga_convert(os_u32 color)
{
    GLOBALDIF const os_u32 vga_color[] = {
        VGA_COLOR_BLACK,
        VGA_COLOR_NAVY,
        VGA_COLOR_GREEN,
        VGA_COLOR_TEAL,
        VGA_COLOR_MAROON,
        VGA_COLOR_PURPLE,
        VGA_COLOR_OLIVE,
        VGA_COLOR_SILVER,
        VGA_COLOR_GRAY,
        VGA_COLOR_BLUE,
        VGA_COLOR_LIME,
        VGA_COLOR_AQUA,
        VGA_COLOR_RED,
        VGA_COLOR_FUCHSIA,
        VGA_COLOR_YELLOW,
        VGA_COLOR_WHITE
    };

    return vga_color[color];
}

/***************************************************************
 * description : 采用读模式0
 * history     :
 ***************************************************************/
enum vga_color vga_read_pix(screen_csys *p)
{
    enum vga_color color = 0;

    cassert(OS_NULL != p);

    /* 公式为x/8+y*80, 其中x/8的余数作为掩码, 其余加到偏移量中.
       ebx存放显存地址
       eax用于计算
       edx存放中间结果
       ebx=ebx+x/8+y*8*(2+8)
       下面的计算都是无符号数, 屏幕坐标都是无符号数. */
    __asm__ __volatile__("movl $"asm_str(VGA_MEM_ADDR)",%%ebx\n\t"
                         /* 坐标y置于寄存器eax中 */
                         "movl %1,%%eax\n\t"
                         /* 备份到ecx */
                         "movl %%eax,%%ecx\n\t"
                         /* 乘法操作, 并依次加到ebx中 */
                         "shll $4,%%eax\n\t"
                         "addl %%eax,%%ebx\n\t"
                         "shll $6,%%ecx\n\t"
                         "addl %%ecx,%%ebx\n\t"
                         /* 坐标x置于寄存器eax */
                         "movl %0,%%eax\n\t"
                         /* 获取余数, 置于cl(循环移位必须用cl) */
                         "movl %%eax,%%ecx\n\t"
                         "andb $0x07,%%cl\n\t"
                         /* 逻辑右移, 对应无符号除法 */
                         "shrl $3,%%eax\n\t"
                         /* ebx=ebx+x/8+y*80 */
                         "addl %%eax,%%ebx\n\t"
                         /* 计算掩码, 结果置于ch中 */
                         "movb $0x80,%%ch\n\t"
                         "shrb %%cl,%%ch\n\t"
                         /* 关中断 */
                         "pushfl\n\t"
                         "cli\n\t"
                         /* clear */
                         "xorw %%ax,%%ax\n\t"
                         /* 设置读位面选择寄存器 */
                         "movw $0x3ce,%%dx\n\t"
                         "movb $4,%%al\n\t"
                         "outb %%al,%%dx\n\t"
                         /* 读位面3 */
                         "movw $0x3cf,%%dx\n\t"
                         "movb $3,%%al\n\t"
                         "outb %%al,%%dx\n\t"
                         "movb (%%ebx),%%al\n\t"
                         "andb %%ch,%%al\n\t"
                         "orb %%al,%%ah\n\t"
                         "rolb $1,%%ah\n\t"
                         /* 读位面2 */
                         "movw $0x3cf,%%dx\n\t"
                         "movb $2,%%al\n\t"
                         "outb %%al,%%dx\n\t"
                         "movb (%%ebx),%%al\n\t"
                         "andb %%ch,%%al\n\t"
                         "orb %%al,%%ah\n\t"
                         "rolb $1,%%ah\n\t"
                         /* 读位面1 */
                         "movw $0x3cf,%%dx\n\t"
                         "movb $1,%%al\n\t"
                         "outb %%al,%%dx\n\t"
                         "movb (%%ebx),%%al\n\t"
                         "andb %%ch,%%al\n\t"
                         "orb %%al,%%ah\n\t"
                         "rolb $1,%%ah\n\t"
                         /* 读位面0 */
                         "movw $0x3cf,%%dx\n\t"
                         "movb $0,%%al\n\t"
                         "outb %%al,%%dx\n\t"
                         "movb (%%ebx),%%al\n\t"
                         "andb %%ch,%%al\n\t"
                         "orb %%al,%%ah\n\t"
                         "rolb $1,%%ah\n\t"
                         /* 生成数据 */
                         "rolb %%cl,%%ah\n\t"
                         "movb %%ah,%2\n\t"
                         /* 恢复标志寄存器 */
                         "popfl"
                         :
                         :"m"(p->x),"m"(p->y),"m"(color)
                         :"eax","ebx","ecx");

    return vga_convert(color);
}

LOCALC os_u8 get_mask(os_u16 x) // 输入像素的x坐标
{
    os_u8 mask = 0x80;
    return (mask >> (x % 8));
}

enum vga_color vga_read_mode_0(screen_csys *p)
{
    volatile os_u8 *base;
    os_u8 color;
    os_u8 mask;
    os_u16 t = 0;

    base = (os_u8 *)(VGA_MEM_ADDR + (p->x + p->y*VGA_RESOLUTION_X)/8);
    /* 设置掩码 */
    mask = get_mask(p->x);
    /* 设置索引寄存器 */
    outb(0x3ce, 0x4);
    /* 读位面3 */
    outb(0x3cf, 0x3);
    t += (*base & mask) * 8;
    /* 读位面2 */
    outb(0x3cf, 0x2);
    t += (*base & mask) * 4;
    /* 读位面1 */
    outb(0x3cf, 0x1);
    t += (*base & mask) * 2;
    /* 读位面0 */
    outb(0x3cf, 0x0);
    t += (*base & mask) * 1;
    /* 生成数据 */
    color = t >> (7 - (p->x % 8));
    return vga_convert(color);
}

os_void vga_write_mode_2(screen_csys *p, enum vga_color color)
{
    os_u8 *base;
    os_u8 mask;
    volatile os_u8 dummy;

    /* 将RGB模式颜色转换为4色调色板颜色 */
    color = argb_convert(color & 0x00ffffff);

    base = (os_u8 *)(VGA_MEM_ADDR + (p->x + p->y*VGA_RESOLUTION_X)/8);
    /* 设置索引寄存器 */
    outb(0x3ce, 0x8);
    /* 设置掩码 */
    mask = get_mask(p->x);
    outb(0x3cf, mask);
    dummy = *base; // dummy read to load latch registers
    *base = color; // set color
}

