/****************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name  : cos16_write.cpp
 * version    : 1.0
 * description:
 * author     : gaocheng
 * date       : 2009-04-22
 ****************************************************************/
#include "stdio.h"
#include "stdlib.h"
#include "conio.h"

unsigned long get_file_len(void)
{
    FILE *fpbin;
    unsigned long len = 0;

    fpbin = fopen("cos16.bin", "rb");
    if (NULL == fpbin)
    {
        printf("cannot open file os2.bin\n");
        exit(0);
    }

    fseek(fpbin, 0, SEEK_END);

    len = ftell(fpbin);

    fclose(fpbin);

    return len;
}

int main()
{
    FILE *fpbin;
    FILE *fpdisk;
    int i,j;
    char ch = 'g';
    unsigned long len = 0;

    fpdisk = fopen("FloppyDisk.vfd", "rb+");
    if (NULL == fpdisk)
    {
        printf("cannot open file FlashDisk.ima\n");
        exit(0);
    }

    /* 获取文件长度 */
    len = get_file_len();
    if (512*16*2 < len)
    {
        printf("cos16.bin is too big.\n");
        system("pause");
    }

    fpbin = fopen("cos16.bin", "rb");
    if (NULL == fpbin)
    {
        printf("cannot open file cos16.bin\n");
        exit(0);
    }

    fseek(fpdisk, 512*18*2l, 0);
    for (i=0; i<18; i++)
    for (j=0; j<512; j++)
    {
        fread(&ch,sizeof(char),1,fpbin);
        fwrite(&ch,sizeof(char),1,fpdisk);
    }

    fseek(fpdisk, 512*18*3l, 0);
    for (i=0; i<18; i++)
    for (j=0; j<512; j++)
    {
        fread(&ch,sizeof(char),1,fpbin);
        fwrite(&ch,sizeof(char),1,fpdisk);
    }

    fclose(fpbin);
    fclose(fpdisk);

    printf("cos16.bin\npress any key to continue.\n");
    getch();
}

