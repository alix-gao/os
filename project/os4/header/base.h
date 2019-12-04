/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : base.h
 * version     : 1.0
 * description : kernel函数
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __BASE_H__
#define __BASE_H__

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
/* function */
typedef os_void (*VOID_FUNCPTR)(os_void);

/* 中断处理函数类型定义 */
typedef os_ret (OS_CALLBACK *TASK_FUNC_PTR)(os_u32 arg1, os_u32 arg2, os_u32 arg3, os_u32 arg4, os_u32 arg5, os_u32 arg6, os_u32 arg7);

/* 定时器回调函数类型定义 */
typedef os_ret (OS_CALLBACK *TIMER_FUNC_PTR)(os_u32 event_id);

/* main函数类型定义 */
typedef os_ret (*MAIN_FUNC_PTR)(os_u32 argc, os_u8 *argv[]);

/* function */
typedef os_void (*IRQ_FUNCPTR)(os_u32 irq);

#define OS_MSG_VALUE UINT32_C(0x80000000)

/***************************************************************
 enum define
 ***************************************************************/
/***************************************************************
 * enum name   : task_priority
 * description : 任务优先级, 7为最高, 0为最低.
 ***************************************************************/
enum task_priority {
    TASK_PRIORITY_0 = 0, /* 系统idle任务 */
    TASK_PRIORITY_1 = 1, /* watchdog */
    TASK_PRIORITY_2 = 2, /* log */
    TASK_PRIORITY_3 = 3, /* ... */
    TASK_PRIORITY_4 = 4, /* shell */
    TASK_PRIORITY_5 = 5, /* desktop */
    TASK_PRIORITY_6 = 6, /* 内核高优先级任务 */
    TASK_PRIORITY_7 = 7, /* 中断级别任务 */
    TASK_PRIORITY_CNT
};

#define LOWEST_TASK_PRIORITY TASK_PRIORITY_0
#define INVALID_TASK_PRIORITY TASK_PRIORITY_CNT

/***************************************************************
 * enum name   : timer_mode_enum
 * description : 定时器方式
 ***************************************************************/
enum timer_mode {
    TIMER_MODE_LOOP = 0,
    TIMER_MODE_NOT_LOOP = 1,
    TIMER_MODE_BUTT
};

/***************************************************************
 * enum name   : keyboard_type_enum
 * description :
 ***************************************************************/
enum keyboard_type {
    KEYBOARD_DOWN,
    KEYBOARD_UP,
    KEYBOARD_TYPE_BUTT
};

/***************************************************************
 * enum name   : os_message_enum
 * description : 系统内消息定义
 ***************************************************************/
enum os_message {
    OS_MSG_EXIT         = OS_MSG_VALUE | 0x0000, /* exit message */
    OS_MSG_TIMER        = OS_MSG_VALUE | 0x0001, /* 定时器消息 */
    OS_MSG_TASK         = OS_MSG_VALUE | 0x0002, /* task间消息 */
    OS_MSG_KEYBOARD     = OS_MSG_VALUE | 0x0003, /* 键盘消息 */
    OS_MSG_MOUSE        = OS_MSG_VALUE | 0x0004, /* 鼠标消息 */
    OS_MSG_PAINT        = OS_MSG_VALUE | 0x0005, /* 窗口更新消息 */
    OS_MSG_BUTT
};

/***************************************************************
 struct define
 ***************************************************************/
/***************************************************************
 * description : 消息格式
 ***************************************************************/
struct message {
    enum os_message msg_name;
    os_u32 msg_len;
};

/***************************************************************
 * description : 键盘消息
 ***************************************************************/
struct keyboard_msg {
    os_u32 msg_name;
    os_u32 msg_len;

    enum keyboard_type up_down; /* 按键类型 */
    os_u8 asc; /* 键盘ascII码 */
    os_u8 align[2];
};

/***************************************************************
 * description : 鼠标消息
 ***************************************************************/
struct mouse_msg {
    os_u32 msg_name;
    os_u32 msg_len;

    os_u8 left, right, mid, rsv[1];
    os_s32 x, y, wheel;
};

/***************************************************************
 * description : 任务快照
 ***************************************************************/
