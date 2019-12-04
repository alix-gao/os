/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : keyboard.c
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include "keyboard.h"

/***************************************************************
 global variable declare
 ***************************************************************/

/***************************************************************
 * description :
 ***************************************************************/
struct keyboard {
    os_u8 down;
    os_u8 shift;
};

/* 键盘扫描码-ASCII, make code */
LOCALD const struct keyboard keyboard_scan_code[0x80] = {
    { 0x00, 0x00 }, // 0x00, null
    { 0x1b, 0x1b }, // 0x01, esc
    { 0x31, '!' },  // 0x02, 1
    { 0x32, '@' },  // 0x03, 2
    { 0x33, '#' },  // 0x04, 3
    { 0x34, '$' },  // 0x05, 4
    { 0x35, '%' },  // 0x06, 5
    { 0x36, '^' },  // 0x07, 6
    { 0x37, '&' },  // 0x08, 7
    { 0x38, '*' },  // 0x09, 8
    { 0x39, '(' },  // 0x0a, 9
    { 0x30, ')' },  // 0x0b, 0
    { 0x2d, '_' },  // 0x0c, -
    { 0x3d, '+' },  // 0x0d, =
    { 0x08, 0x08 }, // 0x0e, backspace
    { 0x09, 0x09 }, // 0x0f, tab
    { 0x71, 'Q' },  // 0x10, q
    { 0x77, 'W' },  // 0x11, w
    { 0x65, 'E' },  // 0x12, e
    { 0x72, 'R' },  // 0x13, r
    { 0x74, 'T' },  // 0x14, t
    { 0x79, 'Y' },  // 0x15, y
    { 0x75, 'U' },  // 0x16, u
    { 0x69, 'I' },  // 0x17, i
    { 0x6f, 'O' },  // 0x18, o
    { 0x70, 'P' },  // 0x19, p
    { 0x5b, '{' },  // 0x1a, [
    { 0x5d, '}' },  // 0x1b, ]
    { 0x0d, 0x0d }, // 0x1c, enter
    { 0x00, 0x00 }, // 0x1d, left ctrl
    { 0x61, 'A' },  // 0x1e, a
    { 0x73, 'S' },  // 0x1f, s
    { 0x64, 'D' },  // 0x20, d
    { 0x66, 'F' },  // 0x21, f
    { 0x67, 'G' },  // 0x22, g
    { 0x68, 'H' },  // 0x23, h
    { 0x6a, 'J' },  // 0x24, j
    { 0x6b, 'K' },  // 0x25, k
    { 0x6c, 'L' },  // 0x26, l
    { 0x3b, ':' },  // 0x27, ;
    { 0x27, '"' },  // 0x28, '
    { 0x60, '~' },  // 0x29, `
    { 0x00, 0x00 }, // 0x2a, left shift
    { 0x5c, '|' },  // 0x2b, \0
    { 0x7a, 'Z' },  // 0x2c, z
    { 0x78, 'X' },  // 0x2d, x
    { 0x63, 'C' },  // 0x2e, c
    { 0x76, 'V' },  // 0x2f, v
    { 0x62, 'B' },  // 0x30, b
    { 0x6e, 'N' },  // 0x31, n
    { 0x6d, 'M' },  // 0x32, m
    { 0x2c, '<' },  // 0x33, ,
    { 0x2e, '>' },  // 0x34, .
    { 0x2f, '?' },  // 0x35, /
    { 0x00, 0x00 }, // 0x36, right shift
    { 0x00, 0x00 }, // 0x37, print screen
    { 0x00, 0x00 }, // 0x38(0xe0), right alt
    { 0x20, ' ' },  // 0x39, space
    { 0x00, 0x00 }, // 0x3a, caps lock
    { 0x00, 0x00 }, // 0x3b, f1
    { 0x00, 0x00 }, // 0x3c, f2
    { 0x00, 0x00 }, // 0x3d, f3
    { 0x00, 0x00 }, // 0x3e, f4
    { 0x00, 0x00 }, // 0x3f, f5
    { 0x00, 0x00 }, // 0x40, f6
    { 0x00, 0x00 }, // 0x41, f7
    { 0x00, 0x00 }, // 0x42, f8
    { 0x00, 0x00 }, // 0x43, f9
    { 0x00, 0x00 }, // 0x44, f10
    { 0x00, 0x00 }, // 0x45, num lock
    { 0x00, 0x00 }, // 0x46, scroll lock
    { 0x00, '7' },  // 0x47(0xe0), home
    { 0x00, '8' },  // 0x48(0xe0), up
    { 0x00, '9' },  // 0x49(0xe0), pg up
    { 0x00, '-' },  // 0x4a, -
    { 0x00, '4' },  // 0x4b(0xe0), left
    { 0x00, '5' },  // 0x4c, 5
    { 0x00, '6' },  // 0x4d(0xe0), right
    { 0x00, '+' },  // 0x4e, +
    { 0x00, '1' },  // 0x4f(0xe0), end
    { 0x00, '2' },  // 0x50(0xe0), down
    { 0x00, '3' },  // 0x51(0xe0), pg down
    { 0x00, '0' },  // 0x52(0xe0), ins
    { 0x00, '.' },  // 0x53(0xe0), del
    { 0x00, 0x00 }, // 0x54, enter
    { 0x00, 0x00 }, // 0x55,
    { 0x00, 0x00 }, // 0x56,
    { 0x00, 0x00 }, // 0x57, f11
    { 0x00, 0x00 }, // 0x58, f12
    { 0x00, 0x00 }, // 0x59,
    { 0x00, 0x00 }, // 0x5a,
    { 0x00, 0x00 }, // 0x5b,
    { 0x00, 0x00 }, // 0x5c,
    { 0x00, 0x00 }, // 0x5d,
    { 0x00, 0x00 }, // 0x5e,
    { 0x00, 0x00 }, // 0x5f,
    { 0x00, 0x00 }, // 0x60,
    { 0x00, 0x00 }, // 0x61,
    { 0x00, 0x00 }, // 0x62,
    { 0x00, 0x00 }, // 0x63,
    { 0x00, 0x00 }, // 0x64,
    { 0x00, 0x00 }, // 0x65,
    { 0x00, 0x00 }, // 0x66,
    { 0x00, 0x00 }, // 0x67,
    { 0x00, 0x00 }, // 0x68,
    { 0x00, 0x00 }, // 0x69,
    { 0x00, 0x00 }, // 0x6a,
    { 0x00, 0x00 }, // 0x6b,
    { 0x00, 0x00 }, // 0x6c,
    { 0x00, 0x00 }, // 0x6d,
    { 0x00, 0x00 }, // 0x6e,
    { 0x00, 0x00 }, // 0x6f,
    { 0x00, 0x00 }, // 0x70,
    { 0x00, 0x00 }, // 0x71,
    { 0x00, 0x00 }, // 0x72,
    { 0x00, 0x00 }, // 0x73,
    { 0x00, 0x00 }, // 0x74,
    { 0x00, 0x00 }, // 0x75,
    { 0x00, 0x00 }, // 0x76,
    { 0x00, 0x00 }, // 0x77,
    { 0x00, 0x00 }, // 0x78,
    { 0x00, 0x00 }, // 0x79,
    { 0x00, 0x00 }, // 0x7a,
    { 0x00, 0x00 }, // 0x7b,
    { 0x00, 0x00 }, // 0x7c,
    { 0x00, 0x00 }, // 0x7d,
    { 0x00, 0x00 }, // 0x7e,
    { 0x00, 0x00 }  // 0x7f,
};

