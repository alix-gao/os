/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : bmp.c
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2011-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <lib.h>
#include <vds.h>
#include <image.h>

#include "bmp.h"

/***************************************************************
 global variable declare
 ***************************************************************/

/***************************************************************
 function declare
 ***************************************************************/

/***************************************************************
 * description : bmp要求每行的数据的长度必须是4的倍数
 *               如果不够需要进行比特填充(以0填充)
 *               rowsize = 4 * 上取整(bpp * width / 32)
 * history     :
 ***************************************************************/
LOCALC os_u32 get_bmp_row_skip(os_u16 bpp, os_s32 width)
{
    os_u32 a,b;

    a = (os_u64)(bpp * width) >> 3;
    b = 4;
    divl(a, b, a, b);
    if (0 == b) {
        return 0;
    } else {
        return 4 - b;
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void stretch_bmp_img(os_u32 win_x, os_u32 win_y, os_u32 img_x, os_u32 img_y, HDEVICE hdc, HFILE fp, os_u32 skip)
{
    os_u32 i, j, k, l;
    os_u32 m_x, m_y;
    os_u32 d_x, d_y;
    os_u32 step_x, step_y;
    os_u32 pixel;
    screen_csys p;

    m_x = win_x / img_x;
    if (win_x % img_x) {
        step_x = img_x / (win_x % img_x);
    } else {
        step_x = img_x + 1;
    }

    m_y = win_y / img_y;
    if (win_y % img_y) {
        step_y = img_y / (win_y % img_y);
    } else {
        step_y = img_y + 1;
    }

    d_y = 0;
    p.y = win_y - 1;
    for (j = img_y; j > 0; j--) {
        p.x = 0;
        for (i = 0; i < img_x; i++) {
            /* read file */
            read_file(fp, (os_u8 *) &pixel, 3);

            /* start draw pixel block */
            d_x = m_x + ((0 == (i % step_x)) ? 1 : 0);
            d_y = m_y + ((0 == (j % step_y)) ? 1 : 0);
            for (k = 0; k < d_x; k++) {
                os_u32 save_y = p.y;
                for (l = 0; l < d_y; l++) {
                    win_write_pix(hdc, p, 0x00ffffff & pixel);
                    if (p.y) {
                        p.y--;
                    }
                }
                p.y = save_y;
                p.x++;
            }
            /* complete draw pixel block */
        }
        if (p.y > d_y) {
            p.y -= d_y;
        } else {
            p.y = 0;
        }
        for (k = 0; k < skip; k++) {
            read_file(fp, (os_u8 *) &pixel, 1);
        }
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void draw_bmp_pixel(HDEVICE hdc, os_u32 pixel, os_u32 x0, os_u32 y0, os_u32 x1, os_u32 y1)
{
    screen_csys p;
    os_u32 x, y;

    for (x = x0; x < x1; x++) {
        for (y = y0; y < y1; y++) {
            p.x = x;
            p.y = y;
            win_write_pix(hdc, p, 0x00ffffff & pixel);
        }
    }
}

/***************************************************************
 * description : 水平翻转
 * history     :
 ***************************************************************/
LOCALC os_void scale_revert_bmp_img(os_u32 win_x, os_u32 win_y, os_u32 img_x, os_u32 img_y, HDEVICE hdc, HFILE fp, os_u32 skip)
{
    os_u32 x, y;
    os_u32 pixel;
    os_u32 curr_x, curr_y, last_x, last_y;
    os_u32 i;

    curr_x = curr_y = 0;
    last_y = win_y;
    for (y = img_y; y > 0; y--) {
        last_x = win_x;
        curr_y = ((y - 1) * win_y) / img_y;
        for (x = img_x; x > 0; x--) {
            read_file(fp, (os_u8 *) &pixel, 3);

            curr_x = ((x - 1) * win_x) / img_x;
            if ((curr_x != last_x) && (curr_y != last_y)) {
                draw_bmp_pixel(hdc, pixel, curr_x, curr_y, last_x, last_y);
                last_x = curr_x;
            }
        }
        if (curr_y != last_y) {
            last_y = curr_y;
        }
        for (i = 0; i < skip; i++) {
            read_file(fp, (os_u8 *) &pixel, 1);
        }
    }
}

/***************************************************************
 * description : delay divide
 * history     :
 ***************************************************************/
LOCALC os_void scale_bmp_img(os_u32 win_x, os_u32 win_y, os_u32 img_x, os_u32 img_y, HDEVICE hdc, HFILE fp, os_u32 skip)
{
    os_u32 x, y;
    os_u32 pixel;
    os_u32 curr_x, curr_y, last_x, last_y;
    os_u32 i;

    curr_x = curr_y = 0;
    last_y = win_y;
    for (y = img_y; y > 0; y--) { //s32 y; for (y = img_y - 1; y >= 0; y--) {
        last_x = win_x;
        curr_y = ((y - 1) * win_y) / img_y;
        for (x = img_x; x > 0; x--) {
            read_file(fp, (os_u8 *) &pixel, 3);

            curr_x = ((x - 1) * win_x) / img_x;
            if ((curr_x != last_x) && (curr_y != last_y)) {
                draw_bmp_pixel(hdc, pixel, win_x - last_x, curr_y, win_x - curr_x, last_y);
                last_x = curr_x;
            }
        }
        if (curr_y != last_y) {
            last_y = curr_y;
        }
        for (i = 0; i < skip; i++) {
            read_file(fp, (os_u8 *) &pixel, 1);
        }
    }
}

/***************************************************************
 * description : 24位真彩色没有调色板
 * history     :
 ***************************************************************/
LOCALC os_ret show_24bpp_bmp(HWINDOW handle, HFILE fp, struct bmp_file *head, enum show_image_mode_type mode)
{
    HDEVICE hdc;
    os_s32 size;
    screen_csys p;
    os_s32 x, y;
    os_s32 width;
    os_u32 skip;
    os_u8 pixel[3];
    os_u32 i;

    seek_file(fp, head->header.bmp_offset, SEEK_POS_SET);

    hdc = open_hdc(handle);
    if (OS_NULL == hdc) {
        flog("show bmp, open hdc fail\n");
        return OS_FAIL;
    }

    skip = get_bmp_row_skip(head->bitmap_info.bitspp, head->bitmap_info.width);
    switch (mode) {
    case SI_MODE_1:
        /* 读文件, 限制在文件长度范围内 */
        size = head->header.filesz;
        width = head->bitmap_info.width;
        /* 平铺方式 */
        for (y=head->bitmap_info.height-1; y>=0; y--) {
            for (x = 0; x < width; x++) {
                read_file(fp, pixel, 3);
                p.x = x;
                p.y = y;
                /* *(os_u32 *) pixel高位需要清零 */
                win_write_pix(hdc, p, 0x00ffffff & (*(os_u32 *) pixel));
                size -= 3;
                if (size <= 0) {
                    goto end;
                }
            }
            /* 每行扫描线的跨越对齐字节 */
            for (i = 0; i < skip; i++) {
                read_file(fp, pixel, 1);
            }
        }
        break;

    case SI_MODE_2:
#if 0
        stretch_bmp_img(get_window_width(hdc), get_window_length(hdc),
                        head->bitmap_info.width, head->bitmap_info.height,
                        hdc, fp, skip);
#else
        scale_bmp_img(get_window_width(hdc), get_window_length(hdc),
                      head->bitmap_info.width, head->bitmap_info.height,
                      hdc, fp, skip);
#endif
        break;

    default:
        cassert(OS_FALSE);
        break;
    }
  end:
    close_hdc(handle, hdc);
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret show_32bpp_bmp(HWINDOW handle, HFILE fp, struct bmp_file *head, enum show_image_mode_type mode)
{
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret bmp_show(HWINDOW handle, HFILE fp, enum show_image_mode_type mode)
{
    struct bmp_file data;
    os_ret ret;

    cassert((OS_NULL != handle) && (OS_NULL != fp) && (SI_MODE_BUTT > mode));

    ret = read_file(fp, (os_u8 *) &data, sizeof(struct bmp_file));
    if (OS_SUCC != ret) {
        flog("read file fail\n");
        return OS_FAIL;
    }

    /* parse */
    if (('B' != data.magic.magic[0]) || ('M' != data.magic.magic[1])) {
        flog("file is not bmp\n");
        return OS_FAIL;
    }

    /* 限制, 只支持V3 header, 不压缩 */
    if ((0x28 != data.bitmap_info.header_sz)
     || (0 != data.bitmap_info.compress_type)) {
        flog("restrict bmp\n");
        return OS_FAIL;
    }

    switch (data.bitmap_info.bitspp) {
    case 24:
        show_24bpp_bmp(handle, fp, &data, mode);
        break;
    case 32:
        show_32bpp_bmp(handle, fp, &data, mode);
        break;
    default:
        break;
    }
    return OS_SUCC;
}

/* bmp图像操作 */
LOCALD const struct image_operation bmp_operation = {
    bmp_show
};
LOCALD const struct image_info bmp_info = {
    "bmp",
    &bmp_operation
};

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void init_bmp_driver(os_void)
{
    os_ret result;

    result = register_image(&bmp_info);
    cassert(OS_SUCC == result);
}

device_init_func(init_bmp_driver);

