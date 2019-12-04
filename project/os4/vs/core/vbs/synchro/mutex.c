/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : mutex.c
 * version     : 1.0
 * description : (key) 互斥, 是对信号量的一个扩充.
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include "mutex.h"
#include "semaphore.h"

/***************************************************************
 global variable declare
 ***************************************************************/

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API create_critical_section(OUT HEVENT *handle, os_u32 line)
{
    /* 入参检查 */
    if (OS_NULL != handle) {
        *handle = create_event_handle(EVENT_VALID, "mutex", line);
        if (OS_NULL != *handle) {
            /* panic */
            return OS_SUCC;
        }
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void OS_API enter_critical_section(IN HEVENT handle)
{
    wait_event(handle, 0);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void OS_API leave_critical_section(IN HEVENT handle)
{
    notify_event(handle, __LINE__);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API destroy_critical_section(IN HEVENT handle)
{
    destroy_event_handle(handle);
    return OS_SUCC;
}

