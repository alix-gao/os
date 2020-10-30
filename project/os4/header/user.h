/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : user.h
 * version     : 1.0
 * description : ʹ���߽��溯��
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __USER_H__
#define __USER_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

#ifndef __TYPE_H__
    #error "include type.h before"
#endif

/***************************************************************
 macro define
 ***************************************************************/
/* ������Ϣ���������Ͷ��� */
typedef os_ret (*MSG_PROC_PTR)(IN os_void *msg);

/* ���ڱ������ֳ��� */
#define MAX_TITLE_NAME_LEN 0x10

/* ���ڳ������ֳ��� */
#define MAX_APP_NAME_LEN 0x10

/***************************************************************
 enum define
 ***************************************************************/

/***************************************************************
 * enum name   : vga_color_enum
 * description : ��ARGBģʽ����, ��ɫ��ģʽ��Ҫ�ڲ�ת��
 ***************************************************************/
enum vga_color {
    VGA_COLOR_BLACK     = 0x000000 | 0x0000 | 0x00, /* 0 */
    VGA_COLOR_NAVY      = 0x000000 | 0x0000 | 0x80, /* 1 */
    VGA_COLOR_GREEN     = 0x000000 | 0x8000 | 0x00, /* 2 */
    VGA_COLOR_TEAL      = 0x000000 | 0x8000 | 0x80, /* 3 */
    VGA_COLOR_MAROON    = 0x800000 | 0x0000 | 0x00, /* 4 */
    VGA_COLOR_PURPLE    = 0x800000 | 0x0000 | 0x80, /* 5 */
    VGA_COLOR_OLIVE     = 0x800000 | 0x8000 | 0x00, /* 6 */
    VGA_COLOR_SILVER    = 0xc00000 | 0xc000 | 0xc0, /* 7 */
    VGA_COLOR_GRAY      = 0x800000 | 0x8000 | 0x80, /* 8 */
    VGA_COLOR_BLUE      = 0x000000 | 0x0000 | 0xff, /* 9 */
    VGA_COLOR_LIME      = 0x000000 | 0xff00 | 0x00, /* 10 */
    VGA_COLOR_AQUA      = 0x000000 | 0xff00 | 0xff, /* 11 */
    VGA_COLOR_RED       = 0xff0000 | 0x0000 | 0x00, /* 12 */
    VGA_COLOR_FUCHSIA   = 0xff0000 | 0x0000 | 0xff, /* 13 */
    VGA_COLOR_YELLOW    = 0xff0000 | 0xff00 | 0x00, /* 14 */
    VGA_COLOR_WHITE     = 0xff0000 | 0xff00 | 0xff, /* 15 */
    VGA_COLOR_BUTT
};

/* ͸���� */
#define VGA_TRAN 0xff000000

/* ͼ�� */
#define ICON_ROW 32

/***************************************************************
 struct define
 ***************************************************************/
/***************************************************************
 * description : ƽ������ϵ
 ***************************************************************/
struct plane_csys {
    os_u32 x; /* ������ horizontal */
    os_u32 y; /* ������ vertical */
};

/***************************************************************
 * description : ��Ļ����ϵ
 ***************************************************************/
typedef struct plane_csys screen_csys;

/***************************************************************
 * description : �߼�����ϵ
 ***************************************************************/
typedef struct plane_csys logic_csys;

/***************************************************************
 * description : ͼ������ϵ
 ***************************************************************/
typedef struct plane_csys icon_csys;

/***************************************************************
 * description : �������
 ***************************************************************/
typedef struct plane_csys cursor_csys;

/***************************************************************
 * description :
 ***************************************************************/
struct window_attr {
    /* ���ڵı��� */
    os_u8 title[MAX_TITLE_NAME_LEN];
    /* ���ڵ���Ļ����ϵ */
    screen_csys csys;
    /* ���ڵ�ǰ����ɫ */
    enum vga_color foreground_color;
    /* ���ڵı�����ɫ */
    enum vga_color background_color;
    /* �����Ƿ���ڱ߿� */
    os_bool wframe;
    /* ���ڿ��,x */
    os_u32 width;
    /* ���ڳ���,y */
    os_u32 length;
    /* ����λ�� */
    cursor_csys cursor_pos;
    /* �ֺŴ�С */
    os_u32 font_size;
};

/***************************************************************
 * description : ����ע��
 ***************************************************************/
struct window_class {
    /* ���ڳ�������, ���� */
    os_u8 app_name[MAX_APP_NAME_LEN];
    /* �������� */
    struct window_attr attr;
    /* ���ڳ�ʼ������(ͬ������) */
    VOID_FUNCPTR app_init;
    /* ��Ϣ������ */
    MSG_PROC_PTR msg_proc;
};

/***************************************************************
 * description : ͼ������ݽṹ����
 ***************************************************************/
struct pix {
    os_u8 b, g, r, alpha;
} __attribute((aligned(1)));

/***************************************************************
 * description : ͼ������ݽṹ����
 ***************************************************************/
