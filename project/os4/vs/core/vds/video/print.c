/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : print.c
 * version     : 1.0
 * description : (key)
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include "print.h"

/***************************************************************
 global variable declare
 ***************************************************************/
LOCALD const os_u8 hex_template[17] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

/* 打印窗口设备句柄上下文 */
LOCALD struct device_context msg_log_hdc;

/* x轴分辨率 */
LOCALD volatile os_u32 print_resolution_x _CPU_ALIGNED_ = 0;

/* y轴分辨率 */
LOCALD volatile os_u32 print_resolution_y _CPU_ALIGNED_ = 0;

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description : count为有符号数
 *               该函数有几处可能产生除0操作
 * history     :
 ***************************************************************/
LOCALC os_void update_cursor(struct device_context *hdc, os_s32 count)
{
    cursor_csys *cursor;
    os_u32 char_num;

    /* 后台任务 */
    if (OS_NULL != hdc) {
        /* 光标的逻辑坐标 */
        cursor = &hdc->cursor_pos;

        /* 每行字符数量 */
        char_num = hdc->width / hdc->font_size; //参数检查, 防止除零.
        cursor->y = (cursor->x + count) / char_num + cursor->y;
        cursor->x = (cursor->x + count) % char_num; /* 这句对上一句有影响, 置后 */
    }
}

/***************************************************************
 * description : 光标的屏幕坐标系
 * history     :
 ***************************************************************/
