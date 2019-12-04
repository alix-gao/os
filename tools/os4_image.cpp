/****************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name  : os4_image.cpp
 * version    : 1.0
 * description:
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

struct pe_section_table_info {
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
struct pe_section_table_info *section_table = NULL;

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
    for (i = 0; i < section_table[index].sizeofrawdata; i++) {
        fread(&onec,sizeof(char),1,fpexe);
        fwrite(&onec,sizeof(char),1,fpbin);
    }

    printf("section %s has been writen ...\n", section_table[index].name);
}

int main(void)
{
    unsigned long section_table_offset;
    unsigned int section_num = 0;
	unsigned int i;

    /* init global variable */
    fpexe = fopen("os4.exe", "rb+");
    if (NULL == fpexe) {
        printf("cannot open file os4.exe\n");
        exit(0);
    }

    fpbin = fopen("os4.bin", "wb");
    if (NULL == fpbin) {
        printf("cannot open file os4.bin\n");
        exit(0);
    }

    /* signature is 4, machine is 2 */
    fseek(fpexe, DOS_MZ_LEN + DOS_STUB_LEN + 4 + 2, 0);
    fread(&section_num, 2, 1, fpexe);

    printf("section num: %d\n", section_num);

    section_table_offset = DOS_MZ_LEN+DOS_STUB_LEN+PE_HEADER_LEN+PE_SPACE_LEN;
    section_table = (struct pe_section_table_info *)malloc(section_num * sizeof(struct pe_section_table_info));
    if (NULL == section_table) {
		printf("alloc memory fail.\n");
		return 0;
	}

	for (i = 0; i < section_num; i++) {
		section_table[i].section_offset = section_table_offset + i * SECTION_TABLE_LEN;
		read_section_info(i);
		write_section_table(i);
	}

	free(section_table);

    fclose(fpexe);
    fclose(fpbin);
}

