/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : task.h
 * version     : 1.0
 * description : task
 * author      : gaocheng
 * date        : 2013-04-22
 ***************************************************************/

#ifndef __TASK_H__
#define __TASK_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/
/* 任务名字长度 */
#define TASK_NAME_LEN 8

/* 参数保留区长度 */
#define TASK_ARGS_RESERVE 0x100

/* 任务参数个数 */
#define TASK_ARGS_NUM 0x7

#define STACK_FILL 0x22

/* 任务回调函数类型定义 */
typedef os_void OS_API (*TASK_WRAPPER_FUNC_PTR)(TASK_FUNC_PTR entry_task, os_u32 arg1, os_u32 arg2, os_u32 arg3, os_u32 arg4, os_u32 arg5, os_u32 arg6, os_u32 arg7);

#define IDLE_TASK_PRIORITY TASK_PRIORITY_0

/***************************************************************
 enum define
 ***************************************************************/
/***************************************************************
 * enum name   : task_status
 * description :
 ***************************************************************/
enum task_status {
    TASK_STATUS_READY = (1 << 0),
    TASK_STATUS_PEND = (1 << 1),
    TASK_STATUS_DELAY = (1 << 2),
    TASK_STATUS_SUSPEND = (1 << 3),
    TASK_STATUS_BUTT = (1 << 4)
};

/***************************************************************
 struct define
 ***************************************************************/

/***************************************************************
 * description : task control block
 ***************************************************************/
struct task_control_block {
    spinlock_t flock;
    atomic_t ref;
    os_bool del_flag;

    struct list_node node;
    os_u8 name[TASK_NAME_LEN];
    struct task_handle handle; /* handle is more security and pointer is not sure */
    struct _x86_task resource;
    enum task_priority priority;
    enum task_status status;
    os_u32 delay_tick;
    os_u32 line; /* cause */
    os_void *station; /* vtask msg queue */
#define TCB_CHECK 0xcbcbcbcb
    os_u32 check;
} _CPU_ALIGNED_;

/***************************************************************
 * description : task queue head
 ***************************************************************/
struct task_queue_head {
    struct list_node head;
    spinlock_t lock;
};

/***************************************************************
 extern function
 ***************************************************************/
os_void *get_task_station(struct task_handle *handle);
os_ret modify_entity_station(struct task_control_block *handle, os_void *station);

#pragma pack()

#endif /* end of header */

