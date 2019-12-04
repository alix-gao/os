/****************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name  : os1_image.cpp
 * version    : 1.0
 * description:
 * author     : gaocheng
 * date       : 2009-04-22
 ****************************************************************/
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

int main()
{
    FILE *fpexe;
    FILE *fpbin;
    char ch[5],buf[5],onec;
    int i,j;
    int text_addr,data_addr,rdata_addr,tmp_addr;
    int result,rdata_len;

    fpexe = fopen("os1.exe", "rb+");
    if (NULL == fpexe)
    {
        printf("cannot open file os1.exe\n");
        exit(0);
    }

    /* .text */
    fseek(fpexe, 0l, 0);

    strcpy(ch, ".tex");
    buf[4]='\0';

    i = 1;
    do
    {
        /* 4个字节 */
        fread(buf, sizeof(char), 4, fpexe);
        if (!strcmp(buf, ch))
        i = 0;
        else
        /* 当前位置倒退3步 */
        fseek(fpexe, -3l, 1);
    }while (i);

    printf("%s\n",buf);

    for (i=0; i<3; i++)
    fread(buf, sizeof(char), 4, fpexe);

    printf("virtualaddress: %d-%d-%d-%d\n", buf[0], buf[1], buf[2], buf[3]);

    /* not same.+4*16*16+9*16 */
    tmp_addr = buf[0]*16*16*16 + buf[1]*16*16 + buf[2]*16 + buf[3];
    printf("_addr: %d\n", tmp_addr);

    fread(buf, sizeof(char), 4, fpexe);
    printf("sizeofrawdata: %d-%d-%d-%d\n", buf[0], buf[1], buf[2], buf[3]);

    fread(buf, sizeof(char), 4, fpexe);
    printf("*pointertorawdata: %d-%d-%d-%d\n", buf[0], buf[1], buf[2], buf[3]);

    /* not same.+4*16*16+9*16 */
    text_addr = buf[0]*16*16*16 + buf[1]*16*16 + buf[2]*16 + buf[3];

    printf("text_addr: %d\n", text_addr);

    fseek(fpexe, 0l, 0);

    strcpy(ch, ".dat");
    buf[4] = '\0';

    i = 1;
    do
    {
        /* 4个字节 */
        fread(buf, sizeof(char), 4, fpexe);
        if (!strcmp(buf, ch))
        i = 0;
        else
        fseek(fpexe, -3l, 1);//当前位置倒退3步。
    }while (i);

    printf("%s\n", buf);

    for (i=0; i<3; i++)
    fread(buf, sizeof(char), 4, fpexe);

    printf("virtualaddress: %d-%d-%d-%d\n", buf[0], buf[1], buf[2], buf[3]);

    /* not same.+4*16*16+9*16 */
    tmp_addr = buf[0]*16*16*16 + buf[1]*16*16 + buf[2]*16 + buf[3];
    printf("_addr:%d\n",tmp_addr);

    fread(buf, sizeof(char), 4, fpexe);
    printf("sizeofrawdata: %d-%d-%d-%d\n", buf[0], buf[1], buf[2], buf[3]);

    fread(buf, sizeof(char), 4, fpexe);
    printf("*pointertorawdata: %d-%d-%d-%d\n", buf[0], buf[1], buf[2], buf[3]);

    /* 0-12-0-0 */
    data_addr = buf[0]*16*16*16 + buf[1]*16*16 + buf[2]*16 + buf[3];
    printf("data_addr: %d\n", data_addr);

    /* .rdata */
    fseek(fpexe,0l,0);

    strcpy(ch,".rda");
    buf[4]='\0';

    i = 1;
    do
    {
        fread(buf, sizeof(char), 4, fpexe);
        if (!strcmp(buf, ch))
        i = 0;
        else
        fseek(fpexe, -3l, 1);//当前位置倒退3步。
    }while (i);

    printf("%s\n", buf);

    for (i=0; i<3; i++)
    fread(buf, sizeof(char), 4, fpexe);
    printf("virtualaddress: %d-%d-%d-%d\n", buf[0], buf[1], buf[2], buf[3]);

    tmp_addr = buf[0]*16*16*16 + buf[1]*16*16 + buf[2]*16 + buf[3];
    printf("_addr: %d\n", tmp_addr);

    fread(buf, sizeof(char), 4, fpexe);
    printf("sizeofrawdata: %d-%d-%d-%d\n", buf[0], buf[1], buf[2], buf[3]);

    rdata_len = buf[0]*16*16*16 + buf[1]*16*16 + buf[2]*16 + buf[3];
    fread(buf, sizeof(char), 4, fpexe);
    printf("*pointertorawdata: %d-%d-%d-%d\n", buf[0], buf[1], buf[2], buf[3]);

    /* 0-14-0-0 */
    rdata_addr = buf[0]*16*16*16 + buf[1]*16*16 + buf[2]*16 + buf[3];
    printf("rdat_addr: %d\n", rdata_addr);

    printf("jump 0x1000: %d\n", 1*16*16*16/512);
    result = (data_addr-text_addr)/512;
    printf("text-data: %d\n", result);

    /* begin to write */
    fpbin = fopen("os1.bin", "wb");
    if (NULL == fpbin)
    {
        printf("cannot open file os1.bin\n");
        exit(0);
    }

    /* write .text */
    fseek(fpexe, text_addr, 0);

    result = (result-1)/8+1;

    for (j=0; j<result; j++)
    for (i=0; i<16*16*16; i++)
    {
        fread(&onec,sizeof(char),1,fpexe);
        fwrite(&onec,sizeof(char),1,fpbin);
    }
    printf(".text has been writen ...\n");

    /* write .data */
    result = (rdata_addr-data_addr)/512;
    printf("data-rdata: %d\n", result);

    fseek(fpexe, data_addr, 0);

    result = (result-1)/8+1;
    for (j=0; j<result; j++)
    for (i=0; i<16*16*16; i++)
    {
        fread(&onec, sizeof(char), 1, fpexe);
        fwrite(&onec, sizeof(char), 1, fpbin);
    }
    printf(".data has been writen ...\n");

    /* write .rdata */
    fseek(fpexe, rdata_addr, 0);

    rdata_len = (rdata_len-1)/(16*16*16)+1;
    printf("rdata-bss:%d\n",rdata_len);

    for (j=0; j<rdata_len; j++)
    for (i=0; i<16*16*16; i++)//每次写1000H。
    {
        fread(&onec,sizeof(char),1,fpexe);
        fwrite(&onec,sizeof(char),1,fpbin);
    }
    printf(".rdata has been writen ...\n");

    fclose(fpexe);
    fclose(fpbin);

    system("pause");
}