LOCALC screen_csys cursor_screen_csys(struct device_context *hdc)
{
    cursor_csys *cursor;
    screen_csys scsys;
    os_u32 font_size;

    if (OS_NULL != hdc) {
        /* 光标的逻辑坐标 */
        cursor = &hdc->cursor_pos;

        /* 窗口的屏幕坐标系 */
        scsys = hdc->csys;

        /* 当前窗口的字体大小 */
        font_size = hdc->font_size;

        scsys.x = scsys.x + font_size * cursor->x;

        //scsys.y = scsys.y + font_size*2*cursor->y;
        scsys.y = scsys.y + (font_size + font_size) * cursor->y;
        /* 只检查输入, 不检查输出 */
        return scsys;
    }

    /* 后台任务, 不打印 */
    scsys.x = print_resolution_x;
    scsys.y = print_resolution_y;
    return scsys;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret print_char(os_u8 character, screen_csys p, enum vga_color color)
{
    /* 坐标检查失败不打印 */
    if ((p.y + 2 * msg_log_hdc.font_size) < print_resolution_y) {
        print_asc(character, p, color);
        update_cursor(&msg_log_hdc, 1);
        return OS_SUCC;
    }
    /* y坐标溢出 */
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void print_str(os_u8 *string, enum vga_color color)
{
    /* 坐标检查失败不打印 */
    while (*string) {
        print_char(*string, cursor_screen_csys(&msg_log_hdc), color);
        string++;
    }
}

/***************************************************************
 * description : 显示十六进制格式
 * history     :
 ***************************************************************/
LOCALC os_void print_int(os_u32 integer, enum vga_color color)
{
    os_s32 i = 0;
    os_u32 pos = 0;

    /* 坐标检查失败不打印 */
    for (i = 7; i > -1; i--) {
        pos = integer >> (i * 4);
        pos = pos & 0x0f;
        print_char(hex_template[pos], cursor_screen_csys(&msg_log_hdc), color);
    }
}

/***************************************************************
 * description : 十进制格式
 * history     :
 ***************************************************************/
LOCALC os_void print_dec(os_u32 integer, enum vga_color color)
{
    os_u32 i;
    os_u32 temp;
    os_u32 flag = 1;

    /* 坐标检查失败不打印 */
    for (i = 1000000000; i > 1; i = i / 10) {
        temp = integer / i;
        if ((0 != temp) || (0 == flag)) {
            flag = 0;
            integer -= (temp * i);
            print_char('0' + temp, cursor_screen_csys(&msg_log_hdc), color);
        }
    }
    print_char('0' + integer, cursor_screen_csys(&msg_log_hdc), color);
}

/***************************************************************
 * description : 返回失败, 则屏幕越界
 * history     :
 ***************************************************************/
LOCALC os_ret win_print_char(struct device_context *hdc, os_u8 character, screen_csys p, enum vga_color color)
{
    if ((p.y + 2 * hdc->font_size) < (hdc->csys.y + hdc->length)) {
        /* y坐标未溢出 */
        print_asc(character, p, color);
        update_cursor(hdc, 1);
        return OS_SUCC;
    } /* y坐标溢出 */
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void win_print_str(struct device_context *hdc, os_u8 *string, enum vga_color color)
{
    /* 坐标检查失败不打印 */
    while (*string) {
        win_print_char(hdc, *string, cursor_screen_csys(hdc), color);
        string++;
    }
}

/***************************************************************
 * description : 显示十六进制格式
 * history     :
 ***************************************************************/
LOCALC os_void win_print_int(struct device_context *hdc, os_u32 integer, enum vga_color color)
{
    os_s32 i = 0;
    os_u32 pos = 0;

    /* 坐标检查失败不打印 */
    for (i = 7; i > -1; i--) {
        pos = integer >> (i * 4);
        pos = pos & 0x0f;
        win_print_char(hdc, hex_template[pos], cursor_screen_csys(hdc), color);
    }
}

/***************************************************************
 * description : 十进制格式
 * history     :
 ***************************************************************/
LOCALC os_void win_print_dec(struct device_context *hdc, os_u32 integer, enum vga_color color)
{
    os_u32 i;
    os_u32 temp;
    os_u32 flag = 1;

    /* 坐标检查失败不打印 */
    for (i = 1000000000; i > 1; i = i / 10) {
        temp = integer/i;
        if ((0 != temp) || (0 == flag)) {
            flag = 0;
            integer -= (temp * i);
            win_print_char(hdc, '0' + temp, cursor_screen_csys(hdc), color);
        }
    }
    win_print_char(hdc, '0' + integer, cursor_screen_csys(hdc), color);
}

/***************************************************************
 * description : 文本退格键处理
 * history     :
 ***************************************************************/
os_ret OS_API win_backspace(IN HDEVICE hdc)
{
    struct device_context *device_handle;
    screen_csys p0, p1;

    /* 入参检查 */
    if (OS_NULL != hdc) {
        device_handle = hdc;
        if (device_handle->cursor_pos.x) {
            update_cursor(device_handle, -1);
            p0 = cursor_screen_csys(device_handle);
            p1.x = p0.x + device_handle->font_size;
            p1.y = p0.y + device_handle->font_size * 2;
            draw_rect(p0, p1, device_handle->background_color);
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
os_void eraser(os_u32 pos)
{
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void reset_cursor(struct device_context *hdc)
{
    cursor_csys *cursor;

    cassert(OS_NULL != hdc);

    /* 光标的逻辑坐标 */
    cursor = &hdc->cursor_pos;
    cursor->y = 0;
    cursor->x = 0;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void deal_newline(struct device_context *hdc)
{
    cursor_csys *cursor;

    /* 光标的逻辑坐标 */
    cursor = &hdc->cursor_pos;
    cursor->x = 0;
    cursor->y++;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void print_buffer(HDEVICE hdc, const os_u8 *format, os_u32 *args)
{
    enum vga_color color;
    os_u32 i = 0;

    /* 前景颜色 */
    color = ((struct device_context *) hdc)->foreground_color;

    while (format[i]) {
        switch (format[i]) {
        case '%':
            i++;
            switch (format[i]) {
            case 'd':
                win_print_dec(hdc, *args, color);
                args++;
                i++;
                break;
            case 'x':
                win_print_int(hdc, *args, color);
                args++;
                i++;
                break;
            case 's':
                win_print_str(hdc, (os_u8 *)*args, color);
                args++;
                i++;
                break;
            case 'c':
                win_print_char(hdc, *args, cursor_screen_csys(hdc), color);
                args++;
                i++;
                break;
            default:
                win_print_char(hdc, '%', cursor_screen_csys(hdc), color);
                break;
            }
            break;
        case '\n':
            deal_newline(hdc);
            i++;
            break;
        case '\t':
            update_cursor(hdc, 4);
            i++;
            break;
        default:
            win_print_char(hdc, format[i], cursor_screen_csys(hdc), color);
            i++;
            break;
        }
    }
}

/***************************************************************
 * description : text out
 * history     :
 ***************************************************************/
os_void __cdecl kprint(IN HDEVICE hdc, IN os_u8 *format, ...)
{
    GLOBALDIF HEVENT cs_printk = OS_NULL; // not in map file
    os_u32 *arg_list;

    /* 入参检查失败 */
    cassert((OS_NULL != hdc) && (OS_NULL != format));

    if (OS_NULL == cs_printk) {
        create_critical_section(&cs_printk, __LINE__);
    }

    /* 初始化参数列表, kprintf(hdc, "xxx", arg1, arg2, arg3), 入栈顺序arg3, arg2, arg1 */
    arg_list = (os_u32 *) &format + 1;

    /* 进入临界区 */
    enter_critical_section(cs_printk);

    /* 打印 */
    print_buffer(hdc, format, arg_list);

    /* 离开临界区 */
    leave_critical_section(cs_printk);
}

#include <stdarg.h>
/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret vprint_buffer(const os_u8 *format, va_list args)
{
    os_u32 i;
    enum vga_color color;

    cassert(OS_NULL != format);

    i = 0;

    /* 前景颜色 */
    color = msg_log_hdc.foreground_color;

    while (format[i]) {
        switch (format[i]) {
        case '%':
            i++;
            switch (format[i]) {
            case 'd':
                print_dec(va_arg(args, os_u32), color);
                i++;
                break;
            case 'x':
                print_int(va_arg(args, os_u32), color);
                i++;
                break;
            case 's':
                /* 取得字符串首地址 */
                print_str((os_u8 *) va_arg(args, os_u32), color);
                i++;
                break;
            case 'c':
                /* 取参数 */
                print_char(va_arg(args, os_u32), cursor_screen_csys(&msg_log_hdc), color);
                i++;
                break;
            default:
                print_char('%', cursor_screen_csys(&msg_log_hdc), color);
                break;
            }
            break;
        case '\n':
            deal_newline(&msg_log_hdc);
            i++;
            break;
        case '\t':
            update_cursor(&msg_log_hdc, 4);
            i++;
            break;
        default:
            print_char(format[i], cursor_screen_csys(&msg_log_hdc), color);
            i++;
            break;
        }
    }

    /* auto clear and print */
    if (0) {
        screen_csys pos;

        pos = cursor_screen_csys(&msg_log_hdc);
        if ((pos.y + 2 * msg_log_hdc.font_size) >= print_resolution_y) {
            clear_screen(VGA_COLOR_BLACK);
            init_print();
        }
    }
    return OS_SUCC;
}

LOCALD enum flog_redirect log_file_direct = FLOG_DISK;

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
void OS_API direct_log_file(enum flog_redirect dest)
{
    if (FLOG_BOTTOM >= dest) {
        log_file_direct = dest;
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_bool ascii(os_u8 c)
{
    /* 0a:LF, 0b:VT, 0c:FF, 0d:CR */
    if ((0x0a == c) || (0x0b == c) || (0x0c == c) || (0x0d == c)) {
        return OS_TRUE;
    }
    if ((0x20 <= c) && (0x7e >= c)) {
        return OS_TRUE;
    }
    return OS_FALSE;
}

LOCALD os_u8 log_buffer[0x100000];
LOCALD os_u32 log_pos = 0;

LOCALD HEVENT log_sem_id = 0;

#define LOG_FILE "flog.txt"

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
void dump_log_file(void)
{
    enum flog_redirect flag;

    flag = log_file_direct;
    log_file_direct = FLOG_NONE;
    log_buffer[log_pos] = '\0';
    print(log_buffer);
    log_file_direct = flag;
}

LOCALD HTASK log_handle = OS_NULL;

/***************************************************************
 * description : 非阻塞型打印
 * history     :
 ***************************************************************/
os_ret __cdecl flog(const os_u8 *format, ...)
{
    va_list args;
    lock_t eflag;
    os_bool flag;

    flag = OS_FALSE;

    lock_int(eflag);

    /* 初始化参数列表 */
    va_start(args, format);
    if ((FLOG_DISK == log_file_direct) || (FLOG_BOTH == log_file_direct)) {
        if (sizeof(log_buffer) > log_pos) {
            log_pos += snprint(&log_buffer[log_pos], sizeof(log_buffer) - 1 - log_pos, format, args);
            if ((sizeof(log_buffer) - 1) <= log_pos) {
                flag = OS_TRUE;
            }
        }
    }
    if ((FLOG_SCREEN == log_file_direct) || (FLOG_BOTH == log_file_direct)) {
        vprint_buffer(format, args);
    }
    /* 结束参数列表 */
    va_end(args);

    unlock_int(eflag);

    if (flag) {
        notify_event(log_sem_id, __LINE__);
    }

    return OS_SUCC;
}

LOCALD volatile os_bool wtlog_update _CPU_ALIGNED_;

/***************************************************************
 * description : write through file log
 * history     :
 ***************************************************************/
os_void wtflog(os_void)
{
    if (log_handle) {
        wtlog_update = OS_FALSE;
        modify_task_priority(log_handle, TASK_PRIORITY_6);
        notify_event(log_sem_id, __LINE__);
        modify_task_priority(log_handle, LOG_TASK_PRIO);
    }
}

/***************************************************************
 * description : TASK_FUNC_PTR
 * history     :
 ***************************************************************/
LOCALC os_ret OS_CALLBACK log_entry_point(os_u32 arg1, os_u32 arg2, os_u32 arg3, os_u32 arg4, os_u32 arg5, os_u32 arg6, os_u32 arg7)
{
    os_u32 id;
    HFILE fp;
    lock_t eflag;
    os_ret ret;

    ret = OS_FAIL;
    for (;;) {
        wait_event(log_sem_id, 0x100);

        if ((FLOG_DISK == log_file_direct) || (FLOG_BOTH == log_file_direct)) {
            /* flush device id always */
            id = curr_disk_device_id();
            /* write to file */
            if (OS_FAIL != ret) {
                if (log_pos) {
                    lock_int(eflag);
                    if (log_pos) {
                        fp = open_file(id, LOG_FILE);
                        if (OS_NULL != fp) {
                            seek_file(fp, 0, SEEK_POS_END);
                            write_file(fp, log_buffer, log_pos);
                            log_pos = 0;
                            close_file(fp);
                        } else {
                            ret = OS_FAIL;
                            vprint_buffer("open flog file fail\n", OS_NULL);
                            //dump_log_file();
                        }
                    }
                    unlock_int(eflag);
                }
            } else {
                delete_file(id, LOG_FILE);
                ret = create_file(id, LOG_FILE);
            }
        }

        /* if the logfile or disk does not exist, write through is left in next time */
        wtlog_update = OS_TRUE;
    }
    cassert(OS_FALSE);
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void init_print_module(os_void)
{
    log_sem_id = create_event_handle(EVENT_INVALID, "log", __LINE__);
    cassert(OS_NULL != log_sem_id);

    log_handle = create_task("flog", log_entry_point, LOG_TASK_PRIO, 0, 0, 0, 0, 0, 0, 0);
    cassert(OS_NULL != log_handle);
}
init_module_call(init_print_module);

/***************************************************************
 * description : 初始化打印相关
 * history     :
 ***************************************************************/
os_void init_print(os_void)
{
    screen_csys r;

    r = current_resolution();

    print_resolution_x = r.x;
    print_resolution_y = r.y;

    msg_log_hdc.background_color = VGA_COLOR_BLACK;
    msg_log_hdc.csys.x = msg_log_hdc.csys.y = 0;
    msg_log_hdc.cursor_pos.x = msg_log_hdc.cursor_pos.y = 0;
    msg_log_hdc.font_size = FONT_SIZE;
    msg_log_hdc.foreground_color = VGA_COLOR_WHITE;
    msg_log_hdc.length = print_resolution_y;
    msg_log_hdc.width = print_resolution_x;
}

