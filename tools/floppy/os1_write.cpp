/****************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name  : os1_write.cpp
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
    int i,j;
    char ch='g';

    fpdisk = fopen("FloppyDisk.vfd", "rb+");
    if (NULL == fpdisk) {
        printf("cannot open file FloppyDisk.vfd\n");
        exit(0);
    }

    fpbin = fopen("os1.bin", "rb");
    if (NULL == fpbin) {
        printf("cannot open file system.bin\n");
        exit(0);
    }

    fseek(fpdisk, 512*18*6l, 0);

    for(i=0; i<18*4; i++)
    for(j=0; j<512; j++) {
        fread(&ch, sizeof(char), 1, fpbin);
        fwrite(&ch, sizeof(char), 1, fpdisk);
    }

    fclose(fpdisk);
    fclose(fpbin);

    printf("os1.bin\npress any key to continue.\n");
    getch();
}

