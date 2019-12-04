/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : window.c
 * version     : 1.0
 * description : 消息处理的最小单元
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include <window.h>
#include <message.h>

/***************************************************************
 global variable declare
 ***************************************************************/
/* 任务窗口 */
LOCALD struct window_class *window_class_tab[MAX_WINDOW_NUM] = { 0 };

LOCALD spinlock_t window_lock;

/* 窗口句柄对象 */
LOCALD struct window_handle window_handle_tab[MAX_WINDOW_NUM] = { 0 };

/* 空闲窗口资源表 */
LOCALD os_u32 idle_window_rc[MAX_WINDOW_NUM] = { 0 };

/* 当前窗口 */
LOCALD volatile HWINDOW current_window_handle _CPU_ALIGNED_ = OS_NULL;

/* 消息钩子 */
GLOBALD MSG_PROC_PTR hook = OS_NULL;

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description : 画窗口背景 (x0, y0)-(x1, y1)
 * history     :
 ***************************************************************/
LOCALC os_void win_draw_background(struct window_attr *win_attr)
{
    screen_csys p0, p1;

    /* 入参检查 */
    if (OS_NULL != win_attr) {
        p0 = p1= win_attr->csys;

        p0.x += 2 * WND_FRAME_SIZE;
        p0.y += TITLE_HIGH + 2 * WND_FRAME_SIZE;

        p1.x = p1.x + win_attr->width - 2 * WND_FRAME_SIZE;
        p1.y = p1.y + win_attr->length - 2 * WND_FRAME_SIZE;

        draw_rect(p0, p1, win_attr->background_color);
    }
}

/***************************************************************
 * description : 标题字符串
 * history     :
 ***************************************************************/
LOCALC os_void show_title(struct window_attr *win_attr, enum vga_color title_name_color)
{
    screen_csys pos;
    os_u32 i;
    os_u8 *str;

    /* 入参检查 */
    if (OS_NULL != win_attr) {
        pos.x = win_attr->csys.x + WND_FRAME_SIZE;
        pos.y = win_attr->csys.y + WND_FRAME_SIZE;
        i = 0;

        str = win_attr->title;
        while (('\0' != str[i]) && (MAX_TITLE_NAME_LEN > i)) {
            print_asc(win_attr->title[i], pos, title_name_color);
            pos.x = pos.x + 8;
            i++;
        }
    }
}

/***************************************************************
 * description : 显示标题栏
 * history     :
 ***************************************************************/
LOCALC os_void show_title_bar(struct window_attr *win_attr, enum vga_color title_color)
{
    screen_csys p0, p1;

    /* 入参检查 */
    if (OS_NULL != win_attr) {
        /* 标题栏 */
        p0.x = win_attr->csys.x + WND_FRAME_SIZE;
        p0.y = win_attr->csys.y + WND_FRAME_SIZE;

        p1.x = win_attr->csys.x + win_attr->width - WND_FRAME_SIZE;
        //p1.y = win_attr->csys.y + 2*FONT_SIZE+WND_FRAME_SIZE;
        p1.y = win_attr->csys.y + (TITLE_HIGH + WND_FRAME_SIZE);

        draw_rect(p0, p1, title_color);
    }
}

/***************************************************************
 * description : 画边框, 目前为3像素宽度的边框
 * history     :
 ***************************************************************/
