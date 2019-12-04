/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : paint.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __VDS_PAINT_H__
#define __VDS_PAINT_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/

/***************************************************************
 enum define
 ***************************************************************/

/***************************************************************
 struct define
 ***************************************************************/

/***************************************************************
 extern function
 ***************************************************************/
os_void draw_point(screen_csys *p, enum vga_color color);
os_void draw_rect(screen_csys p0, screen_csys p1, enum vga_color color);
os_void draw_line(screen_csys p0, screen_csys p1, enum vga_color color);
os_void draw_curve(screen_csys p0, screen_csys p1, screen_csys p2, os_u32 level, enum vga_color color);
os_void win_draw_line(HDEVICE hdc, logic_csys p0, logic_csys p1);
os_void win_draw_rect(HDEVICE hdc, logic_csys p0, logic_csys p1);
os_void win_draw_curve(HDEVICE hdc, logic_csys p0, logic_csys p1, logic_csys p2, os_u32 level);

#pragma pack()

#endif

