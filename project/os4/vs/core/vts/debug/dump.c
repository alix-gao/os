/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : dump.c
 * version     : 1.0
 * description : abstract
 * author      : gaocheng
 * date        : 2011-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>

/***************************************************************
 global variable declare
 ***************************************************************/

/***************************************************************
 * description :
 ***************************************************************/
struct dump_info_list {
    os_u32 flag; /* 0:invalid, 1:valid */
    struct dump_info *info;
};

#define DUMP_NUM 0x20
LOCALD struct dump_info_list dump_list[DUMP_NUM] = { 0 };

#define DUMP_EVENT_NUM 0x100
struct dump_event_info {
    os_u32 count;
    os_u8 *event[DUMP_EVENT_NUM];
};
LOCALD struct dump_event_info dump_event = { 0 };

/***************************************************************
 function declare
 ***************************************************************/

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret record_event(HTASK handle, os_u8 *name, os_u8 *event)
{
    if (0 == str_cmp(name, get_task_name(handle))) {
        if (DUMP_EVENT_NUM > dump_event.count) {
            dump_event.event[dump_event.count++] = event;
        } else {
            print("record event fail\n");
        }
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret dump_all_events(os_void)
{
    os_u32 i;

    print("dump event: ");
    for (i = 0; i < dump_event.count; i++) {
        print("%s ", dump_event.event[i]);
    } print("\n");
}

/***************************************************************
 * description : 注册信息要求全局且不保证并发安全
 * history     :
 ***************************************************************/
os_ret register_dump(struct dump_info *info)
{
    os_u32 i;
    struct dump_info_list *t;

    cassert(OS_NULL != info);

    for (i = 0, t = dump_list; i < DUMP_NUM; i++, t++) {
        if (0 == t->flag) {
            t->flag = 1;
            t->info = info;
            barrier();
            return OS_SUCC;
        }
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret dump(os_u8 *name)
{
    os_u32 i;
    struct dump_info_list *t;

    if (OS_NULL == name) {
        for (i = 0, t = dump_list; i < DUMP_NUM; i++, t++) {
            if (1 == t->flag) {
                print("(%s)", t->info->name);
            }
        }
        print("\n");
        return OS_SUCC;
    } else {
        for (i = 0, t = dump_list; i < DUMP_NUM; i++, t++) {
            if ((1 == t->flag) && (0 == str_cmp(name, t->info->name))) {
                t->info->func();
                return OS_SUCC;
            }
        }
        return OS_FAIL;
    }
}

/***************************************************************
 * description : 初始化dump, 为了dump不依赖于任何模块, 模块不使用动态内存
 * history     :
 ***************************************************************/
os_void init_dump(os_void)
{
}

