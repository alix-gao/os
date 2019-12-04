/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : message.c
 * version     : 1.0
 * description : (key) kernel message memory
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include <message.h>
#include <task.h>

/***************************************************************
 global variable declare
 ***************************************************************/
/* ��Ϣ�ڴ�(IO)������Ϣ, λ��0��ʱ����Ϣ�ڴ�Ԥ�� */
LOCALD const struct bmm_cfg bmm_cfg[BMMB_NUM] = {
    {0x10, 0x20}, //timer
    {0x80, 0x20}
};

/* ��Ϣ�ڴ���ƿ���Ϣ */
LOCALD struct bmmcb_info bmmcb_info[BMMB_NUM];
LOCALD spinlock_t bmmcb_info_lock;

/* �ڴ���ƿ���ʼ��ַ */
LOCALD os_u32 bmmcb_addr = 0;

/* �ں˶�̬�ڴ���ʼ��ַ */
LOCALD os_u32 bmm_addr = 0;

/* task station */
LOCALD struct task_station task_station[STATION_NUM];

/* ����station��Դ�� */
LOCALD struct task_station *idle_station;
LOCALD spinlock_t idle_station_lock;

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void init_bmmcb(os_void)
{
    os_u32 i = 0;
    os_u32 j = 0;
    os_u32 bmmcb_num = 0;
    os_u32 bmm_size = 0;
    struct bmm_head *bmm_head_addr = OS_NULL;
    struct bmmcb *temp_bmmcb_addr = OS_NULL;

    /* �����ڴ���ƿ���������ڴ��Ĵ�С */
    for (i = 0; i < BMMB_NUM; i++) {
        bmmcb_num += bmm_cfg[i].count;
        bmm_size += (sizeof(struct bmm_head)+bmm_cfg[i].size) * bmm_cfg[i].count;
    }

    /* �����ڴ���ƿ龲̬�ڴ� */
    bmmcb_addr = alloc_ksm(bmmcb_num * sizeof(struct bmmcb));
    cassert(0 != bmmcb_addr);

    /* ���䶯̬�ڴ�ռ� */
    bmm_addr = alloc_ksm(bmm_size);
    cassert(0 != bmm_addr);

    temp_bmmcb_addr = (struct bmmcb *) bmmcb_addr;

    bmm_head_addr = (struct bmm_head *) bmm_addr;

    for (i = 0; i < BMMB_NUM; i++) {
        /* �ڴ���С */
        bmmcb_info[i].size = bmm_cfg[i].size;
        /* �ڴ������ */
        bmmcb_info[i].idle_num = bmm_cfg[i].count;
        /* �ڴ���п����� */
        bmmcb_info[i].idle_block = temp_bmmcb_addr;

        for (j = 0; j < bmm_cfg[i].count; j++) {
            /* �ڴ��ͷ���� */
            bmm_head_addr->crc = BMM_CRC;
            bmm_head_addr->bmmcb = temp_bmmcb_addr;

            /* �ڴ���ƿ�״̬ */
            temp_bmmcb_addr->status = MSG_STATUS_IDLE;
            temp_bmmcb_addr->bmmcb_info_index = i;
            /* �����ڴ���ַ */
            temp_bmmcb_addr->addr = (os_u8 *) bmm_head_addr + sizeof(struct bmm_head);

            bmm_head_addr = (struct bmm_head *)(temp_bmmcb_addr->addr + bmm_cfg[i].size);

            /* �����ڴ���ƿ��ַ */
            temp_bmmcb_addr->next = temp_bmmcb_addr + 1;
            temp_bmmcb_addr++;
        }

        temp_bmmcb_addr--;

        /* ������β */
        temp_bmmcb_addr->next = OS_NULL;

        temp_bmmcb_addr++;
    }

    init_spinlock(&bmmcb_info_lock);
}

/***************************************************************
 * description : msg = control+data
 * history     :
 ***************************************************************/
