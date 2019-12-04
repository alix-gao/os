/****************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name  : asm16_exe2bin.cpp
 * version    : 1.0
 * description:
 * author     : gaocheng
 * date       : 2009-04-22
 ****************************************************************/
#include "stdio.h"
#include "stdlib.h"

int main(int argc,char *argv[])
{
    FILE *fpexe;
    FILE *fpbin;
    char ch;
    int i;

    fpexe = fopen("asm16.exe", "rb");
    if (NULL == fpexe)
    {
        printf("cannot open file asm16.exe\n");
        exit(0);
    }

    fpbin = fopen("asm16.bin", "wb");
    if (NULL == fpbin)
    {
        printf("cannot open file asm16.bin\n");
        exit(0);
    }

    fseek(fpexe, 512+0x2000l, 0);
    for (i=0; i<512*16; i++)
    {
        fread(&ch, sizeof(char), 1, fpexe);
        fwrite(&ch, sizeof(char), 1, fpbin);
    }

    fclose(fpexe);
    fclose(fpbin);

    printf("asm16.bin\n");

    if (1 == argc) {
        system("pause");
    }
}