struct task_snapshot {
    /* 固定指向 */
    os_u8 *name;
    /* 固定指向 */
    HTASK handle;
    /* last tick */
    os_u32 tick;
    /* cpu */
    os_u32 usage;
};

/***************************************************************
 extern function
 ***************************************************************/
os_void OS_API *alloc_kdm(os_u32 size, os_u32 align, os_u32 line, IN os_u8 *file_name);
os_void OS_API *free_kdm(INOUT os_void **addr, os_u32 line);

os_void OS_API *alloc_coherent_mem(os_u32 size, os_u32 align, os_u32 line);
os_void OS_API *free_coherent_mem(os_void **addr, os_u32 line);

typedef os_void *klm_handle;
os_ret OS_API create_klm(klm_handle *handle, os_u32 line_no);
os_ret OS_API add_klm(IN klm_handle handle, os_u32 size, os_u32 line_no);
os_u32 OS_API klm_size(IN klm_handle handle);
os_ret OS_API write_klm(IN klm_handle handle, os_u32 offset, os_u8 *data, os_u32 size);
os_ret OS_API read_klm(IN klm_handle handle, os_u32 offset, os_u8 *data, os_u32 size);
os_u8 OS_API *loop_klm(IN klm_handle handle, IN os_u8 *last_addr, os_u32 *next_size);
os_u8 OS_API *klm_addr(IN klm_handle handle, os_u32 offset, os_u32 size);
os_ret OS_API destroy_klm(IN klm_handle handle);


os_u32 OS_API get_core_id(os_void);
os_u32 OS_API get_cpu_id(os_void);

pointer OS_API virt_to_phys(pointer addr);
/* kernel only */
pointer OS_API phys_to_virt(pointer addr);
os_bool OS_API virt_is_abs(pointer addr);
os_ret OS_API amap(pointer virt, pointer phys, os_uint length);

HTASK OS_API create_task(IN os_u8 *name, IN TASK_FUNC_PTR entry_task, enum task_priority priority, os_u32 arg1, os_u32 arg2, os_u32 arg3, os_u32 arg4, os_u32 arg5, os_u32 arg6, os_u32 arg7);
os_void OS_API exit_task(os_void);
os_ret OS_API ready_task(IN HTASK handle, os_u32 line);
os_ret OS_API pend_task(IN HTASK handle, os_u32 line);
os_ret OS_API resume_task(IN HTASK handle, os_u32 line);
os_ret OS_API suspend_task(IN HTASK handle, os_u32 line);
os_ret OS_API delay_task(os_u32 count, os_u32 line);
HTASK OS_API current_task_handle(os_void);

os_u32 OS_API get_task_status(IN HTASK handle);

os_ret OS_API modify_task_priority(IN HTASK handle, enum task_priority priority);
os_u32 OS_API get_task_priority(IN HTASK handle);

os_void OS_API schedule(os_void);
os_void OS_API lock_schedule(os_void);
os_void OS_API unlock_schedule(os_void);

os_ret OS_API active_task_station(IN HTASK handle);

os_ret OS_API install_int(os_u32 vector_no, IN IRQ_FUNCPTR IRQ_FUNC func_1, IN IRQ_FUNCPTR IRQ_FUNC func_2);
os_ret OS_API uninstall_int(os_u32 vector_no);
os_ret OS_API install_trap(os_u32 vector_no, IN VOID_FUNCPTR IRQ_FUNC func);
os_ret OS_API uninstall_trap(os_u32 vector_no);

os_ret OS_API install_debugger(IN VOID_FUNCPTR func, IN HTASK debuger);
os_ret OS_API uninstall_debugger(IN HTASK debuger);
enum bp_type {
    BREAK_ON_EXE, /* Break on instruction execution only. */
    BREAK_ON_WRITE, /* Break on data writes only. */
    BREAK_ON_IO, /* Break on I/O reads or writes. */
    BREAK_ON_READ /* Break on data reads or writes but not instruction fetches. */
};
enum bp_data_len {
    BREAK_LEN_1 = 0,
    BREAK_LEN_2 = 1,
    BREAK_LEN_4 = 3
};
os_ret OS_API set_bp(os_u32 addr, enum bp_type type, enum bp_data_len len, IN HTASK debuger);
os_ret OS_API clear_bp(os_u32 addr);