struct os_icon {
#define ICON_RGB24_16_32 1
#define ICON_RGB24_32_32 2
#define ICON_RGB32_16_32 3
#define ICON_RGB32_32_32 4
    os_u32 choice;
    union {
        const enum vga_color (*data_16_32_24)[16]; /* 16*32 */
        const enum vga_color (*data_32_32_24)[32]; /* 32*32 */
        const enum vga_color (*data_16_32_32)[16]; /* 16*32 */
        const enum vga_color (*data_32_32_32)[32]; /* 32*32 */
        /* other formate */
    } u;
};

/***************************************************************
 extern function
 ***************************************************************/
HTASK OS_API create_ui_task(IN os_u8 *name, IN struct window_class *window_class, enum task_priority priority);
HWINDOW OS_API create_window(IN os_u8 *app_name);

os_ret OS_API get_message(INOUT os_void **msg);
os_ret OS_API dispatch_msg(IN os_void *msg);

os_ret OS_API show_window(IN HWINDOW hwnd);
os_ret OS_API update_window(IN HWINDOW hwnd);
os_ret OS_API repaint_window(IN HWINDOW hwnd);

enum show_image_mode_type {
    SI_MODE_1, /* ƽ�� */
    SI_MODE_2, /* ���� */
    SI_MODE_BUTT
};
os_ret OS_API show_image(IN HWINDOW handle, IN os_u8 *file_name, enum show_image_mode_type mode);

HDEVICE OS_API open_hdc(IN HWINDOW hwnd);
os_ret OS_API close_hdc(IN HWINDOW hwnd, IN HDEVICE hdc);
os_u32 OS_API get_window_width(IN HDEVICE hdc);
os_ret OS_API set_window_width(IN HWINDOW hwnd, os_u32 width);
os_u32 OS_API get_window_length(IN HDEVICE hdc);
os_ret OS_API set_window_length(IN HWINDOW hwnd, os_u32 length);
os_void __cdecl kprint(IN HDEVICE hdc, IN os_u8 *format, ...);
os_ret __cdecl flog(const os_u8 *format, ...);

HWINDOW OS_API lookup_handle_by_app_name(IN os_u8 *app_name);

os_ret OS_API register_static_window_class(IN struct window_class *window_class);
os_ret OS_API register_window_class(IN struct window_class *window_class);
os_ret OS_API register_hook(IN MSG_PROC_PTR func);

os_ret OS_API modify_current_window_handle(IN HWINDOW handle);

HWINDOW OS_API get_current_window_handle(os_void);
MSG_PROC_PTR OS_API get_msgproc(IN HWINDOW handle);
struct window_attr OS_API *get_window_attr(IN HWINDOW handle);
os_void OS_API post_quit_msg(os_void);

os_ret OS_API win_clear_screen(IN HDEVICE hdc);
os_ret OS_API read_screen(IN screen_csys *sp, OUT enum vga_color (*data)[16], IN icon_csys *ips, IN icon_csys *ipe);
os_ret OS_API paint_icon(IN screen_csys *sp, IN struct os_icon *icon, IN icon_csys *ips, IN icon_csys *ipe);
os_ret OS_API win_eraser(IN HDEVICE hdc, logic_csys p0, logic_csys p1);
os_ret OS_API win_backspace(IN HDEVICE hdc);
screen_csys OS_API current_resolution(os_void);

os_ret OS_API read_disk(os_u32 device_id, os_u32 start_sector, os_u16 num, OUT os_u8 *buffer);
os_ret OS_API write_disk(os_u32 device_id, os_u32 start_sector, os_u16 num, OUT os_u8 *buffer);

os_void OS_API list_file(os_u32 device_id);
HFILE OS_API open_file(os_u32 device_id, IN os_u8 *file_name);
os_ret OS_API create_file(os_u32 device_id, IN os_u8 *file_name);
os_ret OS_API delete_file(os_u32 device_id, IN os_u8 *file_name);

/***************************************************************
 * enum name   :
 * description :
 ***************************************************************/
enum file_seek {
    SEEK_POS_SET = 0,
    SEEK_POS_CUR = 1,
    SEEK_POS_END = 2,
    SEEK_POS_BUT
};
os_ret OS_API seek_file(HFILE file_handle, os_s32 offset, enum file_seek pos);
os_ret OS_API read_file(HFILE file_handle, os_u8 *buffer, os_u32 len);
os_ret OS_API write_file(HFILE file_handle, os_u8 *buffer, os_u32 len);
os_ret OS_API flush_file(HFILE file_handle);
os_bool OS_API end_of_file(HFILE file_handle);
os_u32 OS_API size_of_file(HFILE file_handle);
os_ret OS_API close_file(HFILE handle);
HDIR OS_API open_dir(os_u32 device_id, IN os_u8 *dir_name);

enum flog_redirect {
    FLOG_DISK = 0,
    FLOG_SCREEN = 1,
    FLOG_BOTH = 2,
    FLOG_NONE = 3,
    FLOG_BOTTOM
};
void OS_API direct_log_file(enum flog_redirect dest);

#pragma pack()

#endif /* end of header */