os_void OS_API *alloc_msg(os_u32 size)
{
    os_u32 i;
    struct bmmcb_info *t;
    struct bmmcb *idle_block;
    lock_t eflag;

    lock_int(eflag);

    /* �ӵ�һ��λ�ÿ�ʼ���ҷ��� */
    for (i = 1, t = &bmmcb_info[i]; i < BMMB_NUM; i++, t++) {
        /* �ҵ���һ����ֹͣ */
        if (t->size >= size) {
            if (0 < t->idle_num) {
                idle_block = t->idle_block;

                /* idle_block��idle_num��Ϊ0ʱ, ����Ϊ����β��ʱΪ��ָ�� */
                t->idle_block = idle_block->next;
                t->idle_num--;

                /* �ڴ���ƿ���ʹ�� */
                idle_block->status = MSG_STATUS_BUSY;
                idle_block->next = OS_NULL;

                unlock_int(eflag);

                return idle_block->addr;
            }

            unlock_int(eflag);

            /* ���Ͽ��С���ڴ�������� */
            return OS_NULL;
        }
    }

    unlock_int(eflag);

    /* ���С�ߴ粻�Ϸ� */
    return OS_NULL;
}

/***************************************************************
 * description : ���䶨ʱ����Ϣ�ڴ�
 * history     :
 ***************************************************************/
os_void OS_API *alloc_timer_msg(os_void)
{
    struct bmmcb *idle_block;
    lock_t eflag;

    lock_int(eflag);

    /* ��ʱ����Ϣ�ڴ����� */
    if (0 != bmmcb_info[0].idle_num) {
        /* double check for smp */
        if (0 != bmmcb_info[0].idle_num) {
            /* �ӵ�һ��λ�ÿ�ʼ���� */
            idle_block = bmmcb_info[0].idle_block;

            /* idle_block��idle_num��Ϊ0ʱ, ����Ϊ����β��ʱΪ��ָ�� */
            bmmcb_info[0].idle_block = idle_block->next;
            bmmcb_info[0].idle_num--;

            /* �ڴ���ƿ���ʹ�� */
            idle_block->status = MSG_STATUS_BUSY;
            idle_block->next = OS_NULL;
        }

        unlock_int(eflag);

        return idle_block->addr;
    }

    unlock_int(eflag);

    /* ��ʱ����Ϣ�ڴ������� */
    return OS_NULL;
}

/***************************************************************
 * description : msg = control+data
 * history     :
 ***************************************************************/
os_void OS_API *free_msg(IN os_void *addr)
{
    struct bmm_head *bmm_head = OS_NULL;
    struct bmmcb *temp_bmmcb = OS_NULL;
    lock_t eflag;

    /* ����У�� */
    if (OS_NULL != addr) {
        bmm_head = (struct bmm_head *)(addr - sizeof(struct bmm_head));

        /* ���У��λ */
        if (BMM_CRC == bmm_head->crc) {
            temp_bmmcb = bmm_head->bmmcb;

            /* �������ڴ���ƿ�״̬ */
            if (MSG_STATUS_IDLE != temp_bmmcb->status) {
                lock_int(eflag);

                /* ����Ϣ�ڴ�״̬ */
                temp_bmmcb->status = MSG_STATUS_IDLE;
                temp_bmmcb->next = bmmcb_info[temp_bmmcb->bmmcb_info_index].idle_block;

                bmmcb_info[temp_bmmcb->bmmcb_info_index].idle_block = temp_bmmcb;
                bmmcb_info[temp_bmmcb->bmmcb_info_index].idle_num++;

                unlock_int(eflag);

                return bmm_head+1;
            }
        }
    }

    /* �ظ��ͷ��ڴ� | У��λʧ��, ���Ƕ�̬�ڴ�� | status */
    return OS_NULL;
}

/***************************************************************
 * description : ������Ϣ������ʵ��, �첽��ʽ.
 * history     :
 ***************************************************************/
