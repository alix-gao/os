/****************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name  : asm16_write.cpp
 * version    : 1.0
 * description:
 * author     : gaocheng
 * date       : 2009-04-22
 ****************************************************************/
#include "stdio.h"
#include "stdlib.h"
#include "conio.h"

int main()
{
    FILE *fpbin;
    FILE *fpdisk;
    int i;
    char ch='g';

    fpbin = fopen("asm16.bin", "rb");
    if (NULL == fpbin) {
        printf("cannot open file asm16.bin\n");
        exit(0);
    }

    fpdisk = fopen("FloppyDisk.vfd", "rb+");
    if (NULL == fpdisk) {
        printf("cannot open file floppydis.vfd\n");
        exit(0);
    }

    fseek(fpdisk, 512*18l, 0);
    for(i=0; i<512*16; i++) {
        fread(&ch, sizeof(char), 1, fpbin);
        fwrite(&ch, sizeof(char), 1, fpdisk);
    }

    fclose(fpbin);
    fclose(fpdisk);

    printf("asm16.bin\npress any key to continue.\n");

    getch();
}

