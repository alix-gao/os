/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : semaphore.c
 * version     : 1.0
 * description : (key) 同步与互斥
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include "semaphore.h"

/***************************************************************
 global variable declare
 ***************************************************************/
/* 信号量控制块信息 */
LOCALD struct semcb_info_struct semcb_info;
LOCALD spinlock_t semcb_lock;

/* 信号量控制块起始地址 */
LOCALD os_u32 semcb_addr = 0;

/* 信号量内存起始地址 */
LOCALD os_u32 sem_addr = 0;

/***************************************************************
 function declare
 ***************************************************************/

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void check_sem(os_uint line)
{
    os_u32 i;
    os_uint cnt;
    lock_t eflag;
    struct sem_struct *sem_entity;
    struct semcb_queue_struct *element;

    sem_entity = (struct sem_struct *) sem_addr;

    lock_int(eflag);
    for (i = 0; i < SEMB_NUM; i++) {
        element = sem_entity->head;
        cnt = 0;
        while (element) {
            cnt++;
            if (OS_FAIL == verify_htask(element->handle)) {
                break;
            }
            element = element->next;
        }
        sem_entity++;
    }
    unlock_int(eflag);

    if (i < SEMB_NUM) {
        print("name: %d-%s %d, %d\n", line, sem_entity->name, sem_entity->line, sem_entity->num);
        cassert(OS_FALSE);
    }
    print("sem check ok\n");
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void dump_sem_info(os_void)
{
    os_u32 i;
    struct sem_struct *sem_entity;
    struct semcb_queue_struct *element;

    sem_entity = (struct sem_struct *) sem_addr;

    print("sem addr: %x %x\n", semcb_addr, sem_addr);

    for (i = 0; i < SEMB_NUM; i++) {
        if (sem_entity->line) { // used
            print("name: %s %d %d,", sem_entity->name, sem_entity->line, sem_entity->sem.count);
            if (sem_entity->holder) {
                print("user:%s,", get_task_name(sem_entity->holder));
            } else {
                print("user:null,");
            }
            print("waiter %d:", sem_entity->num);
            element = sem_entity->head;
            while (element) {
                if (get_task_name(element->handle)) {
                    print("xxx ");
                } else {
                    print("%s ", get_task_name(element->handle));
                }
                element = element->next;
            } print("%x\n", &sem_entity->sem);
        }
        sem_entity++;
    }
}

LOCALD os_u8 sem_debug_name[] = { "sem" };
LOCALD struct dump_info sem_debug = {
    sem_debug_name,
    dump_sem_info
};

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void init_semaphore(os_void)
{
    os_u32 i = 0;
    struct sem_struct *sem_entity = OS_NULL;
    struct semcb_struct *temp_semcb_addr = OS_NULL;

    /* 分配信号量静态内存, 一起分配防止浪费 */
    semcb_addr = alloc_ksm(SEMB_NUM * (sizeof(struct semcb_struct) + sizeof(struct sem_struct)));
    cassert(0 != semcb_addr);

    /* 分配信号量内存空间 */
    sem_addr = semcb_addr + SEMB_NUM*sizeof(struct semcb_struct);

    /* 内存块数量 */
    semcb_info.idle_num = SEMB_NUM;
    /* 内存空闲块链表 */
    semcb_info.idle_block = (struct semcb_struct *) semcb_addr;

    for (i = 0; i < SEMB_NUM; i++) {
        /* 内存块部分 */
        sem_entity = (struct sem_struct *) sem_addr + i;
        temp_semcb_addr = (struct semcb_struct *) semcb_addr + i;

        sem_entity->semcb = temp_semcb_addr;
        sem_entity->line = 0;
        sem_entity->holder = OS_NULL;
        sem_entity->crc = SEMB_CRC;
        sem_entity->sem.count = 0;
        sem_entity->sem.lock.lock = 0;
        sem_entity->sem.lock.eflag.eflag = 0;
        sem_entity->num = 0;
        sem_entity->head = OS_NULL;

        /* 设置内存块地址, 指向信号量实体 */
        temp_semcb_addr->addr = sem_entity;
        /* 设置内存控制块地址 */
        temp_semcb_addr->next = temp_semcb_addr + 1;
    }
    /* 链表收尾 */
    temp_semcb_addr->next = OS_NULL;

    init_spinlock(&semcb_lock);

    if (OS_SUCC != register_dump(&sem_debug)) {
        flog("sem register dump fail\n");
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC HEVENT alloc_sem(os_s32 count, os_u8 *name, os_u32 line_no)
{
    struct semcb_struct *idle_block;
    struct sem_struct *sem_entity;

    /* 无符号数 */
    if (0 != semcb_info.idle_num) {
        spin_lock(&semcb_lock);
        /* double check */
        if (0 != semcb_info.idle_num) {
            idle_block = semcb_info.idle_block;

            semcb_info.idle_block = idle_block->next;
            semcb_info.idle_num--;

            idle_block->next = OS_NULL;

            /* 记录内存块头信息 */
            sem_entity = idle_block->addr;
            if (name) {
                sem_entity->name = name;
            } else {
#define SEM_NONAME "unkown"
                sem_entity->name = SEM_NONAME;
            }
            sem_entity->num = 0;
            sem_entity->head = OS_NULL;
            sem_entity->holder = current_task_handle();
            sem_entity->line = line_no;
            sem_entity->sem.count = count;
            init_spinlock(&sem_entity->sem.lock);
        }
        spin_unlock(&semcb_lock);

        return &sem_entity->sem;
    }
    flog("alloc_sem, idle %d\n", semcb_info.idle_num);
    /* 符合块大小的内存块分配完毕 */
    return OS_NULL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC HEVENT free_sem(HEVENT addr)
{
    struct sem_struct *sem_entity = OS_NULL;
    struct semcb_struct *temp_semcb = OS_NULL;

    if (OS_NULL != addr) {
        /* 取偏移量 */
        sem_entity = (struct sem_struct *)((pointer) addr - (pointer) &((struct sem_struct *) 0)->sem);
        cassert(SEMB_CRC == sem_entity->crc);
        if (SEMB_CRC == sem_entity->crc) {
            spin_lock(&semcb_lock);

            /* 清行号 */
            sem_entity->line = 0;

            temp_semcb = sem_entity->semcb;
            temp_semcb->next = semcb_info.idle_block;

            semcb_info.idle_block = temp_semcb;
            semcb_info.idle_num++;

            spin_unlock(&semcb_lock);

            return &sem_entity->sem;
        }
    }
    /* 重复释放内存 | 校验位错误 */
    return OS_NULL;
}

/***************************************************************
 * description : 记录阻塞的任务task_id
 * history     :
 ***************************************************************/
LOCALC os_ret record_sem_task_id(struct sem_struct *sem, HTASK handle)
{
    struct semcb_queue_struct *element;

    cassert(OS_NULL != sem);

    if ((0 != str_cmp(sem->name, "pit"))
     && (0 == str_cmp(get_task_name(handle), "pit"))) {
        print("warning: wait for sem in timer callback\n");
        dump_stack(print);
    }

    cassert(SEMB_CRC == sem->crc);
    if (SEMB_CRC == sem->crc) {
        element = sem->head;
        while (OS_NULL != element) {
            if (element->handle == handle) {
                return OS_SUCC;
            }
            element = element->next;
        }

        /* 分配动态内存 */
        element = kmalloc(sizeof(struct semcb_queue_struct));
        if (OS_NULL == element) {
            /* panic */
            cassert(OS_FALSE);
            return OS_FAIL;
        }

        /* 插入节点 */
        element->handle = handle;
        element->next = sem->head;
        element->who = (pointer) sem;

        sem->head = element;
        sem->num++;

#ifdef SEM_DEBUG
        print("@@%s's %s add %d waiter %s\n", get_task_name(sem->holder), sem->name, sem->num, get_task_name(element->handle));
#endif
        return OS_SUCC;
    }
    flog("record sem, crc fail.\n");
    return OS_FAIL;
}

/***************************************************************
 * description : 激活所有阻塞的任务
 * history     :
 ***************************************************************/
LOCALC os_ret active_sem_task_id(struct sem_struct *sem, os_u32 line)
{
    struct semcb_queue_struct *element = OS_NULL;
    os_u32 priority;
    os_u32 high_priority = LOWEST_TASK_PRIORITY;

    cassert(OS_NULL != sem);
    cassert(SEMB_CRC == sem->crc);
    if (SEMB_CRC == sem->crc) {
        if (OS_NULL == sem->head) {
            /* 不需要任务切换 */
            return OS_FAIL;
        }

#ifdef SEM_DEBUG
        print("!!%s's %s del %d waiters: ", get_task_name(sem->holder), sem->name, sem->num);
#endif
        while ((OS_NULL != sem->head) && (0 < sem->num)) {
            element = sem->head;
            ready_task(element->handle, __LINE__);
#ifdef SEM_DEBUG
            print("%s ", get_task_name(element->handle));
#endif
            /* 记录优先级 */
            priority = get_task_priority(element->handle);
            if (high_priority < priority) {
                high_priority = priority;
            }

            /* 任务就绪失败, 同样删除节点 */
            sem->head = element->next;
            sem->num--;

            /* 内部置空 */
            kfree(element);

            /* 此处显式强制调度会导致另一个任务pend强制调度后.
               任务回到此处, 而此时的head指针因为任务切换取信号量改变.
               造成一个cpu的循环浪费. */
            //schedule();
        }
        cassert_word((OS_NULL == sem->head) && (0 == sem->num), "%s's %s, head: %x, num: %d\n", get_task_name(sem->holder), sem->name, sem->head, sem->num);
        sem->holder = OS_NULL;

        /* 正确的做法是在此处显式强制调度
           将所有的任务全部激活后, 一次性调度. 可以由优先级来保证执行顺序.
           注意优先级的数值越大, 优先级越高. */
        if (high_priority > get_task_priority(current_task_handle())) {
            /* 返回成功表示产生调度 */
            return OS_SUCC;
        }
    }
    return OS_FAIL;
}

/***************************************************************
 * description : 创建01信号量
 * history     :
 ***************************************************************/
HEVENT OS_API create_event_handle(os_s32 status, os_u8 *name, os_u32 line_no)
{
    if ((EVENT_INVALID == status) || (EVENT_VALID == status)) {
        /* 分配信号量资源 */
        return alloc_sem(status, name, line_no);
    } else {
        return OS_NULL;
    }
}

/***************************************************************
 * description : 释放01信号量
 * history     :
 ***************************************************************/
HEVENT OS_API destroy_event_handle(IN HEVENT handle)
{
    if (OS_NULL != handle) {
        return free_sem(handle);
    }
    return OS_NULL;
}

/***************************************************************
 * description : 释放信号量, 存在阻塞任务优先级高于当前任务时产生调度.
 *               返回是否产生了调度.
 * history     :
 ***************************************************************/
os_ret OS_API notify_event(IN HEVENT handle, os_u32 line)
{
    os_ret ret;
    struct sem_struct *sem_entity;

    if (OS_NULL != handle) {
        sem_entity = (struct sem_struct *)(handle - (pointer) &((struct sem_struct *) 0)->sem);
        cassert(SEMB_CRC == sem_entity->crc);
        if (SEMB_CRC == sem_entity->crc) {
            spin_lock(&sem_entity->sem.lock);
            mb();
            sem_entity->sem.count = EVENT_VALID;
            /* 唤醒该信号量阻塞的任务 */
            ret = active_sem_task_id(sem_entity, line);
            spin_unlock(&sem_entity->sem.lock);
            if (OS_SUCC == ret) {
                schedule();
            }
            return ret;
        }
    }
    cassert(OS_NULL != handle);
    flog("give bsem, input para error %d\n", line);
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret OS_CALLBACK process_sem_timeout(os_u32 data)
{
    HTASK handle;

    handle = (HTASK) data;
    cassert(OS_NULL != handle);
    ready_task(handle, __LINE__);
    return OS_SUCC;
}

/***************************************************************
 * description : 获取信号量, 0表示永远
 *               返回OS_TIMEOUT表示等待超时
 * history     :
 ***************************************************************/
os_ret OS_API wait_event(IN HEVENT handle, os_u32 ms)
{
    struct sem_struct *sem_entity;
    HTIMER sem_timer;
    os_u32 mem;
    HTASK task;

    /* 入参检查 */
    if (OS_NULL != handle) {
        sem_entity = (struct sem_struct *)(handle - (pointer) &((struct sem_struct *) 0)->sem);
        cassert(SEMB_CRC == sem_entity->crc);
        if (SEMB_CRC == sem_entity->crc) {
            task = current_task_handle();
            for (;;) {
                spin_lock(&sem_entity->sem.lock);
                __asm__ __volatile__("movl $"asm_str(EVENT_INVALID)",%%eax\n\t"
                                     "leal %1,%%ebx\n\t"
                                     "xchgl (%%ebx),%%eax\n\t"
                                     "movl %%eax,%0"
                                     :"=m"(mem) /* 使用内存变量 */
                                     :"m"(sem_entity->sem.count) /* variable->address */
                                     :"eax","ebx","memory"); /* no mb() */
                if (EVENT_VALID == mem) { /* 取到信号量 */
                    sem_entity->holder = current_task_handle();
                    spin_unlock(&sem_entity->sem.lock);
                    break;
                }

                /* 记录当前信号量阻塞的任务 */
                record_sem_task_id(sem_entity, task);
                /* 阻塞当前任务, 强制调度 */
                pend_task(task, __LINE__);
                /* bugfix: record and pend should be locked together,
                   task is removed from sem list, and status is pend */
                spin_unlock(&sem_entity->sem.lock);
                if (0 != ms) {
                    sem_timer = set_timer_callback((os_u32) task, divl_cell(ms, 1000/OS_HZ), process_sem_timeout, TIMER_MODE_NOT_LOOP);
                    if (OS_NULL == sem_timer) {
                        /* 没有定时器资源, 暂时一直死等 */
                        flog("bsem alloc timer fail!\n");
                        cassert(OS_FALSE);
                        schedule();
                    } else {
                        os_u32 tick;
                        tick = system_tick();
                        schedule();
                        kill_timer(sem_timer);
                        if (ms < (1000/OS_HZ)*(system_tick() - tick)) {
                            sem_entity->sem.count = EVENT_VALID;
                            return OS_TIMEOUT;
                        }
                    }
                } else {
                    schedule();
                }
            }
            return OS_SUCC;
        }
        flog("take bsem(%x) crc fail\n", handle);
    }
    /* 入参检查失败 */
    return OS_FAIL;
}

/***************************************************************
 * description : binary semphore and counter semphore
 * history     :
 ***************************************************************/
os_ret OS_API query_event(IN HEVENT handle, os_s32 *status)
{
    struct sem_struct *sem_entity;

    if ((OS_NULL != handle) && (OS_NULL != status)) {
        sem_entity = (struct sem_struct *)(handle - (pointer) &((struct sem_struct *) 0)->sem);
        if (SEMB_CRC == sem_entity->crc) {
            *status = sem_entity->sem.count;
            return OS_SUCC;
        }
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
HEVENT OS_API create_mevent(os_s32 count, os_u8 *name, os_u32 line)
{
    return alloc_sem(count, name, line);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API destroy_mevent(IN HEVENT handle)
{
    if (OS_NULL != handle) {
        free_sem(handle);
        return OS_SUCC;
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API notify_mevent(IN HEVENT handle, os_u32 line)
{
    os_ret ret;
    struct sem_struct *sem_entity;

    if (OS_NULL != handle) {
        sem_entity = (struct sem_struct *)(handle - (pointer) &((struct sem_struct *) 0)->sem);
        cassert(SEMB_CRC == sem_entity->crc);
        if (SEMB_CRC == sem_entity->crc) {
            spin_lock(&sem_entity->sem.lock);
            __asm__ __volatile__("lock; incl %0\n\t"
                                 :
                                 :"m"(sem_entity->sem.count) /* variable->address */
                                 :"memory"); /* no mb() */
            ret = active_sem_task_id(sem_entity, line);
            spin_unlock(&sem_entity->sem.lock);
            if (OS_SUCC == ret) {
                schedule();
            }
            return ret;
        }
    }
    flog("give csem, input para error %d\n", line);
    return OS_FAIL;
}

/***************************************************************
 * description : 获取信号量, time为tick值, 0 表示永远
 *               返回OS_TIMEOUT表示等待超时
 * history     :
 ***************************************************************/
os_ret OS_API wait_mevent(IN HEVENT handle, os_u32 ms)
{
    struct sem_struct *sem_entity;
    HTIMER sem_timer;
    HTASK task;

    if (OS_NULL != handle) {
        sem_entity = (struct sem_struct *)(handle - (pointer) &((struct sem_struct *) 0)->sem);
        cassert(SEMB_CRC == sem_entity->crc);
        if (SEMB_CRC == sem_entity->crc) {
            task = current_task_handle();
            for (;;) {
                os_u8 flag = 0;

                spin_lock(&sem_entity->sem.lock);
                if (0 < sem_entity->sem.count) {
                    __asm__ __volatile__("lock; decl %0\n\t"
                                         :
                                         :"m"(sem_entity->sem.count) /* variable->address */
                                         :"memory"); /* no mb() */
                    flag = 1;
                }
                if (flag) { /* get sem */
                    sem_entity->holder = current_task_handle();
                    spin_unlock(&sem_entity->sem.lock);
                    return OS_SUCC;
                }

                record_sem_task_id(sem_entity, task);
                pend_task(task, __LINE__);
                spin_unlock(&sem_entity->sem.lock);
                if (0 != ms) {
                    sem_timer = set_timer_callback((pointer) task, divl_cell(ms, 1000/OS_HZ), process_sem_timeout, TIMER_MODE_NOT_LOOP);
                    if (OS_NULL == sem_timer) {
                        flog("csem alloc timer fail!\n");
                        schedule();
                    } else {
                        os_u32 tick;
                        tick = system_tick();
                        schedule();
                        kill_timer(sem_timer);
                        if (ms < (1000/OS_HZ)*(system_tick() - tick)) {
                            return OS_TIMEOUT;
                        }
                    }
                } else {
                    schedule();
                }
            }
            return OS_SUCC;
        }
        flog("take csem(%x) crc fail\n", handle);
        dump_stack(flog);
    }
    return OS_FAIL;
}

