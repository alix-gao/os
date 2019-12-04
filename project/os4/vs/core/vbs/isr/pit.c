/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : pit.c
 * version     : 1.0
 * description : (key) 系统定时器, IRQ 0, programmable interval timer.
 *               中断服务例程(maskable interrupt)
 *               如果isr中存在任务切换操作, 那么在任务切换之前发送eoi.
 *               否则切换到其它任务后cpu允许中断, 但中断控制器不允许中断.
 *               导致被中断的任务在被重新调度之前无法接收中断. 严重影响实时性.
 *               中断服务例程中不能有阻塞操作(例如取信号量, 接收消息等).
 *               中断不属于任何一个线程, 仅仅是提供一种异步的手段.
 *               一个任务可以因为获取消息或者和其它任务竞争等因素而阻塞, 这是合理的.
 *               但不能因为中断而阻塞.
 *               中断不能阻塞被中断任务.
 *               在中断中阻塞任务, 从本质上讲, 只要在这之前发送eoi, 可能重新调度后恢复运行态. 但具有不可预测性.
 *               中断结束的标志不是iret, 而是eoi.
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include <vts.h>
#include <vds.h>
#include "pit.h"

/***************************************************************
 global variable declare
 ***************************************************************/
/* RTC定时器信号量 */
LOCALD HEVENT pit_sem_id = 0;

/* 系统tick, 4字节对齐可以减少锁的使用. */
LOCALD volatile os_u32 tick _CPU_ALIGNED_ = 0;

/* 线程tick记录表 */
LOCALD volatile os_u32 tick_record[CORE_NUM][CPU_NUM][OS_TASK_NUM] _CPU_ALIGNED_ = {0};

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description : 系统tick, 可能不准.
 * history     :
 ***************************************************************/
os_u32 system_tick(os_void)
{
    return tick;
}

/***************************************************************
 * description : 任务a执行, 中断, cpu压栈保存eflag(使用任务a的堆栈),IF,TF置0.
 *               保存中断上下文到任务a的堆栈中.
 *               (因为apic只有一个, 在切换前恢复8259a)
 *               执行handover_task()时停止中断服务函数.
 *               保存任务a的上下文到任务a的tss.
 *               从任务b的tss中恢复任务b的切换上下文(包括eflag), 允许中断.
 *               执行任务b
 *               ......
 *               中断, 保存中断上下文到任务b的堆栈中.
 *               调度可能恢复任务a, 保存任务b的切换上下文到任务b的tss.
 *               从任务b的tss中恢复任务b的切换上下文, 任务a的eflag禁用中断.
 *               handover_task()后面继续执行.
 *               从任务a的堆栈中恢复中断上下文.
 *               iret, 恢复eflag, 中断允许.
 *               整个中断过程结束.
 *               IRQ_FUNCPTR
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC handle_system_timer_int_1(os_u32 irq)
{
    os_u32 core_id;
    os_u32 cpu_id;

    /* 任务切换之前要重置中断控制器.
       如果isr中任务切换前没有发送eoi, 当前任务切换回来之前不能产生新的中断.
       这段时间只能通过同步调度器返回当前任务(实验通过).
       如果当前任务阻塞, 那么调度器不会调度pend状态任务, 如果又没有其它任务激活当前任务, 整个中断不可用
       一是可能出现一段时间内中断无响应.
       二是可能出现死机现象. */
    // master_8259a_eoi();

    /* 检查各个任务的消息队列, 该任务的station是否有寄存器消息, 同时修改任务状态 */
    /* 每个时钟脉冲各个芯片都要检测是否有控制或者数据 */

    /* 处理delay任务 */

    /* tick加加 */
    tick++;

    /* 粗略统计任务的cpu占有率 */
    core_id = get_core_id();
    cpu_id = get_cpu_id();
    tick_record[core_id][cpu_id][get_task_id(core_id, cpu_id)]++;
}

/***************************************************************
 * description : 时钟中断下半部处理, 时间片用完, 因此必须重新调度
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC handle_system_timer_int_2(os_u32 irq)
{
    /* delay任务唤醒 */
    delay_function();

    /* 中断下半部分 */
    if (OS_SUCC != notify_event(pit_sem_id, __LINE__)) {
        /* 释放信号量没有产生调度 */
        schedule();
    }
}

/***************************************************************
 * description : pit中断下半部分
 *               TASK_FUNC_PTR
 * history     :
 ***************************************************************/
