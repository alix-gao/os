/****************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name  : os2_image.cpp
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
#define COFF_TABLE_LEN 18

struct image_file_header
{
	/* pe */
	unsigned long signature;

	short machine; /* 2byte */

	short section_num;

	unsigned long time_stamp; /* 4byte */

	unsigned long symboltab_addr;

	unsigned long symbol_num;

	/* 未完 */
};

struct coff_symbol_table
{
    char name[8];

    unsigned long value;

    short section_num;

    short type;

    char storage_class;

    char num_of_aux_symbols;
};

FILE *fpexe;
FILE *fpbin;
char *str_tab = NULL;

struct image_file_header image_file_header_info = {0};

void read_section_info(void)
{
	/* 读文件头信息 */
	fseek(fpexe, DOS_MZ_LEN+DOS_STUB_LEN, 0);

	fread(&image_file_header_info, sizeof(struct image_file_header), 1, fpexe);

	printf("signature: 0x%x\n", image_file_header_info.signature);
	printf("machine: 0x%x\n", image_file_header_info.machine);
	printf("section num: %d\n", image_file_header_info.section_num);
	printf("time stamp: %d\n", image_file_header_info.time_stamp);
	printf("symboltab addr: %x\n", image_file_header_info.symboltab_addr);
	printf("symbol num: %d\n", image_file_header_info.symbol_num);
	printf("\n");
}

void print_name_directly(char *name)
{   
    int j;
    for (j=0; j<8; j++)
    {
        if (0 == name[j])
        break;

        printf("%c", name[j]);
    }
}

void print_name_strtab(unsigned long offset)
{
    char *name;

    //printf("%x", offset);

    if (4 > offset)
        return;

    name = str_tab+offset;

    while (*name)
    {
        printf("%c", *name);
        name++;
    }
}

void print_symboltab_info(struct coff_symbol_table *temp)
{
    /* 为了避免打印出嘀声音 */
    if (0 == temp->value)
    {
         return;
    }
    
    if ((0 == temp->name[0])
     && (0 == temp->name[1])
     && (0 == temp->name[2])
     && (0 == temp->name[3]))
    {
        print_name_strtab(*(unsigned long *)&temp->name[4]);
    }
    else
    {
        print_name_directly(temp->name);
    }

    printf("(0x%x)", temp->value);
}

void write_symboltab_info(struct coff_symbol_table *temp)
{
    fwrite(temp, sizeof(struct coff_symbol_table), 1, fpbin);
}

void handle_symbol_table(void)
{
     int i;
     char test[COFF_TABLE_LEN];
     struct coff_symbol_table *temp;

     fseek(fpexe, image_file_header_info.symboltab_addr, 0);

     for (i=0; i<image_file_header_info.symbol_num; i++)
     {
		fread(&test, COFF_TABLE_LEN, 1, fpexe);
		i++;

		if (('.' == test[0])
 		 && ('f' == test[1])
 		 && ('i' == test[2])
		 && ('l' == test[3])
		 && ('e' == test[4]))
		{
			#define SYMBOL_POINT_FILE 36
			fseek(fpexe, SYMBOL_POINT_FILE-sizeof(test), 1);
			fread(&test, COFF_TABLE_LEN, 1, fpexe);
			i++;
		}

		if (('.' == test[0])
		 && ('t' == test[1])
		 && ('e' == test[2])
		 && ('x' == test[3])
		 && ('t' == test[4]))
		{
			#define SYMBOL_POINT_TEXT 108
			fseek(fpexe, SYMBOL_POINT_TEXT-sizeof(test), 1);

			continue;
		}

		temp = (struct coff_symbol_table *)test;

        /* 打印符号表信息 */
		print_symboltab_info(temp);

		/* 写符号表信息 */
		write_symboltab_info(temp);
     }
}

unsigned long get_string_table_len(void)
{
    unsigned long len;

    fseek(fpexe, image_file_header_info.symboltab_addr+COFF_TABLE_LEN*image_file_header_info.symbol_num, 0);

    fread(&len, 4, 1, fpexe);

    printf("string len: 0x%x\n", len);

    return len;
}

int main()
{
    unsigned long len = 0;

    /* init global variable */
    fpexe = fopen("os3.exe", "rb+");
    if (NULL == fpexe)
    {
        printf("cannot open file os2.exe\n");
        exit(0);
    }

    fpbin = fopen("os3.bin", "ab+");
    if (NULL == fpbin)
    {
        printf("cannot open file os2.bin\n");
        exit(0);
    }

	/* 获取文件头信息 */
	read_section_info();

	/* 读字符串表 */
	len = get_string_table_len();
	str_tab = (char *)malloc(len);
    if (NULL == str_tab)
    {
        printf("memory alloc fail.\n");
        system("pause");
    }
    fread(str_tab+4, len-4, 1, fpexe);

    *(unsigned long *)str_tab = len;
#if 1
    /* 先写入字符串表 */
    fwrite(str_tab, len, 1, fpbin);

    /* 写入符号表 */
    handle_symbol_table();
#endif
    free(str_tab);
    
    //printf("\007\n"); //打印这个字符会'滴'的一声

    fclose(fpexe);
    fclose(fpbin);
}