os_u32 OS_API get_pc_thunk(os_void);

os_void OS_API open_pic_int(os_u32 vector);
os_void OS_API close_pic_int(os_u32 vector);

os_void OS_API open_lapic_int(os_u32 vector);
os_void OS_API close_lapic_int(os_u32 vector);


HTIMER OS_API set_timer_callback(os_u32 event_id, os_u32 tick, IN TIMER_FUNC_PTR timer_func, enum timer_mode mode);
HTIMER OS_API set_timer_msg(IN HWINDOW hwnd, os_u32 event_id, os_u32 tick, enum timer_mode mode);
HTIMER OS_API kill_timer(IN HTIMER handle);
HTIMER OS_API reset_timer(IN HTIMER handle);
os_u32 OS_API timer_left(IN HTIMER handle);
os_ret OS_API modify_timer(IN HTIMER handle, os_u32 tick);

/* rtc定时器不提供取消定时器和重置定时器功能 */
HTIMER OS_API set_rtc_callback(os_u32 event_id, os_u32 ms, IN TIMER_FUNC_PTR rtc_func, enum timer_mode mode);
HTIMER OS_API set_rtc_msg(IN HWINDOW hwnd, os_u32 event_id, os_u32 length, enum timer_mode mode);

/***************************************************************
 * description :
 ***************************************************************/
typedef struct {
    os_u32 check;
    volatile os_u32 lock;
    lock_t eflag;
} spinlock_t;

os_void OS_API init_spinlock(spinlock_t *lock);
os_ret OS_API spin_lock(spinlock_t *lock);
os_ret OS_API spin_unlock(spinlock_t *lock);

/***************************************************************
 * description :
 ***************************************************************/
typedef struct {
    spinlock_t spin;
    os_u32 count;
} rwlock_t;
os_void OS_API init_rw_lock(rwlock_t *lock);
os_void OS_API read_lock(rwlock_t *lock);
os_void OS_API read_unlock(rwlock_t *lock);
os_void OS_API write_lock(rwlock_t *lock);
os_void OS_API write_unlock(rwlock_t *lock);

#define EVENT_INVALID 0
#define EVENT_VALID 1
HEVENT OS_API create_event_handle(os_s32 status, os_u8 *name, os_u32 line_no);
os_ret OS_API notify_event(IN HEVENT handle, os_u32 line);
os_ret OS_API wait_event(IN HEVENT handle, os_u32 ms);
os_ret OS_API query_event(IN HEVENT handle, os_s32 *status);
HEVENT OS_API destroy_event_handle(IN HEVENT handle);

HEVENT OS_API create_mevent(os_s32 count, os_u8 *name, os_u32 line);
os_ret OS_API notify_mevent(IN HEVENT handle, os_u32 line);
os_ret OS_API wait_mevent(IN HEVENT handle, os_u32 ms);
os_ret OS_API destroy_mevent(IN HEVENT handle);

os_ret OS_API create_critical_section(OUT HEVENT *handle, os_u32 line);
os_void OS_API enter_critical_section(IN HEVENT handle);
os_void OS_API leave_critical_section(IN HEVENT handle);
os_ret OS_API destroy_critical_section(IN HEVENT handle);

os_void OS_API *alloc_msg(os_u32 size);
os_void OS_API *alloc_timer_msg(os_void);
os_void OS_API *free_msg(IN os_void *addr);
os_ret OS_API send_msg(IN HWINDOW handle, IN os_void *msg);
os_ret OS_API post_msg(IN HWINDOW handle, IN os_void *msg);
os_ret OS_API post_thread_msg(IN HTASK handle, IN os_void *msg);
os_ret OS_API broadcast_msg(IN os_void *msg);


os_ret OS_API create_toolhelp32snapshot(OUT struct task_snapshot **snapshot);
os_ret OS_API get_process32next(INOUT struct task_snapshot **snapshot);
HWINDOW OS_API get_desktop_window(os_void);

#define debug_return return
#define debug_continue continue

#pragma pack()

#endif /* end of header */

