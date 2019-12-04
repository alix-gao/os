/****************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name  : asm16_write.cpp
 * version    : 1.0
 * description: asm16.bin is after the hidden sector(1 track) & dbr(1 track).
 * author     : gaocheng
 * date       : 2009-04-22
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <memory.h>

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

static u32 get_file_len(FILE *fp)
{
    fseek(fp, 0, SEEK_END);
    return ftell(fp);
}

int main(int argc,char *argv[])
{
    FILE *fpbin;
    FILE *fpdisk;
    u8 *sector;
    struct fat32_bpb bpb = { 0 };
    u32 dbrstartSector;
    u32 i;
    u8 ch;
    u32 len;

    init_asm_variable();

    sector = NULL;
    fpbin = fpdisk = NULL;

    sector = (u8 *) malloc(BYTE_PER_SECTOR);
    if (NULL == sector) {
        info("malloc sector fail\n");
        goto end;
    }

    fpdisk = fopen("FlashDisk.ima", "rb+");
    if (NULL == fpdisk) {
        printf("cannot open file FlashDisk.ima\n");
        goto end;
        exit(0);
    }

    fpbin = fopen("asm16.bin", "rb");
    if (NULL == fpbin) {
        printf("cannot open file asm16.bin\n");
        goto end;
        exit(0);
    }

    /* read mbr */
    fread(sector, sizeof(char), BYTE_PER_SECTOR, fpdisk);

    /* file system of dpt 0 */
    switch (sector[DPT_0_OFFSET + FS_TYPE_OFFSET]) {
    case FS_TYPE_FAT32:
    case FS_TYPE_WIN95_FAT32_1:
    case FS_TYPE_WIN95_FAT32_2:
        info("file system: 0x%x\n", sector[DPT_0_OFFSET + FS_TYPE_OFFSET]);
        break;
    default:
        info("unsupport file system (%x)\n", sector[DPT_0_OFFSET + FS_TYPE_OFFSET]);
        return 0;
        break;
    }

    /* relative position of dbr */
    dbrstartSector = *(u32 *)(&sector[DPT_0_OFFSET + DPT_POS_OFFSET]);
    info("dbr start sector: 0x%x\n", dbrstartSector);

    /* read dbr */
    fseek(fpdisk, dbrstartSector * BYTE_PER_SECTOR, SEEK_SET);
    info("fat32 info size: %d\n", sizeof(struct fat32_bpb));
    fread(sector, sizeof(char), BYTE_PER_SECTOR, fpdisk);

    memcpy(&bpb, sector, sizeof(struct fat32_bpb));

    if (BYTE_PER_SECTOR != bpb.BPB_BytsPerSec) {
        info("bpb bytes per second fail\n");
        goto end;
    }

    len = get_file_len(fpbin);
    if (ASM16_LEN < len) {
        printf("asm16.bin is too big. len = %d\n", len);
        goto end;
    }
    fseek(fpbin, 0, SEEK_SET);

    fseek(fpdisk, bpb.BPB_BytsPerSec * ASM16_LBA, SEEK_SET);
    for (i = 0; i < len; i++) {
        fread(&ch,sizeof(char),1,fpbin);
        fwrite(&ch,sizeof(char),1,fpdisk);
    }

    printf("asm16.bin\npress any key to continue.\n");
  end:
    if (fpbin) { fclose(fpbin); }
    if (fpdisk) { fclose(fpdisk); }
    info("pause");
    if (1 == argc) {
        getch();
    }
}

