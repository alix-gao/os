/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : apic.c
 * version     : 1.0
 * description : (key) local apic
 * author      : gaocheng
 * date        : 2018-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include "apic.h"

/***************************************************************
 global variable declare
 ***************************************************************/

/* max lapic int */
#define LAPIC_INT_COUNT 0x7

/* lapic.s: */
/* int 0x30 */
os_void IRQ_FUNC lapic_timer_int(os_void);
/* int 0x31 */
os_void IRQ_FUNC lapic_cmci_int(os_void);
/* int 0x32 */
os_void IRQ_FUNC lapic_lint0_int(os_void);
/* int 0x33 */
os_void IRQ_FUNC lapic_lint1_int(os_void);
/* int 0x34 */
os_void IRQ_FUNC lapic_error_int(os_void);
/* int 0x35 */
os_void IRQ_FUNC lapic_pmc_int(os_void);
/* int 0x36 */
os_void IRQ_FUNC lapic_ts_int(os_void);

/* note: no NULL is filled! */
LOCALD const VOID_FUNCPTR local_ventor_table[LAPIC_INT_COUNT] = {
    /* int 0x30 */
    lapic_timer_int,
    /* int 0x31 */
    lapic_cmci_int,
    /* int 0x32 */
    lapic_lint0_int,
    /* int 0x33 */
    lapic_lint1_int,
    /* int 0x34 */
    lapic_error_int,
    /* int 0x35 */
    lapic_pmc_int,
    /* int 0x36 */
    lapic_ts_int
};

/* 中断控制器起始向量 */
LOCALD const os_u8 LAPIC_VECTOR = 0x30;

/* register VOID_FUNCPTR IRQ_FUNC */
LOCALD volatile os_void (*lapic_function_table[LAPIC_INT_COUNT][2])(os_u32 irq) _CPU_ALIGNED_ = { 0 };

/***************************************************************
 function declare
 ***************************************************************/

/***************************************************************
 * description : 获取apic的起始向量号
 * history     :
 ***************************************************************/
os_u32 get_lapic_start_vector(os_void)
{
    return LAPIC_VECTOR;
}

/***************************************************************
 * description : local vector table
 * history     :
 ***************************************************************/
VOID_FUNCPTR get_lvt_func(os_u32 vector_no)
{
    if (LAPIC_INT_COUNT > vector_no) {
        return local_ventor_table[vector_no];
    }
    cassert(OS_FALSE);
    return OS_NULL;
}

/***************************************************************
 * description : func_1 上半部只做同步操作, 硬件处理
 *               func_2 上半部可以异步操作, 软件处理
 * history     :
 ***************************************************************/
os_ret register_lapic_func(os_u32 vector_no, IRQ_FUNCPTR IRQ_FUNC func_1, IRQ_FUNCPTR IRQ_FUNC func_2)
{
    if (LAPIC_INT_COUNT > vector_no) {
        /* 写入数据中而不是代码段内存中(flush d-cache and i-cache) */
        lapic_function_table[vector_no][0] = func_1; /* 上半部只做同步操作、硬件处理 */
        lapic_function_table[vector_no][1] = func_2; /* 上半部可以异步操作、软件处理 */
        wmb();
        return OS_SUCC;
    }
    return OS_FAIL;
}

/***************************************************************
 * description : VOID_FUNCPTR
 *               中断包装函数
 *               注意eoi要在中断处理函数后面
 * history     :
 ***************************************************************/
