/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : usb_hub.c
 * version     : 1.0
 * description : (key) usb 2.0 hub, abstract
 * author      : gaocheng
 * date        : 2012-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include <vds.h>
#include <vbs.h>
#include "usb.h"
#include "usb_hub.h"

/***************************************************************
 global variable declare
 ***************************************************************/

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret clear_hub_port_feature(struct usb_device *hub, os_u8 port, enum usb_hub_class_feature_selectors_type feature_selector)
{
    struct usb_setup_data setup = {0};

    setup.bmRequestType = USB_TYPE_CLASS | USB_RECIP_OTHER | USB_DIR_OUT;
    setup.bRequest = USB_HUB_CLEAR_FEATURE;
    setup.wValue = feature_selector;
    setup.wIndex = port; // there is no PORT_TEST
    setup.wLength = 0;

    if (OS_SUCC != hub->hc_operation->usb_send_control_transfer(hub, hub->usb_addr, 0, &setup, 0, 0)) {
        return OS_FAIL;
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret get_hub_descriptor(struct usb_device *hub, struct usb_hub_descriptor *hub_descriptor)
{
    struct usb_setup_data setup = { 0 };

    setup.bmRequestType = USB_TYPE_CLASS | USB_RECIP_DEVICE | USB_DIR_IN; // table 11-15
    setup.bRequest = USB_HUB_GET_DESCRIPTOR;
    /* The standard hub descriptor is denoted by using the
       value bDescriptorType defined in Section 11.23.2.1.
       All hubs are required to implement one hub descriptor,
       with descriptor index zero. */
    setup.wValue = USB_DT_HUB << 8 | 0;
    setup.wIndex = 0;
    setup.wLength = sizeof(struct usb_hub_descriptor); /* get max length */

    if (OS_SUCC != hub->hc_operation->usb_receive_control_transfer(hub, hub->usb_addr, 0, &setup, hub_descriptor, sizeof(struct usb_hub_descriptor))) {
        return OS_FAIL;
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret get_hub_port_status(struct usb_device *hub, os_u8 port, struct Port_Status_and_Change_Status *status)
{
    struct usb_setup_data setup = { 0 };

    setup.bmRequestType = USB_TYPE_CLASS | USB_RECIP_OTHER | USB_DIR_IN;
    setup.bRequest = USB_HUB_GET_STATUS;
    setup.wValue = 0;
    setup.wIndex = port;
    setup.wLength = 4; // not sizeof(struct Port_Status_and_Change_Status)

    if (OS_SUCC != hub->hc_operation->usb_receive_control_transfer(hub, hub->usb_addr, 0, &setup, status, 4)) {
        return OS_FAIL;
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret get_hub_status(struct usb_device *usb, struct Hub_Status_and_Change_Status *status)
{
    struct usb_setup_data setup = { 0 };

    setup.bmRequestType = USB_TYPE_CLASS | USB_RECIP_DEVICE | USB_DIR_IN;
    setup.bRequest = USB_HUB_GET_STATUS;
    setup.wValue = 0;
    setup.wIndex = 0;
    setup.wLength = 4;

    if (OS_SUCC != usb->hc_operation->usb_receive_control_transfer(usb, usb->usb_addr, 0, &setup, status, 4)) {
        return OS_FAIL;
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret set_hub_port_feature(struct usb_device *usb, os_u8 port, enum usb_hub_class_feature_selectors_type feature_selector)
{
    struct usb_setup_data setup = { 0 };

    setup.bmRequestType = USB_TYPE_CLASS | USB_RECIP_OTHER | USB_DIR_OUT;
    setup.bRequest = USB_HUB_SET_FEATURE;
    setup.wValue = feature_selector;
    setup.wIndex = port; // there is no PORT_TEST
    setup.wLength = 0;

    if (OS_SUCC != usb->hc_operation->usb_send_control_transfer(usb, usb->usb_addr, 0, &setup, 0, 0)) {
        return OS_FAIL;
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret set_hub_feature(struct usb_device *usb, os_u16 feature_selector)
{
    struct usb_setup_data setup = { 0 };

    setup.bmRequestType = USB_TYPE_CLASS | USB_RECIP_DEVICE | USB_DIR_OUT;
    setup.bRequest = USB_HUB_SET_FEATURE;
    setup.wValue = feature_selector;
    setup.wIndex = 0;
    setup.wLength = 0;

    if (OS_SUCC != usb->hc_operation->usb_send_control_transfer(usb, usb->usb_addr, 0, &setup, 0, 0)) {
        return OS_FAIL;
    }
    return OS_SUCC;
}

/***************************************************************
 * description : Setting the reset feature PORT_RESET causes the hub to signal reset on that port. When the
 *               reset signaling is complete, the hub sets the C_PORT_RESET status change and immediately enables the
 *               port. refer to 11.24.2.13 Set Port Feature
 * history     :
 ***************************************************************/
LOCALC os_ret reset_hub_port(struct usb_device *usb, os_u8 port, struct Port_Status_and_Change_Status *status_data)
{
    os_ret result;
    os_u32 i;

    /* reset */
    result = set_hub_port_feature(usb, port, USB_PORT_RESET);
    if (OS_SUCC != result) {
        usb_dbg(USB_ERROR, "reset hub port fail");
        return OS_FAIL;
    }

    /* get status to judge reset complete */
    for (i = 0; i < 4; i++) {
        delay_ms(EHCI_HUB_TIMEOUT);
        result = get_hub_port_status(usb, port, status_data);
        if (OS_SUCC != result) {
            continue;
        }
        if ((PSB_CCS & status_data->wPortStatus) && !(PSB_RESET & status_data->wPortStatus)) {
            return OS_SUCC;
        }
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC enum usb_speed_type get_port_speed_info(os_u16 status)
{
    if (0 != (PSB_LSDA & status)) {
        if (0 == (PSB_HSDA & status)) {
            return USB_LOW_SPEED;
        } else {
            usb_dbg(USB_ERROR, "speed status error");
            return USB_LOW_SPEED;
        }
    } else {
        if (0 == (PSB_HSDA & status)) {
            return USB_FULL_SPEED;
        } else {
            return USB_HIGH_SPEED;
        }
    }
}

/***************************************************************
 * description :
 ***************************************************************/
struct usb_hub_data_format {
    struct usb_device *usb;
    os_u8 len;
    os_u8 data[255];
};

LOCALD SBUFFER_HANDLE hub_status_handle = OS_NULL;

LOCALD HEVENT hub_monitor_sem = OS_NULL;

LOCALD volatile os_bool monitor_flag _CPU_ALIGNED_ = OS_FALSE;

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void hub_status_change(os_u8 *data)
{
    struct usb_hub_data_format *buffer;
    lock_t eflag;

    buffer = (struct usb_hub_data_format *)((pointer)(data) - (pointer)(&((struct usb_hub_data_format *) 0)->data));

    if (OS_FALSE == monitor_flag) {
        return;
    }

    /* serialize writes */
    lock_int(eflag);
    (os_void) push_sbuffer(hub_status_handle, buffer);
    unlock_int(eflag);

    notify_event(hub_monitor_sem, __LINE__);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret monitor_hub_port(struct usb_hdevice *usb, os_u8 port_num)
{
    struct usb_endpoint_descriptor *endpoint;
    struct usb_hub_data_format *hub_buff;
    os_u8 pipe_num;
    os_u8 i;

    /* find the interrupt endpoint */
    pipe_num = usb->itf->interface->bNumEndpoints;
    for (i = 0; i < pipe_num; i++) {
        if (USB_INTERRUPT_TRANSFER == get_usb_transfer_type(usb->itf->endpoint[i]->bmAttributes)) {
            if (endpoint_is_in(usb->itf->endpoint[i]->bEndpointAddress)) {
                endpoint = usb->itf->endpoint[i];
            }
        }
    }
    if (OS_NULL == endpoint) {
        usb_dbg(USB_ERROR, "no hub int endpoint");
        return OS_FAIL;
    }

    /* get endpoint num */
    pipe_num = endpoint_num(endpoint->bEndpointAddress); /* usb_20.pdf 9.6.6 endpoint */
    usb_dbg(USB_INFO, "hub int endpoint addr %d", pipe_num);

    hub_buff = kmalloc(sizeof(struct usb_hub_data_format));
    if (OS_NULL == hub_buff) {
        usb_dbg(USB_ERROR, "alloc hub buffer fail");
        return OS_FAIL;
    }
    hub_buff->usb = usb->dev;
#define port_byte(num) (((num) >> 3) + ((num) & 0x7) ? 1 : 0)
    hub_buff->len = port_byte(port_num);

    add_usb_int_pipe(usb, pipe_num,
                     hub_buff->data, endpoint->wMaxPacketSize, //hub_buff->len, this will generate ehci host error because of short packet.
                     endpoint->bInterval,
                     hub_status_change);

    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret usb_ehci_hub_port_device(struct usb_device *usb, os_u32 port)
{
    struct Port_Status_and_Change_Status port_status;
    enum usb_speed_type speed;
    struct usb_device *device;
    os_ret result;

    device = OS_NULL;

    /* get status */
    result = get_hub_port_status(usb, port, &port_status);
    if (OS_SUCC != result) {
        usb_dbg(USB_ERROR, "get hub port status error");
        goto exit;
    }
    if (!(PSB_CCS & port_status.wPortStatus)) {
        goto exit;
    }

    /* reset(enable) and check speed */
    result = reset_hub_port(usb, port, &port_status);
    if (OS_SUCC != result) {
        usb_dbg(USB_ERROR, "reset hub port fail");
        goto exit;
    }
    speed = get_port_speed_info(port_status.wPortStatus);
    usb_dbg(USB_INFO, "port %d status: %x %d", port, port_status.wPortStatus, speed);

    /* alloc entity for device on the port */
    device = alloc_usb_device(usb->host_controller, speed, usb->hc_operation);
    if (OS_NULL == device) {
        goto exit;
    }
    device->hub_addr = usb->usb_addr;
    device->hub_port_num = port;

    /* enum device on the port */
    usb_dbg(USB_INFO, "enum device from hub");
    result = enum_usb_device(device);
    if (OS_SUCC != result) {
        usb_dbg(USB_ERROR, "enum device fail, hub addr %d, port %d", usb->usb_addr, port);
        goto exit;
    }

    return OS_SUCC;
  exit:
    clear_hub_port_feature(usb, port, USB_PORT_POWER);
    if (device) { free_usb_device(device); }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret add_usb_ehci_hub_event(HDEVICE usb)
{
    struct usb_device *hub;
    struct usb_hub_private *user;
    os_u8 port_num;
    os_u8 i;
    struct Hub_Status_and_Change_Status hub_status;
    os_ret result;

    cassert(OS_NULL != usb);

    user = OS_NULL;

    hub = ((struct usb_hdevice *) usb)->dev;

    usb_dbg(USB_INFO, "usb hub %d", hub->usb_addr);

    user = kmalloc(sizeof(struct usb_hub_private));
    if (OS_NULL == user) {
        usb_dbg(USB_ERROR, "allocate usb hub user fail");
        goto fail;
    }
    set_usb_dedicated(usb, user);

    /* get hub endpoint */
    result = get_hub_descriptor(hub, &user->hub_descriptor);
    if (OS_SUCC != result) {
        usb_dbg(USB_ERROR, "get ehci hub descriptor fail");
        goto fail;
    }
    port_num = user->hub_descriptor.bNbrPorts;
    usb_dbg(USB_INFO, "hub port %d", user->hub_descriptor.bNbrPorts);

    /* hub local power */
#if 0
    result = set_hub_feature(hub, USB_C_HUB_LOCAL_POWER);
    if (OS_SUCC != result) {
        usb_dbg(USB_ERROR, "set hub power fail");
        return OS_FAIL;
    }
#endif
    result = get_hub_status(hub, &hub_status);
    if (OS_SUCC != result) {
        usb_dbg(USB_ERROR, "get hub status fail");
        return OS_FAIL;
    } usb_dbg(USB_INFO, "hub status: %x", hub_status.wHubStatus);

    delay_ms(200);

    /* power on all ports */
    for (i = 1; i <= port_num; i++) {
        set_hub_port_feature(hub, i, USB_PORT_POWER);
    }
    delay_ms(user->hub_descriptor.bPwrOn2PwrGood * 2);

    /* bugfix: don't try to find device actively */
#if 0
    /* enum device */
    for (i = 1; i <= port_num; i++) { // 11.24.2.7 The port number must be a valid port number for that hub, greater than zero.
        usb_ehci_hub_port_device(hub, i);
    }
#endif

    /* start monitor */
    monitor_hub_port(usb, port_num);
    monitor_flag = OS_TRUE;

    /* delay for port change */
    delay_ms(100);

    return OS_SUCC;
  fail:
    if (user) kfree(user);
    return OS_FAIL;
}

LOCALD const struct usb_device_id usb_ehci_hub_id = {
    USB_ANY_ID, USB_ANY_ID, /* any vender, any product id. */
    USB_CLASS_HUB,
    USB_ANY_ID,
    USB_ANY_ID
};

/* 通用usb设备驱动表 */
LOCALD const struct usb_driver ehci_hub_driver = {
    "usb-ehci-hub",
    &usb_ehci_hub_id,
    add_usb_ehci_hub_event,
    OS_NULL
};

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void init_ehci_hub(os_void)
{
    os_ret result;

    /* 注册的时序无法保证, 因此要求注册了即可使用设备 */
    result = register_usb_driver(&ehci_hub_driver);
    cassert(OS_SUCC == result);
}
device_init_func(init_ehci_hub);

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC struct usb_device *find_hub_device(struct usb_device *usb, os_u32 port)
{
    return find_usb_device(usb->host_controller, usb->usb_addr, port);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret clear_device_resource(struct usb_device *hub, os_u32 port)
{
    struct usb_device *device;

    /* find device by port */
    device = find_hub_device(hub, port);
    if (OS_NULL == device) {
        usb_dbg(USB_ERROR, "clear hub device, find device(%d %d) fail", hub->usb_addr, port);
        return OS_FAIL;
    }

    unenum_usb_device(device);

    free_usb_device(device);

    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret usb_hub_port_changes(struct usb_device *hub, os_u8 port)
{
    os_u32 i;
    struct Port_Status_and_Change_Status status_data;
    os_ret result;

    /* Table 11-22. Port Change Field, wPortChange */
    GLOBALDIF const port_change_field[5] = {
        USB_C_PORT_CONNECTION,
        USB_C_PORT_ENABLE,
        USB_C_PORT_SUSPEND,
        USB_C_PORT_OVER_CURRENT,
        USB_C_PORT_RESET
    };
    os_u16 pcf_mask;

    /* get and clear port feature */
    result = get_hub_port_status(hub, port, &status_data);
    if (OS_SUCC != result) {
        usb_dbg(USB_ERROR, "usb hub port changes, get hub port status error");
        return OS_FAIL;
    }
    for (i = 0, pcf_mask = 0x1; i < 5; i++, pcf_mask = pcf_mask << 1) {
        if (pcf_mask & status_data.wPortChange) {
            result = clear_hub_port_feature(hub, port, port_change_field[i]);
            if (OS_SUCC != result) {
                usb_dbg(USB_ERROR, "clear feature(%d) fail", i);
            }
        }
    }
    usb_dbg(USB_INFO, "port(%d) status: %x %x", port, status_data.wPortStatus, status_data.wPortChange);

    /* hot plug */
    if (PSB_CCS & status_data.wPortStatus) {
        /* device has been enumed ? */
        if (OS_NULL == find_hub_device(hub, port)) {
            /* delay before power on */
            delay_ms(200);
            /* reenum device */
            (os_void) usb_ehci_hub_port_device(hub, port);
        }
    } else {
        /* clear device resource */
        clear_device_resource(hub, port);
    }

    return OS_SUCC;
}

/***************************************************************
 * description : TASK_FUNC_PTR
 * history     :
 ***************************************************************/
LOCALC os_ret OS_CALLBACK usb_hub_entry_point(os_u32 arg1, os_u32 arg2, os_u32 arg3, os_u32 arg4, os_u32 arg5, os_u32 arg6, os_u32 arg7)
{
    struct usb_hub_data_format buffer;
    os_u8 i, len;
    os_u8 port, mask, temp;

    while (1) {
        (os_void) wait_event(hub_monitor_sem, 0);

        /* get data from sbuffer */
        while (SUCC == pop_sbuffer(hub_status_handle, &buffer)) {
            /* parse port num */
            port = 0;
            len = buffer.len;
            for (i = 0; i < len; i++) {
                temp = buffer.data[i];
                mask = 0x1;
                while (mask) {
                    if (0 != port) {
                        if (mask & temp) {
                            monitor_flag = OS_FALSE;
                            usb_hub_port_changes(buffer.usb, port);
                            monitor_flag = OS_TRUE;
                        }
                    }

                    port++;
                    mask = mask << 1;
                }
            }
        }
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void dump_hub(os_void)
{
    do {
        struct usb_hub_data_format test;
        lock_t eflag;
        os_ret result;

        lock_int(eflag);
        test.usb = OS_NULL;
        test.len = 2;
        test.data[0] = 0x2;
        test.data[1] = 0x3;
        push_sbuffer(hub_status_handle, &test);
        test.data[0] = 4;
        test.data[1] = 5;
        result = pop_sbuffer(hub_status_handle, &test);
        if (OS_SUCC != result) { print("pop error\n"); }
        print("%d %d\n", test.data[0], test.data[1]);

        unlock_int(eflag);
    } while (0);
}

LOCALD os_u8 hub_debug_name[] = { "hub" };
LOCALD struct dump_info hub_debug = {
    hub_debug_name,
    dump_hub
};

/***************************************************************
 * description : ALLOC_FUNCPTR
 * history     :
 ***************************************************************/
LOCALC os_void *alloc_hub_sbuffer(os_u32 len)
{
    return kmalloc(len);
}

/***************************************************************
 * description : FREE_FUNCPTR
 * history     :
 ***************************************************************/
LOCALC os_void *free_hub_sbuffer(os_void *addr)
{
    return kfree(addr);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void create_hub_monitor(os_void)
{
    HTASK handle;

#define MAX_HUB_STATUS_CACHE 4
    hub_status_handle = create_sbuffer(sizeof(struct usb_hub_data_format), MAX_HUB_STATUS_CACHE, alloc_hub_sbuffer, free_hub_sbuffer);
    cassert(OS_NULL != hub_status_handle);

    hub_monitor_sem = create_event_handle(EVENT_INVALID, "hub", __LINE__);
    cassert(OS_NULL != hub_monitor_sem);

#define USB_HUB_TASK_PRIO TASK_PRIORITY_6
    handle = create_task("usb hub", usb_hub_entry_point, USB_HUB_TASK_PRIO, 0, 0, 0, 0, 0, 0, 0);
    cassert(OS_NULL != handle);

    if (OS_SUCC != register_dump(&hub_debug)) {
        usb_dbg(USB_ERROR, "hub register dump fail");
    }
}

