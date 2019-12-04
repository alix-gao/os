/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : exception.c
 * version     : 1.0
 * description : cpuÄÚ²¿Òì³£
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>

/***************************************************************
 global variable declare
 ***************************************************************/

/* Exceptions are classified as faults, traps, or aborts depending on the way they are reported and
   whether the instruction that caused the exception can be restarted with no loss of program or task
   continuity.

   Faults
   A fault is an exception that can generally be corrected and that, once corrected,
   allows the program to be restarted with no loss of continuity. When a fault is
   reported, the processor restores the machine state to the state prior to the beginning
   of execution of the faulting instruction. The return address (saved contents
   of the CS and EIP registers) for the fault handler points to the faulting instruction,
   rather than the instruction following the faulting instruction.

   Traps
   A trap is an exception that is reported immediately following the execution of
   the trapping instruction. Traps allow execution of a program or task to be
   continued without loss of program continuity. The return address for the trap
   handler points to the instruction to be executed after the trapping instruction.

   Aborts
   An abort is an exception that does not always report the precise location of the
   instruction causing the exception and does not allow restart of the program or
   task that caused the exception. Aborts are used to report severe errors, such as
   hardware errors and inconsistent or illegal values in system tables. */
enum exception_type {
    EXCEPTION_FAULT = 0x1,
    EXCEPTION_TRAP = 0x2,
    EXCEPTION_ABORT = 0x4,
    EXCEPTION_INT = 0x8,
    EXCEPTION_RSVD = 0x10
};

struct exception_info {
    os_void IRQ_FUNC (*isr)(os_void);
    enum exception_type type;
};

/* To return from an exception- or interrupt-handler procedure, the handler must use the IRET (or
   IRETD) instruction. The IRET instruction is similar to the RET instruction except that it restores
   the saved flags into the EFLAGS register. */

/* int.s: _system_timer_int */
os_void error_code(os_void);
/* int 0x0 */
os_void IRQ_FUNC divide_error(os_void);
/* int 0x1 */
os_void IRQ_FUNC debug(os_void);
/* int 0x2 */
os_void IRQ_FUNC nmi(os_void);
/* int 0x3 */
os_void IRQ_FUNC int3(os_void);
/* int 0x4 */
os_void IRQ_FUNC overflow(os_void);
/* int 0x5 */
os_void IRQ_FUNC bounds(os_void);
/* int 0x6 */
os_void IRQ_FUNC invalid_op(os_void);
/* int 0x7 */
os_void IRQ_FUNC device_not_available(os_void);
/* int 0x8 */
os_void IRQ_FUNC double_fault(os_void);
/* int 0x9 */
os_void IRQ_FUNC coprocessor_segment_overrun(os_void);
/* int 0xa */
os_void IRQ_FUNC invalid_tss(os_void);
/* int 0xb */
os_void IRQ_FUNC segment_not_present(os_void);
/* int 0xc */
os_void IRQ_FUNC stack_segment(os_void);
/* int 0xd */
os_void IRQ_FUNC general_protection(os_void);
/* int 0xe */
os_void IRQ_FUNC page_fault(os_void);
/* int 0xf */
os_void IRQ_FUNC reserved(os_void);
/* int 0x10 */
os_void IRQ_FUNC x87_fpu_floating_point_error(os_void);
/* int 0x11 */
os_void IRQ_FUNC alignment_check(os_void);
/* int 0x12 */
os_void IRQ_FUNC machine_check(os_void);
/* int 0x13 */
os_void IRQ_FUNC simd_floating_point_exception(os_void);

