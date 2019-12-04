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

/* ������Ϣ(io)���г���, �ú궨��Ϊ2^n */
#define TQUEUE_MAX_NUM 32

/* ��Ϣ�������� */
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
 * description : ������Ϣ�ڴ�������Ϣ
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
    /* ���յ�λ�� */
    os_u32 bmmcb_info_index;
    /* ��ǰ����Ϣ�ڴ��״̬, ��ֹ�ظ��ͷ� */
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
 * description : ��Ϣ·�ɱ�
 ***************************************************************/
struct message_route {
    /* Դ���ھ��, �������� */
    struct window_handle src;
    /* Ŀ�Ĵ��ھ�� */
    struct window_handle dest;
};

/***************************************************************
 * description : ��Ϣ�ڴ�ͷ��
 ***************************************************************/
struct bmm_head {
    /* ��ָ�ڴ���ƿ� */
    struct bmmcb *bmmcb;
    /* У��λ */
    os_u32 crc;
    /* ��Ϣ·����Ϣ */
    struct message_route msg_head;
};

/***************************************************************
 * description : task station, �����io�Ĵ���
 ***************************************************************/
struct task_station {
    struct task_station *next;
    /* ��Ϣ��������Ϣ���� */
    os_u32 num;
    /* ��һ����Ϣλ�� */
    os_u32 head;
    /* ��Ϣ���� */
    os_void *station_queue[TQUEUE_MAX_NUM];
};

/***************************************************************
 * description : ��ʱ����Ϣ��ʽ
 ***************************************************************/
struct timer_msg {
    enum os_message msg_name; // ��Ϣ����
    os_u32 msg_len; // ��Ϣ����
    os_u32 event_id; // ��ʱ���¼�id, һ����Ϣ����ʵ��ֻ����Ψһ��event id.
};

/***************************************************************
 extern function
 ***************************************************************/
struct window_handle *get_dest_hwindow(IN os_void *msg);

#pragma pack()

#endif /* end of header */