/* shift */
LOCALD volatile os_u32 keyboard_shift _CPU_ALIGNED_ = 0;

/* caps lock */
LOCALD volatile os_u32 keyboard_caps _CPU_ALIGNED_ = 0;

LOCALD volatile os_u8 kb_scan_code _CPU_ALIGNED_ = 0;

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description : 等待键盘缓冲区空
 * history     :
 ***************************************************************/
LOCALC os_void wait_keyboard_buffer(os_void)
{
    os_u8 kb_code;

    /* 参考8042芯片, 对于状态寄存器0x64端口读取, 位1为1表示输入缓冲区满 */
    do {
        inb(KEYBOARD_STATUS_REG, kb_code);
    } while (kb_code & 0x02);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void wait_keyboard_ack(os_void)
{
    os_u8 kb_code;

    do {
        inb(KEYBOARD_DATA_REG, kb_code);
    } while (KEYBOARD_ACK != kb_code);
}

/***************************************************************
 * description : 键盘, IRQ_FUNCPTR
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC handle_keyboard_int_1(os_u32 irq)
{
    /* 读取扫描码 */
    inb(KEYBOARD_DATA_REG, kb_scan_code);

    switch (kb_scan_code) {
    case 0xe0:
        return;
        break;

    case 0xe1:
        return;
        break;

    /* shift make */
    case 0x2a:
    case 0x36:
        keyboard_shift = 1;
        return;
        break;

    /* shift break */
    case 0xaa:
    case 0xb6:
        keyboard_shift = 0;
        return;
        break;

    /* caps lock */
    case 0x3a:
        /* led */
        wait_keyboard_buffer();
        outb(KEYBOARD_DATA_REG, KEYBOARD_LED_CMD);
        wait_keyboard_ack();
        wait_keyboard_buffer();
        outb(KEYBOARD_DATA_REG, keyboard_caps ^ 0x04);
        wait_keyboard_ack();
        keyboard_caps ^= 4;
        return;
        break;

    default:
        break;
    }
}

