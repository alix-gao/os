/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : bmp.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2011-04-22
 ***************************************************************/

#ifndef __BMP_H__
#define __BMP_H__

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
 * enum name   :
 * description :
 ***************************************************************/
enum bmp_compression_method {
    BI_RGB = 0,
    BI_RLE8,
    BI_RLE4,
    BI_BITFIELDS, //Also Huffman 1D compression for BITMAPCOREHEADER2
    BI_JPEG,      //Also RLE-24 compression for BITMAPCOREHEADER2
    BI_PNG,
};

/***************************************************************
 struct define
 ***************************************************************/

/***************************************************************
 * description : bmp文件魔数
 ***************************************************************/
struct bmpfile_magic {
    os_u8 magic[2];
};

/***************************************************************
 * description :
 ***************************************************************/
struct bmpfile_header {
    os_u32 filesz;
    os_u16 creator1;
    os_u16 creator2;
    os_u32 bmp_offset;
};

/***************************************************************
 * description :
 ***************************************************************/
struct DIB_header {
    os_u32 header_sz;
    os_s32 width;
    os_s32 height;
    os_u16 nplanes;
    os_u16 bitspp;
    os_u32 compress_type;
    os_u32 bmp_bytesz;
    os_s32 hres;
    os_s32 vres;
    os_u32 ncolors;
    os_u32 nimpcolors;
};

/***************************************************************
 * description : 注意一字节对齐, 54字节
 ***************************************************************/
struct bmp_file {
    struct bmpfile_magic magic;
    struct bmpfile_header header;
    struct DIB_header bitmap_info;
} __attribute__((packed));

/***************************************************************
 extern function
 ***************************************************************/

#pragma pack()

#endif

