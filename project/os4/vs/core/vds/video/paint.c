/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : paint.c
 * version     : 1.0
 * description :
 *                 x0     x1
 *             O┏━━━━━━━━━━ X轴
 *              ┃
 *            y0┃(x0, y0)
 *              ┃
 *            y1┃       (x1, y1)
 *              ┃
 *              ┃
 *             Y轴
 *
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include "paint.h"
#include "vvs.h"

/***************************************************************
 global variable declare
 ***************************************************************/
/* x轴分辨率 */
LOCALD volatile os_u32 paint_resolution_x _CPU_ALIGNED_ = 0;

/* y轴分辨率 */
LOCALD volatile os_u32 paint_resolution_y _CPU_ALIGNED_ = 0;

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description : (x0, y0)-(x1, y1)
 * history     :
 ***************************************************************/
LOCALC inline os_void write_pix(screen_csys *p, enum vga_color color)
{
    switch (current_bpp()) {
    case 32:
        vesa_32bpp_write_pix(p, color);
        return;
        break;
    case 24:
        vesa_24bpp_write_pix(p, color);
        return;
        break;
    case 16:
        vesa_16bpp_write_pix(p, color);
        return;
        break;
    case 4:
        vga_write_pix(p, color);
        return;
        break;
    default:
        break;
    }
}

/***************************************************************
 * description : (x0, y0)-(x1, y1)
 * history     :
 ***************************************************************/
LOCALC inline enum vga_color read_pix(screen_csys *p)
{
    switch (current_bpp()) {
    case 32:
        return vesa_32bpp_read_pix(p);
        break;
    case 24:
        return vesa_24bpp_read_pix(p);
        break;
    case 16:
        return vesa_16bpp_read_pix(p);
        break;
    case 4:
        return vga_read_pix(p);
        break;
    default:
        cassert(OS_FALSE);
        break;
    }
    return 0;
}

/***************************************************************
 * description : (x0, y0)-(x1, y1)
 * history     :
 ***************************************************************/
os_void draw_rect(screen_csys p0, screen_csys p1, enum vga_color color)
{
    screen_csys pos;

    /* bug fix, 增加保护 */
    p0.x = (paint_resolution_x < p0.x) ? paint_resolution_x : p0.x;
    p0.y = (paint_resolution_y < p0.y) ? paint_resolution_y : p0.y;

    p1.x = (paint_resolution_x < p1.x) ? paint_resolution_x : p1.x;
    p1.y = (paint_resolution_y < p1.y) ? paint_resolution_y : p1.y;

    for (pos.y = p0.y; pos.y < p1.y; pos.y++) {
        for (pos.x = p0.x; pos.x < p1.x; pos.x++) {
            write_pix(&pos, color);
        }
    }
}

/***************************************************************
 * description : bresenham arithmetic (x0, y0)-(x1, y1)
 * history     :
 ***************************************************************/
