/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : gdi.h
 * version     : 1.0
 * description : 图形装置界面函数
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __GDI_H__
#define __GDI_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

#ifndef __TYPE_H__
    #error "include type.h before"
#endif

/***************************************************************
 macro define
 ***************************************************************/
/* 字母占列像素数 */
#define FONT_SIZE 8

/***************************************************************
 enum define
 ***************************************************************/
/***************************************************************
 * description :
 ***************************************************************/
enum graphics_mode {
    GRAPHICES_MODE_VGA,
    GRAPHICES_MODE_VESA,
    GRAPHICES_MODE_BUTT
};

/***************************************************************
 struct define
 ***************************************************************/
/***************************************************************
 * description : graphics mode info
 ***************************************************************/
struct graphics_mode_info {
    os_u16 x_resolution;
    os_u16 y_resolution;
    os_u8 bits_per_pixel; // 16, 32, etc.
    os_u8 plane_count;
    os_u8 memory_model;
    os_u32 PhysBasePtr;
};

/***************************************************************
 extern function
 ***************************************************************/
os_ret OS_API open_graphics_mode(enum graphics_mode mode);
os_u8 OS_API current_bpp(os_void);

#pragma pack()

#endif /* end of header */

