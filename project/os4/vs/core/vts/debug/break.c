/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : break.c
 * version     : 1.0
 * description : 断点调试
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include "break.h"

/***************************************************************
 global variable declare
 ***************************************************************/

/* 调试陷阱向量号 */
#define DB_VECTOR UINT32_C(0x1)

/* 断点陷阱向量号 */
#define BP_VECTOR UINT32_C(0x3)

/***************************************************************
 * description :
 ***************************************************************/
struct debugger_info_struct {
    /* 是否使用 */
    os_bool flag;
    /* 调试器回调函数 */
    VOID_FUNCPTR func;
    /* 调试器句柄 */
    HTASK handle;
};

/* 调试器最大数量 */
#define MAX_DEBUGER_NUM 0x10

/* 调试器注册信息记录 */
LOCALD struct debugger_info_struct debugger_record[MAX_DEBUGER_NUM] = {0};
LOCALD spinlock_t debug_record_lock;

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description : 设置调试寄存器dr7
 *               31 30 29 28 27 26 25 24 23 22 21 20 19 18 17 16 15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
 *               len3  rw3   len2  rw2   len1  rw1   len0  rw0   0  0  gd 0  0  1  ge le g3 l3 g2 l2 g1 l1 g0 l0
 * history     :
 ***************************************************************/
LOCALC os_void set_dr7(os_void)
{
    os_u32 dr7 = 0;

    /* clear LE. set GE. GD */
    dr7 |= 0x00002600;

    /* R/W
       00—Break on instruction execution only.
       01—Break on data writes only.
       10—Break on I/O reads or writes.
       11—Break on data reads or writes but not instruction fetches.
       LEN
       00—1-byte length
       01—2-byte length
       10—Undefined
       11—4-byte length */
    dr7 |= 0x00000000; // !!! If the corresponding RWn field in register DR7 is 00 (instruction execution), then the LENn field should also be 00.

    dr7 |= 0x000000aa; // active four global breakpoint.

    /* write to register */
    __asm__ __volatile__("movl %0,%%eax\n\t"
                         "movl %%eax,%%dr7"
                         :
                         :"m"(dr7)
                         :"eax");
}

/***************************************************************
 * description : 清空调试寄存器dr0-dr3
 * history     :
 ***************************************************************/
