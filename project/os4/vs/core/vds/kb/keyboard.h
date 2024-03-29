/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : keyboard.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/
/* �����ж������� */
#define SYS_KEYBOARD_INT_VECTOR 1

/* ���̰������� */
#define KEYBOARD_BUTTON_NUM 0x80

/* break code and make code */
#define BREAK_2_MAKE_CODE 0x7f

/* �������ݼĴ���, 8042 */
#define KEYBOARD_DATA_REG 0x60

/* ����״̬�Ĵ���, 8042 */
#define KEYBOARD_STATUS_REG 0x64

/* ����ȷ���� */
#define KEYBOARD_ACK 0xfa

/* ������ʾ�ƿ����� */
#define KEYBOARD_LED_CMD 0xed

/***************************************************************
 enum define
 ***************************************************************/

/***************************************************************
 struct define
 ***************************************************************/

/***************************************************************
 extern function
 ***************************************************************/

#pragma pack()

#endif /* end of header */

