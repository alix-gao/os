/****************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name  : asm32_write.cpp
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
    int i, j;
    char ch='g';

    fpdisk = fopen("FloppyDisk.vfd", "rb+");
    if (NULL == fpdisk) {
        printf("cannot open file floppydis.vfd\n");
        exit(0);
    }

    fpbin = fopen("asm32.bin", "rb");
    if (NULL == fpbin) {
        printf("cannot open file asm32.bin\n");
        exit(0);
    }

    fseek(fpdisk, 512*18*4l, 0);
    for (j=0; j<2*18; j++)
    for (i=0; i<512; i++) {
        fread(&ch, sizeof(char), 1, fpbin);
        fwrite(&ch, sizeof(char), 1, fpdisk);
    }

    fclose(fpbin);
    fclose(fpdisk);

    printf("asm32.bin\npress any key to continue.\n");
    getch();
}