/* see ia32-3.pdf, Table 5-1. Protected-Mode Exceptions and Interrupts */
#define IA_INTEL_RSVD_INT UINT32_C(0x20)
LOCALD const struct exception_info exception_ventor_table[IA_INTEL_RSVD_INT] = {
    /* int 0x0, Divide Error */
    { divide_error, EXCEPTION_FAULT },
    /* int 0x1, Debug */
    { debug, EXCEPTION_FAULT | EXCEPTION_TRAP },
    /* int 0x2, NMI Interrupt */
    { nmi, EXCEPTION_INT },
    /* int 0x3, Breakpoint */
    { int3, EXCEPTION_TRAP },
    /* int 0x4, Overflow */
    { overflow, EXCEPTION_TRAP },
    /* int 0x5, BOUND Range Exceed */
    { bounds, EXCEPTION_FAULT },
    /* int 0x6, Invalid Opcode (Undefined Opcode) */
    { invalid_op, EXCEPTION_FAULT },
    /* int 0x7, Device Not Available (No Math Coprocessor) */
    { device_not_available, EXCEPTION_FAULT },
    /* int 0x8, Double Fault */
    { double_fault, EXCEPTION_ABORT },
    /* int 0x9, Coprocessor Segment Overrun (reserved) */
    { coprocessor_segment_overrun, EXCEPTION_FAULT },
    /* int 0xa, Invalid TSS */
    { invalid_tss, EXCEPTION_FAULT },
    /* int 0xb, Segment Not Present */
    { segment_not_present, EXCEPTION_FAULT },
    /* int 0xc, Stack-Segment Fault */
    { stack_segment, EXCEPTION_FAULT },
    /* int 0xd, General Protection */
    { general_protection, EXCEPTION_FAULT },
    /* int 0xe, Page Fault */
    { page_fault, EXCEPTION_FAULT },
    /* int 0xf, (Intel reserved. Do not use.) */
    { reserved, EXCEPTION_FAULT },
    /* int 0x10, x87 FPU Floating-Point Error (Math Fault) */
    { x87_fpu_floating_point_error, EXCEPTION_FAULT },
    /* int 0x11, Alignment Check */
    { alignment_check, EXCEPTION_FAULT },
    /* int 0x12, Machine Check */
    { machine_check, EXCEPTION_ABORT },
    /* int 0x13, SIMD Floating-Point Exception */
    { simd_floating_point_exception, EXCEPTION_FAULT },
    /* int 0x14, Intel reserved. Do not use. */
    { reserved, EXCEPTION_RSVD },
    /* int 0x15, Intel reserved. Do not use. */
    { reserved, EXCEPTION_RSVD },
    /* int 0x16, Intel reserved. Do not use. */
    { reserved, EXCEPTION_RSVD },
    /* int 0x17, Intel reserved. Do not use. */
    { reserved, EXCEPTION_RSVD },
    /* int 0x18, Intel reserved. Do not use. */
    { reserved, EXCEPTION_RSVD },
    /* int 0x19, Intel reserved. Do not use. */
    { reserved, EXCEPTION_RSVD },
    /* int 0x1a, Intel reserved. Do not use. */
    { reserved, EXCEPTION_RSVD },
    /* int 0x1b, Intel reserved. Do not use. */
    { reserved, EXCEPTION_RSVD },
    /* int 0x1c, Intel reserved. Do not use. */
    { reserved, EXCEPTION_RSVD },
    /* int 0x1d, Intel reserved. Do not use. */
    { reserved, EXCEPTION_RSVD },
    /* int 0x1e, Intel reserved. Do not use. */
    { reserved, EXCEPTION_RSVD },
    /* int 0x1f, Intel reserved. Do not use. */
    { reserved, EXCEPTION_RSVD }
};

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description : backtrace, interrupt function and not interrupt function.
 * history     :
 ***************************************************************/
LOCALC os_void dump_call_stack(os_void)
{
    os_u32 reg_ebp;
    os_u8 *reg_eip = OS_NULL;
    /* os_u8 prolog[] = { 0xe5, 0x89, 0x55 }; //5589e5 */
    os_u32 count;

    print("call stack:\n");

    __asm__ __volatile__("movl %%ebp, %0"
                        :"=g"(reg_ebp)
                        :
                        :"memory");

    /* stop when task_wrapper_func */
    while (0 != reg_ebp) {
        /* return address */
        reg_eip = (os_u8 *)(*(os_u32 *)(reg_ebp + 4));
        /* the value of the frame pointer of the caller */
        reg_ebp = *(os_u32 *) reg_ebp; /* caller function variables address */

#define CALLSTACK_FUNC_SIZE 0x1000
        count = CALLSTACK_FUNC_SIZE;
        while ((OS_NULL != reg_eip) && (--count)) {
            if ((reg_eip[0] == 0x55) && (reg_eip[1] == 0x89) && (reg_eip[2] == 0xe5)) {
                print("(%x)", reg_eip);
                break;
            }
            reg_eip--;
        }
        if (0 == count) {
            print("(end)");
            break;
        }
    } print("\n");
}

/***************************************************************
 * description : refer to _trap_wrap in trap.s
 * history     :
 ***************************************************************/
LOCALC os_void dump_cpu(os_u32 *regs)
{
    print("dump cpu:\n");
    print("gs: %x\n", *regs++);
    print("fs: %x\n", *regs++);
    print("es: %x\n", *regs++);
    print("ds: %x\n", *regs++);
    print("ebp: %x\n", *regs++);
    print("esi: %x\n", *regs++);
    print("edi: %x\n", *regs++);
    print("edx: %x\n", *regs++);
    print("ecx: %x\n", *regs++);
    print("ebx: %x\n", *regs++);
    print("eax: %x\n", *regs++);
    print("eip: %x\n", *regs++);
    print("cs: %x\n", *regs++);
    print("eflags: %x\n", *regs++);
}

#define init_exception_screen() \
    do { \
        clear_screen(VGA_COLOR_BLACK); \
        init_print(); \
    } while (0)

