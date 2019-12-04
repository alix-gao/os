/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : ps2_mouse.c
 * version     : 1.0
 * description : ps/2 mouse. IRQ 12.
 *               鼠标坐标系以左下角作为原点
 * author      : gaocheng
 * date        : 2011-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include <vds.h>

#include "ps2_mouse.h"

/***************************************************************
 global variable declare
 ***************************************************************/
/* 偏移量 */
LOCALD volatile os_s32 x _CPU_ALIGNED_ = 0;
LOCALD volatile os_s32 y _CPU_ALIGNED_ = 0;
LOCALD volatile os_u8 left _CPU_ALIGNED_ = 0;
LOCALD volatile os_u8 right _CPU_ALIGNED_ = 0;

LOCALD volatile os_s32 send_flag _CPU_ALIGNED_ = 0;

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void enable_ps2_mouse(os_void)
{
    /* 使能鼠标通道 */
    outb_p(PS2_MOUSE_STATUS_REG, 0xa8);

    /* 下次写入0x60端口的数据发送给鼠标 */
    outb_p(PS2_MOUSE_STATUS_REG, 0xd4);

    /* 允许鼠标主动发送数据 */
    outb_p(PS2_MOUSE_DATA_REG, 0xf4);

    /* 下次写入0x60端口的数据发送给8042命令寄存器 */
    outb_p(PS2_MOUSE_STATUS_REG, 0x60);

    /* 允许鼠标和键盘中断 */
    outb_p(PS2_MOUSE_DATA_REG, 0x47);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void clear_8042_buffer(os_void)
{
    os_u32 i;
    os_u8 data;

    for (i = 0; i < 0x10; i++) {
        inb(PS2_MOUSE_DATA_REG, data);
    }
}

/***************************************************************
 * description : 鼠标中断响应, IRQ_FUNCPTR
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC handle_ps2_mouse_int_1(os_u32 irq)
{
    GLOBALDIF os_u32 cnt = 0;
    os_u8 mouse_data;

    inb(PS2_MOUSE_DATA_REG, mouse_data);

    send_flag = 0;

    switch (cnt++) {
    case 1:
        /* 按键信息 */
        left = mouse_data & 0x01;
        right = !!(mouse_data & 0x02);

        /* x y 的偏移量符号 */
        x = mouse_data & 0x10 ? 0xffffff00 : 0;
        y = mouse_data & 0x20 ? 0xffffff00 : 0; /* 此处不能进行坐标系的转换 */
        break;

    case 2:
        /* 补码 */
        x |= mouse_data;
        break;

    /* 二维鼠标 */
    case 3:
        y |= mouse_data;
        /* -y */
        y = ~y + 1;
        cnt = 1;
        send_flag = 1;
        break;

    case 0: /* 打开鼠标后第一个中断丢弃 */
        print("ps2 mouse startup.\n");
        break;

    default:
        break;
    }
}

/***************************************************************
 * description : 鼠标中断响应, IRQ_FUNCPTR
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC handle_ps2_mouse_int_2(os_u32 irq)
{
    if (send_flag) {
        send_mouse_msg(left, right, 0, x, y, 0);
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void init_ps2_mouse(os_void)
{
    /* 初始化实时时钟中断 */
    install_int(PS2_MOUSE_INT_VECTOR, handle_ps2_mouse_int_1, handle_ps2_mouse_int_2);

    /* 设置前必须清空8042缓冲区 */
    clear_8042_buffer();

    /* 设置8042 */
    enable_ps2_mouse();
}

