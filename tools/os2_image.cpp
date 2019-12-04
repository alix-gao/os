/****************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name  : os2_image.cpp
 * version    : 1.0
 * description: 生成内存镜像文件
 * author     : gaocheng
 * date       : 2009-04-22
 ****************************************************************/
#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define DOS_MZ_LEN 0x40
#define DOS_STUB_LEN 0x40
#define PE_HEADER_LEN 0x88
#define PE_SPACE_LEN 0x70
#define SECTION_TABLE_LEN 0x28

struct pe_section_table_info
{
    /* .text .data .rdata .bss .idata */
    char name[8];

    unsigned long section_offset;

    /* 虚拟内存地址 */
    unsigned long virtualaddress;

    /* 文件偏移地址 */
    unsigned long pointertorawdata;

    /* 数据长度 */
    unsigned long sizeofrawdata;
};

FILE *fpexe;
FILE *fpbin;
struct pe_section_table_info section_table[5];

void read_section_info(int index)
{
    unsigned long tmp_dword;

    fseek(fpexe, section_table[index].section_offset, 0);

    fread(section_table[index].name, 8, 1, fpexe);
    printf("section: %s\n", section_table[index].name);

    fread(&tmp_dword, sizeof(unsigned long), 1, fpexe);

    fread(&tmp_dword, sizeof(unsigned long), 1, fpexe);
    printf("virtualaddress: %x\n", tmp_dword+0x400000);
    section_table[index].virtualaddress = tmp_dword;

    fread(&tmp_dword, sizeof(unsigned long), 1, fpexe);
    printf("sizeofrawdata: %x\n", tmp_dword);
    section_table[index].sizeofrawdata = tmp_dword;

    fread(&tmp_dword, sizeof(unsigned long), 1, fpexe);
    printf("*pointertorawdata: %x\n", tmp_dword);
    section_table[index].pointertorawdata = tmp_dword;
}

void write_section_table(int index)
{
    unsigned long text_v_addr;
    int i;
    char onec;

    text_v_addr = section_table[0].virtualaddress;

    /* write .text */
    fseek(fpbin, section_table[index].virtualaddress-text_v_addr, 0);
    fseek(fpexe, section_table[index].pointertorawdata, 0);
    for (i=0; i<section_table[index].sizeofrawdata; i++)
    {
        fread(&onec,sizeof(char),1,fpexe);
        fwrite(&onec,sizeof(char),1,fpbin);
    }
    printf("section %s has been writen ...\n", section_table[index].name);
}

int main()
{
    unsigned long section_table_offset;

    /* init global variable */
    fpexe = fopen("os2.exe", "rb+");
    if (NULL == fpexe)
    {
        printf("cannot open file os2.exe\n");
        exit(0);
    }

    fpbin = fopen("os2.bin", "wb");
    if (NULL == fpbin)
    {
        printf("cannot open file os2.bin\n");
        exit(0);
    }

    section_table_offset = DOS_MZ_LEN+DOS_STUB_LEN+PE_HEADER_LEN+PE_SPACE_LEN;

    section_table[0].section_offset = section_table_offset+0*SECTION_TABLE_LEN;
    section_table[1].section_offset = section_table_offset+1*SECTION_TABLE_LEN;
    section_table[2].section_offset = section_table_offset+2*SECTION_TABLE_LEN;
    section_table[3].section_offset = section_table_offset+3*SECTION_TABLE_LEN;
    section_table[4].section_offset = section_table_offset+4*SECTION_TABLE_LEN;

    /* .text */
    read_section_info(0);

    /* .data */
    read_section_info(1);

    /* .rdata */
    read_section_info(2);

    /* .bss */
    read_section_info(3);

    /* .idata */
    read_section_info(4);

    /* write .text */
    write_section_table(0);

    /* write .data */
    write_section_table(1);

    /* write .rdata */
    write_section_table(2);

    /* write .bss */
    write_section_table(3);

    /* write .idata */
    write_section_table(4);

    fclose(fpexe);
    fclose(fpbin);
}

