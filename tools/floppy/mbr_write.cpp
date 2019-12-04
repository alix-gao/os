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
#include"stdio.h"
#include"stdlib.h"
#include"conio.h"

int main()
{
    FILE *fpbin;
    FILE *fpdisk;
    char ch;
    int i;

    fpdisk=fopen("FloppyDisk.vfd", "rb+");
    if (NULL == fpdisk) {
        printf("cannot open floppydisk.vfd.\n");
        exit(0);
    }

    fpbin=fopen("mbr.bin", "rb");
    if (NULL == fpbin) {
        printf("cannot open file mbr.bin.\n");
        exit(0);
    }

    for (i=0; i<512; i++) {
        fread(&ch, sizeof(char), 1, fpbin);
        fwrite(&ch, sizeof(char), 1, fpdisk);
    }

    fclose(fpdisk);
    fclose(fpbin);

    printf("mbr.bin.\npress any key to continue.\n");
    getch();
}

