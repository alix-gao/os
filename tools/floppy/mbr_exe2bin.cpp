/****************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name  : mbr_exe2bin.cpp
 * version    : 1.0
 * description:
 * author     : gaocheng
 * date       : 2009-04-22
 ****************************************************************/
#include "stdio.h"
#include "stdlib.h"

int main()
{
    FILE *fpexe;
    FILE *fpbin;
    char ch;
    int i;

    fpexe = fopen("mbr.exe", "rb");
    if (NULL == fpexe) {
        printf("cannot open mbr.exe.\n");
        goto end;
        exit(0);
    }

    fpbin = fopen("mbr.bin", "wb");
    if (NULL == fpbin) {
        printf("cannot open mbr.bin.\n");
        goto end;
        exit(0);
    }

    //跨越程序段前缀和7c00H
    fseek(fpexe, 512+7*16*16*16+12*16*16l, 0);

    for(i=0; i<512; i++) {
        fread(&ch, sizeof(char), 1, fpexe);
        fwrite(&ch, sizeof(char), 1, fpbin);
    }

    fclose(fpexe);
    fclose(fpbin);

    printf("mbr.bin\n");
  end:
    system("pause");
}

