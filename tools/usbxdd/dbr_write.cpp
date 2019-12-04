/****************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name  : dbr_write.cpp
 * version    : 1.0
 * description:
 * author     : gaocheng
 * date       : 2009-04-22
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

struct fat32_bpb bpb = { 0 };

int main(int argc,char *argv[])
{
    FILE *fpbin;
    FILE *fpdisk;
    u32 dbrstartSector;
	u8 *sector;

    init_asm_variable();

	sector = NULL;
    fpdisk = fpbin = NULL;

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

    fpbin = fopen("dbr.bin", "rb");
    if (NULL == fpbin) {
        printf("cannot open file dbr.bin\n");
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
        info("bpb bytes per sector error\n");
        goto end;
    }

    fread(sector, sizeof(char), BYTE_PER_SECTOR, fpbin);

    bpb.BS_jmpBoot[0] = sector[0];
    bpb.BS_jmpBoot[1] = sector[1];
    bpb.BS_jmpBoot[2] = sector[2];
    memcpy(sector, &bpb, sizeof(struct fat32_bpb));

    /* write dbr */
    fseek(fpdisk, dbrstartSector * BYTE_PER_SECTOR, SEEK_SET);
    fwrite(sector, sizeof(char), BYTE_PER_SECTOR, fpdisk);

    printf("dbr.bin\npress any key to continue\n");
  end:
    if (fpdisk) { fclose(fpdisk); }
    if (fpbin) { fclose(fpbin); }
    if (1 == argc) {
        getch();
    }
    return 0;
}

