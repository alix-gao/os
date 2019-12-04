/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : vesa_bios.h
 * version     : 1.0
 * description : vesa protect mode bios function
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __VESA_BIOS_H__
#define __VESA_BIOS_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/
#define VESA_REAL_CODE_64K_NO RM_CODE_STACK_SEG_NO

/* vga code segment */
#define VESA_REAL_CODE_SEG (VESA_REAL_CODE_64K_NO * 0x1000)

/* real mode stack */
#define VESA_REAL_MODE_STACK VESA_REAL_CODE_SEG

#define VESA_REAL_DATA_SEG (RM_DATA_SEG_NO * 0x1000)
#define VESA_REAL_DATA_OFF 0x0

/***************************************************************
 enum define
 ***************************************************************/

/***************************************************************
 struct define
 ***************************************************************/

/***************************************************************
 extern function
 ***************************************************************/
os_ret set_vesa_graphics_mode(struct graphics_mode_info *data);

#pragma pack()

#endif /* end of header */

