/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : i386.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __VTS_I386_H__
#define __VTS_I386_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/
/* 中断向量最大个数 */
#define MAX_IDT_ITEM 0x100

/* 单core */
#define CORE_ID_0 0

/* 单cpu */
#define CPU_ID_0 0

#if 0
/* i3(sandy bridge) */
#define CORE_NUM 2
#define CPU_NUM 2
#else
/* i386 */
#define CORE_NUM 1
#define CPU_NUM 1
#endif

/*************************** alloc gdt begin ********************/
#define GDT_NULL_INDEX 0

#define GDT_CODE16_INDEX 1

#define GDT_DATA16_INDEX 2

#define GDT_CODE32_INDEX 3

#define GDT_DATA32_INDEX 4

#define GDT_CODE64_INDEX 5

#define GDT_DATA64_INDEX 6

#define GDT_NULL_TSS_INDEX 7

#define GDT_NULL_LDT_INDEX 8
/*************************** alloc gdt end **********************/

/*************************** alloc ldt begin ********************/
#define LDT_NULL_INDEX 0

#define LDT_CODE32_INDEX 1

#define LDT_DATA32_INDEX 2

#define LDT_STACK32_INDEX 3
/*************************** alloc ldt end **********************/

/* cpu最大任务数量, 该宏定义为2^n */
#define CPU_MAX_TASK_NUM 0x1000

/* ldt为8个描述符 */
#define LDT_DESCRIPTOR_NUM 0x8

/* ti=0, 全局描述符表标志 */
#define TI_GDT 0x0

/* ti=1, 局部描述符表标志 */
#define TI_LDT 0x4

/* 段描述符特权等级 */
#define RPL_0 0

/* 段描述符特权等级 */
#define RPL_1 1

/* 段描述符特权等级 */
#define RPL_2 2

/* 段描述符特权等级 */
#define RPL_3 3

/* 0x4000, 32位代码段标志 */
#define D_32 0xc000

/* 代码段和数据段属于segment descriptor */
//{
    /* 存在的只执行代码段类型值 */
    #define AT_CODE_EXE 0x98

    /* 存在的只执行代码段类型值 */
    #define AT_CODE_EXE_READ 0x9a

    /* 存在的已访问可读写数据段类型值 */
    #define AT_DATA_WRITE 0x92
//}

/* When the S (descriptor type) flag in a segment descriptor is clear, the descriptor type is a system descriptor. The processor recognizes the following types of system descriptors: */
//{
    /* Local descriptor-table (LDT) segment descriptor. */
    #define DT_DPL0_LDT 0x82

    /* Task-state segment (TSS) descriptor. */
    #define DT_DPL0_TSS 0x89

    /* Call-gate descriptor. */
    #define DT_DPL0_CALL_GATE 0x8c // Call gates facilitate controlled transfers of program control between different privilege levels.

    /* Interrupt-gate descriptor. */
    #define DT_DPL0_INT_GATE 0x8e

    /* Trap-gate descriptor. */
    #define DT_DPL0_TRAP_GATE 0x8f

    /* Task-gate descriptor. */
    #define DT_DPL0_TASK_GATE 0x85
//}

/* 页大小为4k */
#define PAGE_ALIGN 12
#define PAGE_SIZE (1 << PAGE_ALIGN)

/***************************************************************
 enum define
 ***************************************************************/

/***************************************************************
 struct define
 ***************************************************************/
/***************************************************************
 * description : gdt/ldt
 ***************************************************************/
struct seg_desc_struct {
    os_u8 limit_l_1;
    os_u8 limit_l_2;
    os_u8 base_l_1;
    os_u8 base_l_2;
    os_u8 base_m;
    os_u8 attr1;
    os_u8 limit_high_attr2;
    os_u8 base_h;
};

/***************************************************************
 * description : gdt/idt
 ***************************************************************/
struct gate_desc_struct {
    /* 内存低地址 */
    os_u16 offset_l;
    os_u16 selector;
    os_u8 reserve;
    os_u8 attr;
    os_u16 offset_h;
    /* 内存高地址 */
};

/***************************************************************
 * description : 切换上下文(非中断上下文)保存空间
 ***************************************************************/
struct x86_tss {
    os_u32 link;         /* 32bit */
    os_u32 stack0_addr;  /* 0级堆栈指针 */
    os_u32 stack0_sel;
    os_u32 stack1_addr;  /* 1级堆栈指针 */
    os_u32 stack1_sel;
    os_u32 stack2_addr;  /* 2级堆栈指针 */
    os_u32 stack2_sel;
    os_u32 cr3;
    os_u32 eip;
    os_u32 eflags;
    os_u32 eax,ecx,edx,ebx;
    os_u32 esp;
    os_u32 ebp;
    os_u32 esi,edi;
    os_u32 es_sel,cs_sel,ss_sel,ds_sel,fs_sel,gs_sel;
    os_u32 ldtr;
    os_u16 rsvd;
    os_u16 io_map_base_address;

/* The I/O address space consists of 64K individually addressable 8-bit I/O ports,
   numbered 0 through FFFFH. */
#define IO_ADDR_LEN 0x10000
    os_u32 bitmap[IO_ADDR_LEN/8/sizeof(os_u32)];
    os_u32 bitmap_end;
};

/***************************************************************
 * description : gdt
                 0:null
                 1:32code 2:32data
                 3:16code 4:16data
                 5:tss0   6:ldt0
 ***************************************************************/
struct _x86_task {
    os_u32 tss_selector; // gdt index
    os_u32 ldt_selector; // gdt index
    /* cpu保存任务上下文空间 */
    struct x86_tss tss;
    struct seg_desc_struct ldt[LDT_DESCRIPTOR_NUM];

    /* mm */
    os_u32 page_dir_addr;
    os_u32 page_addr;

    /* stack */
    os_u32 stack_addr;
    os_u32 stack_len;
};

/***************************************************************
 * description :
 ***************************************************************/
struct pseudo_descriptor {
    os_u16 reserve;
    os_u16 limit;
    os_u32 base;
};

/***************************************************************
 * description :
 ***************************************************************/
struct ljmp_para {
    os_u32 offset;
    os_u32 sel;
};

/***************************************************************
 extern function
 ***************************************************************/
os_void init_processor(os_void);
os_void init_tss(struct _x86_task *task, os_u32 wrapper_func);
os_void init_ldt(struct _x86_task *task);
os_void init_task_gdt(struct _x86_task *task);
os_void _x86_switch_task(struct _x86_task *task);

#pragma pack()

#endif /* end of header */