os_void IRQ_FUNC wrap_lapic_int(os_u32 vector)
{
    cassert(OS_FALSE);

    /* first interrupt function */
    if (lapic_function_table[vector][0]) {
        lock_schedule();
        lapic_function_table[vector][0](vector);
        unlock_schedule();
    }

    /* close interrupt.
       when leave this function, iret is executed, interrupt is opened. */
    cli();

    /* open 8259a interrupt */
    //open_apic_int(vector);

    /* second interrupt function */
    if (lapic_function_table[vector][1]) {
        lapic_function_table[vector][1](vector);
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void OS_API open_lapic_int(os_u32 vector)
{
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void OS_API close_lapic_int(os_u32 vector)
{
}

static void apic_write(os_u32 reg, os_u32 v)
{
    *((volatile os_u32 *)(APIC_DEFAULT_PHYS_BASE+reg)) = v;
}

static os_u32 apic_read(os_u32 reg)
{
    return *((volatile os_u32 *)(APIC_DEFAULT_PHYS_BASE+reg));
}

static void enable_NMI_through_LVT0(void)
{
    os_u32 v, ver;

    ver = apic_read(APIC_LVR);
    ver = GET_APIC_VERSION(ver);
    v = APIC_DM_NMI; /* unmask and set to NMI */
    if (!APIC_INTEGRATED(ver)) /* 82489DX */
        v |= APIC_LVT_LEVEL_TRIGGER;
    apic_write(APIC_LVT0, v);
}

/*
 * Includes eventsel and unit mask as well:
 */
#define ARCH_PERFMON_EVENT_MASK 0xffff

#define ARCH_PERFMON_UNHALTED_CORE_CYCLES_SEL 0x3c
#define ARCH_PERFMON_UNHALTED_CORE_CYCLES_UMASK (0x00 << 8)
#define ARCH_PERFMON_UNHALTED_CORE_CYCLES_INDEX 0
#define ARCH_PERFMON_UNHALTED_CORE_CYCLES_PRESENT \
    (1 << (ARCH_PERFMON_UNHALTED_CORE_CYCLES_INDEX))

#define ARCH_PERFMON_BRANCH_MISSES_RETIRED			 6

#define MSR_ARCH_PERFMON_PERFCTR0			      0xc1
#define MSR_ARCH_PERFMON_PERFCTR1			      0xc2

#define MSR_ARCH_PERFMON_EVENTSEL0			     0x186
#define MSR_ARCH_PERFMON_EVENTSEL1			     0x187

#define ARCH_PERFMON_EVENTSEL0_ENABLE			  (1 << 22)
#define ARCH_PERFMON_EVENTSEL_INT			  (1 << 20)
#define ARCH_PERFMON_EVENTSEL_OS			  (1 << 17)
#define ARCH_PERFMON_EVENTSEL_USR			  (1 << 16)

static inline void native_write_msr(unsigned int msr, unsigned low, unsigned high)
{
    asm volatile("wrmsr" : : "c" (msr), "a"(low), "d" (high) : "memory");
}

#define wrmsrl(msr, val) \
    native_write_msr((msr), (u32)((u64)(val)), (u32)((u64)(val) >> 32))

/*
 * Watchdog using the Intel architected PerfMon.
 * Used for Core2 and hopefully all future Intel CPUs.
 */
#define ARCH_PERFMON_NMI_EVENT_SEL	ARCH_PERFMON_UNHALTED_CORE_CYCLES_SEL
#define ARCH_PERFMON_NMI_EVENT_UMASK	ARCH_PERFMON_UNHALTED_CORE_CYCLES_UMASK

static void write_watchdog_counter32(unsigned int perfctr_msr,
                const char *descr, unsigned nmi_hz)
{
	wrmsr(perfctr_msr, (u32)(-1), 0);
}

static int setup_intel_arch_watchdog(unsigned nmi_hz)
{
    unsigned int ebx;
    unsigned int eax;
    unsigned int unused;
    unsigned int perfctr_msr, evntsel_msr;
    unsigned int evntsel;

    /*
     * Check whether the Architectural PerfMon supports
     * Unhalted Core Cycles Event or not.
     * NOTE: Corresponding bit = 0 in ebx indicates event present.
     */
    cpuid(10, &eax, &ebx, &unused, &unused);
    if (((eax >> 24) < (ARCH_PERFMON_UNHALTED_CORE_CYCLES_INDEX+1)) ||
        (ebx & ARCH_PERFMON_UNHALTED_CORE_CYCLES_PRESENT)) {
        print("cpu do not support pc!!!\n");
        return 0;
    }

    perfctr_msr = MSR_ARCH_PERFMON_PERFCTR0;
    evntsel_msr = MSR_ARCH_PERFMON_EVENTSEL0;

    wrmsrl(perfctr_msr, 0UL);

    evntsel = ARCH_PERFMON_EVENTSEL_INT
        | ARCH_PERFMON_EVENTSEL_OS
        | ARCH_PERFMON_EVENTSEL_USR
        | ARCH_PERFMON_NMI_EVENT_SEL
        | ARCH_PERFMON_NMI_EVENT_UMASK;

    /* setup the timer */
    wrmsr(evntsel_msr, evntsel, 0);
    write_watchdog_counter32(perfctr_msr, "INTEL_ARCH_PERFCTR0", nmi_hz);

    apic_write(APIC_LVTPC, APIC_DM_NMI);
    evntsel |= ARCH_PERFMON_EVENTSEL0_ENABLE;
    wrmsr(evntsel_msr, evntsel, 0);
    return 1;
}

/***************************************************************
 * description : 初始化local apic
 * history     :
 ***************************************************************/
os_void init_apic(os_void)
{
    os_u32 features;
    os_u32 l, h;
    os_u32 v;

    init_pic();

    features = cpuid_edx(1);
    if (!(features & (1 << X86_FEATURE_APIC))) {
        print("Could not enable APIC!\n");
        return;
    }

    rdmsr(MSR_IA32_APICBASE, l, h);
    print("msr apic: %x, %x\n", l, h);
    l |= MSR_IA32_APICBASE_ENABLE;
    wrmsr(MSR_IA32_APICBASE, l, h);

    v = apic_read(APIC_LVT0);
    print("lvt0: %x\n", v);
    v = apic_read(APIC_LVT1);
    print("lvt1: %x\n", v);

    v = apic_read(APIC_LVTPC);
    print("pc: %x\n", v);

#if 0
    /*
     * Dirty trick to enable the NMI watchdog ...
     * We put the 8259A master into AEOI mode and
     * unmask on all local APICs LVT0 as NMI.
     *
     * The idea to use the 8259A in AEOI mode ('8259A Virtual Wire')
     */
    enable_NMI_through_LVT0();
#endif
}

