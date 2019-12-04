/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : timer.c
 * version     : 1.0
 * description : 系统定时器, 精度为10ms定时器
 *               采用分时调度, pit中断线程化的实现没有意义.
 *               pit中断线程化必须采用高优先级任务抢占式调度.
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <vds.h>
#include <core.h>
#include <message.h>
#include "timer.h"

/***************************************************************
 global variable declare
 ***************************************************************/

/***************************************************************
 * description :
 ***************************************************************/
enum {
    TIMER_IDLE,
    TIMER_BUSY,
    TIMER_TIMEOUT
};
struct timer_handle {
    struct timer_entity entity;
#define TIMER_CHECK 0xeeeeeeee
    os_u32 check;
    os_u32 status;
    struct list_node node;
};

LOCALD struct timer_handle timer_table[TIMER_MAX_NUM];

LOCALD struct list_node idle_timer;
LOCALD struct list_node busy_timer;

LOCALD spinlock_t timer_lock; /* idle or busy list */

/***************************************************************
 function declare
 ***************************************************************/

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void dump_timer(os_void)
{
    struct timer_handle *timer;
    struct list_node *list, *_save;
    os_u32 i;
    lock_t eflag;

    lock_int(eflag);

    print("timer_table [%x, %x] %x %x\n", timer_table, timer_table + TIMER_MAX_NUM - 1, &idle_timer, &busy_timer);

    i = 0;
    loop_list(list, &idle_timer) {
        timer = list_addr(list, struct timer_handle, node);
        print("(%x %d,%d %d)", timer, timer->entity.elapse, timer->entity.current_time, timer->entity.mode);
        i++;
    } print("\n");
    print("idle timer: %d\n", i);

    i = 0;
    loop_list(list, &busy_timer) {
        timer = list_addr(list, struct timer_handle, node);
        i++;
        print("(%x %d,%d %d)", timer, timer->entity.elapse, timer->entity.current_time, timer->entity.mode);
    } print("\n");
    print("busy timer: %d\n", i);

    loop_del_list(list, _save, &busy_timer) {
        timer = list_addr(list, struct timer_handle, node);
        print("(%x %d,%d %d)", timer, timer->entity.elapse, timer->entity.current_time, timer->entity.mode);
    } print("\n");

    unlock_int(eflag);
}

LOCALD os_u8 timer_debug_name[] = { "timer" };
LOCALD struct dump_info timer_debug = {
    timer_debug_name,
    dump_timer
};

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void init_timer(os_void)
{
    os_u32 i;

    init_spinlock(&timer_lock);

    init_list_head(&idle_timer);
    init_list_head(&busy_timer);

    for (i = 0; i < TIMER_MAX_NUM; i++) {
        timer_table[i].entity.mode = TIMER_MODE_BUTT;
        timer_table[i].entity.current_time = 0;
        timer_table[i].entity.elapse = 0;
        timer_table[i].entity.event_id = 0;
        timer_table[i].entity.hwnd = OS_NULL;
        timer_table[i].entity.timer_func = OS_NULL;
        timer_table[i].check = TIMER_CHECK;
        timer_table[i].status = TIMER_IDLE;
        add_list_tail(&idle_timer, &timer_table[i].node);
    }

    if (OS_SUCC != register_dump(&timer_debug)) {
        flog("timer register dump fail\n");
    }
}

/***************************************************************
 * description : 注册设置定时器
 *               回调方式
 *               允许非窗口(非消息实体)创建, 例如线程.
 *               定时器模式为TIMER_MODE_NOT_LOOP, 将自动删除定时器.
 * history     :
 ***************************************************************/
HTIMER OS_API set_timer_callback(os_u32 event_id, os_u32 tick, IN TIMER_FUNC_PTR timer_func, enum timer_mode mode)
{
    struct timer_handle *timer;
    struct timer_entity *entity;

    timer = OS_NULL;

    if ((OS_NULL != timer_func) && (TIMER_MODE_BUTT > mode)) {
        spin_lock(&timer_lock);

        if (!list_empty(&idle_timer)) {
            timer = list_addr(idle_timer.next, struct timer_handle, node);
            cassert(TIMER_CHECK == timer->check);
            del_list(&timer->node);
            /* this could be called in timer's callback, so put new timer in head */
            add_list_head(&busy_timer, &timer->node);
            cassert_word(TIMER_IDLE == timer->status, "timer status %d\n", timer->status);
            timer->status = TIMER_BUSY;

            entity = &timer->entity;
            entity->mode = mode;
            entity->current_time = 0;
            entity->elapse = tick;
            entity->event_id = event_id;
            entity->hwnd = OS_NULL;
            entity->timer_func = timer_func;
        }

        spin_unlock(&timer_lock);
    }

    return timer;
}

