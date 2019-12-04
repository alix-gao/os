/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : task.c
 * version     : 1.0
 * description : (key) task
 *               ready->pended---------take_sem() / receive_msg()
 *               ready->delayed--------delay_task()
 *               ready->suspended------suspend_task()
 *               pended->ready---------give_sem() / send_msg()
 *               pended->suspended-----suspend_task()
 *               delayed->ready--------expired delay
 *               delayed->suspended----suspend_task()
 *               suspended->ready------resume_task() / activate_task()
 *               suspended->pended-----resume_task()
 *               suspended->delayed----resume_task()
 *               core �ں˸�����, ���ڼ��س�ʼ���ڲ�ģ��
 *               loader �ں˼�������, �û����س�ʼ���ⲿ����ģ��, ��������shell��
 * author      : gaocheng
 * date        : 2013-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include <vds.h>
#include <vbs.h>
#include <vts.h>
#include "task.h"

#if (SYS_TASK_NUM > CPU_MAX_TASK_NUM)
    #error "wrong configuration about system task num."
#endif

/***************************************************************
 global variable declare
 ***************************************************************/
/* task entity table, ��0��λ��Ϊnull */
LOCALD struct task_control_block tcb[CORE_NUM][CPU_NUM][OS_TASK_NUM] = { 0 };

LOCALD struct task_control_block *current_task[CORE_NUM][CPU_NUM] _CPU_ALIGNED_ = { OS_NULL };

LOCALD struct task_queue_head idle_tcb[CORE_NUM][CPU_NUM];
LOCALD struct task_queue_head ready_tcb[CORE_NUM][CPU_NUM][TASK_PRIORITY_CNT];
LOCALD struct task_queue_head pend_tcb[CORE_NUM][CPU_NUM];
LOCALD struct task_queue_head suspend_tcb[CORE_NUM][CPU_NUM];
LOCALD struct task_queue_head delay_tcb[CORE_NUM][CPU_NUM];

GLOBALREFC void DUMMY_CODE dummy(void);

/***************************************************************
 function declare
 ***************************************************************/

#if defined(PREEMPTIVE_SCHEDULE)
/***************************************************************
 * description : �������ȼ�����, ��ͬ���ȼ���ʱ����
 *               ����task_id(task index or task_entity index)
 * history     :
 ***************************************************************/
LOCALC struct task_control_block *task_schedule(os_void)
{
    struct task_queue_head *head;
    struct list_node *node;
    struct list_node *_save;
    struct task_control_block *task;
    os_u32 core_id;
    os_u32 cpu_id;
    enum task_priority i;

    core_id = get_core_id();
    cpu_id = get_cpu_id();

    /* �������ȼ����� */
    for (i = TASK_PRIORITY_7; i >= TASK_PRIORITY_0; i--) {
        head = &ready_tcb[core_id][cpu_id][i];
        spin_lock(&head->lock);
        loop_del_list(node, _save, &head->head) {
            task = list_addr(node, struct task_control_block, node);

            cassert(TCB_CHECK == task->check);
            spin_lock(&task->flock);
            if (OS_FALSE == task->del_flag) {
                /* ��ǰ�����Ƴ���������β */
                del_list(&task->node);
                add_list_tail(&head->head, &task->node);

                /* increase and unlock could be earlier (after if () { ), but it is rare. */
                atomic_inc(&task->ref);
                spin_unlock(&task->flock);
                spin_unlock(&head->lock);
                return task;
            }
            spin_unlock(&task->flock);
        }
        spin_unlock(&head->lock);
    }
    return OS_NULL;
}
#elif defined(ROUND_ROBIN_SCHEDULE)
/***************************************************************
 * description : ��ƽ����
 *               ��ʱ����¼���ж��е�tick����ͳ��cpuռ����
 *               ����task_id(task index or task_entity index)
 * history     :
 ***************************************************************/
LOCALC struct task_control_block *task_schedule(os_void)
{
    struct task_queue_head *head;
    struct list_node *node;
    struct list_node *_save;
    struct task_control_block *task;

    head = &ready_tcb[get_core_id()][get_cpu_id()][TIME_SHARE_PRIORITY];
    spin_lock(&head->lock);
    loop_del_list(node, _save, &head->head) {
        task = list_addr(node, struct task_control_block, node);

        cassert(TCB_CHECK == task->check);
        spin_lock(&task->flock);
        if (OS_FALSE == task->del_flag) {
            /* ��ǰ�����Ƴ���������β */
            del_list(&task->node);
            add_list_tail(&head->head, &task->node);

            atomic_inc(&task->ref);
            spin_unlock(&task->flock);
            spin_unlock(&head->lock);

            return task;
        }
        spin_unlock(&task->flock);
    }
    spin_unlock(&head->lock);
    return OS_NULL;
}
#else
    #error "no task schedule mechanism defined!!!"
#endif

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC inline os_void handover_task(struct task_control_block *task)
{
    _x86_switch_task(&task->resource);
}

/* nesting */
LOCALD volatile os_u32 schedule_flag _CPU_ALIGNED_ = 0;

/***************************************************************
 * description : ǿ�Ƶ���
 * history     :
 ***************************************************************/