os_void draw_line(screen_csys p0, screen_csys p1, enum vga_color color)
{
    os_u32 steep;
    os_u32 x, y;
    os_u32 deltax, deltay;
    os_s32 error; // 误差
    os_s32 ystep;

    steep = abs(p1.y - p0.y) > abs(p1.x - p0.x);
    if (steep) {
        exchange(p0.x, p0.y);
        exchange(p1.x, p1.y);
    }
    if (p0.x > p1.x) {
        exchange(p0.x, p1.x);
        exchange(p0.y, p1.y);
    }

    deltax = p1.x - p0.x;
    deltay = abs(p1.y - p0.y);

    error = deltax / 2; // error = deltax >> 1;

    y = p0.y;
    if (p0.y < p1.y) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (x = p0.x; x < p1.x; x++) {
        screen_csys p;
        if (steep) {
            p.x = y;
            p.y = x;
        } else {
            p.x = x;
            p.y = y;
        }
        write_pix(&p, color);

        error -= deltay;
        if (0 > error) {
            y += ystep;
            error += deltax;
        }
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
struct plane_csys midpoint(struct plane_csys p0, struct plane_csys p1)
{
    struct plane_csys mid;

    mid.x = (p0.x+p1.x)/2;
    mid.y = (p0.y+p1.y)/2;
    return mid;
}

/***************************************************************
 * description : 贝塞尔曲线
 *               该递归函数的复杂度是2^n.
 * history     :
 ***************************************************************/
os_void draw_curve(screen_csys p0, screen_csys p1, screen_csys p2, os_u32 level, enum vga_color color)
{
    screen_csys mid;
    screen_csys tmp_1;
    screen_csys tmp_2;

    /* BEZIER_DEEPTH为2的倍数 */
    level = level & (BEZIER_DEEPTH - 1);

    /* 递归完毕 */
    if ((0 == level) || (BEZIER_DEEPTH < level)) {
        draw_line(p0, p2, color);
        return;
    }

    tmp_1 = midpoint(p0, p1);
    tmp_2 = midpoint(p1, p2);

    mid = midpoint(tmp_1, tmp_2);

    draw_curve(p0, tmp_1, mid, level-1, color);
    draw_curve(mid, tmp_2, p2, level-1, color);
}

/***************************************************************
 * description : (x0, y0)-(x1, y1)
 * history     :
 ***************************************************************/
os_void draw_point(screen_csys *p, enum vga_color color)
{
    cassert(OS_NULL != p);
    write_pix(p, color);
}

/***************************************************************
 * description : 逻辑坐标转化为屏幕坐标
 * history     :
 ***************************************************************/
LOCALC inline screen_csys logic_csys_2_screen_csys(screen_csys *wsc, logic_csys p)
{
    screen_csys lc;

    lc.x = wsc->x+p.x;
    lc.y = wsc->y+p.y;
    return lc;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void win_write_pix(HDEVICE hdc, logic_csys p, enum vga_color color)
{
    screen_csys tmp;

    /* 入参检查 */
    cassert(OS_NULL != hdc);

    if ((get_window_width(hdc) > p.x) && (get_window_length(hdc) > p.y)) {
        tmp = logic_csys_2_screen_csys(&((struct device_context *) hdc)->csys, p);
        if ((paint_resolution_x > tmp.x) && (paint_resolution_y > tmp.y)) {
            write_pix(&tmp, color);
        }
    }
}

/***************************************************************
 * description : bresenham arithmetic (x0, y0)-(x1, y1)
 * history     :
 ***************************************************************/
os_void win_draw_line(HDEVICE hdc, logic_csys p0, logic_csys p1)
{
    os_u32 steep;
    os_u32 x, y;
    os_u32 deltax, deltay;
    os_s32 error; // 误差
    os_s32 ystep;
    enum vga_color color;

    cassert(OS_NULL != hdc);

    /* 取前景颜色 */
    color = ((struct device_context *) hdc)->foreground_color;

    steep = abs(p1.y - p0.y) > abs(p1.x - p0.x);
    if (steep) {
        exchange(p0.x, p0.y);
        exchange(p1.x, p1.y);
    }
    if (p0.x > p1.x) {
        exchange(p0.x, p1.x);
        exchange(p0.y, p1.y);
    }

    deltax = p1.x - p0.x;
    deltay = abs(p1.y - p0.y);

    error = deltax / 2; // error = deltax >> 1;

    y = p0.y;
    if (p0.y < p1.y) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (x = p0.x; x < p1.x; x++) {
        screen_csys p;
        if (steep) {
            p.x = y;
            p.y = x;
        } else {
            p.x = x;
            p.y = y;
        }
        write_pix(&p, color);

        error -= deltay;
        if (0 > error) {
            y += ystep;
            error += deltax;
        }
    }
}

/***************************************************************
 * description : (x0, y0)-(x1, y1)
 * history     :
 ***************************************************************/
os_void win_draw_rect(HDEVICE hdc, logic_csys p0, logic_csys p1)
{
    screen_csys pos;
    struct device_context *device_handle;
    enum vga_color color;
    screen_csys tmp;

    /* 入参检查 */
    cassert(OS_NULL != hdc);

    device_handle = hdc;

    color = device_handle->foreground_color;

    /* 入参检查 */
    p0.x = (paint_resolution_x < p0.x) ? paint_resolution_x : p0.x;
    p0.y = (paint_resolution_y < p0.y) ? paint_resolution_y : p0.y;

    p1.x = (paint_resolution_x < p1.x) ? paint_resolution_x : p1.x;
    p1.y = (paint_resolution_y < p1.y) ? paint_resolution_y : p1.y;

    for (pos.y = p0.y; pos.y < p1.y; pos.y++) {
        for (pos.x = p0.x; pos.x < p1.x; pos.x++) {
            tmp = logic_csys_2_screen_csys(&device_handle->csys, pos);
            write_pix(&tmp, color);
        }
    }
}

/***************************************************************
 * description : 贝塞尔曲线, BEZIER
 * history     :
 ***************************************************************/
os_void win_draw_curve(HDEVICE hdc, logic_csys p0, logic_csys p1, logic_csys p2, os_u32 level)
{
    screen_csys mid;
    screen_csys tmp_1;
    screen_csys tmp_2;

    cassert(OS_NULL != hdc);

    /* BEZIER_DEEPTH为2的倍数 */
    level = level & (BEZIER_DEEPTH - 1);

    /* 递归未完毕 */
    if ((0 != level) && ((1 < abs(p0.x - p2.x)) || (1 < abs(p0.y - p2.y)))) {
        tmp_1 = midpoint(p0, p1);
        tmp_2 = midpoint(p1, p2);
        mid = midpoint(tmp_1, tmp_2);
        win_draw_curve(hdc, p0, tmp_1, mid, level-1);
        win_draw_curve(hdc, mid, tmp_2, p2, level-1);
        return;
    }
    /* 递归完毕 */
    win_draw_line(hdc, p0, p2);
}

/***************************************************************
 * description : 橡皮擦
 * history     :
 ***************************************************************/
os_ret OS_API win_eraser(IN HDEVICE hdc, logic_csys p0, logic_csys p1)
{
    struct device_context *device_handle;

    /* 入参检查 */
    if (OS_NULL != hdc) {
        device_handle = hdc;
        p0.x += device_handle->csys.x;
        p0.y += device_handle->csys.y;
        p1.x += device_handle->csys.x;
        p1.y += device_handle->csys.y;
        draw_rect(p0, p1, device_handle->background_color);
        return OS_SUCC;
    }
    /* 入参检查失败 */
    return OS_FAIL;
}

/***************************************************************
 * description : 清空窗口区域
 * history     :
 ***************************************************************/
os_ret OS_API win_clear_screen(IN HDEVICE hdc)
{
    struct device_context *device_handle;
    screen_csys csys0;
    screen_csys csys1;

    if (OS_NULL != hdc) {
        device_handle = hdc;

        /* 初始化光标位置 */
        device_handle->cursor_pos.x = device_handle->cursor_pos.y = 0;

        csys0.x = device_handle->csys.x;
        csys0.y = device_handle->csys.y;
        csys1.x = device_handle->csys.x + device_handle->width;
        csys1.y = device_handle->csys.y + device_handle->length;

        draw_rect(csys0, csys1, device_handle->background_color);
        return OS_SUCC;
    }
    /* 入参检查失败 */
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API paint_icon(IN screen_csys *sp, IN struct os_icon *icon, IN icon_csys *ips, IN icon_csys *ipe)
{
    screen_csys pos;
    const enum vga_color (*data)[16];
    os_u32 i, j; /* data偏移 */

    data = icon->u.data_16_32_24;

    for (j = ips->y; j < ipe->y; j++) {
        for (i = ips->x; i < ipe->x; i++) {
            pos.x = sp->x + i - ips->x;
            pos.y = sp->y + j - ips->y;
            if (VGA_TRAN != data[j][i]) {
                write_pix(&pos, data[j][i]);
            }
        }
    }
    return OS_SUCC;
}

/***************************************************************
 * description : 读屏幕按照行来读
 * history     :
 ***************************************************************/
os_ret OS_API read_screen(IN screen_csys *sp, OUT enum vga_color (*data)[16], IN icon_csys *ips, IN icon_csys *ipe)
{
    screen_csys pos;
    os_u32 i, j; /* data偏移 */

    for (j = ips->y; j < ipe->y; j++) {
        for (i = ips->x; i < ipe->x; i++) {
            pos.x = sp->x + i - ips->x;
            pos.y = sp->y + j - ips->y;
            data[j][i] = read_pix(&pos);
        }
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void init_paint(os_void)
{
    screen_csys r;

    r = current_resolution();

    paint_resolution_x = r.x - 1;
    paint_resolution_y = r.y - 1;
}

