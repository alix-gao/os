/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : vbs.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __VBS_H__
#define __VBS_H__

/***************************************************************
 include header file
 ***************************************************************/
#include <lib.h>
#include <os.h>
#include <vbs/usb.h>
#include <vbs/pci_id.h>
#include <vbs/pci.h>

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/
#define bus_init_func(priority, x) init_bus_info(priority, x)

/***************************************************************
 enum define
 ***************************************************************/
/***************************************************************
 * enum name   : bus_init_priority
 * description : for bus_init_func
 ***************************************************************/
enum bus_init_priority {
    BUS_P0, /* abstract, highest */
    BUS_P1, /* low */
    BUS_P2, /* middle */
    BUS_P3, /* high */
    BUS_Px /* invalid */
};

/***************************************************************
 struct define
 ***************************************************************/

/***************************************************************
 extern function
 ***************************************************************/
os_void init_task_station(os_void);
os_void init_idle_station(os_void);
os_void init_bmmcb(os_void);
os_void init_window_class(os_void);
os_void init_window_handle_tab(os_void);
os_void init_idle_window_rc(os_void);
os_void init_semaphore(os_void);
os_void init_pit_int(os_void);
os_ret free_task_station(struct task_handle *handle);
os_u64 alloc_ksm(os_u32 len);

/***************************************************************
 * description : allocate absolute kernel static memory (absolute virtual address)
 *               virtual memory whose linear address is identity mapped to physical address
 * history     :
 ***************************************************************/
os_u64 alloc_ksm_absv(os_u32 len);

#pragma pack()

#endif /* end of header */

