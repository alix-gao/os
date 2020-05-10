/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : watchdog.c
 * version     : 1.0
 * description : 内核异常处理
 * author      : gaocheng
 * date        : 2016-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>

/***************************************************************
 global variable declare
 ***************************************************************/
LOCALD HTASK dog_handle = OS_NULL;

#define DOG_TASK_DELAY 0x10
#define DOG_FOOD_FULL 0x1000
#define DOG_FOOD_DEADLINE 0x10

LOCALD volatile os_u32 dog_food _CPU_ALIGNED_ = DOG_FOOD_FULL;

LOCALD struct task_snapshot *usage_snapshot = OS_NULL;

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description : called on every tick
 * history     :
 ***************************************************************/
os_void dog_event(os_void)
{
    lock_t eflag;
    struct task_snapshot snap;

    lock_int(eflag);
    if (dog_food) {
        dog_food--;
    }
    unlock_int(eflag);

    if (DOG_FOOD_DEADLINE > dog_food) {
        clear_screen(VGA_COLOR_BLACK);
        init_print();
        print("watch dog!\n");

        lock_schedule();

        mem_set(&snap, 0, sizeof(struct task_snapshot));
        while (OS_SUCC == get_process32next(&usage_snapshot)) {
            print("%s %d\n", usage_snapshot->name, usage_snapshot->usage);
            if (snap.usage <= usage_snapshot->usage) {
                mem_cpy(&snap, usage_snapshot, sizeof(struct task_snapshot));
            }
        }
        print("task: %s, usage: %d\n", snap.name, snap.usage);
        print("pc: 0x%x\n", dump_task_pc(snap.handle));

        dump("tentity");

        dead();
    }
}

/***************************************************************
 * description : TASK_FUNC_PTR
 * history     :
 ***************************************************************/
LOCALC os_ret OS_CALLBACK dog_entry_point(os_u32 arg1, os_u32 arg2, os_u32 arg3, os_u32 arg4, os_u32 arg5, os_u32 arg6, os_u32 arg7)
{
    lock_t eflag;

    for (;;) {
        delay_task(DOG_TASK_DELAY, __LINE__);

        lock_int(eflag);
        dog_food = DOG_FOOD_FULL;
        unlock_int(eflag);
    }
    cassert(OS_FALSE);
    return OS_FAIL;
}

/***************************************************************
 * description : kill dog
 * history     :
 ***************************************************************/
LOCALC os_void kill_dog(os_void)
{
    dog_food = DOG_FOOD_DEADLINE;
}

/***************************************************************
 * description : kill dog
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_kdog(os_u32 argc, os_u8 *argv[])
{
    if (0 == argc) {
        kill_dog();
        return OS_SUCC;
    }
    /* input para error */
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void init_watch_dog(os_void)
{
    cassert(DOG_FOOD_FULL > (DOG_TASK_DELAY * OS_HZ));
    dog_handle = create_task("dog", dog_entry_point, TASK_PRIORITY_1, 0, 0, 0, 0, 0, 0, 0);
    cassert(OS_NULL != dog_handle);

    register_cmd("kdog", cmd_kdog);

    create_toolhelp32snapshot(&usage_snapshot);
}

