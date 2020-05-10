/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : vga_bios.h
 * version     : 1.0
 * description : vga protect mode bios function
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __VGA_BIOS_H__
#define __VGA_BIOS_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/
#define VGA_REAL_CODE_64K_NO RM_CODE_STACK_SEG_NO

/* vga code segment */
#define VGA_REAL_CODE_SEG (VGA_REAL_CODE_64K_NO * 0x1000)

/* real mode stack */
#define VGA_REAL_MODE_STACK VGA_REAL_CODE_SEG

/***************************************************************
 enum define
 ***************************************************************/

/***************************************************************
 struct define
 ***************************************************************/

/***************************************************************
 extern function
 ***************************************************************/
os_ret init_vga_graphics_mode(struct graphics_mode_info *data, enum graphics_mode mode);

#pragma pack()

#endif

