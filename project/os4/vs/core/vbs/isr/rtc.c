/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : rtc.c
 * version     : 1.0
 * description : 实时时钟定时器, real time clock. IRQ 8.
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include <message.h>
#include <vds.h>
#include "rtc.h"

/***************************************************************
 global variable declare
 ***************************************************************/
enum {
    RTC_IDLE,
    RTC_BUSY,
    RTC_TIMEOUT
};
struct rtc_handle {
    struct timer_entity entity;
#define RTC_CHECK 0xcccccccc
    os_u32 check;
    os_u32 status;
    struct list_node node;
};

LOCALD struct rtc_handle rtc_table[TIMER_MAX_NUM];

LOCALD struct list_node idle_rtc;
LOCALD struct list_node busy_rtc;

LOCALD spinlock_t rtc_lock; /* idle or busy list */

/***************************************************************
 function declare
 ***************************************************************/

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void dump_rtc(os_void)
{
    struct rtc_handle *timer;
    struct list_node *list, *_save;
    os_u32 i;
    lock_t eflag;

    lock_int(eflag);

    print("rtc_table [%x, %x] %x %x\n", rtc_table, rtc_table + TIMER_MAX_NUM - 1, &idle_rtc, &busy_rtc);

    i = 0;
    loop_list(list, &idle_rtc) {
        timer = list_addr(list, struct rtc_handle, node);
        print("(%x %d,%d %d)", timer, timer->entity.elapse, timer->entity.current_time, timer->entity.mode);
        i++;
    } print("\n");
    print("idle timer: %d\n", i);

    i = 0;
    loop_list(list, &busy_rtc) {
        timer = list_addr(list, struct rtc_handle, node);
        i++;
        print("(%x %d,%d %d)", timer, timer->entity.elapse, timer->entity.current_time, timer->entity.mode);
    } print("\n");
    print("busy timer: %d\n", i);

    loop_del_list(list, _save, &busy_rtc) {
        timer = list_addr(list, struct rtc_handle, node);
        print("(%x %d,%d %d)", timer, timer->entity.elapse, timer->entity.current_time, timer->entity.mode);
    } print("\n");

    unlock_int(eflag);
}

LOCALD os_u8 rtc_debug_name[] = { "rtc" };
LOCALD struct dump_info rtc_debug = {
    rtc_debug_name,
    dump_rtc
};

/***************************************************************
 * description : 注册设置定时器
 *               回调方式
 *               允许非窗口(非消息实体)创建, 例如线程.
 *               定时器模式为TIMER_MODE_NOT_LOOP, 将自动删除定时器.
 * history     :
 ***************************************************************/
HTIMER OS_API set_rtc_callback(os_u32 event_id, os_u32 tick, IN TIMER_FUNC_PTR timer_func, enum timer_mode mode)
{
    struct rtc_handle *timer;
    struct timer_entity *entity;

    timer = OS_NULL;

    if ((OS_NULL != timer_func) && (TIMER_MODE_BUTT > mode)) {
        spin_lock(&rtc_lock);

        if (!list_empty(&idle_rtc)) {
            timer = list_addr(idle_rtc.next, struct rtc_handle, node);
            cassert(RTC_CHECK == timer->check);
            del_list(&timer->node);
            /* this could be called in timer's callback, so put new timer in head */
            add_list_head(&busy_rtc, &timer->node);
            cassert_word(RTC_IDLE == timer->status, "timer status %d\n", timer->status);
            timer->status = RTC_BUSY;

            entity = &timer->entity;
            entity->mode = mode;
            entity->current_time = 0;
            entity->elapse = tick;
            entity->event_id = event_id;
            entity->hwnd = OS_NULL;
            entity->timer_func = timer_func;
        }

        spin_unlock(&rtc_lock);
    }

    return timer;
}

/***************************************************************
 * description : 设置定时器
 *               消息方式
 *               不允许非窗口(非消息实体)创建
 * history     :
 ***************************************************************/
