/***************************************************************
 * copyright (c) 2012, gaocheng
 * all rights reserved.
 *
 * file name   : taskmgr.c
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <configuration.h>
#include <lib.h>

/***************************************************************
 global variable declare
 ***************************************************************/

/* ������Ϣ��, ʹ��������ٲ��������ɾ������ */
struct task_snapshot_table {
	/* �Ƿ�ʹ�ñ�־ */
    os_bool flag;
    struct task_snapshot snap;
};
LOCALD struct task_snapshot_table system_snapshot[SYS_TASK_NUM];

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void init_system_snapshot(os_void)
{
    os_u32 i;

    for (i = 0; i < SYS_TASK_NUM; i++) {
        /* Ĭ����Ч */
        system_snapshot[i].flag = OS_FALSE;
        system_snapshot[i].snap.handle = OS_NULL;
        system_snapshot[i].snap.name = OS_NULL;
        system_snapshot[i].snap.tick = 0;
        system_snapshot[i].snap.usage = 0;
    }
}

/***************************************************************
 * description : ��������, ͨ�����տ��Եõ��߳̾��, ͨ���������closehandle.
 *               �������տ��Է�ֹ�û��޸��ں����ݽṹ.
 *               idle_task_id�Ѿ���ʼ��
 * history     :
 ***************************************************************/
LOCALC os_ret snapshot_entity(os_void)
{
    struct task_control_block *task;
    struct task_queue_head *head;
    struct list_node *node;
    os_u32 core_id;
    os_u32 cpu_id;
    os_u32 i;

    core_id = CORE_ID_0;
    cpu_id = CPU_ID_0;

    for (i = 1; i < OS_TASK_NUM; i++) {
        task = &tcb[core_id][cpu_id][i];
        cassert(TCB_CHECK == task->check);
        system_snapshot[i].snap.handle = &task->handle;
        system_snapshot[i].snap.name = task->name;
        system_snapshot[i].flag = OS_TRUE;
    }

    /* update flag */
    head = &idle_tcb[core_id][cpu_id];
    loop_list(node, &head->head) {
        task = list_addr(node, struct task_control_block, node);

        system_snapshot[task->handle.task_id].flag = OS_FALSE;
    }

    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret snapshot_cpu_usage(os_void)
{
    os_u32 i;
    os_u32 temp_tick;

    for (i = 1; (i < OS_TASK_NUM) && (OS_TRUE == system_snapshot[i].flag); i++) {
        temp_tick = get_thread_tick(system_snapshot[i].snap.handle);
        system_snapshot[i].snap.usage = temp_tick - system_snapshot[i].snap.tick;
        system_snapshot[i].snap.tick = temp_tick;
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret fresh_snapshot(INOUT struct task_snapshot **snapshot)
{
    if (OS_NULL != snapshot) {
        snapshot_entity();
        snapshot_cpu_usage();
        *snapshot = &system_snapshot[0].snap;
        return OS_SUCC;
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API create_toolhelp32snapshot(OUT struct task_snapshot **snapshot)
{
    cassert(OS_NULL != snapshot);
#if 0
    *pos = (struct task_snapshot *) kmalloc(SYS_TASK_NUM * sizeof(struct task_snapshot));
    if (OS_NULL == *pos) {
        /* �ڴ����ʧ�� */
        return OS_NULL;
    }

    /* ȡ�õ�ǰ������� */
    snapshot_entity(system_snapshot);

    mem_cpy(*pos, system_snapshot, SYS_TASK_NUM * sizeof(struct task_snapshot));

    /* ����λ��Ϊindex 0 */
    return *pos;
#endif
    *snapshot = &system_snapshot[0].snap;
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API destroy_toolhelp32snapshot(HANDLE handle)
{
    if (OS_NULL != handle) {
        //kfree(handle);
        return OS_SUCC;
    }
    /* ��μ��ʧ�� */
    return OS_FAIL;
}

/***************************************************************
 * description : ����ȡ����һ���߳̿���
 * history     :
 ***************************************************************/
os_ret OS_API get_process32next(INOUT struct task_snapshot **snapshot)
{
    struct task_snapshot_table *temp;

    if ((OS_NULL != snapshot) && (OS_NULL != *snapshot)) {
        temp = (struct task_snapshot_table *)((pointer) *snapshot - struct_offset(struct task_snapshot_table, snap));
        do {
            temp++;
        } while ((OS_TRUE != temp->flag) && (temp != &system_snapshot[SYS_TASK_NUM]));

        if (temp < &system_snapshot[SYS_TASK_NUM]) {
            *snapshot = &temp->snap;
            return OS_SUCC;
        } else {
            *snapshot = OS_NULL;
            return OS_FAIL;
        }
    }
    return OS_FAIL;
}

