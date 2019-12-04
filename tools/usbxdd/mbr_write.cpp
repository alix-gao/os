/****************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name  : mbr_write.cpp
 * version    : 1.0
 * description:
 * author     : gaocheng
 * date       : 2009-04-22
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>

#include "fat32.h"

int main(int argc,char *argv[])
{
    FILE *fpbin;
    FILE *fpdisk;
    char ch;
    int i;

    fpdisk=fopen("FlashDisk.ima", "rb+");
    if (NULL == fpdisk) {
        printf("cannot open FlashDisk.ima.\n");
        exit(0);
    }

    fpbin=fopen("mbr.bin", "rb");
    if (NULL == fpbin) {
        printf("cannot open file mbr.bin.\n");
        exit(0);
    }

	/* do not modify the dpt[4] and 0x55aa */
    for (i = 0; i < DPT_0_OFFSET; i++) {
        fread(&ch, sizeof(char), 1, fpbin);
        fwrite(&ch, sizeof(char), 1, fpdisk);
    }

    fclose(fpdisk);
    fclose(fpbin);

    printf("mbr.bin.\npress any key to continue.\n");
    if (1 == argc) {
        getch();
    }
}

