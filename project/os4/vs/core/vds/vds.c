/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : vds.c
 * version     : 1.0
 * description : (key) virtual device system
 * author      : gaocheng
 * date        : 2011-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <vds.h>

/***************************************************************
 global variable declare
 ***************************************************************/

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description : 返回设备唯一id
 * history     : 可重入
 ***************************************************************/
os_u32 alloc_device_id(os_void)
{
    GLOBALDIF os_u32 device_id = 1;

    return device_id++;
}

/***************************************************************
 * description : 返回设备唯一id
 * history     :
 ***************************************************************/
os_ret register_device(struct virtual_device *device)
{
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
HDEVICE OS_API open_device(os_u8 *name)
{
    struct virtual_device *device;

    if ((OS_NULL == device) || (OS_NULL == device->operation)
     || (OS_NULL == device->operation->open)) {
        return OS_NULL;
    }
    return device->operation->open(device->device_data);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret close_device(os_void)
{
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void read_device(os_void)
{
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void write_device(os_void)
{
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void list_device(os_void)
{
    flog("id name\n");
}

#include <gate.h>

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret update_device_parameters(os_u32 function, os_void *in, os_void *out)
{
    os_u32 *i;
    GATE_FUNC func;
    os_ret ret;
    os_u8 exe;
    GLOBALREFD os_u32 _dev_para_start;
    GLOBALREFD os_u32 _dev_para_end;

    for (i = &_dev_para_start; i < &_dev_para_end; i++) {
        func = (GATE_FUNC)(*i);
        if (OS_NULL != func) {
            ret = (*func)(function, in, out, &exe);
            if (OS_TRUE == exe) {
                return ret;
            }
        }
    }
    return OS_FAIL;
}

