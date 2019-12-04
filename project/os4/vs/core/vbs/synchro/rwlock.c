/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : rwlock.c
 * version     : 1.0
 * description : ¶ÁÐ´×ÔÐýËø
 * author      : gaocheng
 * date        : 2013-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include "rwlock.h"

/***************************************************************
 global variable declare
 ***************************************************************/

/***************************************************************
 function declare
 ***************************************************************/

#define RW_LOCK_CNT 0x01000000

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void OS_API init_rw_lock(rwlock_t *lock)
{
    cassert(OS_NULL != lock);
    init_spinlock(&lock->spin);
    lock->count = RW_LOCK_CNT;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void OS_API read_lock(rwlock_t *lock)
{
    cassert(OS_NULL != lock);

    spin_lock(&lock->spin);

    /* here, we use prefix lock, so no rm instruction */
    __asm__ __volatile__("lock; subl $1,%0\n\t"
                         "jns 2f\n\t"
                         "1:\n\t" /* lock fail */
                         "lock; incl %0\n\t"
                         "3:\n\t"
                         "pause\n\t"
                         "cmpl $1,%0\n\t"
                         "js 3b\n\t"
                         "lock; decl %0\n\t"
                         "js 1b\n\t"
                         "2:\n\t" /* lock succ */
                         :
                         :"m"(lock->count)
                         :"memory");
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void OS_API read_unlock(rwlock_t *lock)
{
    cassert(OS_NULL != lock);

    /* here, we use prefix lock, so no rm instruction */
    __asm__ __volatile__("lock; incl %0\n\t"
                         :
                         :"m"(lock->count)
                         :"memory");

    spin_unlock(&lock->spin);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void OS_API write_lock(rwlock_t *lock)
{
    cassert(OS_NULL != lock);

    spin_lock(&lock->spin);

    /* here, we use prefix lock, so no rm instruction */
    __asm__ __volatile__("lock; subl $"asm_str(RW_LOCK_CNT)",%0\n\t"
                         "jz 2f\n\t"
                         "1:\n\t" /* lock fail */
                         "lock; addl $"asm_str(RW_LOCK_CNT)",%0\n\t"
                         "3:\n\t"
                         "pause\n\t"
                         "cmpl $"asm_str(RW_LOCK_CNT)",%0\n\t"
                         "jne 3b\n\t"
                         "lock; subl $"asm_str(RW_LOCK_CNT)",%0\n\t"
                         "jnz 1b\n\t"
                         "2:\n\t" /* lock succ */
                         :
                         :"m"(lock->count)
                         :"memory");
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void OS_API write_unlock(rwlock_t *lock)
{
    cassert(OS_NULL != lock);

    /* here, we use prefix lock, so no rm instruction */
    __asm__ __volatile__("lock; addl $"asm_str(RW_LOCK_CNT)",%0\n\t"
                         :
                         :"m"(lock->count)
                         :"memory");

    spin_unlock(&lock->spin);
}

