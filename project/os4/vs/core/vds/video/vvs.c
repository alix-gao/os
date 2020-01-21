/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : vvs.c
 * version     : 1.0
 * description : 虚拟视频设备接口
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include "vvs.h"

/***************************************************************
 global variable declare
 ***************************************************************/
LOCALD struct graphics_mode_info curr_graphics_mode = { 0 };

/***************************************************************
 function declare
 ***************************************************************/

/***************************************************************
 * description : 启动显卡图形模式
 * history     :
 ***************************************************************/
os_ret OS_API open_graphics_mode(enum graphics_mode mode)
{
    os_ret ret;
    struct graphics_mode_info info;
    lock_t eflag;
    os_u16 save;

    lock_int(eflag);

    save = close_pic(0xffff);

    switch (mode) {
    case GRAPHICES_MODE_VGA:
        ret = init_vga_graphics_mode(&info);
        break;

    case GRAPHICES_MODE_SVGA:
    case GRAPHICES_MODE_VESA:
        ret = set_vesa_graphics_mode(&info, mode);
        break;

    default:
        cassert(OS_FALSE);
        break;
    }
    if (OS_SUCC == ret) {
        mem_cpy(&curr_graphics_mode, &info, sizeof(struct graphics_mode_info));
    }

    open_pic(save);

    unlock_int(eflag);

    return ret;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_u8 OS_API current_bpp(os_void)
{
    return curr_graphics_mode.bits_per_pixel;
}

/***************************************************************
 * description : 获取当前系统的分辨率
 * history     :
 ***************************************************************/
screen_csys OS_API current_resolution(os_void)
{
    screen_csys res;

    res.x = curr_graphics_mode.x_resolution;
    res.y = curr_graphics_mode.y_resolution;

    return res;
}

/***************************************************************
 * description : 清空屏幕
 * history     :
 ***************************************************************/
os_void clear_screen(enum vga_color color)
{
    screen_csys csys0;
    screen_csys csys1;

    csys0.x = csys0.y = 0;
    csys1 = current_resolution();

    draw_rect(csys0, csys1, color);
}