LOCALC os_void win_draw_frame(os_u32 x, os_u32 y, os_u32 width, os_u32 length)
{
    screen_csys p0, p1;
    enum vga_color color;

    /* 背景部分 */
    color = VGA_COLOR_SILVER;

    /* 上横, 背景 */
    p0.x = x;
    p0.y = y;
    p1.x = x + width;
    p1.y = y + WND_FRAME_SIZE;
    draw_rect(p0, p1, color);

    /* 下横, 背景 */
    p0.x = x;
    p0.y = y + length - WND_FRAME_SIZE;
    p1.x = x + width;
    p1.y = y + length;
    draw_rect(p0, p1, color);

    /* 左竖, 背景 */
    p0.x = x;
    p0.y = y + WND_FRAME_SIZE;
    p1.x = x + WND_FRAME_SIZE;
    p1.y = y + length - WND_FRAME_SIZE;
    draw_rect(p0, p1, color);

    /* 右竖, 背景 */
    p0.x = x + width - WND_FRAME_SIZE;
    p0.y = y + WND_FRAME_SIZE;
    p1.x = x + width;
    p1.y = y + length - WND_FRAME_SIZE;
    draw_rect(p0, p1, color);

    /* 高亮部分 */
    color = VGA_COLOR_WHITE;

    /* 上横, 高亮部分 */
    p0.x = (x) + 1;
    p0.y = (y) + 1;
    p1.x = (x + width) - 1;
    p1.y = (y + WND_FRAME_SIZE) - 1;
    draw_rect(p0, p1, color);

    /* 下横, 高亮部分 */
    p0.x = (x) + 1;
    p0.y = (y + length - WND_FRAME_SIZE) + 1;
    p1.x = (x + width) - 1;
    p1.y = (y + length) - 1;
    draw_rect(p0, p1, color);

    /* 左竖, 高亮部分 */
    p0.x = (x) + 1;
    p0.y = (y) + 1;
    p1.x = (x) + WND_FRAME_SIZE - 1;
    p1.y = (y) + length - 1;
    draw_rect(p0, p1, color);

    /* 右竖, 高亮部分 */
    p0.x = (x + width - WND_FRAME_SIZE) + 1;
    p0.y = (y) + 1;
    p1.x = (x + width) - 1;
    p1.y = (y + length) - 1;
    draw_rect(p0, p1, color);
}

/***************************************************************
 * description : 显示窗口
 * history     :
 ***************************************************************/
os_ret OS_API show_window(IN HWINDOW hwnd)
{
    struct window_attr *window_attr;

    /* 入参检查 */
    if (OS_NULL != hwnd) {
        window_attr = &window_class_tab[((struct window_handle *) hwnd)->window_id]->attr;
        if (OS_TRUE == window_attr->wframe) {
            /* 画外边框 */
            win_draw_frame(window_attr->csys.x, window_attr->csys.y, window_attr->width, window_attr->length);
            /* 画内边框 */
            win_draw_frame(window_attr->csys.x+WND_FRAME_SIZE, window_attr->csys.y+(TITLE_HIGH+WND_FRAME_SIZE), window_attr->width-WND_FRAME_SIZE-WND_FRAME_SIZE, window_attr->length-TITLE_HIGH-2*WND_FRAME_SIZE);
            /* 画标题栏 */
            show_title_bar(window_attr, VGA_COLOR_BLUE);
            /* 显示标题 */
            show_title(window_attr, VGA_COLOR_SILVER);
            /* 画窗口背景 */
            win_draw_background(window_attr);
        }
        return OS_SUCC;
    }
    /* 当前窗口句柄无效 */
    return OS_FAIL;
}

/***************************************************************
 * description : 重画窗口
 * history     :
 ***************************************************************/
