/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : vts.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __VTS_H__
#define __VTS_H__

/***************************************************************
 include header file
 ***************************************************************/
#include <lib.h>
#include <os.h>
#include <vts/i386.h>
#include <vts/dump.h>

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/

/***************************************************************
 enum define
 ***************************************************************/

/***************************************************************
 struct define
 ***************************************************************/

/***************************************************************
 extern function
 ***************************************************************/
os_void init_entity(os_void);
os_void init_task_queue(os_void);
os_void init_core_task(os_void);
os_void init_desktop_task(os_void);
os_void init_debuger(os_void);
os_u32 alloc_gdt_item(os_void);
os_u32 get_task_id(os_u32 core_id, os_u32 cpu_id);

#pragma pack()

#endif

