/****************************************************************
 * copyright (c) 2014, gaocheng
 * all rights reserved.
 *
 * file name  : reformat.c
 * version    : 1.0
 * description: format disk image again
 *              1. dbr reserved sector should more than 4*BPB_SecPerTrk
 * author     : gaocheng
 * date       : 2013-04-22
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#include "fat32.h"

u32 get_file_len(FILE *fp)
{
    u32 len;

    fseek(fp, 0, SEEK_END);

    len = ftell(fp);

    return len;
}

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

u8 rsvd_track_num(u16 BPB_SecPerTrk)
{
    u8 num;

    /* check asm16 length */
    if (COS16_LBA < (ASM16_LBA + ASM16_LEN/BYTE_PER_SECTOR + 1)) {
        info("asm16, cos16 conflict!\n");
        system("pause");
        while (1) ;
    }

    /* cos16 is the last one. */
    num = (COS16_LBA + (COS16_LEN / BYTE_PER_SECTOR)) / BPB_SecPerTrk + 1;
    info("rsvd track num: %d\n", num);
    return num;
}

int main(void)
{
    FILE *fp_src;
    FILE *fp_dst;
    u8 *src_data, *dst_data;
    u32 src_len, dst_len;
    u32 dbrstartSector;
    struct fat32_bpb *bpb;
    struct fat32_bpb *dst_bpb;
    u32 rsvd_sector;

    init_asm_variable();

    /* before goto end */
    src_data = dst_data = NULL;
    fp_src = fp_dst = NULL;

    fp_dst = fopen("FlashDisk.ima", "wb");
    if (NULL == fp_dst) {
        info("cannot open file fat32.bin\n");
        goto end;
    }

    fp_src = fopen("fat32.ima", "rb");
    if (NULL == fp_src) {
        info("cannot open file u.ima\n");
        goto end;
    }

    src_len = get_file_len(fp_src);
    info("file length: %d\n", src_len);
    fseek(fp_src, 0, SEEK_SET);

    src_data = malloc(src_len);
    if (NULL == src_data) {
        info("mallo fail\n");
        goto end;
    }

    fread(src_data, sizeof(char), src_len, fp_src);

    /* file system of dpt 0 */
    switch (src_data[DPT_0_OFFSET + FS_TYPE_OFFSET]) {
    case FS_TYPE_FAT32:
    case FS_TYPE_WIN95_FAT32_1:
    case FS_TYPE_WIN95_FAT32_2:
        info("file system: 0x%x\n", src_data[DPT_0_OFFSET + FS_TYPE_OFFSET]);
        break;
    default:
        info("unsupport file system (%x)\n", src_data[DPT_0_OFFSET + FS_TYPE_OFFSET]);
        goto end;
        break;
    }

    /* add boot flag */
    src_data[DPT_0_OFFSET + BOOT_FLAG_OFFSET] = 0x80;

    /* relative position of dbr */
    dbrstartSector = *(u32 *)(&src_data[DPT_0_OFFSET + DPT_POS_OFFSET]);
    info("dbr start sector: 0x%x\n", dbrstartSector);

    bpb = (struct fat32_bpb *) &src_data[BYTE_PER_SECTOR * dbrstartSector];
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

    if (BYTE_PER_SECTOR != bpb->BPB_BytsPerSec) {
        info("byte per sector error\n");
        goto end;
    }

    if (dbrstartSector != bpb->BPB_HiddSec) {
        info("hidden sector error\n");
        goto end;
    }

    info("valid fat32!\n");

    info("BPB_SecPerClus: 0x%x\n", bpb->BPB_SecPerClus);
    info("BPB_SecPerTrk: 0x%x\n", bpb->BPB_SecPerTrk);
    info("BPB_NumHeads: 0x%x\n", bpb->BPB_NumHeads);
    info("BPB_RsvdSecCnt: 0x%x\n", bpb->BPB_RsvdSecCnt);
    info("BPB_HiddSec: 0x%x\n", bpb->BPB_HiddSec);

    /* ajust reserved sectors */
    rsvd_sector = rsvd_track_num(bpb->BPB_SecPerTrk) * bpb->BPB_SecPerTrk;
    if (rsvd_sector < bpb->BPB_RsvdSecCnt) {
        rsvd_sector = bpb->BPB_RsvdSecCnt;
    }
    dst_len = src_len
            + BYTE_PER_SECTOR * (rsvd_sector - bpb->BPB_RsvdSecCnt)
            + BYTE_PER_SECTOR * (bpb->BPB_SecPerTrk - dbrstartSector);
    dst_data = (u8 *) malloc(dst_len);
    if (NULL == dst_data) {
        info("mallo fail\n");
        goto end;
    }
    memset(dst_data, 0, dst_len);

    /* [mbr, dbr) */
    memcpy(dst_data, src_data, BYTE_PER_SECTOR * bpb->BPB_SecPerTrk);
    dst_data[DPT_0_OFFSET + DBR_HEAD_OFFSET] = DBR_HEAD;
    dst_data[DPT_0_OFFSET + DBR_SECTOR_OFFSET] = DBR_SECTOR;
    dst_data[DPT_0_OFFSET + DBR_CYLINDER_OFFSET] = DBR_CYLINDER;
    *(u32 *) &dst_data[DPT_0_OFFSET + DPT_POS_OFFSET] = bpb->BPB_SecPerTrk;

    /* [dbr, rsvd) */
    memcpy(&dst_data[BYTE_PER_SECTOR * bpb->BPB_SecPerTrk],
           &src_data[BYTE_PER_SECTOR * dbrstartSector],
           BYTE_PER_SECTOR * rsvd_sector);
    dst_bpb = (struct fat32_bpb *) &dst_data[BYTE_PER_SECTOR * bpb->BPB_SecPerTrk];
    dst_bpb->BPB_HiddSec = bpb->BPB_SecPerTrk;
    dst_bpb->BPB_RsvdSecCnt = rsvd_sector;
    info("rsvd sector: %x %x\n", bpb->BPB_RsvdSecCnt, rsvd_sector);

    /* [rsvd, 8) */
    memcpy(&dst_data[BYTE_PER_SECTOR * (bpb->BPB_SecPerTrk + rsvd_sector)],
           &src_data[BYTE_PER_SECTOR * (dbrstartSector + bpb->BPB_RsvdSecCnt)],
           src_len - (BYTE_PER_SECTOR * (dbrstartSector + bpb->BPB_RsvdSecCnt)));

    fwrite(dst_data, sizeof(char), dst_len, fp_dst);
    info("reformat\n");

  end:
    if (src_data) { free(src_data); }
    if (dst_data) { free(dst_data); }

    if (fp_dst) { fclose(fp_dst); }
    if (fp_src) { fclose(fp_src); }

    printf("\npress any key to continue...");
    getch();
}

