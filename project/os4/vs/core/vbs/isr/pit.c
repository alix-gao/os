/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : pit.c
 * version     : 1.0
 * description : (key) ϵͳ��ʱ��, IRQ 0, programmable interval timer.
 *               �жϷ�������(maskable interrupt)
 *               ���isr�д��������л�����, ��ô�������л�֮ǰ����eoi.
 *               �����л������������cpu�����ж�, ���жϿ������������ж�.
 *               ���±��жϵ������ڱ����µ���֮ǰ�޷������ж�. ����Ӱ��ʵʱ��.
 *               �жϷ��������в�������������(����ȡ�ź���, ������Ϣ��).
 *               �жϲ������κ�һ���߳�, �������ṩһ���첽���ֶ�.
 *               һ�����������Ϊ��ȡ��Ϣ���ߺ����������������ض�����, ���Ǻ����.
 *               ��������Ϊ�ж϶�����.
 *               �жϲ����������ж�����.
 *               ���ж�����������, �ӱ����Ͻ�, ֻҪ����֮ǰ����eoi, �������µ��Ⱥ�ָ�����̬. �����в���Ԥ����.
 *               �жϽ����ı�־����iret, ����eoi.
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
/* RTC��ʱ���ź��� */
LOCALD HEVENT pit_sem_id = 0;

/* ϵͳtick, 4�ֽڶ�����Լ�������ʹ��. */
LOCALD volatile os_u32 tick _CPU_ALIGNED_ = 0;

/* �߳�tick��¼�� */
LOCALD volatile os_u32 tick_record[CORE_NUM][CPU_NUM][OS_TASK_NUM] _CPU_ALIGNED_ = {0};

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description : ϵͳtick, ���ܲ�׼.
 * history     :
 ***************************************************************/
os_u32 system_tick(os_void)
{
    return tick;
}

/***************************************************************
 * description : ����aִ��, �ж�, cpuѹջ����eflag(ʹ������a�Ķ�ջ),IF,TF��0.
 *               �����ж������ĵ�����a�Ķ�ջ��.
 *               (��Ϊapicֻ��һ��, ���л�ǰ�ָ�8259a)
 *               ִ��handover_task()ʱֹͣ�жϷ�����.
 *               ��������a�������ĵ�����a��tss.
 *               ������b��tss�лָ�����b���л�������(����eflag), �����ж�.
 *               ִ������b
 *               ......
 *               �ж�, �����ж������ĵ�����b�Ķ�ջ��.
 *               ���ȿ��ָܻ�����a, ��������b���л������ĵ�����b��tss.
 *               ������b��tss�лָ�����b���л�������, ����a��eflag�����ж�.
 *               handover_task()�������ִ��.
 *               ������a�Ķ�ջ�лָ��ж�������.
 *               iret, �ָ�eflag, �ж�����.
 *               �����жϹ��̽���.
 *               IRQ_FUNCPTR
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC handle_system_timer_int_1(os_u32 irq)
{
    os_u32 core_id;
    os_u32 cpu_id;

    /* �����л�֮ǰҪ�����жϿ�����.
       ���isr�������л�ǰû�з���eoi, ��ǰ�����л�����֮ǰ���ܲ����µ��ж�.
       ���ʱ��ֻ��ͨ��ͬ�����������ص�ǰ����(ʵ��ͨ��).
       �����ǰ��������, ��ô�������������pend״̬����, �����û���������񼤻ǰ����, �����жϲ�����
       һ�ǿ��ܳ���һ��ʱ�����ж�����Ӧ.
       ���ǿ��ܳ�����������. */
    // master_8259a_eoi();

    /* �������������Ϣ����, �������station�Ƿ��мĴ�����Ϣ, ͬʱ�޸�����״̬ */
    /* ÿ��ʱ���������оƬ��Ҫ����Ƿ��п��ƻ������� */

    /* ����delay���� */

    /* tick�Ӽ� */
    tick++;

    /* ����ͳ�������cpuռ���� */
    core_id = get_core_id();
    cpu_id = get_cpu_id();
    tick_record[core_id][cpu_id][get_task_id(core_id, cpu_id)]++;
}

/***************************************************************
 * description : ʱ���ж��°벿����, ʱ��Ƭ����, ��˱������µ���
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC handle_system_timer_int_2(os_u32 irq)
{
    /* delay������ */
    delay_function();

    /* �ж��°벿�� */
    if (OS_SUCC != notify_event(pit_sem_id, __LINE__)) {
        /* �ͷ��ź���û�в������� */
        schedule();
    }
}

/***************************************************************
 * description : pit�ж��°벿��
 *               TASK_FUNC_PTR
 * history     :
 ***************************************************************/
os_ret OS_CALLBACK pit_thread(os_u32 arg1, os_u32 arg2, os_u32 arg3, os_u32 arg4, os_u32 arg5, os_u32 arg6, os_u32 arg7)
{
    while (1) {
        /* ȡ�ź��� */
        wait_event(pit_sem_id, 0);

        dog_event();

        /* tick��ʱ�� */
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

    /* ��ʼ����ʱ�� */
    init_timer();

    /* ��ʼ��tick��¼ */
    init_tick_reord();

    /* ��ʼ��pit�ź��� */
    pit_sem_id = create_event_handle(EVENT_INVALID, "pit", __LINE__);
    cassert(OS_NULL != pit_sem_id);

    /* ����ϵͳ��ʱ������ */
    handle = create_task("pit", pit_thread, PIT_TASK_PRIO, 0, 0, 0, 0, 0, 0, 0);
    cassert(OS_NULL != handle);

    /* init hardware */
    os_set_timer_channel0();

    /* ��ʼ��ϵͳʱ�� */
    install_int(SYS_TIMER_INT_VECTOR, handle_system_timer_int_1, handle_system_timer_int_2);
}

