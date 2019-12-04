/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : pic.c
 * version     : 1.0
 * description : (key) 可编程中断控制器
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include "pic.h"

/***************************************************************
 global variable declare
 ***************************************************************/

/* 中断控制器起始向量 */
LOCALD const os_u8 APIC_VECTOR = 0x20;

/* cpu外部中断个数 */
#define APIC_INT_COUNT 0x10

/* int.s: _system_timer_int */
/* int 0x20 */
os_void IRQ_FUNC system_timer_int(os_void);
/* int 0x21 */
os_void IRQ_FUNC keyboard_int(os_void);
/* int 0x22 */
os_void IRQ_FUNC irq2_int(os_void);
/* int 0x23 */
os_void IRQ_FUNC irq3_int(os_void);
/* int 0x24 */
os_void IRQ_FUNC irq4_int(os_void);
/* int 0x25 */
os_void IRQ_FUNC irq5_int(os_void);
/* int 0x26 */
os_void IRQ_FUNC irq6_int(os_void);
/* int 0x27 */
os_void IRQ_FUNC irq7_int(os_void);
/* int 0x28 */
os_void IRQ_FUNC real_time_clock_int(os_void);
/* int 0x29 */
os_void IRQ_FUNC irq9_int(os_void);
/* int 0x2a */
os_void IRQ_FUNC irq10_int(os_void);
/* int 0x2b */
os_void IRQ_FUNC irq11_int(os_void);
/* int 0x2c */
os_void IRQ_FUNC ps2_mouse(os_void);
/* int 0x2d */
os_void IRQ_FUNC irq13_int(os_void);
/* int 0x2e */
os_void IRQ_FUNC harddisk_int(os_void);
/* int 0x2f */
os_void IRQ_FUNC irq15_int(os_void);

/* note: no NULL is filled! */
LOCALD const VOID_FUNCPTR int_ventor_table[APIC_INT_COUNT] = {
    /* int 0x20 */
    system_timer_int,
    /* int 0x21 */
    keyboard_int,
    /* int 0x22 */
    irq2_int,
    /* int 0x23 */
    irq3_int,
    /* int 0x24 */
    irq4_int,
    /* int 0x25 */
    irq5_int,
    /* int 0x26 */
    irq6_int,
    /* int 0x27 */
    irq7_int,
    /* int 0x28 */
    real_time_clock_int,
    /* int 0x29 */
    irq9_int,
    /* int 0x2a */
    irq10_int,
    /* int 0x2b */
    irq11_int,
    /* int 0x2c */
    ps2_mouse,
    /* int 0x2d */
    irq13_int,
    /* int 0x2e */
    harddisk_int,
    /* int 0x2f */
    irq15_int
};

/* register VOID_FUNCPTR IRQ_FUNC */
LOCALD volatile os_void (*interrupt_function_table[APIC_INT_COUNT][2])(os_u32 irq) _CPU_ALIGNED_ = { 0 };

/* 8259a中断屏蔽掩码 */
LOCALD volatile os_u16 pic_mask _CPU_ALIGNED_ = 0xffffU;

#define mask_byte(x,y) (((os_u8 *) &(y))[x])
#define MASK_21 (mask_byte(0, pic_mask))
#define MASK_A1 (mask_byte(1, pic_mask))

/* 中断控制器恢复操作 */
#define master_8259a_eoi() do { outb(0x20, 0x20); } while (0)

#define slave_8259a_eoi() do { outb(0xa0, 0x20); outb(0x20, 0x20); } while (0)

LOCALD spinlock_t pic_lock;

