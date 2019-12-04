/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : window.c
 * version     : 1.0
 * description : ��Ϣ�������С��Ԫ
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
/* ���񴰿� */
LOCALD struct window_class *window_class_tab[MAX_WINDOW_NUM] = { 0 };

LOCALD spinlock_t window_lock;

/* ���ھ������ */
LOCALD struct window_handle window_handle_tab[MAX_WINDOW_NUM] = { 0 };

/* ���д�����Դ�� */
LOCALD os_u32 idle_window_rc[MAX_WINDOW_NUM] = { 0 };

/* ��ǰ���� */
LOCALD volatile HWINDOW current_window_handle _CPU_ALIGNED_ = OS_NULL;

/* ��Ϣ���� */
GLOBALD MSG_PROC_PTR hook = OS_NULL;

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description : �����ڱ��� (x0, y0)-(x1, y1)
 * history     :
 ***************************************************************/
LOCALC os_void win_draw_background(struct window_attr *win_attr)
{
    screen_csys p0, p1;

    /* ��μ�� */
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
 * description : �����ַ���
 * history     :
 ***************************************************************/
LOCALC os_void show_title(struct window_attr *win_attr, enum vga_color title_name_color)
{
    screen_csys pos;
    os_u32 i;
    os_u8 *str;

    /* ��μ�� */
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
 * description : ��ʾ������
 * history     :
 ***************************************************************/
LOCALC os_void show_title_bar(struct window_attr *win_attr, enum vga_color title_color)
{
    screen_csys p0, p1;

    /* ��μ�� */
    if (OS_NULL != win_attr) {
        /* ������ */
        p0.x = win_attr->csys.x + WND_FRAME_SIZE;
        p0.y = win_attr->csys.y + WND_FRAME_SIZE;

        p1.x = win_attr->csys.x + win_attr->width - WND_FRAME_SIZE;
        //p1.y = win_attr->csys.y + 2*FONT_SIZE+WND_FRAME_SIZE;
        p1.y = win_attr->csys.y + (TITLE_HIGH + WND_FRAME_SIZE);

        draw_rect(p0, p1, title_color);
    }
}

/***************************************************************
 * description : ���߿�, ĿǰΪ3���ؿ�ȵı߿�
 * history     :
 ***************************************************************/
LOCALC os_void win_draw_frame(os_u32 x, os_u32 y, os_u32 width, os_u32 length)
{
    screen_csys p0, p1;
    enum vga_color color;

    /* �������� */
    color = VGA_COLOR_SILVER;

    /* �Ϻ�, ���� */
    p0.x = x;
    p0.y = y;
    p1.x = x + width;
    p1.y = y + WND_FRAME_SIZE;
    draw_rect(p0, p1, color);

    /* �º�, ���� */
    p0.x = x;
    p0.y = y + length - WND_FRAME_SIZE;
    p1.x = x + width;
    p1.y = y + length;
    draw_rect(p0, p1, color);

    /* ����, ���� */
    p0.x = x;
    p0.y = y + WND_FRAME_SIZE;
    p1.x = x + WND_FRAME_SIZE;
    p1.y = y + length - WND_FRAME_SIZE;
    draw_rect(p0, p1, color);

    /* ����, ���� */
    p0.x = x + width - WND_FRAME_SIZE;
    p0.y = y + WND_FRAME_SIZE;
    p1.x = x + width;
    p1.y = y + length - WND_FRAME_SIZE;
    draw_rect(p0, p1, color);

    /* �������� */
    color = VGA_COLOR_WHITE;

    /* �Ϻ�, �������� */
    p0.x = (x) + 1;
    p0.y = (y) + 1;
    p1.x = (x + width) - 1;
    p1.y = (y + WND_FRAME_SIZE) - 1;
    draw_rect(p0, p1, color);

    /* �º�, �������� */
    p0.x = (x) + 1;
    p0.y = (y + length - WND_FRAME_SIZE) + 1;
    p1.x = (x + width) - 1;
    p1.y = (y + length) - 1;
    draw_rect(p0, p1, color);

    /* ����, �������� */
    p0.x = (x) + 1;
    p0.y = (y) + 1;
    p1.x = (x) + WND_FRAME_SIZE - 1;
    p1.y = (y) + length - 1;
    draw_rect(p0, p1, color);

    /* ����, �������� */
    p0.x = (x + width - WND_FRAME_SIZE) + 1;
    p0.y = (y) + 1;
    p1.x = (x + width) - 1;
    p1.y = (y + length) - 1;
    draw_rect(p0, p1, color);
}

/***************************************************************
 * description : ��ʾ����
 * history     :
 ***************************************************************/
os_ret OS_API show_window(IN HWINDOW hwnd)
{
    struct window_attr *window_attr;

    /* ��μ�� */
    if (OS_NULL != hwnd) {
        window_attr = &window_class_tab[((struct window_handle *) hwnd)->window_id]->attr;
        if (OS_TRUE == window_attr->wframe) {
            /* ����߿� */
            win_draw_frame(window_attr->csys.x, window_attr->csys.y, window_attr->width, window_attr->length);
            /* ���ڱ߿� */
            win_draw_frame(window_attr->csys.x+WND_FRAME_SIZE, window_attr->csys.y+(TITLE_HIGH+WND_FRAME_SIZE), window_attr->width-WND_FRAME_SIZE-WND_FRAME_SIZE, window_attr->length-TITLE_HIGH-2*WND_FRAME_SIZE);
            /* �������� */
            show_title_bar(window_attr, VGA_COLOR_BLUE);
            /* ��ʾ���� */
            show_title(window_attr, VGA_COLOR_SILVER);
            /* �����ڱ��� */
            win_draw_background(window_attr);
        }
        return OS_SUCC;
    }
    /* ��ǰ���ھ����Ч */
    return OS_FAIL;
}

/***************************************************************
 * description : �ػ�����
 * history     :
 ***************************************************************/
os_ret OS_API repaint_window(IN HWINDOW hwnd)
{
    struct window_attr *window_attr;

    /* ��μ�� */
    if (OS_NULL != hwnd) {
        if (current_window_handle == hwnd) {
            window_attr = &window_class_tab[((struct window_handle *) hwnd)->window_id]->attr;
            if (OS_TRUE == window_attr->wframe) {
                /* ����߿� */
                win_draw_frame(window_attr->csys.x, window_attr->csys.y, window_attr->width, window_attr->length);
                /* ���ڱ߿� */
                win_draw_frame(window_attr->csys.x+WND_FRAME_SIZE, window_attr->csys.y+(TITLE_HIGH+WND_FRAME_SIZE), window_attr->width-WND_FRAME_SIZE-WND_FRAME_SIZE, window_attr->length-TITLE_HIGH-2*WND_FRAME_SIZE);
                /* �������� */
                show_title_bar(window_attr, VGA_COLOR_NAVY);
                /* ��ʾ���� */
                show_title(window_attr, VGA_COLOR_WHITE);
                /* �����ڱ��� */
                win_draw_background(window_attr);
            }
        } else {
            show_window(hwnd);
        }
        return OS_SUCC;
    }
    /* ��μ��ʧ�� */
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API update_window(IN HWINDOW hwnd)
{
    struct message *msg;

    /* ��μ�� */
    if (OS_NULL != hwnd) {
        msg = alloc_msg(sizeof(struct message));
        if (OS_NULL == msg) {
            /* ��Ϣ�ڴ����ʧ��, ������ */
            return OS_FAIL;
        }

        /* ������Ϊisr */
        msg->msg_name = OS_MSG_PAINT;
        msg->msg_len = sizeof(struct message);

        post_msg(hwnd, msg);
        return OS_SUCC;
    }
    /* ��μ��ʧ�� */
    return OS_FAIL;
}

/***************************************************************
 * description : ��������
 * history     :
 ***************************************************************/
LOCALC os_ret init_instance(os_u8 *app_name)
{
    HWINDOW handle;

    /* ��μ�� */
    if (OS_NULL != app_name) {
        /* create window */
        handle = create_window(app_name);
        if (OS_NULL == handle) {
            /* panic */
            return OS_FAIL;
        }

        /* show window��ʾ���� */
        show_window(handle);

        /* �����߳���Ϣ���� */
        active_task_station(current_task_handle());

        /* update window�ͳ�WM_PAINT */
        update_window(handle);

        return OS_SUCC;
    }

    /* ��μ��ʧ�� */
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
    /* ��ǰ����Ϊ���� */
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
 * description : ��ʼ�����ھ����Դ��
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
 * description : ��Դid
 * history     :
 ***************************************************************/
LOCALC os_u32 lookup_application_id(IN os_u8 *app_name)
{
    os_u32 i;
    struct window_class **t;

    for (i = 0, t = window_class_tab; i < MAX_WINDOW_NUM; i++, t++) {
        if ((OS_NULL != (*t)) && (0 == str_cmp(app_name, (*t)->app_name))) {
            /* �ҵ���� */
            return i;
        }
    }
    /* û���ҵ� */
    return INVALID_WINDOW_ID;
}

/***************************************************************
 * description : ��������
 * history     :
 ***************************************************************/
HWINDOW OS_API create_window(IN os_u8 *app_name)
{
    os_u32 window_rc_id;

    /* ��μ�� */
    if (OS_NULL != app_name) {
        spin_lock(&window_lock);

        /* ����application name */
        window_rc_id = lookup_application_id(app_name);
        if (INVALID_WINDOW_ID == window_rc_id) {
            spin_unlock(&window_lock);

            /* ע����Ϣ��û���ҵ� */
            return OS_NULL;
        }

        window_handle_tab[window_rc_id].htask = current_task_handle();
        window_handle_tab[window_rc_id].window_id = window_rc_id;

        spin_unlock(&window_lock);

        return (HWINDOW) &window_handle_tab[window_rc_id];
    }

    /* ��μ��ʧ�� */
    return OS_NULL;
}

/***************************************************************
 * description : ����appname���Ҵ��ھ��
 * history     :
 ***************************************************************/
HWINDOW OS_API lookup_handle_by_app_name(IN os_u8 *app_name)
{
    os_u32 window_rc_id;

    /* ��μ�� */
    if (OS_NULL != app_name) {
        /* ����application name */
        window_rc_id = lookup_application_id(app_name);
        if (INVALID_WINDOW_ID == window_rc_id) {
            /* ע����Ϣ��û���ҵ� */
            return OS_NULL;
        }
        return (HWINDOW) &window_handle_tab[window_rc_id];
    }
    /* ��μ��ʧ�� */
    return OS_NULL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void init_idle_window_rc(os_void)
{
    os_u32 i;

    /* ���ȫ�ֱ���idle_task_id */
    mem_set(idle_window_rc, 0, MAX_WINDOW_NUM * sizeof(os_u32));

    /* ��ʼ��idle_task_entity */
    for (i=1; i<MAX_WINDOW_NUM-1; i++) {
        idle_window_rc[i] = i+1;
    }

    /* ��β */
    idle_window_rc[MAX_WINDOW_NUM-1] = INVALID_WINDOW_ID;

    /* ��һ��Ҫ����task_entity��λ�� */
    idle_window_rc[0] = 1;
}

/***************************************************************
 * description : ���䴰����Դ
 * history     :
 ***************************************************************/
LOCALC os_u32 alloc_window_rc(os_void)
{
    os_u32 window_id;

    /* �����ж�(INVALID_WINDOW_ID != idle_window_rc[0]) */
    if (MAX_WINDOW_NUM > idle_window_rc[0]) {
        window_id = idle_window_rc[0];
        idle_window_rc[0] = idle_window_rc[window_id];
        return window_id;
    }
    /* ��Դ������� */
    return INVALID_WINDOW_ID;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void free_window_rc(os_u32 window_id)
{
    os_u32 idle_head_id;

    /* �����task id�ڷ�Χ֮�� */
    if (MAX_WINDOW_NUM > window_id) {
        idle_head_id = idle_window_rc[0];
        idle_window_rc[0] = window_id;
        idle_window_rc[window_id] = idle_head_id;
    }
}

/***************************************************************
 * description :
 * history     : ȡ���ṹ��ֱ�Ӹ�ֵ, ��ĳЩ������ں˱���
 ***************************************************************/
os_ret OS_API register_window_class(IN struct window_class *window_class)
{
    struct window_class *task_win_mem;
    os_u32 window_rc_id;

    if (OS_NULL != window_class) {
        /* ���䴰����Դ */
        window_rc_id = alloc_window_rc();
        if (INVALID_WINDOW_ID == window_rc_id) {
            /* ���䴰����Դʧ�� */
            return OS_FAIL;
        }

        /* �����ں��ڴ洴��������Ϣ */
        task_win_mem = kmalloc(sizeof(struct window_class));
        if (OS_NULL == task_win_mem) {
            return OS_FAIL;
        }

        /* �ṹ�����͸�ֵ */
        //*task_win_mem = *window_class;
        //debug. �ڽṹ��ߴ����ĳ��ֵʱ�����ϵͳ����memcpy����. ���������ں˱���.
        mem_cpy(task_win_mem, window_class, sizeof(struct window_class));

        /* ��¼ */
        window_class_tab[window_rc_id] = task_win_mem;

        return OS_SUCC;
    }

    /* ��μ��ʧ�� */
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API register_static_window_class(IN struct window_class *window_class)
{
    os_u32 window_rc_id;

    /* ��μ�� */
    if (OS_NULL != window_class) {
        /* ���䴰����Դ */
        window_rc_id = alloc_window_rc();
        if (INVALID_WINDOW_ID == window_rc_id) {
            /* ���䴰����Դʧ�� */
            return OS_FAIL;
        }

        /* ��¼ */
        window_class_tab[window_rc_id] = (struct window_class *) window_class;
        return OS_SUCC;
    }
    /* ��μ��ʧ�� */
    return OS_FAIL;
}

/***************************************************************
 * description : ע�ᴰ����
 * history     :
 ***************************************************************/
LOCALC os_ret init_application(struct window_class *window_class)
{
    /* window_class���û���д, ����������� */
    /* ע�ᴰ�� */
    register_static_window_class(window_class);
    return OS_SUCC;
}

/***************************************************************
 * description : ��ȡ��ǰ�̵߳���Ϣ, ���߳�Ϊ��λ(���Ǵ��ھ��)
 * history     :
 ***************************************************************/
os_ret OS_API get_message(INOUT os_void **msg)
{
    os_ret result;

    /* ��μ�� */
    if (OS_NULL != msg) {
        for (;;) {
            /* receive_msg����ִ��pend���� */
            result = receive_msg(current_task_handle(), msg);
            if (OS_SUCC != result) {
                return OS_FAIL;
            }

            if (OS_NULL != *msg) {
                return OS_SUCC;
            }

            /* ��Ϣ���д�����Ϣ, ׼����ȡ */
        }
        return OS_SUCC;
    }
    /* ��μ��ʧ�� */
    return OS_FAIL;
}

/***************************************************************
 * description : �ַ���Ϣ���̸߳������ڷֱ���
 * history     :
 ***************************************************************/
os_ret OS_API dispatch_msg(IN os_void *msg)
{
    os_u32 window_id;
    struct window_handle *handle;

    /* ��μ�� */
    if (OS_NULL != msg) {
        /* ������Ϣ��ȡ��� */
        handle = get_dest_hwindow(msg);

        window_id = handle->window_id;

        /* window id�Ϸ� */
        if (MAX_WINDOW_NUM > window_id) {
            /* ��Ϣ�������Ѿ�ע�� */
            if ((OS_NULL != window_class_tab[window_id]) && (OS_NULL != window_class_tab[window_id]->msg_proc)) {
                /* ��Ϣ����ص� */
                (*window_class_tab[window_id]->msg_proc)(msg);

                /* ��Ϣ���� */
                if (OS_NULL != hook) {
                    (*hook)(msg);
                }

                /* �ͷ���Ϣ�ڴ� */
                free_msg(msg);
                return OS_SUCC;
            }
        }
        /* �ͷ���Ϣ�ڴ� */
        free_msg(msg);
    }
    /* ��μ��ʧ��, window_id & window_class_tab */
    return OS_FAIL;
}

/***************************************************************
 * description : �û������̺߳���,
 *               TASK_FUNC_PTR��������
 * history     :
 ***************************************************************/
LOCALC os_ret OS_CALLBACK window_task_func(os_u32 arg1, os_u32 arg2, os_u32 arg3, os_u32 arg4, os_u32 arg5, os_u32 arg6, os_u32 arg7)
{
    os_void *msg;
    struct window_class *window_class;

    /* ����1����� */
    window_class = (struct window_class *) arg1;

    /* ע�ᴰ���� */
    init_application(window_class);

    /* �������� */
    init_instance(window_class->app_name);

    if (OS_NULL != window_class->app_init) {
        /* �û������ʼ������(ͬ�����ڳ�ʼ��) */
        window_class->app_init();
    }

    /* ��Ϣѭ�� */
    while (OS_SUCC == get_message(&msg)) {
        /* ��Ϣ���� */
        dispatch_msg(msg);
    }
    flog("task error %d!\n", __LINE__);
    return OS_SUCC;
}

/***************************************************************
 * description : �����û������߳�
 * history     :
 ***************************************************************/
HTASK OS_API create_ui_task(IN os_u8 *name, IN struct window_class *window_class, enum task_priority priority)
{
    struct window_class *window_class_mem;

    /* name����� */

    if (OS_NULL != window_class) {
        /* �����ں��ڴ洴��������Ϣ */
        window_class_mem = kmalloc(sizeof(struct window_class));
        if (OS_NULL == window_class_mem) {
            return OS_NULL;
        }

        /* �ṹ�����͸�ֵ */
        mem_cpy(window_class_mem, window_class, sizeof(struct window_class));

        /* arg1 = window_class */
        return create_task(name, window_task_func, priority, (pointer) window_class_mem, 0, 0, 0, 0, 0, 0);
    }
    /* ��μ��ʧ�� */
    return OS_NULL;
}

/***************************************************************
 * description : �õ�����Ĵ�������
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
 * description : ��ȡ�����豸������
 *               handle device context
 * history     :
 ***************************************************************/
HDEVICE OS_API open_hdc(IN HWINDOW hwnd)
{
    struct window_handle *window_handle;
    struct device_context *device_handle;
    struct window_attr *attr;

    /* ��μ�� */
    if (OS_NULL != hwnd) {
        window_handle = hwnd;

        /* window class�Ѿ�ע�� */
        if (OS_NULL != window_class_tab[window_handle->window_id]) {
            /* �����ں��ڴ�ʹ�� */
            device_handle = kmalloc(sizeof(struct device_context));
            if (OS_NULL == device_handle) {
                /* �ں��ڴ����ʧ�� */
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
    /* ��μ��ʧ�� | �ô���û��ע�� */
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

    /* ��μ�� */
    if (OS_NULL != hwnd) {
        window_handle = hwnd;

        /* window class�Ѿ�ע�� */
        if (OS_NULL != window_class_tab[window_handle->window_id]) {
            window_class_tab[window_handle->window_id]->attr.width = width;
            return OS_SUCC;
        }
    }
    /* ��μ��ʧ�� | �ô���û��ע�� */
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

    /* ��μ�� */
    if (OS_NULL != hwnd) {
        window_handle = hwnd;

        /* window class�Ѿ�ע�� */
        if (OS_NULL != window_class_tab[window_handle->window_id]) {
            window_class_tab[window_handle->window_id]->attr.length = length;
            return OS_SUCC;
        }
    }
    /* ��μ��ʧ�� | �ô���û��ע�� */
    return OS_FAIL;
}

/***************************************************************
 * description : �ͷ��豸������
 * history     :
 ***************************************************************/
os_ret OS_API close_hdc(IN HWINDOW hwnd, IN HDEVICE hdc)
{
    struct window_handle *window_handle;
    struct device_context *device_handle;

    /* ��μ�� */
    if ((OS_NULL != hwnd) && (OS_NULL != hdc)) {
        window_handle = hwnd;

        /* �ô����Ѿ�ע�� */
        if (OS_NULL != window_class_tab[window_handle->window_id]) {
            device_handle = hdc;

            /* ˢ�´����������� */
            window_class_tab[window_handle->window_id]->attr.cursor_pos = device_handle->cursor_pos;

            kfree(hdc);
            return OS_SUCC;
        }
    }
    /* ��μ��ʧ�� | �ô���û��ע�� */
    return OS_FAIL;
}

/***************************************************************
 * description : ���ñ�����ɫ
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
 * description : �õ����������С
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
 * description : �ͷŴ�����Դ
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

    /* task id�Ϸ� */
    if ((INVALID_TASK_ID != task_id) && (OS_TASK_NUM > task_id)) {
        /* �����������ھ���� */
        for (i = 0, t = window_handle_tab; i < MAX_WINDOW_NUM; i++, t++) {
            if (task_id == t->htask->task_id) {
                window_id = t->window_id;

                /* �ͷ�ע�ᴰ������Դ */
                window_class = window_class_tab[window_id];

                destroy_window(window_class);

                kfree(window_class);

                window_class_tab[window_id] = OS_NULL;

                /* �ͷŴ���id��Դ */
                free_window_rc(window_id);

                t->htask = OS_NULL;
                t->window_id = INVALID_WINDOW_ID;

                return OS_SUCC;
            }
        }
    }
    /* ��μ��ʧ��(task_id)|û����������� */
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
 * description : �޸ĵ�ǰ���ھ��
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
 * description : ���ݴ��ھ��������Ϣ������ָ��
 * history     :
 ***************************************************************/
MSG_PROC_PTR OS_API get_msgproc(IN HWINDOW handle)
{
    struct window_handle *hwnd;

    /* ��μ�� */
    if (OS_NULL != handle) {
        hwnd = (struct window_handle *) handle;

        /* window id�Ϸ� */
        if (MAX_WINDOW_NUM > hwnd->window_id) {
            return window_class_tab[hwnd->window_id]->msg_proc;
        }
    }
    /* ��μ��ʧ�� | window id���Ϸ� */
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

