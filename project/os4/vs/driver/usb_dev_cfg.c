/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : usb_dev_cfg.c
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2016-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <lib.h>
#include <gate.h>

/***************************************************************
 * description :
 ***************************************************************/
struct usb_device_configuration {
    struct usb_dev_para para;

    os_u32 proportion;
};

/***************************************************************
 global variable declare
 ***************************************************************/
LOCALD struct usb_device_configuration usb_dev_cfg[] = {
    { { 0x1005, 0xb113,
        0x08, 0x06, 0x50 }, /* apacer disk */
      8 },
    { { 0x058f, 0x6387,
        0x08, 0x06, 0x50 }, /* alcorMP disk */
      1 }
};

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret usb_dev_proportion(os_u32 function, os_void *in, os_void *out, os_u8 *exe)
{
    struct usb_dev_para *para;
    os_u32 *proportion;
    os_u32 i;

    if ((FUNCTION_USB_PARA == function)
     && (OS_NULL != in) && (OS_NULL != out) && (OS_NULL != exe)) {
        *exe = OS_TRUE;
        para = in;
        proportion = out;
        for (i = 0; i < array_size(usb_dev_cfg); i++) {
            if ((para->vid == usb_dev_cfg[i].para.vid)
             && (para->pid == usb_dev_cfg[i].para.pid)
             && (para->class == usb_dev_cfg[i].para.class)
             && (para->subclass == usb_dev_cfg[i].para.subclass)
             && (para->prot == usb_dev_cfg[i].para.prot)) {
                *proportion = usb_dev_cfg[i].proportion;
                return OS_SUCC;
            }
        }
    }
    *exe = OS_FALSE;
    return OS_FAIL;
}
gate_func(usb_dev_proportion);