/***************************************************************
 * description : 键盘, IRQ_FUNCPTR
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC handle_keyboard_int_2(os_u32 irq)
{
    struct keyboard_msg *kb_msg;

    /* make code or break code is incorrect */
    if (KEYBOARD_BUTTON_NUM > (kb_scan_code & BREAK_2_MAKE_CODE)) {
        /* 不是广播消息, 发送到当前窗口 */
        kb_msg = (struct keyboard_msg *) alloc_msg(sizeof(struct keyboard_msg));
        if (OS_NULL == kb_msg) {
            /* 消息内存分配失败, 不发送 */
            return;
        }

        /* 发送者为isr */
        kb_msg->msg_name = OS_MSG_KEYBOARD;
        kb_msg->msg_len = sizeof(struct keyboard_msg);
        kb_msg->up_down = (kb_scan_code & (~BREAK_2_MAKE_CODE)) ? (KEYBOARD_UP) : (KEYBOARD_DOWN);
        kb_msg->asc = (keyboard_shift) ? (keyboard_scan_code[kb_scan_code & BREAK_2_MAKE_CODE].shift) : (keyboard_scan_code[kb_scan_code & BREAK_2_MAKE_CODE].down);

        post_msg(get_current_window_handle(), kb_msg);
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void clear_keyboard_buffer(os_void)
{
    os_u8 buffer;

    /* 参考8042芯片, 对于状态寄存器0x64端口读取, 位0=1表示输出缓冲区满 */
    do {
        inb_p(KEYBOARD_DATA_REG, buffer);
        inb_p(KEYBOARD_STATUS_REG, buffer);
    } while (buffer & 0x01);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void init_keyboard_int(os_void)
{
    /* 没有按下shift键 */
    keyboard_shift = 0;
    /* 没有按下caps键 */
    keyboard_caps = 0;

    /* 初始化键盘中断 */
    install_int(SYS_KEYBOARD_INT_VECTOR, handle_keyboard_int_1, handle_keyboard_int_2);

    /* 清空键盘缓冲区 */
    clear_keyboard_buffer();
}