os_ret OS_CALLBACK pit_thread(os_u32 arg1, os_u32 arg2, os_u32 arg3, os_u32 arg4, os_u32 arg5, os_u32 arg6, os_u32 arg7)
{
    while (1) {
        /* 取信号量 */
        wait_event(pit_sem_id, 0);

        dog_event();

        /* tick定时器 */
        timer_function();
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void init_tick_reord(os_void)
{
    os_u32 i, j, k;

    for (i=0; i<CORE_NUM; i++)
    for (j=0; j<CPU_NUM; j++)
    for (k=0; k<OS_TASK_NUM; k++) {
        tick_record[i][j][k] = 0;
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_u32 get_thread_tick(HTASK handle)
{
    struct task_handle *htask;

    if (OS_NULL != handle) {
        htask = handle;
        return tick_record[htask->core_id][htask->cpu_id][htask->task_id];
    }
    return 0;
}

#define CMD_COUNTER0 (0 << 6)
#define CMD_COUNTER1 (1 << 6)
#define CMD_COUNTER2 (2 << 6)
#define CMD_LATCH_LOCK (0 << 4)
#define CMD_LATCH_LSB (1 << 4)
#define CMD_LATCH_MSB (2 << 4)
#define CMD_LATCH (3 << 4)
#define CMD_MODE0 (0 << 1)
#define CMD_MODE1 (1 << 1)
#define CMD_MODE2 (2 << 1)
#define CMD_MODE3 (3 << 1)
#define CMD_MODE4 (4 << 1)
#define CMD_MODE5 (5 << 1)
#define CMD_BIN (0 << 0)
#define CMD_BCD (1 << 0)

#define I8253_MODE CMD_MODE3

/***************************************************************
 * description : intel 8253/8254, channel 0. mode 3.
 * history     :
 ***************************************************************/
os_void bios_rst_timer_channel0(os_void)
{
    /* 1 tick = 55ms */
    outb_p(I8253_CMD_ADDR, CMD_COUNTER0 | CMD_LATCH | I8253_MODE | CMD_BIN);

    /* latch max */
    outb_p(I8253_COUNTER0_ADDR, 0x00);

    /* latch max */
    outb(I8253_COUNTER0_ADDR, 0x00);
}

#define I8253_THRESHOLD (I8253_FREQ/OS_HZ)

/***************************************************************
 * description : input frequency: 1.193180MHZ, set tick = 10ms
 * history     :
 ***************************************************************/
os_void os_set_timer_channel0(os_void)
{
    /* control word */
    outb_p(I8253_CMD_ADDR, CMD_COUNTER0 | CMD_LATCH | I8253_MODE | CMD_BIN);

    /* latch (1193180/100)&0xff = 9cH */
    outb_p(I8253_COUNTER0_ADDR, (os_u8) I8253_THRESHOLD);

    /* latch (1193180/100)>>8 = 2eH */
    outb(I8253_COUNTER0_ADDR, (os_u8)(I8253_THRESHOLD >> 8));
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_u16 read_8253_channel0(os_void)
{
    os_u16 count;
    os_u8 reg;
    lock_t eflag;

    lock_int(eflag);
    /* do not lock interrupt, because timer 8253 should generate interrupt. */
    lock_schedule();
    /* in order to read the contents of any counter without effecting or disturbing the countering operation,
       the 8253 has special internal logica that can be accessed using simple wr command to the mode register.
       basically, when the programmer wishes to read the contents of a selected counter on the fly,
       he loads the mode register with a special code which latches the present count value into a storage register so that its contents contain an accurate, stable quantity. */
    outb(I8253_CMD_ADDR, CMD_COUNTER0 | CMD_LATCH_LOCK);
    /* first io read contains the least significant byte,
       second io read contains the most significant byte */
    inb(I8253_COUNTER0_ADDR, reg);
    count = reg;
    inb(I8253_COUNTER0_ADDR, reg);
    unlock_schedule();
    unlock_int(eflag);
    count |= (reg << 8);
    return count;
}

#if (I8253_MODE == CMD_MODE3)
/* mode3, is implemented as follows:
   the initial count is loaded on one clk pulse and then is decremented by TWO on succeeding clk pulse.
   when the count expires out changes value and the counter is reloaded with the initial count. */
#define i8253_ms(m) (((m) * 2 * I8253_FREQ)/1000)
#define i8253_us(u) (((u) * 2 * I8253_FREQ)/1000/1000)
#else
#error "undefined macro!"
#endif

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void delay_ms(os_u32 ms)
{
    os_u16 value;
    os_u16 count;
    os_u16 temp;

    /* mode3, I8253_THRESHOLD~0 is 5ms */
    if ((1000/OS_HZ/2) > ms) {
        cassert(UINT16_MAX >= (1000/OS_HZ/2));
        count = read_8253_channel0();

        temp = i8253_ms(ms) + 1;
        if (temp > count) {
            temp = I8253_THRESHOLD - (temp - count);
            do {
                value = read_8253_channel0();
            } while ((count >= value) || (temp < value));
        } else {
            temp = count - temp;
            while (temp < read_8253_channel0()) {
            }
        }
        return;
    }

    temp = ms / (1000/OS_HZ);
    if (0 == temp) { /* 0 is forever for delay task */
        temp = 1;
    }
    delay_task(temp, __LINE__);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void _delay_us(os_u8 us)
{
    os_u16 value;
    os_u16 count;
    os_u16 temp;

    count = read_8253_channel0();

    temp = i8253_us(us) + 1;
    if (temp > count) {
        temp = I8253_THRESHOLD - (temp - count);
        do {
            value = read_8253_channel0();
        } while ((count >= value) || (temp < value));
    } else {
        temp = count - temp;
        while (temp < read_8253_channel0()) {
        }
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void delay_us(os_u32 us)
{
    if (1000 < us) {
        delay_ms(us / 1000);
        us -= 1000;
    }
    if (UINT8_MAX < us) {
        delay_ms(1);
    } else {
        _delay_us(us);
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void init_pit_int(os_void)
{
    HTASK handle;

    /* 初始化定时器 */
    init_timer();

    /* 初始化tick记录 */
    init_tick_reord();

    /* 初始化pit信号量 */
    pit_sem_id = create_event_handle(EVENT_INVALID, "pit", __LINE__);
    cassert(OS_NULL != pit_sem_id);

    /* 创建系统定时器任务 */
    handle = create_task("pit", pit_thread, PIT_TASK_PRIO, 0, 0, 0, 0, 0, 0, 0);
    cassert(OS_NULL != handle);

    /* init hardware */
    os_set_timer_channel0();

    /* 初始化系统时钟 */
    install_int(SYS_TIMER_INT_VECTOR, handle_system_timer_int_1, handle_system_timer_int_2);
}

