
/******************************************************************************
 file name: process.cpp

 instruction: task attemper, task scheduler, task ...

 history: gaocheng draft
 ******************************************************************************/

/******************************************************************************
 include
 ******************************************************************************/
#include <os_type.h>
#include <process.h>
#include <global_cite.h>

/******************************************************************************
 global var
 ******************************************************************************/

/******************************************************************************
 function
 ******************************************************************************/

/* task scheduler */
void task_attemper()
{
     __asm__ __volatile__("int $0x00");
}

/* get current process id no. */
os_uint32 os_get_task_id()
{
          return last_task_no;
}

void task_sleep_on(os_uint32 ultime)
{
     /* use timer */

     state[os_get_task_id()] = task_status_uninterruptible;

     /* give up cpu. */
     task_attemper();
}

void task_wake_up()
{
     state[os_get_task_id()] = task_status_ready;
}

void task_exit()
{
     state[os_get_task_id()] = task_status_zombie;
}

/******************************************************************************
 function name: task_scheduler
 introduction:
 input:
 output:
 return:
 history: gaocheng draft
 ******************************************************************************/
void task_scheduler()
{
}

