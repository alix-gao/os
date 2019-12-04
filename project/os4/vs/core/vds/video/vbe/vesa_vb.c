/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : vesa_vb.c
 * version     : 1.0
 * description : (key) 画像素没有关中断, 因为不涉及临界io资源的操作
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include "vesa_bios.h"
#include "vesa.h"
#include "vesa_vb.h"

/***************************************************************
 global variable declare
 ***************************************************************/
/* physical address for flat memory frame buffer */
LOCALD os_u8 *vesa_video_buffer = OS_NULL;

LOCALD os_u16 LinBytesPerScanLine = 0;

LOCALD os_u8 BytesPerPixel;

LOCALD os_u32 RedMask;
LOCALD os_u8 RedMaskPos;
LOCALD os_u32 GreenMask;
LOCALD os_u8 GreenMaskPos;
LOCALD os_u32 BlueMask;
LOCALD os_u8 BlueMaskPos;
LOCALD os_u32 ReservedMask;
LOCALD os_u8 ReservedMaskPos;

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void init_vesa_paint(struct ModeInfoBlock *vmib)
{
    cassert(OS_NULL != vmib);

    vesa_video_buffer = (os_u8 *) vmib->PhysBasePtr;

    RedMaskPos = vmib->RedMaskPos;
    RedMask = ((1 << vmib->RedMaskSize) - 1) << vmib->RedMaskPos;
    GreenMaskPos = vmib->GreenMaskPos;
    GreenMask = ((1 << vmib->GreenMaskSize) - 1) << vmib->GreenMaskPos;
    BlueMaskPos = vmib->BlueMaskPos;
    BlueMask = ((1 << vmib->BlueMaskSize) - 1) << vmib->BlueMaskPos;
    ReservedMaskPos = vmib->ReservedMaskPos;
    ReservedMask = ((1 << vmib->ReservedMaskSize) - 1) << vmib->ReservedMaskPos;

    BytesPerPixel = vmib->BitsPerPixel / 8;

    if (0 == vmib->LinBytesPerScanLine) {
        LinBytesPerScanLine = vmib->XResolution * BytesPerPixel;
    } else {
        LinBytesPerScanLine = vmib->LinBytesPerScanLine;
    }
}

#define rgb888_to_565(color) (((((color) >> 19) & 0x1f) << 11) | ((((color) >> 10) & 0x3f) << 5) | (((color) >> 3) & 0x1f))

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void vesa_16bpp_write_pix(screen_csys *p, enum vga_color color)
{
    os_u16 *pixel;

    cassert(OS_NULL != p);

    pixel = (os_u16 *)(vesa_video_buffer + (p->y) * LinBytesPerScanLine + p->x * BytesPerPixel);

    *pixel = rgb888_to_565(color);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
enum vga_color vesa_16bpp_read_pix(screen_csys *p)
{
    os_u16 pixel;

    cassert(OS_NULL != p);

    pixel = *(os_u16 *)(vesa_video_buffer + (p->y) * LinBytesPerScanLine + p->x * BytesPerPixel);
    return (((pixel & 0xf800) << 8) | ((pixel & 0x07e0) << 5) | ((pixel & 0x001f) << 3));
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void vesa_24bpp_write_pix(screen_csys *p, enum vga_color color)
{
    os_u8 *pixel;
    os_u32 r,g,b;

    cassert(OS_NULL != p);

    r = (color >> 0) & 0xff;
    g = (color >> 8) & 0xff;
    b = (color >> 16) & 0xff;

    pixel = vesa_video_buffer + (p->y) * LinBytesPerScanLine + p->x * BytesPerPixel;

    *(pixel + 0) = r;
    *(pixel + 1) = g;
    *(pixel + 2) = b;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
enum vga_color vesa_24bpp_read_pix(screen_csys *p)
{
    os_u32 pixel;

    cassert(OS_NULL != p);

    pixel = *(os_u32 *)(vesa_video_buffer + (p->y) * LinBytesPerScanLine + p->x * BytesPerPixel);
    return pixel & 0x00ffffff;
}

/***************************************************************
 * description : 888 format
 * history     :
 ***************************************************************/
os_void vesa_32bpp_write_pix(screen_csys *p, enum vga_color color)
{
    os_u32 *pixel;

    cassert(OS_NULL != p);

    pixel = (os_u32 *)(vesa_video_buffer + (p->y) * LinBytesPerScanLine + p->x * BytesPerPixel);
    *pixel = color;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
enum vga_color vesa_32bpp_read_pix(screen_csys *p)
{
    os_u32 *pixel;

    cassert(OS_NULL != p);

    pixel = (os_u32 *)(vesa_video_buffer + (p->y) * LinBytesPerScanLine + p->x * BytesPerPixel);
    return *pixel;
}

