/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : i386.c
 * version     : 1.0
 * description : (key) intel cpu
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include <vts.h>

/***************************************************************
 global variable declare
 ***************************************************************/
/* gdtv */
LOCALD struct pseudo_descriptor gdtr = { 0, 0xffff, GDT_ADDR };

/* idtv */
LOCALD struct pseudo_descriptor idtr = { 0, 0xffff, IDT_ADDR };

LOCALD os_u32 gdt_item_index = GDT_NULL_LDT_INDEX + 1;

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description : 获取当前pc
 * history     :
 ***************************************************************/
os_u32 OS_API get_pc_thunk(os_void)
{
    os_u32 addr = 0;

    /* addr: 4, ebp: 4, pc: 4, return: eax */
    __asm__ __volatile__("movl 8(%%esp),%%eax\n\t"
                         "movl %%eax,%0"\
                         :"=m"(addr)
                         :
                         :"eax","esp");

    return addr;
}

/***************************************************************
 * description : 获取core id
 * history     :
 ***************************************************************/
os_u32 OS_API get_core_id(os_void)
{
    return CORE_ID_0;
}

/***************************************************************
 * description : 获取cpu id
 * history     :
 ***************************************************************/
os_u32 OS_API get_cpu_id(os_void)
{
    return CPU_ID_0;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_u32 alloc_gdt_item(os_void)
{
/* 10000/8 = 8192 */
#define GDT_ITEM_NUM 8192
    if (GDT_ITEM_NUM > gdt_item_index) {
        return gdt_item_index++;
    }
    cassert(OS_FALSE);
    return GDT_NULL_INDEX;
}

/***************************************************************
 * description : 修改gdt/ldt段描述符
 *       input @ atr, 属性.
 *               offset, 第no个.
 *               base, 基址.
 *               limit, 界限(20位, 低12位为0xfff).
 *               attribute, 属性.
 * history     :
 ***************************************************************/
LOCALC os_void modify_seg_descriptor(struct seg_desc_struct *addr, os_u32 base, os_u32 limit, os_u16 attribute)
{
    /* 不做参数检查 */

    addr->limit_l_1        = (limit >> 12) & 0x0ff; // 段界限1（16位）
    addr->limit_l_2        = (limit >> 20) & 0x0ff;
    addr->base_l_1         = base & 0x0ff; // 段基址1（16位）
    addr->base_l_2         = (base >> 8) & 0x0ff;
    addr->base_m           = (base >> 16) & 0xff;
    addr->attr1            = attribute & 0xff;
    addr->limit_high_attr2 = ((limit>>28) & 0xf) | ((attribute>>8) & 0xf0);
    addr->base_h           = (base>>24) & 0xff;
    /* g,d,0,avl=0000. tss is 16 bit segment. */
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void init_gdt(os_void)
{
    struct seg_desc_struct *os_gdt_addr = (struct seg_desc_struct *)(GDT_ADDR);
    pointer os_gdtr_addr;

    /* null */
    modify_seg_descriptor(&os_gdt_addr[GDT_NULL_INDEX], 0, 0, 0);

    /* 16 bit code, segment = 0x0000 */
    modify_seg_descriptor(&os_gdt_addr[GDT_CODE16_INDEX], 0, 0xffffffff, AT_CODE_EXE);

    /* 16 bit data, segment = 0x0000 */
    modify_seg_descriptor(&os_gdt_addr[GDT_DATA16_INDEX], 0, 0xffffffff, AT_DATA_WRITE);

    /* 32 bit code */
    modify_seg_descriptor(&os_gdt_addr[GDT_CODE32_INDEX], 0, 0xffffffff, AT_CODE_EXE+D_32);

    /* 32 bit data */
    modify_seg_descriptor(&os_gdt_addr[GDT_DATA32_INDEX], 0, 0xffffffff, AT_DATA_WRITE+D_32);

    gdtr.base = GDT_ADDR;
    gdtr.limit = 0xffff;
    gdtr.reserve = 0;

    idtr.base = IDT_ADDR;
    idtr.limit = 0xffff;
    idtr.reserve = 0;

    os_gdtr_addr = (pointer) &gdtr.limit;

    __asm__ __volatile__("pushl %%eax\n\t"
                         "movl %0,%%eax\n\t"
                         "lgdt (%%eax)\n\t"
                         "movw $32,%%ax\n\t"
                         "movw %%ax,%%ds\n\t"
                         "movw %%ax,%%es\n\t"
                         "movw %%ax,%%fs\n\t"
                         "movw %%ax,%%gs\n\t"
                         "movw %%ax,%%ss\n\t"
                         "popl %%eax\n\t"
                         ::"m"(os_gdtr_addr));

    /* fresh register */
    __asm__ __volatile__("ljmp $"asm_str(GDT_CODE32_INDEX * 8)",$1f;\
                          1:\
                          nop");
}

/***************************************************************
 * description : 4-kbyte page table
 *               only little endian
 ***************************************************************/
struct page_directory_entry {
    os_u32 p : 1; /* present */
    os_u32 rw : 1; /* read/write */
    os_u32 us : 1; /* user/supervisor */
    os_u32 pwt : 1; /* write-through */
    os_u32 pcd : 1; /* cache disabled */
    os_u32 a : 1; /* accessed */
    os_u32 rsvd : 1; /* reserved (set to 0) */
    os_u32 ps : 1; /* page size (0 indicates 4 kbytes) */
    os_u32 g : 1; /* global page (ignored) */
    os_u32 avail : 3; /* available for system programmer's use */
    os_u32 page_table_base_address : 20;
};

/***************************************************************
 * description : 4-kbyte page
 *               only little endian
 ***************************************************************/
struct page_table_entry {
    os_u32 p : 1; /* present */
    os_u32 rw : 1; /* read/write */
    os_u32 us : 1; /* user/supervisor */
    os_u32 pwt : 1; /* write-through */
    os_u32 pcd : 1; /* cache disabled */
    os_u32 a : 1; /* accessed */
    os_u32 d : 1; /* dirty */
    os_u32 pat : 1; /* page table attribute index */
    os_u32 g : 1; /* glable page */
    os_u32 avail : 3; /* available for system programmer's use */
    os_u32 page_base_address : 20;
};

/***************************************************************
 * description : 当前系统默认的page-directory base register, 用于写入cr3寄存器
 * history     :
 ***************************************************************/
os_u32 system_pdbr(os_void)
{
    os_u32 cr3;

    __asm__ __volatile__("movl %%cr3,%%eax\n\t"
                         "movl %%eax,%0\n\t"
                         :"=m"(cr3)::"eax");

#define CR3_ADDR_MASK 0xfffff000
#define CR3_PCD 4
#define CR3_PWT 3
    /* set pcd and pwt
       When the PCD flag is set, caching of the page-directory is prevented;
       When the PWT flag is set, writethrough caching is enabled; when the flag is clear, write-back caching is enabled. */
    cr3 = (PAGE_DIR_ADDR & CR3_ADDR_MASK) | ((cr3 & (~CR3_ADDR_MASK)) | (UINT32_C(1) << CR3_PWT) | (UINT32_C(1) << CR3_PCD));
    return cr3;
}

/***************************************************************
 * description : page dir & page table
 *               32位系统支持最大4g内存, 其中页目录表占4k, 页表占4m。
 * history     :
 ***************************************************************/
LOCALC os_void init_page(os_void)
{
    os_u32 i, j;
    struct page_directory_entry *page_dir = (struct page_directory_entry *) PAGE_DIR_ADDR;
    struct page_table_entry *page_table = (struct page_table_entry *) PAGE_TABLE_ADDR;
    os_u32 page_table_num;

/* 32位系统支持的内存大小为4g, 以M为单位 */
#define SYS_MEM_SIZE (4 * 1024)

    /* 每个页目录表定位4M内存 */
    page_table_num = (SYS_MEM_SIZE%4) ? (SYS_MEM_SIZE/4 + 1) : (SYS_MEM_SIZE/4);

    /* 设置页目录表 */
    for (i = 0; i < page_table_num; i++) {
        page_dir[i].page_table_base_address = (PAGE_TABLE_ADDR >> 12) + i;
        page_dir[i].avail = 0;
        page_dir[i].g = 0;
        /* Determines the page size. When this flag is clear, the page size is 4 KBytes and
           the page-directory entry points to a page table. When the flag is set, the page
           size is 4 MBytes for normal 32-bit addressing (and 2 MBytes if extended physical
           addressing is enabled) and the page-directory entry points to a page. If the
           page-directory entry points to a page table, all the pages associated with that
           page table will be 4-KByte pages. */
        page_dir[i].ps = 0;
        page_dir[i].rsvd = 0;
        page_dir[i].a = 0;
        /* Controls the caching of individual pages or page tables. When the PCD flag is
           set, caching of the associated page or page table is prevented; when the flag is
           clear, the page or page table can be cached. */
        page_dir[i].pcd = 0;
        /* Controls the write-through or write-back caching policy of individual pages or
           page tables. When the PWT flag is set, write-through caching is enabled for the
           associated page or page table; when the flag is clear, write-back caching is
           enabled for the associated page or page table. */
        page_dir[i].pwt = 0;
        page_dir[i].us = 1;
        page_dir[i].rw = 1;
        page_dir[i].p = 1;
    }

    /* 设置页表 */
    for (i = 0; i < page_table_num; i++) {
        /* 一个页目录表项对应1K(0x400)个表项, 0x1000 = 4K */
        for (j = 0; j < 0x400; j++) {
            page_table[j + i*0x400].page_base_address = j + i*0x400;
            page_table[j + i*0x400].avail = 0;
            page_table[j + i*0x400].g = 0;
            page_table[j + i*0x400].pat = 0;
            page_table[j + i*0x400].d = 0;
            page_table[j + i*0x400].a = 0;
            page_table[j + i*0x400].pcd = 0;
            page_table[j + i*0x400].pwt = 0;
            page_table[j + i*0x400].us = 1;
            page_table[j + i*0x400].rw = 1;
            page_table[j + i*0x400].p = 1;
        }
    }

    /* 启动内存分页机制 */
    do {
        os_u32 cr3;
        cr3 = system_pdbr();
        __asm__ __volatile__("movl %0,%%eax\n\t"
                             "movl %%eax,%%cr3\n\t"
                             "movl %%cr0,%%eax\n\t"
                             "orl $0x80000000,%%eax\n\t"
                             "movl %%eax,%%cr0"\
                             :
                             :"m"(cr3)
                             :"eax");
    } while (0);

    /* flush pipeline */
    __asm__ __volatile__("ljmp $"asm_str(GDT_CODE32_INDEX * 8)",$1f;\
                          1:\
                          nop");
}

/* invalidate page.
   this instruction takes a single operand, which is a linear address. */
#define __flush_tlb(addr) __asm__ __volatile__("invlpg %0": :"m" (*(os_u32 *)(addr)))

#define __flush_all_tlb() \
    do { \
        os_u32 _reg; \
        __asm__ __volatile__("movl %%cr3, %0\n\t" \
                             "movl %0, %%cr3\n\t" \
                             : "=r" (_reg) \
                             :: "memory"); \
    } while (0)

/***************************************************************
 * description : address map
 * history     :
 ***************************************************************/
LOCALC pointer get_page_directory(os_void)
{
    os_u32 cr3;

    __asm__ __volatile__("movl %%cr3,%%eax\n\t"
                         "movl %%eax,%0\n\t"
                         :"=m"(cr3)::"eax");

    cr3 &= CR3_ADDR_MASK;

    return cr3;
}

/***************************************************************
 * description : address map
 * history     :
 ***************************************************************/
os_ret OS_API amap(pointer virt, pointer phys, os_uint length)
{
    struct page_directory_entry *page_dir;
    struct page_table_entry *page_table;
    os_uint i, c;

    cassert(0 == (virt & 0x00000fff));
    cassert(0 == (phys & 0x00000fff));

    page_dir = (struct page_directory_entry *) get_page_directory();

    print("cr3: %x, virt: %x, phys: %x, len: %x\n", page_dir, virt, phys, length);

    page_dir += (virt >> 22);
    page_table = (struct page_table_entry *)(page_dir->page_table_base_address << 12);

    /* page table item count */
    if (length & 0x00000fff) {
        length = (length >> 12) + 1;
    } else {
        length = length >> 12;
    }

    virt = virt >> 12;
    phys = phys >> 12;
    while (length) {
        i = virt & 0x000003ff;
        if (((1 << 10) - i) > length) {
            c = i + length;
        } else {
            c = 1 << 10;
        }
        length -= c - i;
        for (; i < c; i++) {
            page_table[i].page_base_address = phys++;
            __flush_tlb(virt++ << 12);
        }
        page_dir++;
        page_table = (struct page_table_entry *)((*(os_u32 *) page_dir) & 0xfffff000);
    } print("%x %x\n", virt, length);
    __flush_all_tlb();

    return OS_SUCC;
}

/***************************************************************
 * description : 虚拟地址转换为物理地址
 * history     :
 ***************************************************************/
pointer OS_API virt_to_phys(pointer addr)
{
    struct page_directory_entry *page_dir;
    struct page_table_entry *page_table;
    pointer phys;

    page_dir = (struct page_directory_entry *) get_page_directory();
    page_dir += addr >> 22;
    page_table = (struct page_table_entry *)(page_dir->page_table_base_address << 12);

    phys = page_table[(addr >> 12) & 0x000003ff].page_base_address;
    phys = (phys << 12) | (addr & 0x00000fff);

    return phys;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
pointer OS_API phys_to_virt(pointer addr)
{
    if ((addr >= ksm_base()) && (addr < get_current_ksm())) {
        addr += OS_DYNAMIC_MEM_OFFSET;
    }
    return addr;
}

/***************************************************************
 * description : 判断是否为绝对虚拟地址
 * history     :
 ***************************************************************/
os_bool OS_API virt_is_abs(pointer addr)
{
    return (addr == virt_to_phys(addr));
}

/***************************************************************
 * description : 启动高速缓存
 *               ia32-3.pdf table 10-5. cache operating modes
 *               cd:0, nw:0, normal cache mode. highest performance cache operation.
 *               cd:0, nw:1, invalid setting.
 *               cd:1, nw:0, no-fill cache mode. memory coherency is maintained.
 *               cd:1, nw:1, memory coherency is not maintained.
 * history     :
 ***************************************************************/
os_void enable_cache(os_void)
{
    /* CD Cache Disable (bit 30 of CR0). When the CD and NW flags are clear, caching of
       memory locations for the whole of physical memory in the processor’s internal (and
       external) caches is enabled. When the CD flag is set, caching is restricted as described
       in Table 10-5. To prevent the processor from accessing and updating its caches, the CD
       flag must be set and the caches must be invalidated so that no cache hits can occur (see
       Section 10.5.3., “Preventing Caching”). See Section 10.5., “Cache Control”, for a
       detailed description of the additional restrictions that can be placed on the caching of
       selected pages or regions of memory. */
    /* NW Not Write-through (bit 29 of CR0). When the NW and CD flags are clear, write-back
       (for Pentium 4, Intel Xeon, P6 family, and Pentium processors) or write-through (for
       Intel486 processors) is enabled for writes that hit the cache and invalidation cycles are
       enabled. See Table 10-5 for detailed information about the affect of the NW flag on
       caching for other settings of the CD and NW flags. */
    __asm__ __volatile__("movl %%cr0,%%eax\n\t"
                         "btr $30,%%eax\n\t" // cdbit:30, nwbit:29
                         "btr $29,%%eax\n\t"
                         "movl %%eax,%%cr0\n\t"
                         "wbinvd"
                         ::);
}

/***************************************************************
 * description : 关闭高速缓存
 * history     :
 ***************************************************************/
os_void disable_cache(os_void)
{
    __asm__ __volatile__("movl %%cr0,%%eax\n\t"
                         "bts $30,%%eax\n\t" // cdbit:30, nwbit:29
                         "btr $29,%%eax\n\t"
                         "movl %%eax,%%cr0\n\t"
                         "wbinvd"
                         ::);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void _x86_switch_task(struct _x86_task *task)
{
    struct ljmp_para ljmp_addr;

    /* set task jump addr */
    ljmp_addr.offset = 0;
    ljmp_addr.sel = (task->tss_selector) << 3; // tss*8;

    __asm__ __volatile__("ljmp *%0\n\t"::"m"(ljmp_addr));
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void init_tss(struct _x86_task *task, os_u32 wrapper_func) // TASK_WRAPPER_FUNC_PTR wrapper_func)
{
    struct x86_tss *tss = OS_NULL;

    cassert(OS_NULL != task);

    tss = &task->tss;

    tss->cr3 = task->page_dir_addr;
    tss->eip = wrapper_func;
    tss->eflags = 0x0202;
    tss->esp = task->stack_addr;

    /* 所有段选择子都置为局部描述符表, core任务由cpu自动填写为TI_GDT */
    tss->cs_sel = LDT_CODE32_INDEX*8 + TI_LDT + RPL_0;
    tss->ds_sel = LDT_DATA32_INDEX*8 + TI_LDT + RPL_0;
    tss->ss_sel = LDT_STACK32_INDEX*8 + TI_LDT + RPL_0;
    tss->es_sel = LDT_DATA32_INDEX*8 + TI_LDT + RPL_0;
    tss->fs_sel = LDT_DATA32_INDEX*8 + TI_LDT + RPL_0;
    tss->gs_sel = LDT_DATA32_INDEX*8 + TI_LDT + RPL_0;

    tss->ldtr = task->ldt_selector * 8;
    tss->link = 0;
    tss->rsvd = 0;
    tss->io_map_base_address = (os_u16)((pointer)(&tss->bitmap)-(pointer)(&tss->link));
    tss->bitmap_end = 0xff;
    /* allow all */
    mem_set(tss->bitmap, 0, sizeof(tss->bitmap));
    tss->eax = 0;
    tss->ebx = 0;
    tss->ecx = 0;
    tss->edx = 0;
    tss->ebp = 0; //task trace
    tss->esi = 0;
    tss->edi = 0;

    /* stack */
    tss->stack0_addr = 0;
    tss->stack0_sel = 0;
    tss->stack1_addr = 0;
    tss->stack1_sel = 0;
    tss->stack2_addr = 0;
    tss->stack2_sel = 0;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void init_ldt(struct _x86_task *task)
{
    struct seg_desc_struct *ldt;

    cassert(OS_NULL != task);

    ldt = task->ldt;

    /* null */
    ldt[LDT_NULL_INDEX].attr1 = 0;
    ldt[LDT_NULL_INDEX].base_h = 0;
    ldt[LDT_NULL_INDEX].base_l_1 = 0;
    ldt[LDT_NULL_INDEX].base_l_2 = 0;
    ldt[LDT_NULL_INDEX].base_m = 0;
    ldt[LDT_NULL_INDEX].limit_high_attr2 = 0;
    ldt[LDT_NULL_INDEX].limit_l_1 = 0;
    ldt[LDT_NULL_INDEX].limit_l_2 = 0;

    /* code */
    modify_seg_descriptor(&ldt[LDT_CODE32_INDEX], 0x0, 0xffffffff, AT_CODE_EXE+D_32);
    /* data */
    modify_seg_descriptor(&ldt[LDT_DATA32_INDEX], 0x0, 0xffffffff, AT_DATA_WRITE+D_32);
    /* stack */
    modify_seg_descriptor(&ldt[LDT_STACK32_INDEX], 0x0, task->stack_addr, AT_DATA_WRITE+D_32);
}

/***************************************************************
 * description : 初始化任务的gdt表描述符, 包括tss和ldt.
 * history     :
 ***************************************************************/
os_void init_task_gdt(struct _x86_task *task)
{
    os_u32 tss_addr = 0;
    os_u32 ldt_addr = 0;
    struct seg_desc_struct *gdt_offset = OS_NULL;

    cassert(OS_NULL != task);

    /* modify gdt */
    tss_addr = (pointer) &(task->tss);
    ldt_addr = (pointer)(task->ldt);

    gdt_offset = (struct seg_desc_struct *)(GDT_ADDR + task->tss_selector * sizeof(struct seg_desc_struct));
    /* 修改tss段描述符 */
    modify_seg_descriptor(gdt_offset, tss_addr, 0xffffffff, DT_DPL0_TSS+D_32);

    gdt_offset = (struct seg_desc_struct *)(GDT_ADDR + task->ldt_selector * sizeof(struct seg_desc_struct));
    /* 修改ldt段描述符 */
    modify_seg_descriptor(gdt_offset, ldt_addr, 0xffffffff, DT_DPL0_LDT+D_32);
}

#if 1
/***************************************************************
 * description : 修改中断向量表
 * history     : 2009-04-22
 ***************************************************************/
LOCALC os_void modify_gate_descriptor(struct gate_desc_struct *desc, os_u32 func, os_u8 attr)
{
    desc->offset_l = (os_u16) func;

    /* 代码段选择子固定 */
    desc->selector = GDT_CODE32_INDEX * 8;
    desc->reserve = 0;
    //P=1, DPL=0.
    desc->attr = attr;
    desc->offset_h = (os_u16)(func >> 16);
}
#else
LOCALC os_void modify_gate_descriptor(os_u32 func, os_u32 vector_no)
{
    os_u32 int_addr = vector_no*8 + IDT_ADDR;

    /* 入参不检查 */

    __asm__ __volatile__("movl %0,%%edx\n\t"
                         "movl %1,%%edi\n\t"
                         "movl $0x80000,%%eax\n\t"
                         "movw %%dx,%%ax\n\t"
                         "movw $0x8e00,%%dx\n\t"
                         "movl %%eax,(%%edi)\n\t"
                         "movl %%edx,4(%%edi)\n\t"\
                         :
                         :"m"(func),"m"(int_addr)
                         :"%eax","%edx","%edi");
}
#endif

#define build_idt_func(num) \
LOCALC os_void IRQ_FUNC idt_##num##_func(os_void) \
{ \
    print("default interrupt %d function!", num); \
    dump_stack(print); \
    while (1) { \
        hlt(); \
    } \
}
build_idt_func(32)
build_idt_func(33)
build_idt_func(34)
build_idt_func(35)
build_idt_func(36)
build_idt_func(37)
build_idt_func(38)
build_idt_func(39)
build_idt_func(40)
build_idt_func(41)
build_idt_func(42)
build_idt_func(43)
build_idt_func(44)
build_idt_func(45)
build_idt_func(46)
build_idt_func(47)

/***************************************************************
 * description : 中断/陷阱/异常通用处理函数
 *               IRQ_FUNC
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC comm_int_func(os_void)
{
    /* 中断使用的打印函数 */
    print("common interrupt function!\n");
    dump_stack(print);
    while (1) {
        hlt();
    }
}

/***************************************************************
 * description : 安装中断
 * history     : 2009-04-22
 ***************************************************************/
os_ret OS_API install_int(os_u32 vector_no, IN IRQ_FUNCPTR IRQ_FUNC func_1, IN IRQ_FUNCPTR IRQ_FUNC func_2)
{
    if (MAX_IDT_ITEM <= (vector_no + get_pic_start_vector())) {
        return OS_FAIL;
    }

    /* 修改中断向量表 */
    modify_gate_descriptor((struct gate_desc_struct *)((vector_no + get_pic_start_vector()) * sizeof(struct gate_desc_struct) + IDT_ADDR),
                           (pointer) get_ivt_func(vector_no), DT_DPL0_INT_GATE);

    /* 注册中断函数 */
    register_interrupt_func(vector_no, func_1, func_2);

    /* 打开中断控制器 */
    open_pic_int(vector_no);

    return OS_SUCC;
}

/***************************************************************
 * description : 卸载中断
 * history     : 2009-04-22
 ***************************************************************/
os_ret OS_API uninstall_int(os_u32 vector_no)
{
    if (MAX_IDT_ITEM <= (vector_no + get_pic_start_vector())) {
        return OS_FAIL;
    }

    /* 修改中断向量表 */
    modify_gate_descriptor((struct gate_desc_struct *)((vector_no + get_pic_start_vector()) * sizeof(struct gate_desc_struct) + IDT_ADDR),
                           (pointer) comm_int_func, DT_DPL0_INT_GATE);

    /* 打开中断控制器 */
    close_pic_int(vector_no);

    return OS_SUCC;
}

/***************************************************************
 * description : 安装陷阱
 * history     : 2009-04-22
 ***************************************************************/
os_ret OS_API install_trap(os_u32 vector_no, IN VOID_FUNCPTR IRQ_FUNC func)
{
    if ((OS_NULL == func) || (MAX_IDT_ITEM <= vector_no)) {
        return OS_FAIL;
    }

    /* 修改中断向量表 */
    modify_gate_descriptor((struct gate_desc_struct *)(vector_no*8 + IDT_ADDR), (pointer) func, DT_DPL0_TRAP_GATE);

    return OS_SUCC;
}

/***************************************************************
 * description : 卸载陷阱
 * history     : 2009-04-22
 ***************************************************************/
os_ret OS_API uninstall_trap(os_u32 vector_no)
{
    if (MAX_IDT_ITEM <= vector_no) {
        return OS_FAIL;
    }
    modify_gate_descriptor((struct gate_desc_struct *)(vector_no*8 + IDT_ADDR), (pointer) comm_int_func, DT_DPL0_TRAP_GATE);
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void init_idt(os_void)
{
    os_u32 i;

    /* 初始化256个中断向量表 */
    for (i = 0; i < MAX_IDT_ITEM; i++) {
        modify_gate_descriptor((struct gate_desc_struct *)(i * sizeof(struct gate_desc_struct) + IDT_ADDR),
                               (pointer) comm_int_func, DT_DPL0_INT_GATE);
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API install_lapic_int(os_u32 vector_no, IN IRQ_FUNCPTR IRQ_FUNC func_1, IN IRQ_FUNCPTR IRQ_FUNC func_2)
{
    if (MAX_IDT_ITEM <= (vector_no + get_lapic_start_vector())) {
        return OS_FAIL;
    }

    /* 修改中断向量表 */
    modify_gate_descriptor((struct gate_desc_struct *)((vector_no + get_lapic_start_vector()) * sizeof(struct gate_desc_struct) + IDT_ADDR),
                           (pointer) get_lvt_func(vector_no), DT_DPL0_INT_GATE);

    /* 注册中断函数 */
    register_lapic_func(vector_no, func_1, func_2);

    /* 打开中断控制器 */
    open_lapic_int(vector_no);

    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API uninstall_lapic_int(os_u32 vector_no)
{
    if (MAX_IDT_ITEM <= (vector_no + get_lapic_start_vector())) {
        return OS_FAIL;
    }

    /* 修改中断向量表 */
    modify_gate_descriptor((struct gate_desc_struct *)((vector_no + get_lapic_start_vector()) * sizeof(struct gate_desc_struct) + IDT_ADDR),
                           (pointer) OS_NULL, DT_DPL0_INT_GATE);

    /* 打开中断控制器 */
    close_lapic_int(vector_no);

    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     : 2009-04-22
 ***************************************************************/
os_ret modify_16code_descriptor(os_u32 index, os_u32 addr, os_u32 limit)
{
    struct seg_desc_struct *os_gdt_addr = (struct seg_desc_struct *)(GDT_ADDR);

    modify_seg_descriptor(&os_gdt_addr[index], addr, limit, AT_CODE_EXE);
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     : 2009-04-22
 ***************************************************************/
os_ret modify_16code_read_descriptor(os_u32 index, os_u32 addr, os_u32 limit)
{
    struct seg_desc_struct *os_gdt_addr = (struct seg_desc_struct *)(GDT_ADDR);

    modify_seg_descriptor(&os_gdt_addr[index], addr, limit, AT_CODE_EXE_READ);
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     : 2009-04-22
 ***************************************************************/
os_ret modify_16data_descriptor(os_u32 index, os_u32 addr, os_u32 limit)
{
    struct seg_desc_struct *os_gdt_addr = (struct seg_desc_struct *)(GDT_ADDR);

    modify_seg_descriptor(&os_gdt_addr[index], addr, limit, AT_DATA_WRITE);
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void init_processor(os_void)
{
    /* 初始化idt */
    init_idt();

    /* 初始化gdt */
    init_gdt();

    /* 主存分页 */
    init_page();

    /* 初始化traps */
    init_trap();

    /* 缓存开启 */
    enable_cache();
}

