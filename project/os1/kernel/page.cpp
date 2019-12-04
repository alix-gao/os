
#define PAGE_DIR_ADDR 0x120000
#define PAGE_TABLE_ADDR 0x400000
#define SYS_MEM_SIZE 32

void init_page(void)
{
	unsigned long i, j;
    unsigned long pagedir = PAGE_DIR_ADDR;
    unsigned long *pagedir_addr = (unsigned long *)PAGE_DIR_ADDR;
    unsigned long *pagetable_addr = (unsigned long *)PAGE_TABLE_ADDR;
    unsigned long page_table_num;

    /* 每个页目录表定位4M内存 */
    page_table_num = (SYS_MEM_SIZE%4)?(SYS_MEM_SIZE/4+1):(SYS_MEM_SIZE/4);

    /* 设置页目录表 */
    for (i=0; i<page_table_num; i++)
    {
        *(pagedir_addr + i) = PAGE_TABLE_ADDR + 0x1000*i + 7;
    }

    /* 设置页表 */
    for (i=0; i<page_table_num; i++)
    {
        /* 一个页目录表项对应1K(0x400)个表项 */
        for (j=0; j<0x400; j++)
        {
            //0x1000 = 4K
            *(pagetable_addr + j + i*0x400) = 0x1000*(j+i*0x400) + 7;
        }
    }

    /* 启动内存分页机制 */
    __asm__ __volatile__("movl %0,%%eax\n\t"
                         "movl %%eax,%%cr3\n\t"
                         "movl %%cr0,%%eax\n\t"
                         "orl $0x80000000,%%eax\n\t"
                         "movl %%eax,%%cr0"\
                         :
                         :"m"(pagedir)
                         :"eax");

    /* fresh register */
    __asm__ __volatile__("ljmp $8,$1f;\
                          1:\
                          nop");
}