os_ret OS_API post_msg(IN HWINDOW handle, IN os_void *msg)
{
    lock_t eflag;
    os_u32 msg_pos;
    struct bmm_head *bmm_head;
    struct task_handle *htask;
    os_u32 core_id;
    os_u32 cpu_id;
    os_u32 task_id;
    struct task_station *station;

    /* ��μ�� */
    if (OS_NULL != msg) {
        /* ��μ�� */
        if (OS_NULL != handle) {
            htask = ((struct window_handle *) handle)->htask;

            /* ��ȡcore id */
            core_id = htask->core_id;
            /* ��ȡtask id */
            cpu_id = htask->cpu_id;
            /* ��ȡtask id */
            task_id = htask->task_id;

            /* У��task id */
            if ((INVALID_TASK_ID != task_id) && (OS_TASK_NUM > task_id) && (CPU_NUM > cpu_id) && (CORE_NUM > core_id)) {
                /* У��station */
                station = get_task_station(htask);
                if ((OS_NULL != station) && (TQUEUE_MAX_NUM > station->num)) {
                    /* оƬ��������io����ʱΪԭ�Ӳ��� */
                    lock_int(eflag);

                    bmm_head = (struct bmm_head *)(msg - sizeof(struct bmm_head));
                    /* ��Ϣͷ·����Ϣ */
                    bmm_head->msg_head.src.htask = current_task_handle();
                    bmm_head->msg_head.src.window_id = 0; //�������̷߳��͵�, ���ֶ�Ԥ��.
                    bmm_head->msg_head.dest.htask = htask;
                    bmm_head->msg_head.dest.window_id = ((struct window_handle *) handle)->window_id;

                    msg_pos = (station->head + station->num) & (TQUEUE_MAX_NUM - 1);
                    station->station_queue[msg_pos] = (os_void *) msg;
                    station->num++;

                    /* ����pend̬������ */
                    ready_task(htask, __LINE__);

                    unlock_int(eflag);

                    /* ��ʽǿ�Ƶ���, ������ʵʱ���� */
                    if (get_task_priority(current_task_handle()) < get_task_priority(htask)) {
                        schedule();
                    }

                    return OS_SUCC;
                }
            }
        }

        /* ��μ��ʧ�� | task id | station */
        free_msg(msg);
    }

    /* ��μ��ʧ�� */
    return OS_FAIL;
}

/***************************************************************
 * description : ������Ϣ������ʵ��, ͬ����ʽ.
 *               ע��÷�ʽ�������Ϣ����������!
 * history     :
 ***************************************************************/
os_ret OS_API send_msg(IN HWINDOW handle, IN os_void *msg)
{
    struct message_route *task_msg;
    os_u32 window_id;
    MSG_PROC_PTR msg_func;

    /* ��μ�� */
    if (OS_NULL != msg) {
        /* ��μ�� */
        if (OS_NULL != handle) {
            msg_func = get_msgproc(handle);
            if (OS_NULL != msg_func) {
                /* msg_route_struct */
                task_msg = (struct message_route *)(msg - sizeof(struct message_route));

                /* ��Ϣͷ·����Ϣ */
                task_msg->src.htask = current_task_handle();
                task_msg->src.window_id = 0; //�������̷߳��͵�, ���ֶ�Ԥ��.
                task_msg->dest.htask = handle;
                task_msg->dest.window_id = window_id;

                /* ֱ�ӵ��ô��ھ������Ϣ������ */
                msg_func(msg);

                return OS_SUCC;
            }
        }

        /* �ͷ���Ϣ handle | msgproc */
        free_msg(msg);
    }

    /* ��μ��ʧ�� */
    return OS_FAIL;
}

/***************************************************************
 * description : ������Ϣ���߳�ʵ��, �첽��ʽ.
 * history     :
 ***************************************************************/
