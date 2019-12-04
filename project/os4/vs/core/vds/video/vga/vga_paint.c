/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : vga_paint.c
 * version     : 1.0
 * description : (key) vga������дģʽ, ���ֶ�ģʽ��
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

    /* �۰� */
    if (0x800000 <= (color & 0xff0000)) {
        i = 7;
    }
    for (; i < 16; i++) {
        if (argb_2_vga[i][0] >= color) {
            return argb_2_vga[i][1];
        }
    }
    /* Ĭ�Ϻ�ɫ */
    return 0;
}

/***************************************************************
 * description : �˺�����������
 *               ���Կ������뱻�޸ĺ��ж�, ���ж����޸�����󷵻�.
 *               �������ж�ǰ���Կ����뱻�޸�, ��ɫ��ʾ�쳣.
 *               �������л������Ĵ������Ĳ���(����cpu�ں�cpu��)Ҫ���ж�.
 *               ����дģʽ2
 * history     :
 ***************************************************************/
os_void vga_write_pix(screen_csys *p, enum vga_color color)
{
    cassert(OS_NULL != p);

    /* ��RGBģʽ��ɫת��Ϊ4ɫ��ɫ����ɫ */
    color = argb_convert(color & 0x00ffffff);

    /* ��ʽΪx/8+y*80, ����x/8��������Ϊ����, ����ӵ�ƫ������.
       ebx����Դ��ַ
       eax���ڼ���
       edx����м���
       ebx=ebx+x/8+y*8*(2+8)
       ����ļ��㶼���޷�����, ��Ļ���궼���޷�����. */
    __asm__ __volatile__("movl $"asm_str(VGA_MEM_ADDR)",%%ebx\n\t"
                         /* ����y���ڼĴ���eax�� */
                         "movl %1,%%eax\n\t"
                         /* ���ݵ�ecx */
                         "movl %%eax,%%ecx\n\t"
                         /* �˷�����, �����μӵ�ebx�� */
                         "shll $4,%%eax\n\t"
                         "addl %%eax,%%ebx\n\t"
                         "shll $6,%%ecx\n\t"
                         "addl %%ecx,%%ebx\n\t"
                         /* ����x���ڼĴ���eax */
                         "movl %0,%%eax\n\t"
                         /* ��ȡ����, ����cl(ѭ����λ������cl) */
                         "movl %%eax,%%ecx\n\t"
                         "andb $0x07,%%cl\n\t"
                         /* �߼�����, ��Ӧ�޷��ų��� */
                         "shrl $3,%%eax\n\t"
                         /* ebx=ebx+x/8+y*80 */
                         "addl %%eax,%%ebx\n\t"
                         /* ��������, �������ch�� */
                         "movb $0x80,%%ch\n\t"
                         "shrb %%cl,%%ch\n\t"
                         /* ���ж� */
                         "pushfl\n\t"
                         "cli\n\t"
                         /* �������� */
                         "movw $0x3ce,%%dx\n\t"
                         "movb $8,%%al\n\t"
                         "outb %%al,%%dx\n\t"
                         "movw $0x3cf,%%dx\n\t"
                         /* ����λ��ch�� */
                         "movb %%ch,%%al\n\t"
                         "outb %%al,%%dx\n\t"
                         /* ����ɫ */
                         "movb (%%ebx),%%cl\n\t"
                         /* д��ɫ */
                         "movl %2,%%ecx\n\t"
                         "movb %%cl,(%%ebx)\n\t"
                         /* �ָ���־�Ĵ��� */
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
 * description : ���ö�ģʽ0
 * history     :
 ***************************************************************/
enum vga_color vga_read_pix(screen_csys *p)
{
    enum vga_color color = 0;

    cassert(OS_NULL != p);

    /* ��ʽΪx/8+y*80, ����x/8��������Ϊ����, ����ӵ�ƫ������.
       ebx����Դ��ַ
       eax���ڼ���
       edx����м���
       ebx=ebx+x/8+y*8*(2+8)
       ����ļ��㶼���޷�����, ��Ļ���궼���޷�����. */
    __asm__ __volatile__("movl $"asm_str(VGA_MEM_ADDR)",%%ebx\n\t"
                         /* ����y���ڼĴ���eax�� */
                         "movl %1,%%eax\n\t"
                         /* ���ݵ�ecx */
                         "movl %%eax,%%ecx\n\t"
                         /* �˷�����, �����μӵ�ebx�� */
                         "shll $4,%%eax\n\t"
                         "addl %%eax,%%ebx\n\t"
                         "shll $6,%%ecx\n\t"
                         "addl %%ecx,%%ebx\n\t"
                         /* ����x���ڼĴ���eax */
                         "movl %0,%%eax\n\t"
                         /* ��ȡ����, ����cl(ѭ����λ������cl) */
                         "movl %%eax,%%ecx\n\t"
                         "andb $0x07,%%cl\n\t"
                         /* �߼�����, ��Ӧ�޷��ų��� */
                         "shrl $3,%%eax\n\t"
                         /* ebx=ebx+x/8+y*80 */
                         "addl %%eax,%%ebx\n\t"
                         /* ��������, �������ch�� */
                         "movb $0x80,%%ch\n\t"
                         "shrb %%cl,%%ch\n\t"
                         /* ���ж� */
                         "pushfl\n\t"
                         "cli\n\t"
                         /* clear */
                         "xorw %%ax,%%ax\n\t"
                         /* ���ö�λ��ѡ��Ĵ��� */
                         "movw $0x3ce,%%dx\n\t"
                         "movb $4,%%al\n\t"
                         "outb %%al,%%dx\n\t"
                         /* ��λ��3 */
                         "movw $0x3cf,%%dx\n\t"
                         "movb $3,%%al\n\t"
                         "outb %%al,%%dx\n\t"
                         "movb (%%ebx),%%al\n\t"
                         "andb %%ch,%%al\n\t"
                         "orb %%al,%%ah\n\t"
                         "rolb $1,%%ah\n\t"
                         /* ��λ��2 */
                         "movw $0x3cf,%%dx\n\t"
                         "movb $2,%%al\n\t"
                         "outb %%al,%%dx\n\t"
                         "movb (%%ebx),%%al\n\t"
                         "andb %%ch,%%al\n\t"
                         "orb %%al,%%ah\n\t"
                         "rolb $1,%%ah\n\t"
                         /* ��λ��1 */
                         "movw $0x3cf,%%dx\n\t"
                         "movb $1,%%al\n\t"
                         "outb %%al,%%dx\n\t"
                         "movb (%%ebx),%%al\n\t"
                         "andb %%ch,%%al\n\t"
                         "orb %%al,%%ah\n\t"
                         "rolb $1,%%ah\n\t"
                         /* ��λ��0 */
                         "movw $0x3cf,%%dx\n\t"
                         "movb $0,%%al\n\t"
                         "outb %%al,%%dx\n\t"
                         "movb (%%ebx),%%al\n\t"
                         "andb %%ch,%%al\n\t"
                         "orb %%al,%%ah\n\t"
                         "rolb $1,%%ah\n\t"
                         /* �������� */
                         "rolb %%cl,%%ah\n\t"
                         "movb %%ah,%2\n\t"
                         /* �ָ���־�Ĵ��� */
                         "popfl"
                         :
                         :"m"(p->x),"m"(p->y),"m"(color)
                         :"eax","ebx","ecx");

    return vga_convert(color);
}

LOCALC os_u8 get_mask(os_u16 x) // �������ص�x����
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
    /* �������� */
    mask = get_mask(p->x);
    /* ���������Ĵ��� */
    outb(0x3ce, 0x4);
    /* ��λ��3 */
    outb(0x3cf, 0x3);
    t += (*base & mask) * 8;
    /* ��λ��2 */
    outb(0x3cf, 0x2);
    t += (*base & mask) * 4;
    /* ��λ��1 */
    outb(0x3cf, 0x1);
    t += (*base & mask) * 2;
    /* ��λ��0 */
    outb(0x3cf, 0x0);
    t += (*base & mask) * 1;
    /* �������� */
    color = t >> (7 - (p->x % 8));
    return vga_convert(color);
}

os_void vga_write_mode_2(screen_csys *p, enum vga_color color)
{
    os_u8 *base;
    os_u8 mask;
    volatile os_u8 dummy;

    /* ��RGBģʽ��ɫת��Ϊ4ɫ��ɫ����ɫ */
    color = argb_convert(color & 0x00ffffff);

    base = (os_u8 *)(VGA_MEM_ADDR + (p->x + p->y*VGA_RESOLUTION_X)/8);
    /* ���������Ĵ��� */
    outb(0x3ce, 0x8);
    /* �������� */
    mask = get_mask(p->x);
    outb(0x3cf, mask);
    dummy = *base; // dummy read to load latch registers
    *base = color; // set color
}

