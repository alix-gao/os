/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : message.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __MESSAGE_H__
#define __MESSAGE_H__

/***************************************************************
 include header file
 ***************************************************************/

#ifndef __CORE_H__
    #error "include core.h must appear in source files before include message.h"
#endif

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/
/* bus msg memory block */
#define BMMB_NUM 0x2

#define BMM_CRC UINT32_C(0x5a5a5a5a)

/* 任务消息(io)队列长度, 该宏定义为2^n */
#define TQUEUE_MAX_NUM 32

/* 消息队列数量 */
#define STATION_NUM (CORE_NUM * CPU_NUM * OS_TASK_NUM)

/***************************************************************
 enum define
 ***************************************************************/
/***************************************************************
 * enum name   : msg_status_enum
 * description :
 ***************************************************************/
enum msg_status {
    MSG_STATUS_IDLE,
    MSG_STATUS_BUSY,
    MSG_STATUS_BUTT
};

/***************************************************************
 struct define
 ***************************************************************/
/***************************************************************
 * description : 总线消息内存配置信息
 ***************************************************************/
struct bmm_cfg {
    os_u32 size;
    os_u32 count;
};

/***************************************************************
 * description :
 ***************************************************************/
struct bmmcb {
    os_u8 *addr;
    /* 回收的位置 */
    os_u32 bmmcb_info_index;
    /* 当前的消息内存块状态, 防止重复释放 */
    enum msg_status status;
    struct bmmcb *next;
};

/***************************************************************
 * description :
 ***************************************************************/
struct bmmcb_info {
    os_u32 size;
    os_u32 idle_num;
    struct bmmcb *idle_block;
};

/***************************************************************
 * description : 消息路由表
 ***************************************************************/
struct message_route {
    /* 源窗口句柄, 保留不用 */
    struct window_handle src;
    /* 目的窗口句柄 */
    struct window_handle dest;
};

/***************************************************************
 * description : 消息内存头部
 ***************************************************************/
struct bmm_head {
    /* 回指内存控制块 */
    struct bmmcb *bmmcb;
    /* 校验位 */
    os_u32 crc;
    /* 消息路由信息 */
    struct message_route msg_head;
};

/***************************************************************
 * description : task station, 任务的io寄存器
 ***************************************************************/
struct task_station {
    struct task_station *next;
    /* 消息队列中消息数量 */
    os_u32 num;
    /* 第一个消息位置 */
    os_u32 head;
    /* 消息队列 */
    os_void *station_queue[TQUEUE_MAX_NUM];
};

/***************************************************************
 * description : 定时器消息格式
 ***************************************************************/
struct timer_msg {
    enum os_message msg_name; // 消息名称
    os_u32 msg_len; // 消息长度
    os_u32 event_id; // 定时器事件id, 一个消息处理实体只能有唯一的event id.
};

/***************************************************************
 extern function
 ***************************************************************/
struct window_handle *get_dest_hwindow(IN os_void *msg);

#pragma pack()

#endif /* end of header */

