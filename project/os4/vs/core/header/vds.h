/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : vds.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __VDS_H__
#define __VDS_H__

/***************************************************************
 include header file
 ***************************************************************/
#include <lib.h>
#include <os.h>
#include <vds/paint.h>
#include <vds/timer.h>

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/
#define device_init_func(x) init_device_call(x)

/***************************************************************
 enum define
 ***************************************************************/

/***************************************************************
 struct define
 ***************************************************************/

/***************************************************************
 * description :
 ***************************************************************/
struct device_operations {
    HDEVICE (*open)(os_void *device_data);
    os_ret (*close)(HDEVICE device);
    os_ret (*read)(HDEVICE device);
    os_ret (*write)(HDEVICE device);
};

/***************************************************************
 * description :
 ***************************************************************/
struct virtual_device {
    os_u8 *name;
    os_void *device_data;
    struct device_operations *operation;
};

/***************************************************************
 extern function
 ***************************************************************/
os_void init_keyboard_int(os_void);
os_void init_rtc_int(os_void);
os_void init_harddisk_int(os_void);
os_void init_print(os_void);

os_ret register_device(struct virtual_device *device);
os_u32 alloc_device_id(os_void);

#pragma pack()

#endif /* end of header */

