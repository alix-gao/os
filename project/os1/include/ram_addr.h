
//一些常用的地址全局变量
#ifndef _ram_addr_h
#define _ram_addr_h
/* 此头文件只被引用一次 */
/***********************/
#include <process.h>

desc_struct*idt_addr=(desc_struct*)(0x100000);
desc_struct*gdt_addr=(desc_struct*)(0x110000);
char*page_dir_addr=(char*)(0x120000);
char*page_table_0_addr=(char*)(0x401000);
char*page_table_1_addr=(char*)(0x402000);
char*page_table_2_addr=(char*)(0x403000);
char*page_table_3_addr=(char*)(0x404000);
long*cos_version=(long*)(0x1000290-4);
long*ip_addr=(long*)(0x1000290-8);
char*curse_x=(char*)(0x1000290-9);
char*curse_y=(char*)(0x1000290-10);
int*esp_sav=(int*)(0x1000290-16);

/* kernel display curse */
int *kernel_disp_curse_x=(int *)(0x1000290-20);
int *kernel_disp_curse_y=(int *)(0x1000290-24);

/* shell display curse */
int *shell_disp_curse_x=(int *)(0x1000290-28);
int *shell_disp_curse_y=(int *)(0x1000290-32);
//-----
#endif