os_ret OS_API post_thread_msg(IN HTASK handle, IN os_void *msg)
{
    lock_t eflag;
    os_u32 msg_pos;
    struct message_route *task_msg;
    struct task_handle *htask;
    os_u32 core_id;
    os_u32 cpu_id;
    os_u32 task_id;
    struct task_station *station;

    /* ��μ�� */
    if (OS_NULL != msg) {
        /* ��μ�� */
        if (OS_NULL != handle) {
            htask = handle;

            core_id = htask->core_id;
            cpu_id = htask->cpu_id;
            task_id = htask->task_id;

            /* У��handle��� */
            if ((CPU_NUM > cpu_id) && (INVALID_TASK_ID != task_id) && (OS_TASK_NUM > task_id)) {
                station = get_task_station(htask);
                if ((OS_NULL != station) && (TQUEUE_MAX_NUM > station->num)) {
                    /* оƬ��������io����ʱΪԭ�Ӳ��� */
                    lock_int(eflag);

                    /* msg_route_struct */
                    task_msg = (struct message_route *)(msg - sizeof(struct message_route));

                    /* ��Ϣͷ·����Ϣ */
                    task_msg->src.htask = current_task_handle();
                    task_msg->src.window_id = 0; //�������жϷ��͵�, ���ֶ�Ԥ��.
                    task_msg->dest.htask = handle;
                    task_msg->dest.window_id = 0; //����

                    //msg_pos = (task_station[task_id].head + task_station[task_id].num) % TQUEUE_MAX_NUM;
                    msg_pos = (station->head + station->num) & (TQUEUE_MAX_NUM - 1);

                    station->station_queue[msg_pos] = (os_void *) msg;
                    station->num++;

                    /* ����pend̬������ */
                    ready_task(handle, __LINE__);

                    unlock_int(eflag);

                    /* ��ʽǿ�Ƶ���, ������ʵʱ���� */
                    if (get_task_priority(current_task_handle()) < get_task_priority(handle)) {
                        schedule();
                    }

                    return OS_SUCC;
                }
            }
        }

        /* �ͷ���Ϣ, handle | task id | station  */
        free_msg(msg);
    }

    /* ��μ�� */
    return OS_FAIL;
}

/***************************************************************
 * description : �㲥��Ϣ�����еĴ���
 * history     :
 ***************************************************************/
os_ret OS_API broadcast_msg(IN os_void *msg)
{
    /* ��ȡ���д��ھ�� */
    return OS_SUCC;
}

/***************************************************************
 * description : ������Ϣ���߳�Ϊ��λ
 * history     :
 ***************************************************************/
os_ret receive_msg(struct task_handle *handle, os_void **msg)
{
    struct task_station *station;
    os_u32 msg_pos;
    lock_t eflag;

    cassert((OS_NULL != handle) && (OS_NULL != msg));

    station = get_task_station(handle);
    if (OS_NULL != station) {
        /* ����Ϣ���ʴ� */
        if (0 == station->num) {
            /* û����Ϣ, ����, �ȴ����� */
            pend_task(current_task_handle(), __LINE__);
            schedule();

            *msg = OS_NULL;
            return OS_SUCC;
        }

        lock_int(eflag);

        station->num--;

        msg_pos = station->head;

        station->head = (msg_pos+1) & (TQUEUE_MAX_NUM-1);

        *msg = station->station_queue[msg_pos];

        unlock_int(eflag);

        return OS_SUCC;
    }
    /* �̵߳���Ϣ�����Ѿ�ɾ�� */
    return OS_FAIL;
}

/***************************************************************
 * description : �����߳���Ϣ��ȡĿ�Ĵ��ھ��
 *               �ں��ڲ�ʹ��, ��������
 * history     :
 ***************************************************************/
struct window_handle *get_dest_hwindow(IN os_void *msg)
{
    struct bmm_head *bmm_head;

    cassert(OS_NULL != msg);

    bmm_head = (struct bmm_head *)(msg - sizeof(struct bmm_head));