/***************************************************************
 * description : 设置定时器
 *               消息方式
 *               不允许非窗口(非消息实体)创建
 * history     :
 ***************************************************************/
HTIMER OS_API set_timer_msg(IN HWINDOW hwnd, os_u32 event_id, os_u32 tick, enum timer_mode mode)
{
    struct timer_handle *timer;
    struct timer_entity *entity;

    timer = OS_NULL;

    if ((OS_NULL != hwnd) && (TIMER_MODE_BUTT > mode)) {
        spin_lock(&timer_lock);

        if (!list_empty(&idle_timer)) {
            timer = list_addr(idle_timer.next, struct timer_handle, node);
            cassert(TIMER_CHECK == timer->check);
            del_list(&timer->node);
            /* this could be called in timer's callback, so put new timer in head */
            add_list_head(&busy_timer, &timer->node);
            cassert_word(TIMER_IDLE == timer->status, "timer status %d\n", timer->status);
            timer->status = TIMER_BUSY;

            entity = &timer->entity;
            entity->mode = mode;
            entity->current_time = 0;
            entity->elapse = tick;
            entity->event_id = event_id;
            entity->hwnd = hwnd;
            entity->timer_func = OS_NULL;
        }

        spin_unlock(&timer_lock);
    }

    return (HTIMER) timer;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_u32 OS_API timer_left(IN HTIMER handle)
{
    struct timer_handle *timer;

    timer = (struct timer_handle *) handle;
    if (timer) {
        cassert(TIMER_CHECK == timer->check);
        return timer->entity.elapse - timer->entity.current_time;
    }
    return 0;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API modify_timer(IN HTIMER handle, os_u32 tick)
{
    struct timer_handle *timer;

    /* 入参检查 */
    if (OS_NULL != handle) {
        timer = (struct timer_handle *) handle;
        cassert(TIMER_CHECK == timer->check);

        spin_lock(&timer_lock);
        timer->entity.elapse = tick;
        spin_unlock(&timer_lock);

        return OS_SUCC;
    }
    return OS_FAIL;
}

/***************************************************************
 * description : 重置定时器
 * history     :
 ***************************************************************/
HTIMER OS_API reset_timer(IN HTIMER handle)
{
    struct timer_handle *timer;

    /* 入参检查 */
    if (OS_NULL != handle) {
        timer = (struct timer_handle *) handle;
        cassert(TIMER_CHECK == timer->check);

        spin_lock(&timer_lock);
        if (TIMER_IDLE != timer->status) {
            del_list(&timer->node);
            add_list_tail(&busy_timer, &timer->node);
            timer->status = TIMER_BUSY;
            timer->entity.current_time = 0;
        }
        spin_unlock(&timer_lock);

        return handle;
    }

    /* 入参检查失败 */
    return OS_NULL;
}

/***************************************************************
 * description : 取消定时器
 * history     :
 ***************************************************************/
HTIMER OS_API kill_timer(IN HTIMER handle)
{
    struct timer_handle *timer;

    if (OS_NULL != handle) {
        timer = (struct timer_handle *) handle;

        cassert(TIMER_CHECK == timer->check);
        spin_lock(&timer_lock);
        del_list(&timer->node);
        add_list_tail(&idle_timer, &timer->node);
        timer->status = TIMER_IDLE;
        spin_unlock(&timer_lock);
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
os_void timer_function(os_void)
{
    struct list_node *list, *_save;
    struct timer_handle *timer;
    HWINDOW hwnd;
    TIMER_FUNC_PTR timer_func;
    os_u32 event_id;

    spin_lock(&timer_lock);
    loop_del_list(list, _save, &busy_timer) {
        timer = list_addr(list, struct timer_handle, node);
        cassert(TIMER_CHECK == timer->check);
        if (TIMER_IDLE == timer->status) {
            /* bugfix: _save maybe delete in user's call back. */
            break;
        }

        if (TIMER_TIMEOUT == timer->status) {
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
                timer->status = TIMER_TIMEOUT;
#else
                /* bugfix: delete timer again in timeout callback */
                del_init_list(&timer->node);
                add_list_tail(&idle_timer, &timer->node);
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
            spin_unlock(&timer_lock);
            if (timer_func) {
                timer_func(event_id);
            }
            if (hwnd) {
                timeout_msg(hwnd, event_id);
            }
            spin_lock(&timer_lock);
        }
    }
    spin_unlock(&timer_lock);
}

