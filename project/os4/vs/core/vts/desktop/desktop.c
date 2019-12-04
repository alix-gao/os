/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : desktop.c
 * version     : 1.0
 * description : desktop系统任务, 系统桌面任务
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include <vds.h>
#include "desktop.h"

/***************************************************************
 global variable declare
 ***************************************************************/
/* 窗口任务句柄 */
LOCALD HWINDOW desktop_handle = OS_NULL;

/* 分辨率参数 */
LOCALD volatile os_u32 rx _CPU_ALIGNED_ = 640;
LOCALD volatile os_u32 ry _CPU_ALIGNED_ = 480;

/***************************************************************
 function declare
 ***************************************************************/

LOCALD HTIMER timer1, timer2, timer3;
LOCALD HEVENT test_sem_id = 0;
LOCALD HEVENT mute_sem_id = 0;
LOCALC os_ret OS_CALLBACK timer_test_func(os_u32 event_id)
{
    print(".%x.\n", event_id);
    //reset_timer(timer3);

    return OS_SUCC;
}
LOCALC os_ret OS_CALLBACK rtc_test_func(os_u32 event_id)
{
    //set_rtc_callback(3,10,rtc_test_func,TIMER_MODE_NOT_LOOP);
    return OS_SUCC;
}

LOCALC os_void test(os_u32 arg1, os_u32 arg2, os_u32 arg3, os_u32 arg4, os_u32 arg5, os_u32 arg6, os_u32 arg7)
{
    do {
        logic_csys r0 = {70-20, 80};
        logic_csys r1 = {70, 80-40};
        logic_csys r2 = {70+20, 80};
        logic_csys r3 = {70, 80+40};
        HDEVICE hdc;
        lock_t eflag;

        flog("arg:%d%d%d%d%d%d%d\n", arg1,arg2,arg3,arg4,arg5,arg6,arg7);

        hdc = open_hdc(desktop_handle);

        //win_draw_curve(hdc, pp0, pp1, pp2);
        //win_draw_line(hdc, pp0, pp1);
        win_draw_line(hdc, r0, r2);
        win_draw_line(hdc, r1, r3);

        win_draw_line(hdc, r0, r1);
        win_draw_line(hdc, r1, r2);
        win_draw_line(hdc, r2, r3);
        win_draw_line(hdc, r3, r0);

        /* 画圆 */
        lock_int(eflag);
        win_draw_curve(hdc, r0, r1, r2, 30);
        win_draw_curve(hdc, r0, r3, r2, 30);
        unlock_int(eflag);

        close_hdc(desktop_handle, hdc);
#if 0
        draw_line(r0, r2, VGA_COLOR_YELLOW);
        draw_line(r1, r3, VGA_COLOR_YELLOW);

        draw_line(r0, r1, VGA_COLOR_YELLOW);
        draw_line(r1, r2, VGA_COLOR_YELLOW);
        draw_line(r2, r3, VGA_COLOR_YELLOW);
        draw_line(r3, r0, VGA_COLOR_YELLOW);
#endif
    } while (0);

    do {
        os_void *pp;

        pp = kmalloc(0x100);
        //flog("alloc: %x\n", pp);
        mem_set(pp, 0xff, 0x100);
        //flog("free: %x, %x\n", kfree(pp), pp);
    } while (0);

    do {
        //dump_timer();

        timer1 = set_timer_callback(1,100,timer_test_func,TIMER_MODE_LOOP);
        timer2 = set_timer_callback(2,200,timer_test_func,TIMER_MODE_LOOP);
        timer3 = set_timer_callback(3,300,timer_test_func,TIMER_MODE_LOOP);

        kill_timer(timer3);
        kill_timer(timer1);
        kill_timer(timer2);
        //dump_timer();

        //set_bp(timer_function, BREAK_ON_EXE, OS_NULL);

        //__asm__("int $3\n\t");

        //set_bp((os_u32) &timer2, BREAK_ON_WRITE, BREAK_LEN_1, OS_NULL);
        print("set write bp %x\n", &timer2);
        timer2 = 0;
        print("write bp\n");
        //__asm__("int $2\n\t");
#if 0
        timer1 = set_rtc_callback(3,10,rtc_test_func,TIMER_MODE_NOT_LOOP);
        create_critical_section(&mute_sem_id, __LINE__);
        test_sem_id = create_event_handle(EVENT_INVALID, OS_NULL, __LINE__);
        timer3 = set_timer_callback(3,3,timer_test_func,TIMER_MODE_NOT_LOOP);
        //dump_rtc();
        //dump_kdm_info();

        //dump_queue_info();

        print("take");
        wait_event(test_sem_id, 0);
        print("->d");
        enter_critical_section(mute_sem_id);
        print("section");
        leave_critical_section(mute_sem_id);
        print("0p");
        destroy_event_handle(test_sem_id);
        dead();
#endif
    } while (0);

    do {
        os_u32 a,b,c,d;
        c = d = 0;
        a=5;
        b=2;
        divl(a,b,c,d);
        flog("%d %d\n",c,d);
    } while (0);

#if 0
    do {
        os_u32 a = 1;
        os_u32 b = 0;
        __asm__("int $4\n\t"); /* trap, not fault. */
        break;
        timer1 = (HTIMER)(a / b);
    } while (0);

    do {
        os_u8 *p1,*p2,*p3;
        p1 = cmalloc(0x20000, 0x10);
        dump("kpm");
        p2 = cmalloc(0x10000, 0x10);
        dump("kpm");
        p3 = cmalloc(0x10000, 0x10);
        dump("kpm");
        cfree(p1);
        dump("kpm");
        cfree(p2);
        dump("kpm");
        cfree(p3);
        dump("kpm");
    } while (0);
#endif

#if 0
    for (os_u32 i = 0; i < 1; i++) {
        break;
    }
#endif

    do {
        os_u64 t;
        t = 0x0000000f0f0f0000LL;
        print("u64: %d %d %x\n", sizeof(os_u64), _align_, t);
    } while (0);

    test_klm();

    rwlock_t tl;
    init_rw_lock(&tl);
    read_lock(&tl);
    read_unlock(&tl);

    delay_task(10, __LINE__);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void send_mouse_msg(os_u8 left, os_u8 right, os_u8 mid, os_s32 x, os_s32 y, os_s32 wheel)
{
    struct mouse_msg *msg;

    /* 不是广播消息, 发送到当前窗口 */
    msg = (struct mouse_msg *) alloc_msg(sizeof(struct mouse_msg));
    if (OS_NULL == msg) {
        flog("alloc msg fail, %d\n", __LINE__);
        return;
    }
    msg->msg_name = OS_MSG_MOUSE;
    msg->msg_len = sizeof(struct mouse_msg);
    msg->left = left;
    msg->right = right;
    msg->mid = mid;
    msg->x = x;
    msg->y = y;
    msg->wheel = wheel;

    post_msg(desktop_handle, msg);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void draw_mouse(os_u8 left, os_u8 right, os_u8 mid, os_s32 x, os_s32 y, os_u32 wheel)
{
    /* 屏幕焦点 */
    GLOBALDIF screen_csys pos = {0,0};

    if ((pos.x+x < rx) && (0 < pos.x+x))
        pos.x += x;

    if ((pos.y+y < ry) && (0 < pos.y+y))
        pos.y += y;

    /* 移动事件 */
    paint_mouse(&pos);

    /* 点击事件 */
    if (1 == left) {
        draw_point(&pos, VGA_COLOR_RED);
    }
    if (1 == right) {
        draw_point(&pos, VGA_COLOR_YELLOW);
    }
    if (1 == mid) {
        draw_point(&pos, VGA_COLOR_GREEN);
    }
    if (1 == mid) {
        draw_point(&pos, VGA_COLOR_BLACK);
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void init_desktop_resolution(os_void)
{
    screen_csys resolution;

    /* 分辨率参数 */
    resolution = current_resolution();

    rx = resolution.x;
    ry = resolution.y;
    wmb();
}

/***************************************************************
 * description : MSG_PROC_PTR函数类型
 * history     :
 ***************************************************************/
LOCALC os_void desktop_proc_mouse(IN struct mouse_msg *msg)
{
    draw_mouse(msg->left, msg->right, msg->mid, msg->x, msg->y, msg->wheel);
}

/***************************************************************
 * description : MSG_PROC_PTR函数类型
 * history     :
 ***************************************************************/
LOCALC os_ret desktop_msgproc(IN os_void *msg)
{
    struct message *desktop_msg;

    cassert(OS_NULL != msg);

    desktop_msg = (struct message *) msg;

    switch (desktop_msg->msg_name) {
    case OS_MSG_MOUSE:
        desktop_proc_mouse(msg);
        break;

    case OS_MSG_PAINT:
        init_desktop_resolution();
        init_paint();
        init_print();
        draw_mouse(0, 0, 0, 0, 0, 0); /* 重新绘制鼠标 */
        set_window_width(desktop_handle, rx);
        set_window_length(desktop_handle, ry);
        break;

    default:
        break;
    }
    return OS_SUCC;
}

/***************************************************************
 * description : 创建窗口
 * history     :
 ***************************************************************/
LOCALC os_ret init_desktop_application(os_void)
{
    /* 桌面窗口属性 */
    GLOBALDIF struct window_class desktop_class;
    screen_csys r;

    /* init_application, 越权使用, 不使用动态内存 */
    desktop_class.attr.title[0] = '\0';
    desktop_class.attr.background_color = VGA_COLOR_BLUE;
    desktop_class.attr.csys.x = desktop_class.attr.csys.y = 0;
    desktop_class.attr.wframe = OS_FALSE;
    desktop_class.attr.cursor_pos.x = desktop_class.attr.cursor_pos.y = 0;
    desktop_class.attr.font_size = FONT_SIZE;
    desktop_class.attr.foreground_color = VGA_COLOR_WHITE;
    r = current_resolution();
    desktop_class.attr.length = r.y;
    desktop_class.attr.width = r.x;

    mem_cpy(desktop_class.app_name, "desktop", MAX_APP_NAME_LEN);

    desktop_class.app_init = OS_NULL;
    desktop_class.msg_proc = desktop_msgproc;

    /* register class */
    register_static_window_class(&desktop_class);

    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret init_desktop_instance(os_void)
{
    /* 创建窗口 */
    desktop_handle = create_window("desktop");
    if (OS_NULL == desktop_handle) {
        /* panic */
        return OS_FAIL;
    }

    /* 显示窗口 */
    //show_window(desktop_handle);

    /* 激活任务消息队列 */
    active_task_station(current_task_handle());

    return OS_SUCC;
}

/***************************************************************
 * description : TASK_FUNC_PTR
 * history     :
 ***************************************************************/
LOCALC os_ret OS_CALLBACK desktop_entry_point(os_u32 arg1, os_u32 arg2, os_u32 arg3, os_u32 arg4, os_u32 arg5, os_u32 arg6, os_u32 arg7)
{
    os_void *msg;

    /* 注册窗口类 */
    init_desktop_application();

    /* 产生窗口 */
    init_desktop_instance();

    init_desktop_resolution();

    test(arg1, arg2, arg3, arg4, arg5, arg6, arg7);

    /* 消息循环 */
    while (OS_SUCC == get_message(&msg)) {
        /* 线程单窗口消息处理 */
        desktop_msgproc(msg);
        /* 处理完了释放消息内存, 如果使用的是dispatch_msg则不需要释放消息内存 */
        free_msg(msg);
    }
    /* 不需要删除线程 */
    flog("desktop is deleted\n");
    return OS_SUCC;
}

/***************************************************************
 * description : 创建桌面任务
 * history     :
 ***************************************************************/
os_void init_desktop_task(os_void)
{
    HTASK handle;

    /* 创建桌面进程 */
    handle = create_task("desktop", desktop_entry_point, DESKTOP_TASK_PRIO, 0, 1, 0, 0, 4, 1, 2);
    cassert(OS_NULL != handle);
}

/***************************************************************
 * description : 获取桌面窗口句柄
 * history     :
 ***************************************************************/
HWINDOW OS_API get_desktop_window(os_void)
{
    return desktop_handle;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void notify_desktop(os_void)
{
    struct message *msg;

    /* 不是广播消息, 发送到当前窗口 */
    msg = (struct message *) alloc_msg(sizeof(struct message));
    if (OS_NULL == msg) {
        /* 消息内存分配失败, 不发送 */
        return;
    }

    /* 发送者为isr */
    msg->msg_name = OS_MSG_PAINT;
    msg->msg_len = sizeof(struct message);

    post_msg(get_desktop_window(), msg);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret set_desktop_resolution(enum graphics_mode mode)
{
    if (GRAPHICES_MODE_BUTT >= mode) {
        open_graphics_mode(mode);
        /* 通知桌面任务更新 */
        notify_desktop();
        return OS_SUCC;
    }
    return OS_FAIL;
}

