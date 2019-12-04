/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : panic.c
 * version     : 1.0
 * description : 内核异常处理
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include <vds.h>

/***************************************************************
 global variable declare
 ***************************************************************/

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void panic(os_void)
{
    screen_csys p0,p1;

    /* 黑屏 */
    p0.x = 0;
    p0.y = 0;

    p1 = current_resolution();

    draw_rect(p0, p1, VGA_COLOR_BLACK);

    /* 初始化log msg */
    init_print();
}

