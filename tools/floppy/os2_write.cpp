/****************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name  : os2_write.cpp
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

    fpbin = fopen("os2.bin", "rb");
    if (NULL == fpbin) {
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
    unsigned long i;
    char ch='g';
    unsigned long len = 0;

    fpdisk = fopen("FloppyDisk.vfd", "rb+");
    if (NULL == fpdisk) {
        printf("cannot open file FloppyDisk.vfd\n");
        exit(0);
    }

    /* 获取文件长度 */
    len = get_file_len();

    fpbin = fopen("os2.bin", "rb");
    if (NULL == fpbin) {
        printf("cannot open file os2.bin\n");
        exit(0);
    }

    printf("file length: %d\n", len);

    fseek(fpdisk, 512*18*10l, 0);

    for(i=0; i<len; i++) {
        fread(&ch, sizeof(char), 1, fpbin);
        fwrite(&ch, sizeof(char), 1, fpdisk);
    }

    fclose(fpdisk);
    fclose(fpbin);

    printf("os2.bin\npress any key to continue.\n");
    getch();
}

