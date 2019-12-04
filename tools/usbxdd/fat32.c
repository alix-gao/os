/****************************************************************
 * copyright (c) 2014, gaocheng
 * all rights reserved.
 *
 * file name  : fat32.c
 * version    : 1.0
 * description: read fat32 information from thumb image file
 *              mbr: 0x0
 *              dbr: 0x4000 {
 *                  BPB_BytsPerSec: 0x200
 *                  BPB_SecPerClus: 0x8
 *                  BPB_RsvdSecCnt: 0x246
 *                  BPB_NumFATs: 0x2
 *                  BPB_RootEntCnt: 0x0
 *                  BPB_FATSz32: 0xedd
 *              }
 *
 *              RootDirSectors = ((BPB_RootEntCnt * 32) + (BPB_BytsPerSec ¨C 1)) / BPB_BytsPerSec;
 *                             =
 *              FirstDataSector = BPB_ResvdSecCnt + (BPB_NumFATs * FATSz) + RootDirSectors;
 *                              = 0x246 + (2 * 0xedd) + 0 = 0x2000
 *
 *                              = 0x3f + 0xfc + (2 * 0xff8) = 0x212b
 *
 * author     : gaocheng
 * date       : 2013-04-22
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#include "fat32.h"

/* dlayout.inc */
u16 BYTE_PER_SECTOR = 0x200;
u8 DBR_HEAD = 1;
u8 DBR_SECTOR = 1;
u8 DBR_CYLINDER = 0;
u32 ASM16_LBA = 0x7e;
u16 ASM16_LEN = 0x2000;
u32 COS16_LBA = 0xbd;
u16 COS16_LEN = 0xfc00;

void init_asm_variable(void)
{
#include "../../7/16/dlayout.inc"
}

int main(void)
{
    FILE *fp_src;
    FILE *fp_dst;
    u32 sector_cnt;
    u32 i;
    struct fat32_bpb *bpb;
    u8 *sector;

    init_asm_variable();

    sector = NULL;
    fp_src = fp_dst = NULL;

    sector = (u8 *) malloc(BYTE_PER_SECTOR);
    if (NULL == sector) {
        info("malloc sector fail\n");
        goto end;
    }

    fp_src = fopen("u.ima", "rb");
    if (NULL == fp_src) {
        info("cannot open file u.ima\n");
        goto end;
    }

    fp_dst = fopen("fat32.ima", "wb");
    if (NULL == fp_dst) {
        info("cannot open file fat32.bin\n");
        goto end;
    }

    sector_cnt = 0;

    /* read mbr */
    fseek(fp_src, BYTE_PER_SECTOR * sector_cnt, SEEK_SET);
    fread(sector, sizeof(char), BYTE_PER_SECTOR, fp_src);

    /* file system of dpt 0 */
    switch (sector[DPT_0_OFFSET + FS_TYPE_OFFSET]) {
    case FS_TYPE_FAT32:
    case FS_TYPE_WIN95_FAT32_1:
    case FS_TYPE_WIN95_FAT32_2:
        info("file system: 0x%x\n", sector[DPT_0_OFFSET + FS_TYPE_OFFSET]);
        break;
    default:
        info("unsupport file system (%x)\n", sector[DPT_0_OFFSET + FS_TYPE_OFFSET]);
        goto end;
        break;
    }

    /* relative position of dbr */
    sector_cnt += *(u32 *)(&sector[DPT_0_OFFSET + DPT_POS_OFFSET]);
    info("count dbr start sector: 0x%x\n", sector_cnt);

    /* read dbr */
    fseek(fp_src, BYTE_PER_SECTOR * sector_cnt, SEEK_SET);
    fread(sector, sizeof(char), BYTE_PER_SECTOR, fp_src);

    bpb = (struct fat32_bpb *) sector;
    /* BPB_RsvdSecCnt:
       Number of reserved sectors in the reserved region of the volume
       starting at the first sector of the volume. this field must not be 0.
       for FAT12 and FAT16 volumes, this value should never be
       anything other than 1. for FAT32 volumes, this value is typically
       32. there is a lot of FAT code in the world "hard wired" to 1
       reserved sector for FAT12 and FAT16 volumes and that doesn't
       bother to check this field to make sure it is 1. microsoft operating
       systems will proberly support any non-zero value in this field.
       BPB_RootEntCnt:
       for FAT12 and FAT16 volumes, this field contains the count of 32-
       byte directory entries in the root directory. for FAT32 volumes,
       this field must be set to 0. for FAT12 and FAT16 volumes, this
       value should always specify a count that when multiplied by 32
       results in an even multiple of BPB_BytsPerSec. for maximum
       compatibility, FAT16 volumes should use the value 512. */
    if ((1 == bpb->BPB_RsvdSecCnt) || (0 != bpb->BPB_RootEntCnt)) {
        info("this file system is not fat32\n");
        goto end;
    }

    /* count of bytes per sector.
       this value may take on only the following vlaues:
       512, 1024, 2048 or 4096.
       if maximum compatibility with old implementations is desired,
       only the value 512 should be used.
       there is a lot of fat code in the world that is basically "hard wired" to 512 bytes per sector and doesn't bother to check this field to make sure it is 512.
       microsoft operating systems will properly support 1024, 2048 and 4096. */
    if (BYTE_PER_SECTOR != bpb->BPB_BytsPerSec) {
        info("BPB_BytsPerSec is not 512\n");
        goto end;
    }

    info("valid fat32!\n");

    info("BPB_SecPerClus: 0x%x\n", bpb->BPB_SecPerClus);
    info("BPB_SecPerTrk: 0x%x\n", bpb->BPB_SecPerTrk);
    info("BPB_NumHeads: 0x%x\n", bpb->BPB_NumHeads);
    info("BPB_RsvdSecCnt: 0x%x\n", bpb->BPB_RsvdSecCnt);
    info("BPB_HiddSec: 0x%x\n", bpb->BPB_HiddSec);

    sector_cnt += bpb->BPB_RsvdSecCnt;
    info("count bpb rsvd sector: 0x%x\n", sector_cnt);
    sector_cnt += (bpb->BPB_NumFATs * bpb->BPB_FATSz32);
    info("count bpb fat*num sector: 0x%x\n", sector_cnt);

    /* read one clus sector of dir */
    sector_cnt += bpb->BPB_SecPerClus;
    info("count all sector: 0x%x\n", sector_cnt);

    /* write all */
    fseek(fp_src, 0, SEEK_SET);
    for (i = 0; i < sector_cnt; i++) {
        fread(sector, sizeof(char), BYTE_PER_SECTOR, fp_src);
        fwrite(sector, sizeof(char), BYTE_PER_SECTOR, fp_dst);
    }

  end:
    if (sector) { free(sector); }
    if (fp_dst) { fclose(fp_dst); }
    if (fp_src) { fclose(fp_src); }
    printf("\npress any key to continue...");
    getch();
}