#if 0
/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_0_func(os_void)
{
    init_screen();
    print("exception 0!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_1_func(os_void)
{
    init_screen();
    print("exception 1!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_2_func(os_void)
{
    init_screen();
    print("exception 2!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_3_func(os_void)
{
    init_screen();
    print("exception 3!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_4_func(os_void)
{
    init_screen();
    print("exception 4!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_5_func(os_void)
{
    init_screen();
    print("exception 5!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_6_func(os_void)
{
    init_screen();
    print("exception 6!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_7_func(os_void)
{
    init_screen();
    print("exception 7!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_8_func(os_void)
{
    init_screen();
    print("exception 8!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_9_func(os_void)
{
    init_screen();
    print("exception 9!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_10_func(os_void)
{
    init_screen();
    print("exception 10!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_11_func(os_void)
{
    init_screen();
    print("exception 11!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_12_func(os_void)
{
    init_screen();
    print("exception 12!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_13_func(os_void)
{
    init_screen();
    print("exception 13!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_14_func(os_void)
{
    init_screen();
    print("exception 14!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_15_func(os_void)
{
    init_screen();
    print("exception 15!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_16_func(os_void)
{
    init_screen();
    print("exception 16!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_17_func(os_void)
{
    init_screen();
    print("exception 17!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_18_func(os_void)
{
    init_screen();
    print("exception 18!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_19_func(os_void)
{
    init_screen();
    print("exception 19!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_20_func(os_void)
{
    init_screen();
    print("exception 20!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_21_func(os_void)
{
    init_screen();
    print("exception 21!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_22_func(os_void)
{
    init_screen();
    print("exception 22!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_23_func(os_void)
{
    init_screen();
    print("exception 23!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_24_func(os_void)
{
    init_screen();
    print("exception 24!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_25_func(os_void)
{
    init_screen();
    print("exception 25!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_26_func(os_void)
{
    init_screen();
    print("exception 26!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_27_func(os_void)
{
    init_screen();
    print("exception 27!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_28_func(os_void)
{
    init_screen();
    print("exception 28!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_29_func(os_void)
{
    init_screen();
    print("exception 29!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_30_func(os_void)
{
    init_screen();
    print("exception 30!\n");
    dump_cpu();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC exception_31_func(os_void)
{
    init_screen();
    print("exception 31!\n");
    dump_cpu();
}
#else
#define build_exception_x_func(num) \
LOCALC os_void IRQ_FUNC exception_##num##_func(os_void) \
{ \
    init_exception_screen(); \
    print("[%s] exception(%s) "_str(num)"!\n", get_task_name(current_task_handle()), __FUNCTION__); \
    dump_call_stack(); \
    lock_schedule(); \
    dead(); \
}
build_exception_x_func(0)
build_exception_x_func(1)
build_exception_x_func(2)
build_exception_x_func(3)
build_exception_x_func(4)
build_exception_x_func(5)
build_exception_x_func(6)
build_exception_x_func(7)
build_exception_x_func(8)
build_exception_x_func(9)
build_exception_x_func(10)
build_exception_x_func(11)
build_exception_x_func(12)
build_exception_x_func(13)
build_exception_x_func(14)
build_exception_x_func(15)
build_exception_x_func(16)
build_exception_x_func(17)
build_exception_x_func(18)
build_exception_x_func(19)
build_exception_x_func(20)
build_exception_x_func(21)
build_exception_x_func(22)
build_exception_x_func(23)
build_exception_x_func(24)
build_exception_x_func(25)
build_exception_x_func(26)
build_exception_x_func(27)
build_exception_x_func(28)
build_exception_x_func(29)
build_exception_x_func(30)
build_exception_x_func(31)
#endif

LOCALD os_void (IRQ_FUNC *exception_hook[IA_INTEL_RSVD_INT])(os_void) = { OS_NULL };

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void register_trap(os_u32 id, os_void IRQ_FUNC (*func)(os_void))
{
    cassert(IA_INTEL_RSVD_INT > id);
    exception_hook[id] = func;
}

/***************************************************************
 * description : trap.s
 * history     :
 ***************************************************************/
os_void __stdcall trap_wrap_handle(os_u32 id, os_u32 code)
{
    if (OS_NULL == exception_hook[id]) {
        init_exception_screen();
        print("task [%s] exception(%s) [%d] %d!\n", get_task_name(current_task_handle()), __FUNCTION__, id, code);
        dump_call_stack();
        dump_cpu(&code + 1);
        print("dump all task water lever\n");
        dump_task_stack_wl(get_core_id(), get_cpu_id());

        /* not necessary to check id */
        switch (exception_ventor_table[id].type) {
        case EXCEPTION_FAULT:
        case EXCEPTION_ABORT:
            lock_schedule();
            dead();
            break;
        case EXCEPTION_INT:
        case EXCEPTION_TRAP:
        case EXCEPTION_RSVD:
        default: /* EXCEPTION_FAULT | EXCEPTION_TRAP */
#ifdef _DEBUG_VERSION_
            lock_schedule();
            dead();
#endif
            break;
        }
    } else {
        exception_hook[id]();
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void init_trap(os_void)
{
    os_u32 i;

    for (i = 0; i < IA_INTEL_RSVD_INT; i++) {
        install_trap(i, exception_ventor_table[i].isr);
    }
}

