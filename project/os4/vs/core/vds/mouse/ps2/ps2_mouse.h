/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : ps2_mouse.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2011-04-22
 ***************************************************************/

#ifndef __PS2_MOUSE_H__
#define __PS2_MOUSE_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/
/* ps2����ж������� */
#define PS2_MOUSE_INT_VECTOR 12

/* ������ݼĴ���, 8042 */
#define PS2_MOUSE_DATA_REG 0x60

/* ���״̬�Ĵ���, 8042 */
#define PS2_MOUSE_STATUS_REG 0x64

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