os_void OS_API schedule(os_void)
{
    struct task_control_block *task;
    lock_t eflag;

    /* fix: lock for hiden global vars. task_schedule() and handover_task() together. */
    lock_int(eflag);
    if (0 == schedule_flag) {
        task = task_schedule();
        if (task) {
            /* 7.3 TASK SWITCHING, 64-ia-32-architectures-software-developer-system-programming-manual-325384
               6. If the task switch was initiated with a JMP or IRET instruction, the processor
                  clears the busy (B) flag in the current (old) task��s TSS descriptor; if initiated with
                  a CALL instruction, an exception, or an interrupt: the busy (B) flag is left set.
                  (See Table 7-2.)
               10. If the task switch was initiated with a CALL instruction, JMP instruction, an
                   exception, or an interrupt, the processor sets the busy (B) flag in the new task��s
                   TSS descriptor; if initiated with an IRET instruction, the busy (B) flag is left set. */
            /* JMP��Jump, 64-ia-32-architectures-software-developer-instruction-set-reference-manual-325383
               TASK-GATE:
               IF task gate DPL < CPL
               or task gate DPL < task gate segment-selector RPL
               THEN #GP(task gate selector); FI;
               IF task gate not present
               THEN #NP(gate selector); FI;
               Read the TSS segment selector in the task-gate descriptor;
               IF TSS segment selector local/global bit is set to local
               or index not within GDT limits
               or TSS descriptor specifies that the TSS is busy
               THEN #GP(TSS selector); FI; */
            if (task != current_task[get_core_id()][get_cpu_id()]) { /* current_task is not null */
                //print("[%s->%s:", current_task[get_core_id()][get_cpu_id()]->name, task->name);
                cassert_word(TASK_STATUS_READY == task->status, "%s %d\n", task->name, task->status);

                /* ���µ�ǰ�����task id */
                current_task[get_core_id()][get_cpu_id()] = task;
                /* �����л� */
                handover_task(task);

                //print(":%s-%s]", current_task[get_core_id()][get_cpu_id()]->name, task->name);
                cassert_word(TASK_STATUS_READY == current_task[get_core_id()][get_cpu_id()]->status, "%s %d\n", current_task[get_core_id()][get_cpu_id()]->name, current_task[get_core_id()][get_cpu_id()]->status);

                atomic_dec(&task->ref);
                if ((OS_TRUE == task->del_flag) && (0 == atomic_read(&task->ref))) {
                }
            }
        } else {
            cassert(OS_FALSE);
        }
    }
    unlock_int(eflag);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void OS_API lock_schedule(os_void)
{
    lock_t eflag;

    cassert(UINT32_MAX > schedule_flag);
    lock_int(eflag);
    schedule_flag++;
    unlock_int(eflag);
    wmb();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void OS_API unlock_schedule(os_void)
{
    lock_t eflag;

    cassert(0 != schedule_flag);
    lock_int(eflag);
    schedule_flag--;
    unlock_int(eflag);
    wmb();
}

/***************************************************************
 * description : ��pend״̬��������Ϊ����̬.
 *               �ź����ͷ�/��Ϣ����
 *               ע��, ready_task����resume_task.
 * history     :
 ***************************************************************/
os_ret OS_API ready_task(IN HTASK handle, os_u32 line)
{
    struct task_control_block *entity;
    struct task_queue_head *rq, *pq;
    struct task_handle *id;
    os_u32 core_id;
    os_u32 cpu_id;

    if (OS_NULL != handle) {
        id = (struct task_handle *) handle;
        core_id = id->core_id;
        cpu_id = id->cpu_id;
        cassert_word(id->check == HTASK_CHECK, "wrong hnd: %x\n", handle);
        if ((CORE_NUM > id->core_id) && (CPU_NUM > id->cpu_id) && (OS_TASK_NUM > id->task_id)) {
            entity = &tcb[core_id][cpu_id][id->task_id];
            cassert(TCB_CHECK == entity->check);
            spin_lock(&entity->flock);
            switch (entity->status) {
            case TASK_STATUS_PEND:
                rq = &ready_tcb[core_id][cpu_id][entity->priority];
                pq = &pend_tcb[core_id][cpu_id];
                spin_lock(&rq->lock);
                spin_lock(&pq->lock);

                del_list(&entity->node);
                entity->status = TASK_STATUS_READY;
                entity->line = line;
                /* ����������еĶ�ͷ, �ȴ����� */
                cassert(TASK_PRIORITY_CNT > entity->priority);
                add_list_head(&rq->head, &entity->node);

                spin_unlock(&pq->lock);
                spin_unlock(&rq->lock);
                spin_unlock(&entity->flock);

                /* ǿ�Ƶ���, ֱ�ӽ�Ŀ�����񼤻�,
                   ���Ŀ����������ȼ��ϸ�, ��ô�����õ�����.
                   ���Ŀ����������ȼ�����, ��ʹǿ�Ƶ���һ��Ŀ������Ҳ����õ�����.
                   ����, ������жϷ�������н��������л�֮ǰû�з���eoi,
                   �ô�ǿ�Ƶ��Ȼ�����ж��ڲ��˽��м����������,
                   ��Ϊ������жϻָ�ʱ���Գ�.
                   ��Ҫ�ȵ����������µ��ȵ�ǰ������ָܻ��жϿ�����.
                   ����, ���ܻᵼ�·�����Ϣ���ͷ��ź�����Ҫ����tick���ܴ���.
                   Ӱ��ʵʱ��, ��˸ô�Ҫǿ�Ƶ���.
                   �жϷ������Ҫ��֤�������л�֮ǰ����eoi. */
                //schedule(); // ȥ��, �������׳���ƹ���л����˷�cpu.
                return OS_SUCC;
                break;
            case TASK_STATUS_READY:
            case TASK_STATUS_DELAY:
                spin_unlock(&entity->flock);
                return OS_SUCC;
                break;
            default:
                cassert_word(OS_FALSE, "%s %d %d\n", entity->name, cpu_id, entity->status);
                break;
            }
            spin_unlock(&entity->flock);
        }
    }
    return OS_FAIL;
}

/***************************************************************
 * description : �ڲ�����, ��ȡ��Ϣ/����
 * history     :
 ***************************************************************/
os_ret OS_API pend_task(IN HTASK handle, os_u32 line)
{
    struct task_control_block *entity;
    struct task_queue_head *rq, *pq;
    struct task_handle *id;
    os_u32 core_id;
    os_u32 cpu_id;

    if (OS_NULL != handle) {
        id = (struct task_handle *) handle;
        core_id = id->core_id;
        cpu_id = id->cpu_id;
        cassert(id->check == HTASK_CHECK);
        if ((CORE_NUM > id->core_id) && (CPU_NUM > id->cpu_id) && (OS_TASK_NUM > id->task_id)) {
            entity = &tcb[core_id][cpu_id][id->task_id];
            cassert(TCB_CHECK == entity->check);
            spin_lock(&entity->flock);
            switch (entity->status) {
            case TASK_STATUS_READY:
                /* ��ɾ��, ��ֹ�����ڶ�����ȶ����� */
                cassert(TASK_PRIORITY_CNT > entity->priority);
                rq = &ready_tcb[core_id][cpu_id][entity->priority];
                pq = &pend_tcb[core_id][cpu_id];
                spin_lock(&rq->lock);
                spin_lock(&pq->lock);
                del_list(&entity->node);
                entity->status = TASK_STATUS_PEND;
                entity->line = line;
                add_list_head(&pq->head, &entity->node);
                spin_unlock(&pq->lock);
                spin_unlock(&rq->lock);
                spin_unlock(&entity->flock);
                return OS_SUCC;
                break;
            case TASK_STATUS_PEND:
                spin_unlock(&entity->flock);
                return OS_SUCC;
                break;
            case TASK_STATUS_DELAY:
            default:
                cassert_word(OS_FALSE, "%s %d %d\n", entity->name, cpu_id, entity->status);
                break;
            }
            spin_unlock(&entity->flock);
        }
    }
    return OS_FAIL;
}

/***************************************************************
 * description : ��ʼ���񴴽���ȥ������suspend״̬������.
 *               ���ǵ��ò������ڷ�Ƶ������, ���Ը���һЩ.
 *               �����������Ƶ������, ��Ҫ�򵥸�Ч.
 * history     :
 ***************************************************************/
os_ret OS_API resume_task(IN HTASK handle, os_u32 line)
{
    struct task_control_block *entity;
    struct task_queue_head *queue, *sq;
    struct task_handle *id;
    os_u32 core_id;
    os_u32 cpu_id;

    if (OS_NULL != handle) {
        id = (struct task_handle *) handle;
        core_id = id->core_id;
        cpu_id = id->cpu_id;
        cassert(id->check == HTASK_CHECK);
        if ((CORE_NUM > id->core_id) && (CPU_NUM > id->cpu_id) && (OS_TASK_NUM > id->task_id)) {
            entity = &tcb[core_id][cpu_id][id->task_id];
            cassert(TCB_CHECK == entity->check);
            spin_lock(&entity->flock);
            if (entity->status & TASK_STATUS_SUSPEND) {
                switch (entity->status & ~TASK_STATUS_SUSPEND) {
                case TASK_STATUS_READY:
                    cassert(TASK_PRIORITY_CNT > entity->priority);
                    queue = &ready_tcb[core_id][cpu_id][entity->priority];
                    break;
                case TASK_STATUS_PEND:
                    queue = &pend_tcb[core_id][cpu_id];
                    break;
                case TASK_STATUS_DELAY:
                    queue = &delay_tcb[core_id][cpu_id];
                    break;
                case TASK_STATUS_SUSPEND:
                default:
                    cassert_word(OS_FALSE, "%s %d %d\n", entity->name, cpu_id, entity->status);
                    break;
                }
                sq = &suspend_tcb[core_id][cpu_id];
                spin_lock(&queue->lock);
                spin_lock(&sq->lock);
                del_list(&entity->node);
                add_list_head(&queue->head, &entity->node);
                entity->status &= ~TASK_STATUS_SUSPEND;
                entity->line = line;
                spin_unlock(&sq->lock);
                spin_unlock(&queue->lock);
                spin_unlock(&entity->flock);
                return OS_SUCC;
            }
            spin_unlock(&entity->flock);
        }
    }
    return OS_FAIL;
}

/***************************************************************
 * description : ��������״̬����suspend, ��Ǩ�Ƶ�suspend����.
 * history     :
 ***************************************************************/
os_ret OS_API suspend_task(HTASK handle, os_u32 line)
{
    struct task_control_block *entity;
    struct task_queue_head *queue, *sq;
    struct task_handle *id;
    os_u32 core_id;
    os_u32 cpu_id;

    if (OS_NULL != handle) {
        id = (struct task_handle *) handle;
        core_id = id->core_id;
        cpu_id = id->cpu_id;
        cassert(id->check == HTASK_CHECK);
        if ((CORE_NUM > id->core_id) && (CPU_NUM > id->cpu_id) && (OS_TASK_NUM > id->task_id)) {
            entity = &tcb[core_id][cpu_id][id->task_id];
            cassert(TCB_CHECK == entity->check);
            spin_lock(&entity->flock);
            if (0 == (entity->status & TASK_STATUS_SUSPEND)) {
                /* delete task from old queue */
                switch (entity->status) {
                case TASK_STATUS_READY:
                    cassert(TASK_PRIORITY_CNT > entity->priority);
                    queue = &ready_tcb[core_id][cpu_id][entity->priority];
                    break;
                case TASK_STATUS_PEND:
                    queue = &pend_tcb[core_id][cpu_id];
                    break;
                case TASK_STATUS_DELAY:
                    queue = &delay_tcb[core_id][cpu_id];
                    break;
                case TASK_STATUS_SUSPEND:
                    queue = &suspend_tcb[core_id][cpu_id];
                    break;
                default:
                    cassert_word(OS_FALSE, "%s %d %d\n", entity->name, cpu_id, entity->status);
                    break;
                }
                sq = &suspend_tcb[core_id][cpu_id];
                spin_lock(&queue->lock);
                spin_lock(&sq->lock);
                del_list(&entity->node);
                entity->status |= TASK_STATUS_SUSPEND;
                entity->line = line;
                add_list_head(&sq->head, &entity->node);
                spin_unlock(&sq->lock);
                spin_unlock(&queue->lock);
            }
            spin_unlock(&entity->flock);

            if (current_task_handle() == handle) {
                schedule();
            }
            return OS_SUCC;
        }
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API modify_task_priority(IN HTASK handle, enum task_priority priority)
{
    struct task_control_block *entity;
    struct task_handle *id;
    struct task_queue_head *sq, *dq;

    if ((OS_NULL != handle) && (TASK_PRIORITY_CNT > priority)) {
        id = (struct task_handle *) handle;
        cassert(id->check == HTASK_CHECK);
        if ((CORE_NUM > id->core_id) && (CPU_NUM > id->cpu_id) && (OS_TASK_NUM > id->task_id)) {
            entity = &tcb[id->core_id][id->cpu_id][id->task_id];
            cassert(TCB_CHECK == entity->check);
            spin_lock(&entity->flock);
#if defined(PREEMPTIVE_SCHEDULE)
#elif defined(ROUND_ROBIN_SCHEDULE)
            priority = TIME_SHARE_PRIORITY;
#else
            #error "no task schedule mechanism defined!!!"
#endif
            if ((entity->priority != priority) && (TASK_STATUS_READY == entity->status)) {
                sq = &ready_tcb[id->core_id][id->cpu_id][entity->priority];
                dq = &ready_tcb[id->core_id][id->cpu_id][priority];
                spin_lock(&sq->lock);
                spin_lock(&dq->lock);
                if (TASK_STATUS_READY == entity->status) {
                    del_list(&entity->node);
                    add_list_head(&dq->head, &entity->node);
                }
                entity->priority = priority;
                spin_unlock(&dq->lock);
                spin_unlock(&sq->lock);
            } else {
                entity->priority = priority;
            }
            spin_unlock(&entity->flock);
            return OS_SUCC;
        }
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_u32 OS_API get_task_priority(IN HTASK handle)
{
    struct task_handle *id;

    if (OS_NULL != handle) {
        id = (struct task_handle *) handle;
        cassert(id->check == HTASK_CHECK);
        if ((CORE_NUM > id->core_id) && (CPU_NUM > id->cpu_id) && (OS_TASK_NUM > id->task_id)) {
            return tcb[id->core_id][id->cpu_id][id->task_id].priority;
        }
    }
    return INVALID_TASK_PRIORITY;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_u32 OS_API get_task_status(IN HTASK handle)
{
    struct task_handle *id;

    if (OS_NULL != handle) {
        id = (struct task_handle *) handle;
        cassert(id->check == HTASK_CHECK);
        if ((CORE_NUM > id->core_id) && (CPU_NUM > id->cpu_id) && (OS_TASK_NUM > id->task_id)) {
            return tcb[id->core_id][id->cpu_id][id->task_id].status;
        }
    }
    return TASK_STATUS_BUTT;
}

/***************************************************************
 * description : ��������Ĳ���
 * history     :
 ***************************************************************/
LOCALC os_void set_task_args(os_u32 stack_addr, TASK_FUNC_PTR entry_task, os_u32 *arg_tab)
{
    os_u32 *stack_base;
    os_u32 i;

    stack_base = (os_u32 *) stack_addr;

    //push n: sp=sp-2;(sp)=n;
    //*(stack_base+3) = arg2;
    //*(stack_base+2) = arg1;
    //*(stack_base+1) = arg0;
    //*(stack_base+0) = return addr;
    //*(stack_base-1) = old ebp;

    /* clear return addr */
    stack_base[0] = 0;

    // args 0
    stack_base[1] = (pointer) entry_task;

    // task reserve args
    for (i = 0; i < TASK_ARGS_NUM; i++) {
        stack_base[i + 2] = arg_tab[i];
    }

    /* �з���ֵ�ĺ�������ʱ��һ��int��ָ���32bit����ֵ(����32bit�ṹ)ͨ��eax����,
       (bool,charͨ��al����, shortͨ��ax����),
       �ر��__int64��64bit�ṹ(struct) ͨ��edx,eax�����Ĵ���������
       (ͬ��32bit������16bit������ͨ��dx,ax����);
       ������С�Ľṹ(struct)����ʱ�����ַͨ��eax����; */
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void init_task_context(struct task_control_block *entity, IN os_u8 *name, TASK_WRAPPER_FUNC_PTR wrapper_func, TASK_FUNC_PTR entry_task, os_u32 *arg_tab)
{
    atomic_init(&entity->ref);

    mem_cpy(entity->name, name, TASK_NAME_LEN);
    entity->name[TASK_NAME_LEN - 1] = '\0';

    /* ��ʼ��������tss */
    init_tss(&entity->resource, (pointer) wrapper_func);

    /* ��ʼ��������ldt */
    init_ldt(&entity->resource);

    /* ��ʼ��gdt����, ����tss��ldt */
    init_task_gdt(&entity->resource);

    /* �����̲߳��� */
    set_task_args(entity->resource.stack_addr, entry_task, arg_tab);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
pointer get_task_ldt(HTASK handle)
{
    struct task_handle *id;

    if (OS_NULL != handle) {
        id = (struct task_handle *) handle;
        cassert(id->check == HTASK_CHECK);
        if ((CORE_NUM > id->core_id) && (CPU_NUM > id->cpu_id) && (OS_TASK_NUM > id->task_id)) {
            return (pointer) tcb[id->core_id][id->cpu_id][id->task_id].resource.ldt;
        }
    }
    cassert(OS_FALSE);
    return 0;
}

/***************************************************************
 * description : ȡ������߳�id
 * history     :
 ***************************************************************/
os_u32 get_task_id(os_u32 core_id, os_u32 cpu_id)
{
    if ((CORE_NUM > core_id) && (CPU_NUM > cpu_id)) {
        return current_task[core_id][cpu_id]->handle.task_id;
    }
    return INVALID_TASK_ID;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_u8 *get_task_name(HTASK handle)
{
    struct task_handle *id;

    if (OS_NULL != handle) {
        id = (struct task_handle *) handle;
        cassert(id->check == HTASK_CHECK);
        if ((CORE_NUM > id->core_id) && (CPU_NUM > id->cpu_id) && (OS_TASK_NUM > id->task_id)) {
            return tcb[id->core_id][id->cpu_id][id->task_id].name;
        }
    }
    return OS_NULL;
}

/***************************************************************
 * description : ��ǰ������
 * history     :
 ***************************************************************/
HTASK OS_API current_task_handle(os_void)
{
    if (current_task[get_core_id()][get_cpu_id()]) {
        return &current_task[get_core_id()][get_cpu_id()]->handle;
    }
    /* task module is not initialized. */
    return OS_NULL;
}

/***************************************************************
 * description : ɾ��������Դ
 *               Ҫ�ͷŵ���Դ����:
 *               idle_tid;
 *               ���д��������ڴ�;
 *               ����id��Դ;
 *               ɾ����Ϣ����;
 *               ɾ�����ȶ���;
 * history     :
 ***************************************************************/
LOCALC os_ret delete_task_resource(HTASK handle)
{
    /* 1.�ͷŴ�����Դ */
    free_window_resource(handle);

    /* 2.�ͷŵ��ȶ��� */
    suspend_task(handle, __LINE__);

    /* 3.�ͷ��߳���Ϣ���� */
    free_task_station(handle);

    return OS_SUCC;
}

/***************************************************************
 * description : �߳��˳�����, �ú���������.
 * history     :
 ***************************************************************/
os_void OS_API exit_task(os_void)
{
    HTASK handle;

    /* ��ǰ������ */
    handle = current_task_handle();

    delete_task_resource(handle);

    /* �˴����жϺ���ܷ����ж�.
       ���ʵ���ͷ�, ��ʵ���ڲ���vtask->vcpu->ldt������.
       ���rtc�ж�, ��ǰgs��Ϊ0x14(��ѡ����), pushѹջ, rtc������Ϻ�, pop��ջ.
       ��ʱ��ջ��ջ������Ϊ0x14, ��tss indexѡ����ָʾ���ڴ������Ѿ����.
       ��ջˢ�¶μĴ���ʱ�ͻ��������. */
    schedule();
}

/***************************************************************
 * description : this function is not recommended
 * history     :
 ***************************************************************/
os_void OS_API destroy_task(IN HTASK handle)
{
    struct task_control_block *entity;
    struct task_handle *id;
    os_u32 core_id;
    os_u32 cpu_id;

    if (OS_NULL != handle) {
        id = (struct task_handle *) handle;
        cassert(id->check == HTASK_CHECK);
        core_id = id->core_id;
        cpu_id = id->cpu_id;
        if ((CORE_NUM > id->core_id) && (CPU_NUM > id->cpu_id) && (OS_TASK_NUM > id->task_id)) {
            entity = &tcb[core_id][cpu_id][id->task_id];
            cassert(TCB_CHECK == entity->check);
            spin_lock(&entity->flock);
            entity->del_flag = OS_TRUE;
            if (0 == atomic_read(&entity->ref)) {
                /* delete right now */
                delete_task_resource(handle);
            }
            spin_unlock(&entity->flock);

            if (handle == current_task_handle()) {
                schedule();
            }
        }
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret modify_entity_station(struct task_control_block *handle, os_void *station)
{
    struct task_handle *id;

    cassert((OS_NULL != handle) && (OS_NULL != station));
    id = (struct task_handle *) handle;
    cassert(id->check == HTASK_CHECK);
    if ((CORE_NUM > id->core_id) && (CPU_NUM > id->cpu_id) && (OS_TASK_NUM > id->task_id)) {
        tcb[id->core_id][id->cpu_id][id->task_id].station = station;
        wmb();
        return OS_SUCC;
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void *get_task_station(struct task_handle *handle)
{
    cassert((OS_NULL != handle) && (handle->check == HTASK_CHECK));
    if ((CORE_NUM > handle->core_id) && (CPU_NUM > handle->cpu_id) && (OS_TASK_NUM > handle->task_id)) {
        return tcb[handle->core_id][handle->cpu_id][handle->task_id].station;
    }
    return OS_NULL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_u32 alloc_core_id(os_void)
{
    return CORE_ID_0;
}

/***************************************************************
 * description : task index, ָ��task_entity��task_queue
 *               ��idle_task_index[0]��������index.
 * history     :
 ***************************************************************/
LOCALC os_u32 alloc_cpu_id(os_u32 core_id)
{
    return CPU_ID_0;
}

/***************************************************************
 * description : task index, ָ��task_entity��task_queue
 *               ��idle_task_index[0]��������index.
 * history     :
 ***************************************************************/
LOCALC os_u32 alloc_task_id(os_u32 core_id, os_u32 cpu_id)
{
    return INVALID_TASK_ID;
}

/***************************************************************
 * description : ����������
 * history     :
 ***************************************************************/
LOCALC struct task_control_block *alloc_tcb(os_void)
{
    os_u32 core_id, cpu_id;
    struct task_control_block *task;

    /* 1.����core */
    core_id = alloc_core_id();

    /* 2.����cpu */
    cpu_id = alloc_cpu_id(core_id);

    spin_lock(&idle_tcb[core_id][cpu_id].lock);
    spin_lock(&suspend_tcb[core_id][cpu_id].lock);
    /* unlink from idle */
    task = list_addr(idle_tcb[core_id][cpu_id].head.next, struct task_control_block, node);
    del_list(idle_tcb[core_id][cpu_id].head.next);
    /* add to suspend task */
    add_list_head(&suspend_tcb[core_id][cpu_id].head, &task->node);
    task->status = TASK_STATUS_READY | TASK_STATUS_SUSPEND;
    task->line = __LINE__;
    spin_unlock(&idle_tcb[core_id][cpu_id].lock);
    spin_unlock(&suspend_tcb[core_id][cpu_id].lock);

    cassert(TCB_CHECK == task->check);

    return task;
}

/***************************************************************
 * description : ��װ�̺߳���
 * history     :
 ***************************************************************/
LOCALC os_void OS_CALLBACK task_wrapper_func(TASK_FUNC_PTR entry_task,
                                             os_u32 arg1, os_u32 arg2, os_u32 arg3, os_u32 arg4, os_u32 arg5, os_u32 arg6, os_u32 arg7)
{
    /* �����ڴ����frame, �������ʹ��ebp. */

    entry_task(arg1, arg2, arg3, arg4, arg5, arg6, arg7);

    /* �˳��߳� */
    exit_task();
}

/***************************************************************
 * description : ����������
 * history     :
 ***************************************************************/
HTASK OS_API create_task(IN os_u8 *name, IN TASK_FUNC_PTR entry_task, enum task_priority priority,
                         os_u32 arg1, os_u32 arg2, os_u32 arg3, os_u32 arg4, os_u32 arg5, os_u32 arg6, os_u32 arg7)
{
    struct task_control_block *htask;
    os_u32 arg_tab[TASK_ARGS_NUM];

    cassert((OS_NULL != name) && (OS_NULL != entry_task));

    htask = alloc_tcb();
    if (OS_NULL != htask) {
        /* ���ò����� */
        arg_tab[0] = arg1;
        arg_tab[1] = arg2;
        arg_tab[2] = arg3;
        arg_tab[3] = arg4;
        arg_tab[4] = arg5;
        arg_tab[5] = arg6;
        arg_tab[6] = arg7;

        /* ��ʼ������������ */
        init_task_context(htask, name, task_wrapper_func, entry_task, arg_tab);

        /* ��¼�������ȼ� */
        modify_task_priority(&htask->handle, priority);

        /* ����������ready���ж��� */
        resume_task(&htask->handle, __LINE__);
    }
    return &htask->handle;
}

/***************************************************************
 * description : ��ǰ������delay
 *               �ú������µ�ǰ������������cpu.
 *               0 ��ʾ��Զ
 * history     :
 ***************************************************************/
os_ret OS_API delay_task(os_u32 count, os_u32 line)
{
    struct task_control_block *entity;
    struct task_queue_head *rq, *dq;
    os_u32 core_id;
    os_u32 cpu_id;

    core_id = get_core_id();
    cpu_id = get_cpu_id();

    /* when lock schedule, task will continue running in delay status. */
    cassert(0 == schedule_flag);

    entity = current_task[core_id][cpu_id];
    cassert(OS_NULL != entity);
    cassert(TCB_CHECK == entity->check);
    cassert_word(TASK_STATUS_READY == entity->status, "%s %d %d\n", entity->name, entity->status, schedule_flag);
    cassert(TASK_PRIORITY_CNT > entity->priority);

    rq = &ready_tcb[core_id][cpu_id][entity->priority];
    dq = &delay_tcb[core_id][cpu_id];
    spin_lock(&entity->flock);
    spin_lock(&rq->lock);
    spin_lock(&dq->lock);
    del_list(&entity->node);
    entity->status = TASK_STATUS_DELAY;
    entity->line = line;
    entity->delay_tick = count;
    if (0 == count) { /* forever */
        cassert(OS_FALSE);
    }
    add_list_head(&dq->head, &entity->node);
    spin_unlock(&dq->lock);
    spin_unlock(&rq->lock);
    spin_unlock(&entity->flock);

    /* ǿ�Ƶ���һ�� */
    schedule();
    cassert_word(TASK_STATUS_READY == entity->status, "%s %d %d\n", entity->name, entity->status, schedule_flag);
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void delay_function(os_void)
{
    os_u32 i, j, k;
    struct task_control_block *task;
    struct task_queue_head *rq, *dq;

    for (i = CORE_ID_0; i < CORE_NUM; i++)
    for (j = CPU_ID_0; j < CPU_NUM; j++)
    for (k = MAIN_TASK_ID; k < OS_TASK_NUM; k++) {
        task = &tcb[i][j][k];
        cassert_word(TCB_CHECK == task->check, "tcb: %d-%d-%d", i, j, k);

        spin_lock(&task->flock);
        if (TASK_STATUS_DELAY == task->status) {
            cassert(0 != task->delay_tick);
            task->delay_tick--;
            if (0 == task->delay_tick) {
                rq = &ready_tcb[i][j][task->priority];
                dq = &delay_tcb[i][j];
                spin_lock(&rq->lock);
                spin_lock(&dq->lock);
                task->status = TASK_STATUS_READY;
                task->line = __LINE__;
                cassert(TASK_PRIORITY_CNT > task->priority);
                del_list(&task->node);
                add_list_head(&rq->head, &task->node);
                spin_unlock(&dq->lock);
                spin_unlock(&rq->lock);
            }
        }
        spin_unlock(&task->flock);
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void dump_task_stack_wl(os_u32 core_id, os_u32 cpu_id)
{
    os_u32 i, c;
    struct task_control_block *t;
    os_u8 *stack;

    if ((CORE_NUM > core_id) && (CPU_NUM > cpu_id)) {
        for (i = 0; i < OS_TASK_NUM; i++) {
            t = &tcb[core_id][cpu_id][i];
            if ('\0' != t->name[0]) {
                /* dump the water-level */
                stack = (os_u8 *)(t->resource.stack_addr - (t->resource.stack_len - TASK_ARGS_RESERVE));
                c = 0;
                while (STACK_FILL == *++stack) {
                    c++;
                }
                print("task_id:%s, %d\n", t->name, c);
            }
        }
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_u32 dump_task_pc(HTASK handle)
{
    struct task_handle *h;
    struct task_control_block *t;
    os_u32 eip;

    h = handle;
    eip = 0;
    if (OS_NULL != h) {
        if ((CORE_NUM > h->core_id) && (CPU_NUM > h->cpu_id) && (OS_TASK_NUM > h->task_id)) {
            t = &tcb[h->core_id][h->cpu_id][h->task_id];
            eip = t->resource.tss.eip;
        }
    }
    return eip;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void dump_task_info(os_void)
{
    struct task_queue_head *head;
    struct list_node *node;
    struct task_control_block *task;
    os_u32 i;
    os_u32 c;
    os_u8 *stack;
    lock_t eflag;

    lock_int(eflag);
    print("task info:\n");
    for (i = 0; i < OS_TASK_NUM; i++) {
        if ('\0' != tcb[0][0][i].name[0]) {
            print("task_id:%d, %s, %d,", tcb[0][0][i].handle.task_id, tcb[0][0][i].name, tcb[0][0][i].status);
            stack = (os_u8 *)(tcb[0][0][i].resource.stack_addr - (tcb[0][0][i].resource.stack_len - TASK_ARGS_RESERVE));

            /* dump the water-level */
            c = 0;
            while (STACK_FILL == *++stack) { c++; }
            print("(%x, %x) %x\n", tcb[0][0][i].resource.stack_addr, tcb[0][0][i].resource.stack_len, c);
            /* main task do not have call stack bottom. */
            if (MAIN_TASK_ID != tcb[0][0][i].handle.task_id) {
                _dump_stack(print, tcb[0][0][i].resource.tss.ebp);
            }
        }
    }

    print("ready:");
    for (i = 0; i < TASK_PRIORITY_CNT; i++) {
        head = &ready_tcb[get_core_id()][get_cpu_id()][i];
        loop_list(node, &head->head) {
            task = list_addr(node, struct task_control_block, node);
            print("[%s p%d %d]", task->name, i, task->line);
        }
    } print("\n");

    print("pend:");
    head = &pend_tcb[get_core_id()][get_cpu_id()];
    loop_list(node, &head->head) {
        task = list_addr(node, struct task_control_block, node);
        print("[%s %d]", task->name, task->line);
    } print("\n");

    print("delay:");
    head = &delay_tcb[get_core_id()][get_cpu_id()];
    loop_list(node, &head->head) {
        task = list_addr(node, struct task_control_block, node);
        print("[%s %d, %d %d %d, left: %d]", task->name, task->line, get_core_id(), get_cpu_id(), task->handle.task_id, task->delay_tick);
    } print("\n");

    print("suspend:");
    head = &suspend_tcb[get_core_id()][get_cpu_id()];
    loop_list(node, &head->head) {
        task = list_addr(node, struct task_control_block, node);
        print("[%s %d]", task->name, task->line);
    } print("\n");

    unlock_int(eflag);
}
LOCALD os_u8 task_entity_debug_name[] = { "tentity" };
LOCALD struct dump_info task_entity_debug = {
    task_entity_debug_name,
    dump_task_info
};

#include <entity_snapshot.cp>

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void init_task(os_void)
{
    os_u32 i, j, k;
    os_u32 ktask_addr;

    /* ���task_entity */
    mem_set(tcb, 0, OS_TASK_NUM*CPU_NUM*CORE_NUM * sizeof(struct task_control_block));

    /* allocate absolute virtual memory.
       because stack memory will be used when switching back to real mode.
       real mode switching code requires all linear address is identity mapped to physical address. */
    ktask_addr = alloc_ksm_absv(OS_KTASK_STACK_LEN*OS_TASK_NUM*CPU_NUM*CORE_NUM);
    cassert(0 != ktask_addr);
    mem_set(ktask_addr, STACK_FILL, OS_KTASK_STACK_LEN*OS_TASK_NUM*CPU_NUM*CORE_NUM);

    /* initialize queue head */
    for (i = CORE_ID_0; i < CORE_NUM; i++)
    for (j = CPU_ID_0; j < CPU_NUM; j++) {
        init_spinlock(&idle_tcb[i][j].lock);
        init_list_head(&idle_tcb[i][j].head);
        for (k = TASK_PRIORITY_0; k < TASK_PRIORITY_CNT; k++) {
            init_spinlock(&ready_tcb[i][j][k].lock);
            init_list_head(&ready_tcb[i][j][k].head);
        }
        init_spinlock(&pend_tcb[i][j].lock);
        init_list_head(&pend_tcb[i][j].head);
        init_spinlock(&delay_tcb[i][j].lock);
        init_list_head(&delay_tcb[i][j].head);
        init_spinlock(&suspend_tcb[i][j].lock);
        init_list_head(&suspend_tcb[i][j].head);
    }

    /* dummy section is start from 0x1000000 */
    cassert((CORE_NUM * CPU_NUM) <= ((((pointer) dummy & 0xffff0000) - OS_STACK_BOTTOM)/OS_STACK_LEN));
    mem_set(OS_STACK_BOTTOM, STACK_FILL, ((pointer) dummy & 0xffff0000) - OS_STACK_BOTTOM -TASK_ARGS_RESERVE);

    /* �ں�������task_entity��ʼ�� */
    for (i = CORE_ID_0; i < CORE_NUM; i++)
    for (j = CPU_ID_0; j < CPU_NUM; j++) {
        tcb[i][j][MAIN_TASK_ID].handle.core_id = i;
        tcb[i][j][MAIN_TASK_ID].handle.cpu_id = j;
        tcb[i][j][MAIN_TASK_ID].handle.task_id = MAIN_TASK_ID;
        tcb[i][j][MAIN_TASK_ID].handle.check = HTASK_CHECK;

        init_spinlock(&tcb[i][j][MAIN_TASK_ID].flock);
        atomic_init(&tcb[i][j][MAIN_TASK_ID].ref);
        tcb[i][j][MAIN_TASK_ID].del_flag = OS_FALSE;

        tcb[i][j][MAIN_TASK_ID].name[0] = '\0';

        tcb[i][j][MAIN_TASK_ID].resource.tss_selector = alloc_gdt_item();
        tcb[i][j][MAIN_TASK_ID].resource.ldt_selector = alloc_gdt_item();
        tcb[i][j][MAIN_TASK_ID].resource.page_dir_addr = PAGE_DIR_ADDR;
        tcb[i][j][MAIN_TASK_ID].resource.stack_addr = ((pointer) dummy & 0xffff0000) + i*j*OS_STACK_LEN - TASK_ARGS_RESERVE;
        tcb[i][j][MAIN_TASK_ID].resource.stack_len = OS_STACK_LEN;

        tcb[i][j][MAIN_TASK_ID].priority = IDLE_TASK_PRIORITY;

        init_list_head(&tcb[i][j][MAIN_TASK_ID].node);
        add_list_head(&ready_tcb[i][j][IDLE_TASK_PRIORITY].head, &tcb[i][j][MAIN_TASK_ID].node);

        tcb[i][j][MAIN_TASK_ID].delay_tick = 0;
        tcb[i][j][MAIN_TASK_ID].status = TASK_STATUS_READY;
        tcb[i][j][MAIN_TASK_ID].line = __LINE__;

        tcb[i][j][MAIN_TASK_ID].station = OS_NULL;

        tcb[i][j][MAIN_TASK_ID].check = TCB_CHECK;

        tcb[i][j][MAIN_TASK_ID].handle.core_id = i;
        tcb[i][j][MAIN_TASK_ID].handle.cpu_id = j;
        tcb[i][j][MAIN_TASK_ID].handle.task_id = MAIN_TASK_ID;

        /* current task is main task */
        current_task[i][j] = &tcb[i][j][MAIN_TASK_ID];
    }

    /* ��ʼ�����̶�ջ */
    for (i = CORE_ID_0; i < CORE_NUM; i++)
    for (j = CPU_ID_0; j < CPU_NUM; j++)
    for (k = MAIN_TASK_ID + 1; k < OS_TASK_NUM; k++) {
        /* �̶߳����� */
        tcb[i][j][k].handle.core_id = i;
        tcb[i][j][k].handle.cpu_id = j;
        tcb[i][j][k].handle.task_id = k;
        tcb[i][j][k].handle.check = HTASK_CHECK;

        init_spinlock(&tcb[i][j][k].flock);
        atomic_init(&tcb[i][j][k].ref);
        tcb[i][j][k].del_flag = OS_FALSE;

        init_list_head(&tcb[i][j][k].node);
        add_list_head(&idle_tcb[i][j].head, &tcb[i][j][k].node);

        tcb[i][j][k].name[0] = '\0';

        /* gdt��Դ */
        tcb[i][j][k].resource.tss_selector = alloc_gdt_item();
        tcb[i][j][k].resource.ldt_selector = alloc_gdt_item();
        /* ��ջ��͵�ַ����, ʹ��i */
        tcb[i][j][k].resource.page_dir_addr = PAGE_DIR_ADDR;
        tcb[i][j][k].resource.stack_addr = ktask_addr + k*OS_KTASK_STACK_LEN - TASK_ARGS_RESERVE;
        tcb[i][j][k].resource.stack_len = OS_KTASK_STACK_LEN;

        tcb[i][j][k].priority = INVALID_TASK_PRIORITY;
        tcb[i][j][k].delay_tick = 0;
        tcb[i][j][k].status = TASK_STATUS_READY | TASK_STATUS_SUSPEND;
        tcb[i][j][k].line = __LINE__;

        tcb[i][j][k].station = OS_NULL;

        tcb[i][j][k].check = TCB_CHECK;

        tcb[i][j][k].handle.core_id = i;
        tcb[i][j][k].handle.cpu_id = j;
        tcb[i][j][k].handle.task_id = k;
    }

    init_system_snapshot();
    if (OS_SUCC != register_dump(&task_entity_debug)) {
        flog("task entity register dump fail\n");
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void line_task(HTASK handle, os_u32 line)
{
    struct task_handle *id;

    if (OS_NULL != handle) {
        id = (struct task_handle *) handle;
        cassert(id->check == HTASK_CHECK);
        if ((CORE_NUM > id->core_id) && (CPU_NUM > id->cpu_id) && (OS_TASK_NUM > id->task_id)) {
            tcb[id->core_id][id->cpu_id][id->task_id].line = line;
        }
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret verify_htask(HTASK handle)
{
    struct task_handle *id;

    if (OS_NULL != handle) {
        id = (struct task_handle *) handle;
        if (id->check == HTASK_CHECK) {
            return OS_SUCC;
        }
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void init_screen(os_void)
{
    enum vga_color i;
    screen_csys p0,p1;
    screen_csys r;
    os_u32 init_screen_color[] = {
        VGA_COLOR_BLACK,
        VGA_COLOR_NAVY,
        VGA_COLOR_GREEN,
        VGA_COLOR_TEAL,
        VGA_COLOR_MAROON,
        VGA_COLOR_PURPLE,
        VGA_COLOR_OLIVE,
        VGA_COLOR_SILVER,
        VGA_COLOR_GRAY,
        VGA_COLOR_BLUE,
        VGA_COLOR_LIME,
        VGA_COLOR_AQUA,
        VGA_COLOR_RED,
        VGA_COLOR_FUCHSIA,
        VGA_COLOR_YELLOW,
        VGA_COLOR_WHITE
    };

    r = current_resolution();

    /* ˢ����Ļ */
    for (i = 0; i < array_size(init_screen_color); i++) {
        p0.x = 40 * i;
        p0.y = 0;
        p1.x = 40 * (i+1);
        p1.y = r.y;
        draw_rect(p0, p1, init_screen_color[i]);
    }
}

/***************************************************************
 * description : init_abstract_call()
 * history     :
 ***************************************************************/
LOCALC os_void load_abstract_module(os_void)
{
    os_u32 *tmp;
    INIT_FUNC func;
    lock_t flag;

    GLOBALREFD os_u32 _ainit_start;
    GLOBALREFD os_u32 _ainit_end;

    save_eflag(flag);

    sti();

    for (tmp = &_ainit_start; tmp < &_ainit_end; tmp++) {
        func = (INIT_FUNC)(*tmp);
        if (OS_NULL != func) {
            (*func)();
        }
    }

    restore_eflag(flag);
}

/***************************************************************
 * description : init_module_call()
 * history     :
 ***************************************************************/
LOCALC os_void load_module(os_void)
{
    os_u32 *tmp;
    INIT_FUNC func;
    GLOBALREFD os_u32 _minit_start;
    GLOBALREFD os_u32 _minit_end;

    for (tmp = &_minit_start; tmp < &_minit_end; tmp++) {
        func = (INIT_FUNC)(*tmp);
        if (OS_NULL != func) {
            (*func)();
        }
    }
}

/***************************************************************
 * description : init_device_call()
 * history     :
 ***************************************************************/
LOCALC os_void load_device_driver(os_void)
{
    os_u32 *tmp;
    INIT_FUNC func;
    lock_t flag;

    GLOBALREFD os_u32 _dinit_start;
    GLOBALREFD os_u32 _dinit_end;

    save_eflag(flag);

    sti();

    for (tmp = &_dinit_start; tmp < &_dinit_end; tmp++) {
        func = (INIT_FUNC)(*tmp);
        if (OS_NULL != func) {
            (*func)();
        }
    }

    restore_eflag(flag);
}

/***************************************************************
 * description : init_driver_call()
 * history     :
 ***************************************************************/
LOCALC os_void load_driver(os_void)
{
    os_u32 *tmp;
    INIT_FUNC func;
    lock_t flag;

    GLOBALREFD os_u32 _driver_init_start;
    GLOBALREFD os_u32 _driver_init_end;

    save_eflag(flag);

    sti();

    for (tmp = &_driver_init_start; tmp < &_driver_init_end; tmp++) {
        func = (INIT_FUNC)(*tmp);
        if (OS_NULL != func) {
            (*func)();
        }
    }

    restore_eflag(flag);
}

/***************************************************************
 * description : init_bus_info()
 * history     :
 ***************************************************************/
LOCALC os_void load_bus_driver(os_void)
{
    os_uint *tmp;
    os_uint i;
    struct __init_bus_info *bm;
    lock_t flag;

    GLOBALREFD os_uint _binit_start;
    GLOBALREFD os_uint _binit_end;

    save_eflag(flag);

    sti();

    /* init abstract layer */
    for (tmp = &_binit_start; tmp < &_binit_end; tmp++) {
        bm = (struct __init_bus_info *)(*tmp);
        if (BUS_P0 == bm->priority) {
            cassert(OS_NULL != bm->func);
            bm->func();
        }
    }

    for (i = BUS_Px - 1; i > BUS_P0; i--) {
        for (tmp = &_binit_start; tmp < &_binit_end; tmp++) {
            bm = (struct __init_bus_info *)(*tmp);
            if (i == bm->priority) {
                cassert(OS_NULL != bm->func);
                bm->func();
            }
        }
    }

    restore_eflag(flag);
}

/***************************************************************
 * description : ���ع���ģ��, ��������shell��
 * history     :
 ***************************************************************/
LOCALC os_void load_public_module(os_void)
{
    /* ���س���ģ�� */
    load_abstract_module();

    /* ������������ */
    load_bus_driver();

    /* ������������ */
    load_device_driver();

    /* ����ϵͳģ�� */
    load_module();

    /* ���طǺ������� */
    load_driver();
}

/* ���������� */
LOCALD struct task_control_block *loader_handle = OS_NULL;

/***************************************************************
 * description : ��ʼ����
 * history     :
 ***************************************************************/
os_void active_loader(os_void)
{
    resume_task(&loader_handle->handle, __LINE__);
}

/***************************************************************
 * description : TASK_FUNC_PTR
 * history     :
 ***************************************************************/
LOCALC os_void OS_CALLBACK loader_task(TASK_FUNC_PTR entry_task, os_u32 arg1, os_u32 arg2, os_u32 arg3, os_u32 arg4, os_u32 arg5, os_u32 arg6, os_u32 arg7)
{
    os_u16 main_task_tr_sel = (GDT_NULL_TSS_INDEX + 2) * 8;
    struct ljmp_para ljmp_addr;

    /* ������ת��loaderʱ, Ĭ��IFΪ1, �����ж� */
    //init_screen();

    /* goto root task */
    ljmp_addr.offset = 0;
    ljmp_addr.sel = main_task_tr_sel;
    __asm__ __volatile__("ljmp *%0\n\t" \
                         ::"m"(ljmp_addr));

    flog("load module\n");

    /* ִ�е�����˵��core�����Ѿ�ִ�����, ����idle̬����ʼ�����л� */
    load_public_module();

    /* û�а�װ, ��Ҫ�ָ�����״̬ */
    suspend_task(&loader_handle->handle, __LINE__);
    schedule();
    cassert(OS_FALSE);
}

/***************************************************************
 * description : ԽȨʹ��, create_task
 * history     :
 ***************************************************************/
os_void init_core_task(os_void)
{
    os_u32 core_id;
    os_u32 cpu_id;
    os_u16 tr_sel = (GDT_NULL_TSS_INDEX + 2) * 8;
    os_u16 ldt_sel = (GDT_NULL_LDT_INDEX + 2) * 8;
    os_u32 arg_tab[TASK_ARGS_NUM];

    arg_tab[0] = 1;
    arg_tab[1] = 2;
    arg_tab[2] = 3;
    arg_tab[3] = 4;
    arg_tab[4] = 5;
    arg_tab[5] = 6;
    arg_tab[6] = 7;

    core_id = get_core_id();
    cpu_id = get_cpu_id();

    /* main task */
    init_task_context(&tcb[core_id][cpu_id][MAIN_TASK_ID], "core", OS_NULL, OS_NULL, arg_tab);
    /* main task is ready */

    __asm__ __volatile__("\tmovw %0,%%ax\n"
                         "\tltr %%ax\n"
                         "\tmovw %1,%%ax\n"
                         "\tlldt %%ax\n"
                         :
                         :"m"(tr_sel), "m"(ldt_sel)
                         :"eax");

    /* fresh regs */
    __asm__ __volatile__("ljmp $"asm_str(GDT_CODE32_INDEX * 8)",$1f;\
                          1:\
                          nop");

    /* null task */
    loader_handle = alloc_tcb();
    init_task_context(loader_handle, "loader", loader_task, OS_NULL, arg_tab);
    modify_task_priority(&loader_handle->handle, TASK_PRIORITY_6);
    /* null task״̬Ĭ��Ϊsuspend */

    /* �����л� */
#if 0
    ljmp_para ljmp_addr = {0};
    ljmp_addr.offset = 0;
    ljmp_addr.sel = tcb[task_id].vtask.vcpu.tss_selector*8; //temp_task_tr_sel;
    __asm__ __volatile__("ljmp *%0\n\t"\
                         ::"m"(ljmp_addr));
#endif
    handover_task(loader_handle);
}

