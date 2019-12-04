/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : spinlock.c
 * version     : 1.0
 * description : (key) spinlock for smp
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>

#include "spinlock.h"

/***************************************************************
 global variable declare
 ***************************************************************/

/***************************************************************
 function declare
 ***************************************************************/

/***************************************************************
 * description : no kmalloc or create_spinlock, leave it to upper layer.
 * history     :
 ***************************************************************/
os_void OS_API init_spinlock(spinlock_t *lock)
{
    cassert(OS_NULL != lock);
    lock->lock = EVENT_VALID;
    lock->check = SPIN_LOCK_FLAG;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API spin_lock(spinlock_t *lock)
{
#define MAX_SPIN_TIMES 0x100
    os_u32 times;

    cassert_word((OS_NULL != lock) && (SPIN_LOCK_FLAG == lock->check), "%x %x\n", lock, lock->check);

    if ((OS_NULL != lock) && (SPIN_LOCK_FLAG == lock->check)) {
        lock_int(lock->eflag);
#ifdef _DEBUG_VERSION_
        __asm__ __volatile__("movl $"asm_str(EVENT_INVALID)",%%eax\n\t"
                             "movl $0,%%ecx\n\t"
                             "1:\n\t"
                             "incl %%ecx\n\t"
                             "cmpl $"asm_str(MAX_SPIN_TIMES)",%%ecx\n\t"
                             "ja 2f\n\t"
                             "xchgl %1,%%eax\n\t"
                             "cmpl $"asm_str(EVENT_VALID)",%%eax\n\t"
                             "jne 1b\n\t"
                             "2:\n\t"
                             "movl %%ecx,%0\n\t"
                             :"=m"(times)
                             :"m"(lock->lock) /* input */
                             :"eax","ecx","memory"); /* no mb(), xchg lock ram bus, flush the write buffer. */

        if (MAX_SPIN_TIMES <= times) {
            print("warning: spin_lock maximum %x\n", times);
            cassert(OS_FALSE);
        }
#else
        __asm__ __volatile__("movl $"asm_str(EVENT_INVALID)",%%eax\n\t"
                             "1:\n\t"
                             "xchgl %0,%%eax\n\t"
                             "cmpl $"asm_str(EVENT_VALID)",%%eax\n\t"
                             "jne 1b\n\t"
                             :
                             :"m"(lock->lock) /* input */
                             :"eax","memory"); /* no mb(), xchg lock ram bus, flush the write buffer. */
#endif
        return OS_SUCC;
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API spin_unlock(spinlock_t *lock)
{
    cassert((OS_NULL != lock) && (SPIN_LOCK_FLAG == lock->check));

    if ((OS_NULL != lock) && (SPIN_LOCK_FLAG == lock->check)) {
        __asm__ __volatile__("movl $"asm_str(EVENT_VALID)",%0\n\t"
                             :"=m"(lock->lock) /* output */
                             :
                             :"memory"); /* no mb() */
        wmb();
        unlock_int(lock->eflag);
        return OS_SUCC;
    }
    return OS_FAIL;
}