os_ret OS_API repaint_window(IN HWINDOW hwnd)
{
    struct window_attr *window_attr;

    /* 入参检查 */
    if (OS_NULL != hwnd) {
        if (current_window_handle == hwnd) {
            window_attr = &window_class_tab[((struct window_handle *) hwnd)->window_id]->attr;
            if (OS_TRUE == window_attr->wframe) {
                /* 画外边框 */
                win_draw_frame(window_attr->csys.x, window_attr->csys.y, window_attr->width, window_attr->length);
                /* 画内边框 */
                win_draw_frame(window_attr->csys.x+WND_FRAME_SIZE, window_attr->csys.y+(TITLE_HIGH+WND_FRAME_SIZE), window_attr->width-WND_FRAME_SIZE-WND_FRAME_SIZE, window_attr->length-TITLE_HIGH-2*WND_FRAME_SIZE);
                /* 画标题栏 */
                show_title_bar(window_attr, VGA_COLOR_NAVY);
                /* 显示标题 */
                show_title(window_attr, VGA_COLOR_WHITE);
                /* 画窗口背景 */
                win_draw_background(window_attr);
            }
        } else {
            show_window(hwnd);
        }
        return OS_SUCC;
    }
    /* 入参检查失败 */
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API update_window(IN HWINDOW hwnd)
{
    struct message *msg;

    /* 入参检查 */
    if (OS_NULL != hwnd) {
        msg = alloc_msg(sizeof(struct message));
        if (OS_NULL == msg) {
            /* 消息内存分配失败, 不发送 */
            return OS_FAIL;
        }

        /* 发送者为isr */
        msg->msg_name = OS_MSG_PAINT;
        msg->msg_len = sizeof(struct message);

        post_msg(hwnd, msg);
        return OS_SUCC;
    }
    /* 入参检查失败 */
    return OS_FAIL;
}

/***************************************************************
 * description : 产生窗口
 * history     :
 ***************************************************************/
LOCALC os_ret init_instance(os_u8 *app_name)
{
    HWINDOW handle;

    /* 入参检查 */
    if (OS_NULL != app_name) {
        /* create window */
        handle = create_window(app_name);
        if (OS_NULL == handle) {
            /* panic */
            return OS_FAIL;
        }

        /* show window显示窗口 */
        show_window(handle);

        /* 创建线程消息队列 */
        active_task_station(current_task_handle());

        /* update window送出WM_PAINT */
        update_window(handle);

        return OS_SUCC;
    }

    /* 入参检查失败 */
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void init_window_class(os_void)
{
    os_u32 i;

    for (i = 0; i < MAX_WINDOW_NUM; i++) {
        window_class_tab[i] = OS_NULL;
    }
    /* 当前窗口为桌面 */
    //current_window_handle = desktop_handle;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void dump_window_handle_info(os_void)
{
    os_u32 i;

    // MAX_WINDOW_NUM
    for (i = 0; i < 7; i++) {
        print("%x, %x, %s\n", window_handle_tab[i].htask->task_id, window_handle_tab[i].window_id, window_class_tab[i]->app_name);
    }
}

LOCALD os_u8 win_debug_name[] = { "win" };
LOCALD struct dump_info win_debug = {
    win_debug_name,
    dump_window_handle_info
};

/***************************************************************
 * description : 初始化窗口句柄资源表
 * history     :
 ***************************************************************/
os_void init_window_handle_tab(os_void)
{
    os_u32 i;
    struct window_handle *t;

    mem_set(window_handle_tab, 0, MAX_WINDOW_NUM * sizeof(struct window_handle));

    for (i = 0, t = window_handle_tab; i < MAX_WINDOW_NUM; i++, t++) {
        t->htask = OS_NULL;
        t->window_id = INVALID_WINDOW_ID;
    }

    init_spinlock(&window_lock);

    if (OS_SUCC != register_dump(&win_debug)) {
        flog("win register dump fail\n");
    }
}

/***************************************************************
 * description : 资源id
 * history     :
 ***************************************************************/
LOCALC os_u32 lookup_application_id(IN os_u8 *app_name)
{
    os_u32 i;
    struct window_class **t;

    for (i = 0, t = window_class_tab; i < MAX_WINDOW_NUM; i++, t++) {
        if ((OS_NULL != (*t)) && (0 == str_cmp(app_name, (*t)->app_name))) {
            /* 找到相等 */
            return i;
        }
    }
    /* 没有找到 */
    return INVALID_WINDOW_ID;
}

/***************************************************************
 * description : 创建窗口
 * history     :
 ***************************************************************/
HWINDOW OS_API create_window(IN os_u8 *app_name)
{
    os_u32 window_rc_id;

    /* 入参检查 */
    if (OS_NULL != app_name) {
        spin_lock(&window_lock);

        /* 查找application name */
        window_rc_id = lookup_application_id(app_name);
        if (INVALID_WINDOW_ID == window_rc_id) {
            spin_unlock(&window_lock);

            /* 注册信息中没有找到 */
            return OS_NULL;
        }

        window_handle_tab[window_rc_id].htask = current_task_handle();
        window_handle_tab[window_rc_id].window_id = window_rc_id;

        spin_unlock(&window_lock);

        return (HWINDOW) &window_handle_tab[window_rc_id];
    }

    /* 入参检查失败 */
    return OS_NULL;
}

/***************************************************************
 * description : 根据appname查找窗口句柄
 * history     :
 ***************************************************************/
HWINDOW OS_API lookup_handle_by_app_name(IN os_u8 *app_name)
{
    os_u32 window_rc_id;

    /* 入参检查 */
    if (OS_NULL != app_name) {
        /* 查找application name */
        window_rc_id = lookup_application_id(app_name);
        if (INVALID_WINDOW_ID == window_rc_id) {
            /* 注册信息中没有找到 */
            return OS_NULL;
        }
        return (HWINDOW) &window_handle_tab[window_rc_id];
    }
    /* 入参检查失败 */
    return OS_NULL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void init_idle_window_rc(os_void)
{
    os_u32 i;

    /* 清空全局变量idle_task_id */
    mem_set(idle_window_rc, 0, MAX_WINDOW_NUM * sizeof(os_u32));

    /* 初始化idle_task_entity */
    for (i=1; i<MAX_WINDOW_NUM-1; i++) {
        idle_window_rc[i] = i+1;
    }

    /* 收尾 */
    idle_window_rc[MAX_WINDOW_NUM-1] = INVALID_WINDOW_ID;

    /* 下一个要分配task_entity的位置 */
    idle_window_rc[0] = 1;
}

/***************************************************************
 * description : 分配窗口资源
 * history     :
 ***************************************************************/
LOCALC os_u32 alloc_window_rc(os_void)
{
    os_u32 window_id;

    /* 不必判断(INVALID_WINDOW_ID != idle_window_rc[0]) */
    if (MAX_WINDOW_NUM > idle_window_rc[0]) {
        window_id = idle_window_rc[0];
        idle_window_rc[0] = idle_window_rc[window_id];
        return window_id;
    }
    /* 资源分配完毕 */
    return INVALID_WINDOW_ID;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void free_window_rc(os_u32 window_id)
{
    os_u32 idle_head_id;

    /* 分配的task id在范围之内 */
    if (MAX_WINDOW_NUM > window_id) {
        idle_head_id = idle_window_rc[0];
        idle_window_rc[0] = window_id;
        idle_window_rc[window_id] = idle_head_id;
    }
}

/***************************************************************
 * description :
 * history     : 取消结构体直接赋值, 在某些情况下内核崩溃
 ***************************************************************/
os_ret OS_API register_window_class(IN struct window_class *window_class)
{
    struct window_class *task_win_mem;
    os_u32 window_rc_id;

    if (OS_NULL != window_class) {
        /* 分配窗口资源 */
        window_rc_id = alloc_window_rc();
        if (INVALID_WINDOW_ID == window_rc_id) {
            /* 分配窗口资源失败 */
            return OS_FAIL;
        }

        /* 分配内核内存创建窗口信息 */
        task_win_mem = kmalloc(sizeof(struct window_class));
        if (OS_NULL == task_win_mem) {
            return OS_FAIL;
        }

        /* 结构体类型赋值 */
        //*task_win_mem = *window_class;
        //debug. 在结构体尺寸大于某个值时会产生系统函数memcpy调用. 进而导致内核崩溃.
        mem_cpy(task_win_mem, window_class, sizeof(struct window_class));

        /* 记录 */
        window_class_tab[window_rc_id] = task_win_mem;

        return OS_SUCC;
    }

    /* 入参检查失败 */
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API register_static_window_class(IN struct window_class *window_class)
{
    os_u32 window_rc_id;

    /* 入参检查 */
    if (OS_NULL != window_class) {
        /* 分配窗口资源 */
        window_rc_id = alloc_window_rc();
        if (INVALID_WINDOW_ID == window_rc_id) {
            /* 分配窗口资源失败 */
            return OS_FAIL;
        }

        /* 记录 */
        window_class_tab[window_rc_id] = (struct window_class *) window_class;
        return OS_SUCC;
    }
    /* 入参检查失败 */
    return OS_FAIL;
}

/***************************************************************
 * description : 注册窗口类
 * history     :
 ***************************************************************/
LOCALC os_ret init_application(struct window_class *window_class)
{
    /* window_class由用户填写, 不做参数检查 */
    /* 注册窗口 */
    register_static_window_class(window_class);
    return OS_SUCC;
}

/***************************************************************
 * description : 获取当前线程的消息, 以线程为单位(不是窗口句柄)
 * history     :
 ***************************************************************/
os_ret OS_API get_message(INOUT os_void **msg)
{
    os_ret result;

    /* 入参检查 */
    if (OS_NULL != msg) {
        for (;;) {
            /* receive_msg函数执行pend操作 */
            result = receive_msg(current_task_handle(), msg);
            if (OS_SUCC != result) {
                return OS_FAIL;
            }

            if (OS_NULL != *msg) {
                return OS_SUCC;
            }

            /* 消息队列存在消息, 准备读取 */
        }
        return OS_SUCC;
    }
    /* 入参检查失败 */
    return OS_FAIL;
}

/***************************************************************
 * description : 分发消息给线程各个窗口分别处理
 * history     :
 ***************************************************************/
os_ret OS_API dispatch_msg(IN os_void *msg)
{
    os_u32 window_id;
    struct window_handle *handle;

    /* 入参检查 */
    if (OS_NULL != msg) {
        /* 根据消息获取句柄 */
        handle = get_dest_hwindow(msg);

        window_id = handle->window_id;

        /* window id合法 */
        if (MAX_WINDOW_NUM > window_id) {
            /* 消息处理函数已经注册 */
            if ((OS_NULL != window_class_tab[window_id]) && (OS_NULL != window_class_tab[window_id]->msg_proc)) {
                /* 消息处理回调 */
                (*window_class_tab[window_id]->msg_proc)(msg);

                /* 消息钩子 */
                if (OS_NULL != hook) {
                    (*hook)(msg);
                }

                /* 释放消息内存 */
                free_msg(msg);
                return OS_SUCC;
            }
        }
        /* 释放消息内存 */
        free_msg(msg);
    }
    /* 入参检查失败, window_id & window_class_tab */
    return OS_FAIL;
}

/***************************************************************
 * description : 用户界面线程函数,
 *               TASK_FUNC_PTR函数类型
 * history     :
 ***************************************************************/
LOCALC os_ret OS_CALLBACK window_task_func(os_u32 arg1, os_u32 arg2, os_u32 arg3, os_u32 arg4, os_u32 arg5, os_u32 arg6, os_u32 arg7)
{
    os_void *msg;
    struct window_class *window_class;

    /* 参数1不检查 */
    window_class = (struct window_class *) arg1;

    /* 注册窗口类 */
    init_application(window_class);

    /* 产生窗口 */
    init_instance(window_class->app_name);

    if (OS_NULL != window_class->app_init) {
        /* 用户程序初始化窗口(同步窗口初始化) */
        window_class->app_init();
    }

    /* 消息循环 */
    while (OS_SUCC == get_message(&msg)) {
        /* 消息处理 */
        dispatch_msg(msg);
    }
    flog("task error %d!\n", __LINE__);
    return OS_SUCC;
}

/***************************************************************
 * description : 创建用户界面线程
 * history     :
 ***************************************************************/
HTASK OS_API create_ui_task(IN os_u8 *name, IN struct window_class *window_class, enum task_priority priority)
{
    struct window_class *window_class_mem;

    /* name不检查 */

    if (OS_NULL != window_class) {
        /* 分配内核内存创建窗口信息 */
        window_class_mem = kmalloc(sizeof(struct window_class));
        if (OS_NULL == window_class_mem) {
            return OS_NULL;
        }

        /* 结构体类型赋值 */
        mem_cpy(window_class_mem, window_class, sizeof(struct window_class));

        /* arg1 = window_class */
        return create_task(name, window_task_func, priority, (pointer) window_class_mem, 0, 0, 0, 0, 0, 0);
    }
    /* 入参检查失败 */
    return OS_NULL;
}

/***************************************************************
 * description : 得到任务的窗口属性
 * history     :
 ***************************************************************/
struct window_attr OS_API *get_window_attr(IN HWINDOW handle)
{
    struct window_handle *window_handle;

    if (OS_NULL != handle) {
        window_handle = handle;

        if (OS_NULL != window_class_tab[window_handle->window_id]) {
            return &window_class_tab[window_handle->window_id]->attr;
        }
    }
    /* handle | window class */
    return OS_NULL;
}

/***************************************************************
 * description : 获取窗口设备上下文
 *               handle device context
 * history     :
 ***************************************************************/
HDEVICE OS_API open_hdc(IN HWINDOW hwnd)
{
    struct window_handle *window_handle;
    struct device_context *device_handle;
    struct window_attr *attr;

    /* 入参检查 */
    if (OS_NULL != hwnd) {
        window_handle = hwnd;

        /* window class已经注册 */
        if (OS_NULL != window_class_tab[window_handle->window_id]) {
            /* 分配内核内存使用 */
            device_handle = kmalloc(sizeof(struct device_context));
            if (OS_NULL == device_handle) {
                /* 内核内存分配失败 */
                return OS_NULL;
            }

            attr = &window_class_tab[window_handle->window_id]->attr;

            device_handle->background_color = attr->background_color;
            if (OS_TRUE == attr->wframe) {
                device_handle->csys.x = attr->csys.x + 2*WND_FRAME_SIZE;
                device_handle->csys.y = attr->csys.y + 2*WND_FRAME_SIZE + TITLE_HIGH;
                device_handle->length = attr->length - 4*WND_FRAME_SIZE - TITLE_HIGH;
                device_handle->width = attr->width - 4*WND_FRAME_SIZE;
            } else {
                device_handle->csys.x = attr->csys.x;
                device_handle->csys.y = attr->csys.y;
                device_handle->length = attr->length;
                device_handle->width = attr->width;
            }
            device_handle->cursor_pos = attr->cursor_pos;
            device_handle->font_size = attr->font_size;
            device_handle->foreground_color = attr->foreground_color;

            return device_handle;
        }
    }
    /* 入参检查失败 | 该窗口没有注册 */
    return OS_NULL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_u32 OS_API get_window_width(IN HDEVICE hdc)
{
    struct device_context *device_handle;

    if (OS_NULL != hdc) {
        device_handle = hdc;
        return device_handle->width;
    } else {
        return 0;
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API set_window_width(IN HWINDOW hwnd, os_u32 width)
{
    struct window_handle *window_handle;

    /* 入参检查 */
    if (OS_NULL != hwnd) {
        window_handle = hwnd;

        /* window class已经注册 */
        if (OS_NULL != window_class_tab[window_handle->window_id]) {
            window_class_tab[window_handle->window_id]->attr.width = width;
            return OS_SUCC;
        }
    }
    /* 入参检查失败 | 该窗口没有注册 */
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_u32 OS_API get_window_length(IN HDEVICE hdc)
{
    struct device_context *device_handle;

    if (OS_NULL != hdc) {
        device_handle = hdc;
        return device_handle->length;
    } else {
        return 0;
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API set_window_length(IN HWINDOW hwnd, os_u32 length)
{
    struct window_handle *window_handle;

    /* 入参检查 */
    if (OS_NULL != hwnd) {
        window_handle = hwnd;

        /* window class已经注册 */
        if (OS_NULL != window_class_tab[window_handle->window_id]) {
            window_class_tab[window_handle->window_id]->attr.length = length;
            return OS_SUCC;
        }
    }
    /* 入参检查失败 | 该窗口没有注册 */
    return OS_FAIL;
}

/***************************************************************
 * description : 释放设备上下文
 * history     :
 ***************************************************************/
os_ret OS_API close_hdc(IN HWINDOW hwnd, IN HDEVICE hdc)
{
    struct window_handle *window_handle;
    struct device_context *device_handle;

    /* 入参检查 */
    if ((OS_NULL != hwnd) && (OS_NULL != hdc)) {
        window_handle = hwnd;

        /* 该窗口已经注册 */
        if (OS_NULL != window_class_tab[window_handle->window_id]) {
            device_handle = hdc;

            /* 刷新窗口坐标属性 */
            window_class_tab[window_handle->window_id]->attr.cursor_pos = device_handle->cursor_pos;

            kfree(hdc);
            return OS_SUCC;
        }
    }
    /* 入参检查失败 | 该窗口没有注册 */
    return OS_FAIL;
}

/***************************************************************
 * description : 设置背景颜色
 * history     :
 ***************************************************************/
LOCALC os_void set_bk_color(HWINDOW hwnd)
{
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void create_menu(HWINDOW hwnd)
{
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void delete_menu(HWINDOW hwnd)
{
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret destroy_menu(HWINDOW hwnd)
{
    return OS_SUCC;
}

/***************************************************************
 * description : 得到窗口区域大小
 * history     :
 ***************************************************************/
LOCALC os_void get_window_rgn(HWINDOW hwnd)
{
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API destroy_window(HWINDOW handle)
{
    return OS_SUCC;
}

/***************************************************************
 * description : 释放窗口资源
 * history     :
 ***************************************************************/
os_ret free_window_resource(struct task_handle *handle)
{
    os_u32 i;
    struct window_handle *t;
    os_u32 task_id;
    os_u32 window_id;
    os_void *window_class;

    cassert(OS_NULL != handle);

    task_id = handle->task_id;

    /* task id合法 */
    if ((INVALID_TASK_ID != task_id) && (OS_TASK_NUM > task_id)) {
        /* 搜索整个窗口句柄表 */
        for (i = 0, t = window_handle_tab; i < MAX_WINDOW_NUM; i++, t++) {
            if (task_id == t->htask->task_id) {
                window_id = t->window_id;

                /* 释放注册窗口类资源 */
                window_class = window_class_tab[window_id];

                destroy_window(window_class);

                kfree(window_class);

                window_class_tab[window_id] = OS_NULL;

                /* 释放窗口id资源 */
                free_window_rc(window_id);

                t->htask = OS_NULL;
                t->window_id = INVALID_WINDOW_ID;

                return OS_SUCC;
            }
        }
    }
    /* 入参检查失败(task_id)|没有搜索到句柄 */
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
HWINDOW OS_API get_current_window_handle(os_void)
{
    return current_window_handle;
}

/***************************************************************
 * description : 修改当前窗口句柄
 * history     :
 ***************************************************************/
os_ret OS_API modify_current_window_handle(IN HWINDOW handle)
{
    if (OS_NULL != handle) {
        current_window_handle = handle;
        wmb();
        return OS_SUCC;
    }
    return OS_FAIL;
}

/***************************************************************
 * description : 根据窗口句柄返回消息处理函数指针
 * history     :
 ***************************************************************/
MSG_PROC_PTR OS_API get_msgproc(IN HWINDOW handle)
{
    struct window_handle *hwnd;

    /* 入参检查 */
    if (OS_NULL != handle) {
        hwnd = (struct window_handle *) handle;

        /* window id合法 */
        if (MAX_WINDOW_NUM > hwnd->window_id) {
            return window_class_tab[hwnd->window_id]->msg_proc;
        }
    }
    /* 入参检查失败 | window id不合法 */
    return OS_NULL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API register_hook(IN MSG_PROC_PTR func)
{
    hook = func;
    return OS_SUCC;
}