LOCALC os_void clear_dr(os_void)
{
    __asm__ __volatile__("xorl %%eax,%%eax\n\t"
                         "movl %%eax,%%dr0\n\t"
                         "movl %%eax,%%dr1\n\t"
                         "movl %%eax,%%dr2\n\t"
                         "movl %%eax,%%dr3\n\t"
                         :
                         :
                         :"eax");
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void clear_debugger(os_void)
{
    os_u32 i;
    struct debugger_info_struct *t;

    for (i = 0, t = debugger_record; i < MAX_DEBUGER_NUM; i++, t++) {
        t->flag = OS_FALSE;
        t->func = OS_NULL;
        t->handle = OS_NULL;
    }
}

/***************************************************************
 * description : VOID_FUNCPTR
 * history     :
 ***************************************************************/
os_void IRQ_FUNC int3_handle(os_void)
{
    flog("\n int3\n");
}

/***************************************************************
 * description : breakpoint condition detected
 * history     :
 ***************************************************************/
LOCALC os_void bpcd_func(os_void)
{
    os_u32 i;

    for (i = 0; i < MAX_DEBUGER_NUM; i++) {
        if (OS_TRUE == debugger_record[i].flag) {
            /* 调用调试器函数 */
            debugger_record[i].func();
        }
    }
}

/***************************************************************
 * description : single step
 * history     :
 ***************************************************************/
LOCALC os_void ss_func(os_void)
{
}

/***************************************************************
 * description : debug register access detected
 * history     :
 ***************************************************************/
LOCALC os_void drad_func(os_void)
{
#if 0
    os_u32 dr7 = 0;

    /* read register */
    __asm__ __volatile__("movl %%dr7,%%eax\n\t"
                         "movl %%eax,%0"\
                         :"=m"(dr7)
                         :
                         :"eax");

    /* clear LE. set GE. GD */
    dr7 |= 0x00002000;

    /* write register */
    __asm__ __volatile__("movl %0,%%eax\n\t"
                         "movl %%eax,%%dr7"\
                         :
                         :"m"(dr7)
                         :"eax");
#endif
    flog("debug register is modified.\n");
}

/***************************************************************
 * description : task switch
 * history     :
 ***************************************************************/
LOCALC os_void ts_func(os_void)
{
}

/***************************************************************
 * description : VOID_FUNCPTR
 * history     :
 ***************************************************************/
os_void IRQ_FUNC int1_handle(os_void)
{
    os_u32 dr6 = 0;

    /* 读调试状态寄存器 */
    __asm__ __volatile__("movl %%dr6,%%eax\n\t"
                         "movl %%eax,%0"\
                         :"=m"(dr6)
                         :
                         :"eax");
    print("int1 dr6 %x\n", dr6);

    // 以下按优先级(频率)排列
    if (0 != (dr6 & 0xf)) {
        cassert(OS_FALSE);/* 1.断点生效 */
        bpcd_func();
        return;
    }

    if (0 != (dr6 & 0x4000)) {
        /* 2.单步调试 */
        ss_func();
        return;
    }

    if (0 != (dr6 & 0x2000)) {
        /* 3.修改调试寄存器 */
        drad_func();
        return;
    }

    if (0 != (dr6 & 0x8000)) {
        /* 4.任务切换 */
        ts_func();
        return;
    }

    // 无效
#if 0
    /* 调试器执行完毕
     * 防止loop断点置rf位
     * cpu在下一条非iret/jmp/int指令执行完毕后清rf位
     * Note that the POPF, POPFD, and IRET instructions do not transfer the RF image into the EFLAGS register.
     */
    __asm__ __volatile__("pushfl\n\t"
                         "xorl $0x00010000,%%ss:(%%esp)\n\t"
                         "popfl"::);
#endif
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void update_bp_type(enum bp_type type, enum bp_data_len len, os_u8 dr_index)
{
    os_u32 dr7;
    os_u32 len_rw;

    __asm__ __volatile__("movl %%dr7,%%eax\n\t"
                         "movl %%eax,%0"
                         :"=m"(dr7)
                         :
                         :"eax");

    len_rw = 0x0f;
    dr7 &= ~(len_rw << (16 + (4 * dr_index)));

    len_rw = 0;
    switch (type) {
    case BREAK_ON_EXE: /* Break on instruction execution only. */
        len_rw = 0x00; // 0000
        break;
    case BREAK_ON_WRITE: /* Break on data writes only. */
        len_rw = ((len & 3) << 2) | 0x01; // xx01
        break;
    case BREAK_ON_IO: /* Break on I/O reads or writes. */
        len_rw = ((len & 3) << 2) | 0x02; // xx10
        break;
    case BREAK_ON_READ: /* Break on data reads or writes but not instruction fetches. */
        len_rw = ((len & 3) << 2) | 0x03; // xx11
        break;
    default:
        cassert(OS_FALSE);
        break;
    }
    dr7 |= (len_rw << (16 + (4 * dr_index)));

    /* write to register */
    __asm__ __volatile__("movl %0,%%eax\n\t"
                         "movl %%eax,%%dr7"
                         :
                         :"m"(dr7)
                         :"eax");
}

/* function */
typedef os_void (*BREAK_FUNCPTR)(os_u32 addr, enum bp_type type, enum bp_data_len len);

#if 1
/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void dr0_break(os_u32 addr, enum bp_type type, enum bp_data_len len)
{
    /* GD (general detect enable) flag (bit 13)
       Enables (when set) debug-register protection, which causes a debug exception
       to be generated prior to any MOV instruction that accesses a debug register. */
    __asm__ __volatile__("movl %0,%%eax\n\t"
                         "movl %%eax,%%dr0"\
                         :
                         :"m"(addr)
                         :"eax"); /* this instruction will cause #DB(int1) exception */
    update_bp_type(type, len, 0);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void dr1_break(os_u32 addr, enum bp_type type, enum bp_data_len len)
{
    __asm__ __volatile__("movl %0,%%eax\n\t"
                         "movl %%eax,%%dr1"\
                         :
                         :"m"(addr)
                         :"eax");
    update_bp_type(type, len, 1);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void dr2_break(os_u32 addr, enum bp_type type, enum bp_data_len len)
{
    __asm__ __volatile__("movl %0,%%eax\n\t"
                         "movl %%eax,%%dr2"\
                         :
                         :"m"(addr)
                         :"eax");
    update_bp_type(type, len, 2);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void dr3_break(os_u32 addr, enum bp_type type, enum bp_data_len len)
{
    __asm__ __volatile__("movl %0,%%eax\n\t"
                         "movl %%eax,%%dr3"\
                         :
                         :"m"(addr)
                         :"eax");
    update_bp_type(type, len, 3);
}
#else
#define build_drx_break(num) \
os_void dr##num##_break(os_u32 addr) \
{ \
    __asm__ __volatile__("movl %0,%%eax\n\t" \
                         "movl %%eax,%%dr"#num \
                         : \
                         :"m"(addr) \
                         :"eax"); \
}

build_drx_break(0)
build_drx_break(1)
build_drx_break(2)
build_drx_break(3)
#endif

/***************************************************************
 * description :
 ***************************************************************/
struct break_info_struct {
    /* 是否使用 */
    os_bool flag;
    /* 指令断点地址 */
    os_u32 addr;
    /* 设置硬件断点函数 */
    BREAK_FUNCPTR func;
    /* 设置断点的调试器 */
    HTASK handle;
};

/* intel cpu 断点寄存器数量 */
#define IA_DEBUG_NUM 4

LOCALD struct break_info_struct break_info[IA_DEBUG_NUM] = {
    /* flag,    addr,func,      handle */
    { OS_FALSE, 0x0, dr0_break, OS_NULL },
    { OS_FALSE, 0x0, dr1_break, OS_NULL },
    { OS_FALSE, 0x0, dr2_break, OS_NULL },
    { OS_FALSE, 0x0, dr3_break, OS_NULL }
};
LOCALD spinlock_t break_info_lock;

/***************************************************************
 * description : 最多支持4个硬件断点
 * history     :
 ***************************************************************/
os_ret OS_API set_bp(os_u32 addr, enum bp_type type, enum bp_data_len len, IN HTASK debugger)
{
    os_u32 i;
    struct break_info_struct *t;

    spin_lock(&break_info_lock);
    for (i = 0, t = break_info; i < IA_DEBUG_NUM; i++, t++) {
        if (OS_FALSE == t->flag) {
            /* 设置断点 */
            t->flag = OS_TRUE;
            t->handle = debugger;
            t->addr = addr; // 记录, 不用再从寄存器中读取.
            t->func(addr, type, len);
            spin_unlock(&break_info_lock);
            return OS_SUCC;
        }
    }
    spin_unlock(&break_info_lock);
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API clear_bp(os_u32 addr)
{
    os_u32 i;
    struct break_info_struct *t;

    for (i = 0, t = break_info; i < IA_DEBUG_NUM; i++, t++) {
        spin_lock(&break_info_lock);
        if ((OS_TRUE == t->flag) && (addr == t->addr)) {
            /* 设置断点 */
            t->flag = OS_FALSE;
            //t->func = OS_NULL;
            t->handle = OS_NULL;
            t->addr = 0x0;

            /* 删除cpu断点 */
            t->func(0, 0, 0);
        }
        spin_unlock(&break_info_lock);
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void init_debugger(os_void)
{
    init_spinlock(&break_info_lock);
    init_spinlock(&debug_record_lock);

    /* 清空调试器相关 */
    clear_debugger();

    /* 清空dr0-dr3 */
    clear_dr();

    /* 设置dr7 */
    set_dr7();

    /* 断点陷阱门函数 */
    register_trap(BP_VECTOR, int3_handle);

    /* 调试陷阱门函数 */
    register_trap(DB_VECTOR, int1_handle);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API install_debugger(IN VOID_FUNCPTR func, IN HTASK debugger)
{
    struct debugger_info_struct *t;
    os_u32 i;
    os_ret ret;

    ret = OS_FAIL;
    spin_lock(&debug_record_lock);
    for (i = 0, t = debugger_record; i < MAX_DEBUGER_NUM; i++, t++) {
        if (OS_FALSE == t->flag) {
            t->flag = OS_TRUE;
            t->func = func;
            t->handle = debugger;
            ret = OS_SUCC;
            break;
        }
    }
    spin_unlock(&debug_record_lock);
    return ret;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API uninstall_debugger(IN HTASK debugger)
{
    os_u32 i;
    struct debugger_info_struct *d;
    struct break_info_struct *b;

    /* 删除所有断点 */
    spin_lock(&break_info_lock);
    for (i = 0, b = break_info; i < IA_DEBUG_NUM; i++, b++) {
        if ((OS_TRUE == b->flag) && (debugger == b->handle)) {
            /* 设置断点 */
            b->flag = OS_TRUE;
            //b->func = OS_NULL;
            b->handle = OS_NULL;
            b->addr = 0x0;

            /* 删除cpu断点 */
            b->func(0, 0, 0);
        }
    }
    spin_unlock(&break_info_lock);

    /* 删除注册函数 */
    spin_lock(&debug_record_lock);
    for (i = 0, d = debugger_record; i < MAX_DEBUGER_NUM; i++, d++) {
        if ((OS_TRUE == d->flag) && (debugger == d->handle)) {
            d->flag = OS_FALSE;
            d->func = OS_NULL;
            d->handle = OS_NULL;
            register_trap(BP_VECTOR, OS_NULL);
        }
    }
    spin_unlock(&debug_record_lock);
    return OS_SUCC;
}