#define MASTER_SPURIOUS_INT_VECTOR 7
#define SLAVE_SPURIOUS_INT_VECTOR 15

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void OS_API open_pic_int(os_u32 vector)
{
    os_u16 mask = 1;
    os_u8 reg = 0;

    spin_lock(&pic_lock);

    if ((7 < vector) && (16 > vector)) {
        /* master */
        inb_p(0x21, reg);
        reg = reg & 0xfb;
        outb_p(0x21, reg);
        MASK_21 = reg;

        /* slave */
        inb_p(0xa1, reg);
        reg = reg & (~(mask << (vector-8)));
        outb_p(0xa1, reg);
        MASK_A1 = reg;
    } else if (8 > vector) {
        /* master */
        inb_p(0x21, reg);
        reg = reg & (~(mask << vector));
        outb_p(0x21, reg);
        MASK_21 = reg;
    } else {
        cassert(OS_FALSE);
    }
    wmb();

    spin_unlock(&pic_lock);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void OS_API close_pic_int(os_u32 vector)
{
    os_u16 mask = 1;
    os_u8 reg = 0;

    spin_lock(&pic_lock);

    if ((7 < vector) && (16 > vector)) {
        /* slave */
        inb_p(0xa1, reg);
        reg = reg | (mask << (vector-8));
        outb_p(0xa1, reg);
        MASK_A1 = reg;
    } else if (8 > vector) {
        /* master */
        inb_p(0x21, reg);
        reg = reg | (mask << (vector-8));
        outb_p(0x21, reg);
        MASK_21 = reg;
    } else {
        cassert(OS_FALSE);
    }
    wmb();

    spin_unlock(&pic_lock);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_u16 get_current_pic_mask(os_void)
{
    os_u16 old = 0;
    os_u8 reg = 0;

    inb_p(0x21, reg);

    old = reg;

    inb_p(0xa1, reg);

    old = old << 8;
    old |= reg;

    return old;
}

/***************************************************************
 * description : 关闭所有中断
 * history     :
 ***************************************************************/
os_u16 close_pic(os_u16 mask)
{
    os_u16 old;

    old = get_current_pic_mask();

    /* close 8259a int */
    outb(0xa1, (os_u8) mask);
    outb(0x21, (os_u8)(mask >> 8));

    return old;
}

/***************************************************************
 * description : 恢复中断
 * history     :
 ***************************************************************/
os_u32 open_pic(os_u16 mask)
{
    os_u8 reg;

    reg = get_current_pic_mask();

    outb(0xa1, (os_u8) mask);
    outb(0x21, (os_u8)(mask >> 8));

    return reg;
}

/***************************************************************
 * description : 获取apic的起始向量号
 * history     :
 ***************************************************************/
os_u32 get_pic_start_vector(os_void)
{
    return APIC_VECTOR;
}

/***************************************************************
 * description : VOID_FUNCPTR
 *               中断包装函数
 *               注意eoi要在中断处理函数后面
 * history     :
 ***************************************************************/
os_void IRQ_FUNC wrap_int(os_u32 vector)
{
    os_u8 reg;
    os_u16 irq_mask;

    irq_mask = 1 << vector;

    /* spurious irq */
    if (pic_mask & irq_mask) {
        return;
    }
    switch (vector) {
    default:
        break;
    case MASTER_SPURIOUS_INT_VECTOR:
        /* read IRR */
        outb(0x20, 0x0b);
        inb(0x20, reg);
        outb(0x20, 0x0a);
        if (0 == (reg & irq_mask)) {
            return; /* spurious irq */
        }
        break;
    case SLAVE_SPURIOUS_INT_VECTOR:
        /* read IRR */
        outb(0xa0, 0x0b);
        inb(0xa0, reg);
        outb(0xa0, 0x0a);
        if (0 == (reg & (irq_mask >> 8))) {
            slave_8259a_eoi();
            return; /* spurious irq */
        }
        break;
    }

    /* close 8259a interrupt */
    close_pic_int(vector);

    /* send eoi */
    if (8 > vector) {
        master_8259a_eoi();
    } else if (16 > vector) {
        slave_8259a_eoi();
    } else {
        // imposible, no assert
    }

    /* first interrupt function */
    if (interrupt_function_table[vector][0]) {
        lock_schedule();
        interrupt_function_table[vector][0](vector);
        unlock_schedule();
    }

    /* close interrupt.
       when leave this function, iret is executed, interrupt is opened. */
    cli();

    /* open 8259a interrupt */
    open_pic_int(vector);

    /* second interrupt function */
    if (interrupt_function_table[vector][1]) {
        interrupt_function_table[vector][1](vector);
    }
}

/***************************************************************
 * description : func_1 上半部只做同步操作, 硬件处理
 *               func_2 上半部可以异步操作, 软件处理
 * history     :
 ***************************************************************/
os_ret register_interrupt_func(os_u32 vector_no, IRQ_FUNCPTR IRQ_FUNC func_1, IRQ_FUNCPTR IRQ_FUNC func_2)
{
    if (APIC_INT_COUNT > vector_no) {
        /* 写入数据中而不是代码段内存中(flush d-cache and i-cache) */
        interrupt_function_table[vector_no][0] = func_1; /* 上半部只做同步操作、硬件处理 */
        interrupt_function_table[vector_no][1] = func_2; /* 上半部可以异步操作、软件处理 */
        wmb();
        return OS_SUCC;
    }
    return OS_FAIL;
}

/***************************************************************
 * description : 根据中断向量号获取中断服务例程
 * history     :
 ***************************************************************/
VOID_FUNCPTR get_ivt_func(os_u32 vector_no)
{
    if (APIC_INT_COUNT > vector_no) {
        return int_ventor_table[vector_no];
    }
    cassert(OS_FALSE);
    return OS_NULL;
}

/***************************************************************
 * description : IRQ_FUNCPTR
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC spurious_int(os_u32 irq)
{
    cassert(OS_FALSE);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void dump_apic(os_void)
{
    print("pic mask: %x\n", pic_mask);
}
LOCALD os_u8 apic_debug_name[] = "apic";
LOCALD struct dump_info apic_debug = {
    apic_debug_name,
    dump_apic
};

/***************************************************************
 * description : 初始化8259a
 * history     :
 ***************************************************************/
os_void init_pic(os_void)
{
    /* icw1, edge triggered */
    outb_p(0x20, 0x11);
    outb_p(0xa0, 0x11);
    /* icw2, vector */
    outb_p(0x21, APIC_VECTOR);
    outb_p(0xa1, APIC_VECTOR + 8);
    /* icw3 */
    outb_p(0x21, 0x04);
    outb_p(0xa1, 0x02);
    /* icw4, 普通全嵌套, 非缓冲, 非自动结束中断方式 */
    outb_p(0x21, 0x01);
    outb_p(0xa1, 0x01);
    /* ocw1, mask */
    outb_p(0x21, 0xff);
    outb_p(0xa1, 0xff);
    pic_mask = 0xffff;
    wmb();

    init_spinlock(&pic_lock);

    /* important: install spurious interrupt */
    install_int(MASTER_SPURIOUS_INT_VECTOR, spurious_int, OS_NULL);
    install_int(SLAVE_SPURIOUS_INT_VECTOR, spurious_int, OS_NULL);

    if (OS_SUCC != register_dump(&apic_debug)) {
        flog("apic register dump fail\n");
    }
}