HTIMER OS_API set_rtc_msg(IN HWINDOW hwnd, os_u32 event_id, os_u32 tick, enum timer_mode mode)
{
    struct rtc_handle *timer;
    struct timer_entity *entity;

    timer = OS_NULL;

    if ((OS_NULL != hwnd) && (TIMER_MODE_BUTT > mode)) {
        spin_lock(&rtc_lock);

        if (!list_empty(&idle_rtc)) {
            timer = list_addr(idle_rtc.next, struct rtc_handle, node);
            cassert(RTC_CHECK == timer->check);
            del_list(&timer->node);
            /* this could be called in timer's callback, so put new timer in head */
            add_list_head(&busy_rtc, &timer->node);
            cassert_word(RTC_IDLE == timer->status, "timer status %d\n", timer->status);
            timer->status = RTC_BUSY;

            entity = &timer->entity;
            entity->mode = mode;
            entity->current_time = 0;
            entity->elapse = tick;
            entity->event_id = event_id;
            entity->hwnd = hwnd;
            entity->timer_func = OS_NULL;
        }

        spin_unlock(&rtc_lock);
    }

    return (HTIMER) timer;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_u32 OS_API rtc_left(IN HTIMER handle)
{
    struct rtc_handle *timer;

    timer = (struct rtc_handle *) handle;
    if (timer) {
        cassert(RTC_CHECK == timer->check);
        return timer->entity.elapse - timer->entity.current_time;
    }
    return 0;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API modify_rtc(IN HTIMER handle, os_u32 tick)
{
    struct rtc_handle *timer;

    /* 入参检查 */
    if (OS_NULL != handle) {
        timer = (struct rtc_handle *) handle;
        cassert(RTC_CHECK == timer->check);

        spin_lock(&rtc_lock);
        timer->entity.elapse = tick;
        spin_unlock(&rtc_lock);

        return OS_SUCC;
    }
    return OS_FAIL;
}

/***************************************************************
 * description : 重置定时器
 * history     :
 ***************************************************************/
HTIMER OS_API reset_rtc(IN HTIMER handle)
{
    struct rtc_handle *timer;

    /* 入参检查 */
    if (OS_NULL != handle) {
        timer = (struct rtc_handle *) handle;
        cassert(RTC_CHECK == timer->check);

        spin_lock(&rtc_lock);
        del_list(&timer->node);
        add_list_tail(&busy_rtc, &timer->node);
        cassert(RTC_IDLE != timer->status);
        timer->status = RTC_BUSY;
        timer->entity.current_time = 0;
        spin_unlock(&rtc_lock);

        return handle;
    }

    /* 入参检查失败 */
    return OS_NULL;
}

/***************************************************************
 * description : 取消定时器
 * history     :
 ***************************************************************/
HTIMER OS_API kill_rtc(IN HTIMER handle)
{
    struct rtc_handle *timer;

    if (OS_NULL != handle) {
        timer = (struct rtc_handle *) handle;

        cassert(RTC_CHECK == timer->check);
        spin_lock(&rtc_lock);
        //spin_lock(&timer->lock);
        del_list(&timer->node);
        add_list_tail(&idle_rtc, &timer->node);
        timer->status = RTC_IDLE;
        //spin_unlock(&timer->lock);
        spin_unlock(&rtc_lock);
    }
    return handle;
}

/***************************************************************
 * description : 定时器超时处理
 * history     :
 ***************************************************************/
LOCALC os_void timeout_msg(HWINDOW hwnd, os_u32 event_id)
{
    struct timer_msg *timer_msg;

    if (OS_NULL != hwnd) {
        /* 消息方式, 设置定时器时检查 */
        timer_msg = (struct timer_msg *) alloc_timer_msg();
        if (OS_NULL == timer_msg) {
            /* 消息内存分配失败, 不发送 */
            return;
        }

        /* 发送者为isr */
        timer_msg->msg_name = OS_MSG_TIMER;
        timer_msg->msg_len = sizeof(struct message);
        timer_msg->event_id = event_id;

        post_msg(hwnd, timer_msg);
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void rtc_function(os_void)
{
    struct list_node *list, *_save;
    struct rtc_handle *timer;
    HWINDOW hwnd;
    TIMER_FUNC_PTR timer_func;
    os_u32 event_id;

    spin_lock(&rtc_lock);
    loop_del_list(list, _save, &busy_rtc) {
        timer = list_addr(list, struct rtc_handle, node);
        cassert(RTC_CHECK == timer->check);
        if (RTC_IDLE == timer->status) {
            /* bugfix: _save maybe delete in user's call back. */
            break;
        }

        if (RTC_TIMEOUT == timer->status) {
            continue;
        }

        /* 时间增加1ms */
        timer->entity.current_time++;
        if (timer->entity.current_time >= timer->entity.elapse) {
            switch (timer->entity.mode) {
            case TIMER_MODE_LOOP:
                timer->entity.current_time = 0;
                break;
            case TIMER_MODE_NOT_LOOP:
                /* bugfix: keep the entity and handle consistence */
#if 1
                timer->status = RTC_TIMEOUT;
#else
                /* bugfix: delete timer again in timeout callback */
                del_init_list(&timer->node);
                add_list_tail(&idle_rtc, &timer->node);
#endif
                break;
            case TIMER_MODE_BUTT:
            default:
                cassert(OS_FALSE);
                break;
            }
            timer_func = timer->entity.timer_func;
            event_id = timer->entity.event_id;
            hwnd = timer->entity.hwnd;
            spin_unlock(&rtc_lock);
            if (timer_func) {
                timer_func(event_id);
            }
            if (hwnd) {
                timeout_msg(hwnd, event_id);
            }
            spin_lock(&rtc_lock);
        }
    }
    spin_unlock(&rtc_lock);
}

/***************************************************************
 * description : 设置定时器
 *               消息方式
 *               允许线程实体创建
 * history     :
 ***************************************************************/
HTIMER OS_API set_rtc_thread_msg(os_u32 task_id, os_u32 event_id, os_u32 ms, enum timer_mode mode)
{
    cassert(OS_FALSE);
    return OS_NULL;
}


/***************************************************************
 * description : 1ms实时定时器
 *               用信号量将定时器中断分为两部分.
 *               IRQ_FUNCPTR
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC handle_rtc_int_1(os_u32 irq)
{
    os_u8 reg = 0;

    /* reg c. clear int flag. */
    rtc_read_b(RTC_REG_C, reg);
}

/***************************************************************
 * description : 1ms实时定时器
 *               用信号量将定时器中断分为两部分.
 *               IRQ_FUNCPTR
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC handle_rtc_int_2(os_u32 irq)
{
    /* rtc回调函数 */
    rtc_function();
}

/***************************************************************
 * description : 初始化实时时钟, 1ms中断一次.
 * history     :
 ***************************************************************/
LOCALC os_void init_rtc_chipset(os_void)
{
    /* init chip reg */
    os_u8 reg = 0;

    /* reg b. */
    //we set bit 7-0 = 0.
    rtc_read_b(RTC_REG_B, reg);
    /* reg is 0x2 */

    /* close all int */
    reg = 0xc2;
    rtc_write_b(RTC_REG_B, reg);

    /* we can reset time, day, month, year here. */

    /* reg a, we do not modify. */
    rtc_read_b(RTC_REG_A, reg);

    //divider-control bits = 010.
    //rate selection bits = 0110.
    reg = 0xa6;
    /* uip is read only. */
    rtc_write_b(RTC_REG_A, reg);

    /* reg c. clear int flag. */
    rtc_read_b(RTC_REG_C, reg);

    /* reg d. clear valid ram and time, start work. */
    rtc_read_b(RTC_REG_D, reg);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void init_rtc_int(os_void)
{
    os_u32 i;

    init_spinlock(&rtc_lock);

    init_list_head(&idle_rtc);
    init_list_head(&busy_rtc);

    for (i = 0; i < TIMER_MAX_NUM; i++) {
        rtc_table[i].entity.mode = TIMER_MODE_BUTT;
        rtc_table[i].entity.current_time = 0;
        rtc_table[i].entity.elapse = 0;
        rtc_table[i].entity.event_id = 0;
        rtc_table[i].entity.hwnd = OS_NULL;
        rtc_table[i].entity.timer_func = OS_NULL;
        rtc_table[i].check = RTC_CHECK;
        rtc_table[i].status = RTC_IDLE;
        add_list_tail(&idle_rtc, &rtc_table[i].node);
    }

    /* 初始化实时时钟中断 */
    install_int(RTC_INT_VECTOR, handle_rtc_int_1, handle_rtc_int_2);

    /* 初始化实时时钟, 设置时钟精度为1ms */
    init_rtc_chipset();

    if (OS_SUCC != register_dump(&rtc_debug)) {
        flog("rtc register dump fail\n");
    }
}

