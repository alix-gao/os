/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : core.h
 * version     : 1.0
 * description : core公共信息
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __CORE_H__
#define __CORE_H__

/***************************************************************
 include header file
 ***************************************************************/
#include <lib.h>
#include <os.h>
#include <configuration.h>
#include <compiler.h>

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/

/* 操作系统任务数量 = SYS_TASK_NUM % CPU_MAX_TASK_NUM */
#define OS_TASK_NUM (SYS_TASK_NUM & (CPU_MAX_TASK_NUM - 1))

#define INVALID_ID 0

/* 无效任务id */
#define INVALID_TASK_ID (INVALID_ID)

/* 主任务id */
#define MAIN_TASK_ID 1

/* 抢占式调度策略 */
#define PREEMPTIVE_SCHEDULE

#define init_afunc(x) init_abstract_call(x)

/***************************************************************
 enum define
 ***************************************************************/

/***************************************************************
 struct define
 ***************************************************************/
/***************************************************************
 * description : 任务句柄
 ***************************************************************/
struct task_handle {
    os_u32 core_id; // hardware
    os_u32 cpu_id; // hardware
    os_u32 task_id; // software
#define HTASK_CHECK 0xaaaaaaaa
    os_u32 check;
};

/***************************************************************
 * description : HWINDOW, 窗口句柄 (消息处理实体)
 ***************************************************************/
struct window_handle {
    /* 线程句柄 */
    struct task_handle *htask;
    os_u32 window_id;
};

/***************************************************************
 * description : HDEVICE, 设备句柄
 ***************************************************************/
struct device_context {
    /* 窗口的屏幕坐标系 */
    screen_csys csys;
    /* 窗口的前景颜色 */
    enum vga_color foreground_color;
    /* 窗口的背景颜色 */
    enum vga_color background_color;
    /* 窗口宽度,x */
    os_u32 width;
    /* 窗口长度,y */
    os_u32 length;
    /* 光标的位置 */
    cursor_csys cursor_pos;
    /* 字号大小 */
    os_u32 font_size;
};

/***************************************************************
 extern function
 ***************************************************************/

#define kmalloc(size) alloc_kdm(size, 1, __LINE__, __FILE__)
#define kfree(addr) free_kdm((os_void **)&(addr), __LINE__)

/* 缓存一致性低4G的内存页 */
#define cmalloc(size, align) alloc_coherent_mem(size, align, __LINE__)
#define cfree(addr) free_coherent_mem((os_void **)&(addr), __LINE__)

#include <vms.h>
#include <vts.h>
#include <vbs.h>
#include <vds.h>

#pragma pack()

#endif /* end of header */

