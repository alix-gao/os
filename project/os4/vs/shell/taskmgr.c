/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : taskmgr.c
 * version     : 1.0
 * description : 块线程模型(单线程多块模型STA)
 *               所有的请求通过Windows消息队列进行串行化,
 *               这样保证了每个时刻只能访问一个块, 因而只有一个单独的进程可以在某一个时刻得到执行.
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <lib.h>
#include <os.h>
#include <shell.h>
#include <taskmgr.h>

/***************************************************************
 global variable declare
 ***************************************************************/
/* 任务管理器窗口句柄 */
LOCALD HWINDOW taskmgr_text_handle = OS_NULL;

LOCALD HWINDOW taskmgr_graph_handle = OS_NULL;

LOCALD struct task_snapshot *taskmgr_snapshot = OS_NULL;

/***************************************************************
 function declare
 ***************************************************************/

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void taskmgr_timer_func(os_void)
{
    HDEVICE hdc;

    hdc = open_hdc(taskmgr_text_handle);

    /* 重置光标位置 */
    reset_cursor(hdc);

    /* 刷新文本区 */
    win_clear_screen(hdc);

    /* 更新快照 */
    fresh_snapshot(&taskmgr_snapshot);

    while (OS_SUCC == get_process32next(&taskmgr_snapshot)) {
        kprint(hdc, "%s %d\n", taskmgr_snapshot->name, taskmgr_snapshot->usage);
    }

    close_hdc(taskmgr_text_handle, hdc);
}

/***************************************************************
 * description : 跟踪被破坏的内存
 * history     :
 ***************************************************************/
os_void track_broken_mem(os_u32 cnt)
{
    struct window_attr *att;

    flog("dump tmg %d\n", cnt);
    att = get_window_attr(taskmgr_graph_handle);
    flog("%x %d %d %d %d\n",
            att->background_color,
            att->csys.x,
            att->csys.y,
            att->length,
            att->width);
}

/***************************************************************
 * description : MSG_PROC_PTR函数类型, 消息处理
 * history     :
 ***************************************************************/
LOCALC os_ret taskmgr_graph_msgproc(IN os_void *msg)
{
    struct message *cmd_msg;
    HDEVICE hdc;
    GLOBALDIF os_u32 cnt = 0;

    if (OS_NULL != msg) {
        cmd_msg = (struct message *) msg;

        switch (cmd_msg->msg_name) {
        case OS_MSG_PAINT:
            /* 重画标题栏 */
            repaint_window(taskmgr_graph_handle);
            if (cnt) {
                cnt = 0;
                show_image(taskmgr_graph_handle, "1.bmp", SI_MODE_2);
            } else {
                cnt = 1;
                show_image(taskmgr_graph_handle, "0.bmp", SI_MODE_2);
            }
            break;

        case OS_MSG_TIMER:
        default:
            break;
        }
        return OS_SUCC;
    }
    return OS_FAIL;
}

/***************************************************************
 * description : 线程间消息测试
 * history     :
 ***************************************************************/
LOCALC os_void send_to_cmd(os_void)
{
    HWINDOW handle;
    struct message *msg;

    handle = lookup_handle_by_app_name("cmd");
    if (OS_NULL == handle) {
        /* 获取窗口句柄失败 */
        return;
    }

    msg = (struct message *) alloc_msg(sizeof(struct message));
    if (OS_NULL == msg) {
        /* 消息内存分配失败, 不发送 */
        return;
    }

    msg->msg_name = OS_MSG_TASK;
    msg->msg_len = sizeof(struct message);

    post_msg(handle, msg);
}

/***************************************************************
 * description : MSG_PROC_PTR函数类型, 消息处理
 * history     :
 ***************************************************************/
LOCALC os_ret taskmgr_text_msgproc(IN os_void *msg)
{
    struct message *manager_msg;

    cassert(OS_NULL != msg);

    manager_msg = (struct message *) msg;
    switch (manager_msg->msg_name) {
    case OS_MSG_TIMER:
        taskmgr_timer_func();
        send_to_cmd();
        break;

    case OS_MSG_PAINT:
        /* 重画标题栏 */
        repaint_window(taskmgr_text_handle);
        break;

    default:
        break;
    }
    return OS_SUCC;
}

/***************************************************************
 * description : 产生窗口
 * history     :
 ***************************************************************/
