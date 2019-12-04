
#ifndef _kernel_api_h
#define _kernel_api_h

#include <os_type.h>
/* kernel mode api, not user mode api */

/* int install */
extern void os_int_install(void (*p)(),int no);

/* task scheduler */
extern void task_attemper();

/* get current process id no. */
extern os_uint32 os_get_task_id();

/* sleep on current task */
extern void task_sleep_on(os_uint32 ultime);

/* wake up current task */
extern void task_wake_up();

/* exit current task */
extern void task_exit();

#endif