    return &(bmm_head->msg_head.dest);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void dump_bmm_info(os_void)
{
    os_u32 i;
    struct bmmcb *idle_block;
    lock_t eflag;

    lock_int(eflag);

    print("bmm addr: %x %x\n", bmmcb_addr, bmm_addr);

    for (i=0; i<BMMB_NUM; i++) {
        /* �ڴ���С */
        print("bmm size: %x\n", bmmcb_info[i].size);
        print("bmm num: %x\n", bmmcb_info[i].idle_num);

        idle_block = bmmcb_info[i].idle_block;
        while (idle_block) {
            print("ctrl addr: %x ", idle_block);
            print("status: %x ", idle_block->status);
            print("addr: %x ", idle_block->addr);
            print("back: %x\n", ((struct bmm_head *)(idle_block->addr-sizeof(struct bmm_head)))->bmmcb);
            idle_block = idle_block->next;
        }
    }

    unlock_int(eflag);
}

LOCALD os_u8 bmm_debug_name[] = { "bmm" };
LOCALD struct dump_info bmm_debug = {
    bmm_debug_name,
    dump_bmm_info
};

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void init_task_station(os_void)
{
    os_u32 i;
    struct task_station *t;

    /* station */
    for (i = 0, t = task_station; i < STATION_NUM; i++, t++) {
        t->next = OS_NULL;
        t->num = 0;
        t->head = 0;
    }

    if (register_dump(&bmm_debug)) {
        flog("bmm register dump fail\n");
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void init_idle_station(os_void)
{
    os_u32 i;

    idle_station = &task_station[0];

    /* ��ʼ��idle_station_id */
    for (i = 0; i < (STATION_NUM - 1); i++) {
        task_station[i].next = &task_station[i + 1];
    }
    task_station[STATION_NUM - 1].next = OS_NULL;

    init_spinlock(&idle_station_lock);
}

/***************************************************************
 * description : �����̵߳���Ϣ����, ���ڶര�ڶ���, ���ظ�ִ��.
 * history     :
 ***************************************************************/
os_ret OS_API active_task_station(IN HTASK handle)
{
    struct task_station *station = OS_NULL;

    station = get_task_station(handle);
    if (OS_NULL == station) {
        spin_lock(&idle_station_lock);

        if (OS_NULL != idle_station) {
            station = idle_station;

            idle_station = idle_station->next;

            /* ���Ӽ�����Ϣ���� */
            modify_entity_station(handle, station);

            spin_unlock(&idle_station_lock);

            return OS_SUCC;
        }

        spin_unlock(&idle_station_lock);

        /* ��Ϣ������Դ������� */
        return OS_FAIL;
    }

    /* �߳��Ѿ�������Ϣ���� */
    return OS_SUCC;
}

/***************************************************************
 * description : ȥ�����߳���Ϣ����
 * history     :
 ***************************************************************/
os_ret free_task_station(struct task_handle *handle)
{
    os_u32 i;
    os_u32 msg_pos;
    struct task_station *station;
    os_u32 core_id;
    os_u32 cpu_id;
    os_u32 task_id;

    if (OS_NULL != handle) {
        core_id = handle->core_id;
        cpu_id = handle->cpu_id;
        task_id = handle->task_id;

        if ((OS_TASK_NUM > task_id) && (INVALID_TASK_ID != task_id) && (CPU_NUM > cpu_id)) {
            station = get_task_station(handle);
            if (OS_NULL == station) {
                /* �߳�û����Ϣ���� */
                return OS_SUCC;
            }

            spin_lock(&idle_station_lock);

            msg_pos = station->head;
            for (i = 0; i < station->num; i++) {
                free_msg(station->station_queue[msg_pos]);
                /* �ÿ� */
                station->station_queue[msg_pos] = OS_NULL;
                station->num--;
            }
            station->head = 0;
            station->next = idle_station->next;
            idle_station->next = station;

            spin_unlock(&idle_station_lock);

            return OS_SUCC;
        }
    }
    /* ��μ��ʧ�� | task id */
    return OS_FAIL;
}

