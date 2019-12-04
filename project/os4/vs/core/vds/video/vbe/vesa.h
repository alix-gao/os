/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : vesa.h
 * version     : 1.0
 * description : vesa, 本文件1字节对齐
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __VESA_H__
#define __VESA_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(1)

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
 * description : VESA bios
 ***************************************************************/
struct VbeInfoBlock {
    os_u8 VbeSignature[4]; /* 'VESA' */
    os_u16 VbeVersion; /* 0x0300 */
    os_u32 OemStringPtr;
    os_u8 Capabilities[4];
    os_u32 VideoModePtr;
    os_u16 TotalMemory;
    os_u16 OemSoftwareRev;
    os_u32 OemVendorNamePtr;
    os_u32 OemProductNamePtr;
    os_u32 OemProductRevPtr;
    os_u8 Reserved[222];
    os_u8 OemData[256];
};

/***************************************************************
 * description : VESA information for a specific mode
 ***************************************************************/
struct ModeInfoBlock {
    os_u16 ModeAttributes;
    os_u8  WinAAttributes;
    os_u8  WinBAttributes;
    os_u16 WinGranularity;
    os_u16 WinSize;
    os_u16 WinASegment;
    os_u16 WinBSegment;
    os_u32 WinFuncPtr;
    os_u16 BytesPerScanLine;

    os_u16 XResolution;
    os_u16 YResolution;
    os_u8  XCharSize;
    os_u8  YCharSize;
    os_u8  NumberOfPlanes;
    os_u8  BitsPerPixel; /* bpp */
    os_u8  NumberOfBanks;
    os_u8  MemoryModel;
    os_u8  BankSize;
    os_u8  NumberOfImagePages;
    os_u8  Reserved_page;

    os_u8  RedMaskSize;
    os_u8  RedMaskPos;
    os_u8  GreenMaskSize;
    os_u8  GreenMaskPos;
    os_u8  BlueMaskSize;
    os_u8  BlueMaskPos;
    os_u8  ReservedMaskSize;
    os_u8  ReservedMaskPos;
    os_u8  DirectColorModeInfo;

    /* VBE 2.0 extensions */
    os_u32 PhysBasePtr; /* physical address for flat memory frame buffer */
    os_u32 Reserved2;
    os_u16 Reserved1;

    /* VBE 3.0 extensions */
    os_u16 LinBytesPerScanLine;
    os_u8  BnkNumberOfPages;
    os_u8  LinNumberOfPages;
    os_u8  LinRedMaskSize;
    os_u8  LinRedFieldPos;
    os_u8  LinGreenMaskSize;
    os_u8  LinGreenFieldPos;
    os_u8  LinBlueMaskSize;
    os_u8  LinBlueFieldPos;
    os_u8  LinRsvdMaskSize;
    os_u8  LinRsvdFieldPos;
    os_u32 MaxPixelClock;

    os_u8  Reserved[189];
};

#define INVALID_MODE 0xffff

/***************************************************************
 * description : VESA information for a specific mode
 ***************************************************************/
struct vesa_mode_info {
    struct ModeInfoBlock mib;
    os_u16 mode_number;
};

/***************************************************************
 extern function
 ***************************************************************/
os_void init_vesa_paint(struct ModeInfoBlock *vmib);

enum {
    VESA_STANDARD_MODE_1024_768,
    BEST_VESA_MODE
};
struct vesa_mode_info *choose_vesa_mode(os_u32 choice);

#pragma pack()

#endif /* end of header */

