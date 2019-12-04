/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : gate.h
 * version     : 1.0
 * description : call gate. called directly by kernel to update parameters
 * author      : gaocheng
 * date        : 2016-04-22
 ***************************************************************/

#ifndef __GATE_H__
#define __GATE_H__

/***************************************************************
 include header file
 ***************************************************************/
#include <type.h>

/***************************************************************
 macro define
 ***************************************************************/
typedef os_ret (*GATE_FUNC)(os_u32 function, os_void *in, os_void *out, os_u8 *exe);

#define __GATE_ATTR __attribute__((unused,section(".dev_para")))
#define gate_func(x) static GATE_FUNC __gate_##x##__ __GATE_ATTR = (x)

#define WEAK_DEV_PARA __attribute__((alias("update_device_parameters")))

/***************************************************************
 enum define
 ***************************************************************/
/***************************************************************
 * description :
 ***************************************************************/
enum {
    FUNCTION_USB_PARA,
};

/***************************************************************
 struct define
 ***************************************************************/
/***************************************************************
 * description :
 ***************************************************************/
struct usb_dev_para {
    os_u16 vid;
    os_u16 pid;
    os_u8 class;
    os_u8 subclass;
    os_u8 prot;
};

/***************************************************************
 extern function
 ***************************************************************/
os_ret update_device_parameters(os_u32 function, os_void *in, os_void *out);

#endif /* end of header */