LOCALC os_ret init_taskmgr_instance(os_void)
{
    /* 创建窗口 */
    taskmgr_text_handle = create_window("taskmgr text");
    if (OS_NULL == taskmgr_text_handle) {
        /* panic */
        flog("taskmgr text create win fail.\n");
        return OS_FAIL;
    }

    /* 显示窗口 */
    show_window(taskmgr_text_handle);

    /* 创建窗口 */
    taskmgr_graph_handle = create_window("taskmgr graph");
    if (OS_NULL == taskmgr_graph_handle) {
        /* panic */
        flog("taskmgr graph create win fail.\n");
        return OS_FAIL;
    }

    /* 显示窗口 */
    show_window(taskmgr_graph_handle);

    /* 激活任务消息队列, 一个线程激活一次 */
    active_task_station(current_task_handle());

    return OS_SUCC;
}

/***************************************************************
 * description : 注册窗口类
 * history     :
 ***************************************************************/
LOCALC os_ret init_taskmgr_application(os_void)
{
    /* 桌面窗口属性 */
    struct window_class text_class;
    struct window_class graph_class;

    /* init application(text) */
    mem_cpy(text_class.attr.title, "text", MAX_TITLE_NAME_LEN);
    text_class.attr.background_color = VGA_COLOR_PURPLE;
    text_class.attr.csys.x = TASKMGR_TEXT_WIN_X;
    text_class.attr.csys.y = TASKMGR_TEXT_WIN_Y;
    text_class.attr.wframe = OS_TRUE;
    text_class.attr.cursor_pos.x = 0;
    text_class.attr.cursor_pos.y = 0;
    text_class.attr.font_size = FONT_SIZE;
    text_class.attr.foreground_color = VGA_COLOR_SILVER;
    text_class.attr.length = TASKMGR_TEXT_WIN_LEN;
    text_class.attr.width = TASKMGR_TEXT_WIN_WID;

    mem_cpy(text_class.app_name, "taskmgr text", MAX_APP_NAME_LEN);

    text_class.app_init = OS_NULL;
    text_class.msg_proc = taskmgr_text_msgproc;

    /* register class(graph) */
    register_window_class(&text_class);

    /* init_application(input command) */
    mem_cpy(graph_class.attr.title, "graph", MAX_TITLE_NAME_LEN);
    graph_class.attr.background_color = VGA_COLOR_OLIVE;
    graph_class.attr.csys.x = TASKMGR_GRAPH_WIN_X;
    graph_class.attr.csys.y = TASKMGR_GRAPH_WIN_Y;
    graph_class.attr.wframe = OS_TRUE;
    graph_class.attr.cursor_pos.x = 0;
    graph_class.attr.cursor_pos.y = 0;
    graph_class.attr.font_size = FONT_SIZE;
    graph_class.attr.foreground_color = VGA_COLOR_SILVER;
    graph_class.attr.length = TASKMGR_GRAPH_WIN_LEN;
    graph_class.attr.width = TASKMGR_GRAPH_WIN_WID;

    mem_cpy(graph_class.app_name, "taskmgr graph", MAX_APP_NAME_LEN);

    graph_class.app_init = OS_NULL;
    graph_class.msg_proc = taskmgr_graph_msgproc;

    /* register class (input command) */
    register_window_class(&graph_class);

    return OS_SUCC;
}

/***************************************************************
 * description : VOID_FUNCPTR函数类型
 * history     :
 ***************************************************************/
LOCALC os_void taskmgr_init(os_void)
{
    /* 初始化系统快照 */
    create_toolhelp32snapshot(&taskmgr_snapshot);

    /* 启动100tick定时器, 便于统计 */
    set_timer_msg(taskmgr_text_handle, 0, 100, TIMER_MODE_LOOP);
    set_timer_msg(taskmgr_graph_handle, 0, 100, TIMER_MODE_LOOP);
}

/***************************************************************
 * description : task entry point
 * history     :
 ***************************************************************/
LOCALC os_ret OS_CALLBACK task_manager(os_u32 arg1, os_u32 arg2, os_u32 arg3, os_u32 arg4, os_u32 arg5, os_u32 arg6, os_u32 arg7)
{
    os_void *msg;

    /* 注册窗口类 */
    init_taskmgr_application();

    /* 产生窗口 */
    init_taskmgr_instance();

    taskmgr_init();

    /* 消息循环 */
    while (OS_SUCC == get_message(&msg)) {
        /* 消息路由 */
        dispatch_msg(msg);
    }
    flog("task error %d!\n", __LINE__);
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void init_taskmgr(os_void)
{
    HTASK handle;

    handle = create_task("taskmgr", task_manager, TASKMGR_PRIO, 0, 0, 0, 0, 0, 0, 0);
    /* 前台显示 */
    cassert(OS_NULL != handle);
}
shell_init_func(init_taskmgr);

