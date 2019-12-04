/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : ps2_mouse.c
 * version     : 1.0
 * description : ps/2 mouse. IRQ 12.
 *               �������ϵ�����½���Ϊԭ��
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
/* ƫ���� */
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
    /* ʹ�����ͨ�� */
    outb_p(PS2_MOUSE_STATUS_REG, 0xa8);

    /* �´�д��0x60�˿ڵ����ݷ��͸���� */
    outb_p(PS2_MOUSE_STATUS_REG, 0xd4);

    /* ������������������� */
    outb_p(PS2_MOUSE_DATA_REG, 0xf4);

    /* �´�д��0x60�˿ڵ����ݷ��͸�8042����Ĵ��� */
    outb_p(PS2_MOUSE_STATUS_REG, 0x60);

    /* �������ͼ����ж� */
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
 * description : ����ж���Ӧ, IRQ_FUNCPTR
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
        /* ������Ϣ */
        left = mouse_data & 0x01;
        right = !!(mouse_data & 0x02);

        /* x y ��ƫ�������� */
        x = mouse_data & 0x10 ? 0xffffff00 : 0;
        y = mouse_data & 0x20 ? 0xffffff00 : 0; /* �˴����ܽ�������ϵ��ת�� */
        break;

    case 2:
        /* ���� */
        x |= mouse_data;
        break;

    /* ��ά��� */
    case 3:
        y |= mouse_data;
        /* -y */
        y = ~y + 1;
        cnt = 1;
        send_flag = 1;
        break;

    case 0: /* �������һ���ж϶��� */
        print("ps2 mouse startup.\n");
        break;

    default:
        break;
    }
}

/***************************************************************
 * description : ����ж���Ӧ, IRQ_FUNCPTR
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
    /* ��ʼ��ʵʱʱ���ж� */
    install_int(PS2_MOUSE_INT_VECTOR, handle_ps2_mouse_int_1, handle_ps2_mouse_int_2);

    /* ����ǰ�������8042������ */
    clear_8042_buffer();

    /* ����8042 */
    enable_ps2_mouse();
}

