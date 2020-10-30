/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : usb.c
 * version     : 1.0
 * description : (key) abstract
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include <vbs.h>
#include "usb.h"
#include <gate.h>

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
LOCALC os_ret get_default_device_descriptor(struct usb_device *usb)
{
    struct usb_setup_data setup = { 0 };

    setup.bmRequestType = USB_TYPE_STANDARD | USB_RECIP_DEVICE | USB_DIR_IN;
    setup.bRequest = USB_REQ_GET_DESCRIPTOR;
    setup.wValue = USB_DT_DEVICE << 8 | 0;
    setup.wIndex = 0;
    setup.wLength = DEFAULT_MAX_PACKET_SIZE;

    if (OS_SUCC != usb->hc_operation->usb_receive_control_transfer(usb, 0, 0, &setup, &usb->descriptor, DEFAULT_MAX_PACKET_SIZE)) {
        return OS_FAIL;
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret get_device_descriptor(struct usb_device *usb, os_u16 len)
{
    struct usb_setup_data setup = { 0 };

    setup.bmRequestType = USB_TYPE_STANDARD | USB_RECIP_DEVICE | USB_DIR_IN;
    setup.bRequest = USB_REQ_GET_DESCRIPTOR;
    setup.wValue = USB_DT_DEVICE << 8 | 0;
    setup.wIndex = 0;
    setup.wLength = len;

    if (OS_SUCC != usb->hc_operation->usb_receive_control_transfer(usb, usb->usb_addr, 0, &setup, &usb->descriptor, len)) {
        return OS_FAIL;
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_u32 parse_usb_intf_desc(struct usb_itf_info *intf, os_u8 *addr, os_u32 len)
{
    os_u8 ep_num;
    os_u32 i;

    ep_num = 0;
    i = 0;
    while (i < len) {
        switch (addr[i+1]) { /* 描述符类型在第二个字节 */
        case USB_DT_INTERFACE:
            return i;
            break;
        case USB_DT_ENDPOINT:
            intf->endpoint[ep_num++] = (struct usb_endpoint_descriptor *) &addr[i];
            usb_dbg(USB_INFO, "add endpoint descriptor %d", i);
            break;
        case USB_DT_HID:
            intf->hid = (struct hid_descriptor *) &addr[i];
            usb_dbg(USB_INFO, "add hid descriptor %d", i);
            break;
        case USB_DT_DEVICE:
        case USB_DT_CONFIG:
        case USB_DT_STRING:
        case USB_DT_QUALIFIER:
        case USB_DT_OTHERSPEEDCONFIGURATION:
        case USB_DT_INTERFACEPOWER:
        case USB_DT_REPORT:
        case USB_DT_PHYSICAL:
        case USB_DT_HUB:
            break;
        default:
            usb_dbg(USB_ERROR, "unkown descriptor type %d", addr[i+1]);
            break;
        }
        if (add_u32_overflow(i, addr[i])) {
            usb_dbg(USB_ERROR, "parse descriptor add overflow!");
            return len;
        }
        i += addr[i];
    }
    usb_dbg(USB_ERROR, "parse descriptor length overrun!");
    return len;
}

/***************************************************************
 * description : 解码描述符
 * history     :
 ***************************************************************/
LOCALC os_void parse_usb_descriptor(struct usb_device *usb_dev, os_u32 index)
{
    struct usb_config_info *decode;
    os_u32 interface_num; /* total interface, include multiple-alternatesetting */
    os_u32 endpoint_num;
    os_u8 *addr;
    os_u16 len;
    os_u16 pos;

    /* 描述符的内存地址和总长度 */
    addr = usb_dev->config_buffer[index];
    len = usb_dev->config_info[index].config.wTotalLength;
    decode = &usb_dev->config_info[index];

    flog("config desc\n");
    for (pos = 0; pos < len; pos++) {
        flog("%x ", addr[pos]);
    } flog("\n");

    interface_num = 0;
    decode->intf_info = kmalloc(decode->config.bNumInterfaces * sizeof(struct usb_itf_info));
    if (OS_NULL == decode->intf_info) {
        usb_dbg(USB_ERROR, "parse_usb_descriptor, alloc mem fail");
        goto error;
    }
    mem_set(decode->intf_info, OS_NULL, decode->config.bNumInterfaces * sizeof(struct usb_itf_info));

    /* 先解码出接口描述符 */
    for (pos = 0; pos < len;) {
        if ((sizeof(struct usb_interface_descriptor) == addr[pos]) && (USB_DT_INTERFACE == addr[pos + 1])) {
            decode->intf_info[interface_num].interface = (struct usb_interface_descriptor *) &addr[pos];
            endpoint_num = decode->intf_info[interface_num].interface->bNumEndpoints;

            decode->intf_info[interface_num].endpoint = kmalloc(endpoint_num * sizeof(struct usb_endpoint_descriptor *));
            if (OS_NULL == decode->intf_info[interface_num].endpoint) {
                usb_dbg(USB_ERROR, "parse_usb_descriptor, alloc mem fail");
                goto error;
            }
            mem_set(decode->intf_info[interface_num].endpoint, 0, endpoint_num * sizeof(struct usb_endpoint_descriptor *));

            /* phase endpoint descriptors */
            usb_dbg(USB_ERROR, "interface is founded, ep %d, pos %d, len %d, tlen %d", endpoint_num, addr[pos], pos, len);
            pos += addr[pos];
            pos += parse_usb_intf_desc(&decode->intf_info[interface_num], &addr[pos], len - pos);

            interface_num++;
        } else {
            pos += addr[pos];
        }
    }
    decode->total_intf_count = interface_num;
    return;
  error:
    if (decode->intf_info) {
        while (interface_num--) {
            if (decode->intf_info[interface_num].endpoint) {
                kfree(decode->intf_info[interface_num].endpoint);
            }
        }
        kfree(decode->intf_info);
        decode->intf_info = OS_NULL;
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret clear_config_descriptor(struct usb_device *usb_dev)
{
    os_u8 config_cnt, intf_cnt;
    os_u8 i, j;

    config_cnt = usb_dev->descriptor.bNumConfigurations;
    if (usb_dev->config_info) {
        for (i = 0; i < config_cnt; i++) {
            intf_cnt = usb_dev->config_info[i].total_intf_count;
            if (usb_dev->config_info[i].intf_info) {
                for (j = 0; j < intf_cnt; j++) {
                    if (usb_dev->config_info[i].intf_info[j].endpoint) {
                        kfree(usb_dev->config_info[i].intf_info[j].endpoint);
                    }
                }
                kfree(usb_dev->config_info[i].intf_info);
            }
        }
        kfree(usb_dev->config_info);
    }

    if (usb_dev->config_buffer) {
        for (i = 0; i < config_cnt; i++) {
            if (usb_dev->config_buffer[i]) {
                kfree(usb_dev->config_buffer[i]);
            }
        }
        kfree(usb_dev->config_buffer);
    }
    return OS_SUCC;
}

/***************************************************************
 * description : usb_20.pdf 9.4.2
 * history     :
 ***************************************************************/
LOCALC os_ret get_config_descriptor(struct usb_device *usb_dev)
{
    os_ret result;
    os_u8 config_num;
    struct usb_setup_data setup = { 0 };
    os_u8 i;

    config_num = usb_dev->descriptor.bNumConfigurations;

    usb_dev->config_info = OS_NULL;
    usb_dev->config_buffer = OS_NULL;

    usb_dev->config_info = kmalloc(config_num * sizeof(struct usb_config_info));
    if (OS_NULL == usb_dev->config_info) {
        usb_dbg(USB_ERROR, "get config descriptor, alloc mem fail");
        goto fail;
    }
    mem_set(usb_dev->config_info, OS_NULL, config_num * sizeof(struct usb_config_info));

    usb_dev->config_buffer = kmalloc(config_num * sizeof(os_u8 *));
    if (OS_NULL == usb_dev->config_buffer) {
        usb_dbg(USB_ERROR, "get config descriptor, alloc array mem fail");
        goto fail;
    }
    mem_set(usb_dev->config_buffer, OS_NULL, config_num * sizeof(os_u8 *));

    /* 倒序获取config_descriptor为默认配置 */
    for (i = 0; i < config_num; i++) {
        setup.bmRequestType = USB_TYPE_STANDARD | USB_RECIP_DEVICE | USB_DIR_IN;
        setup.bRequest = USB_REQ_GET_DESCRIPTOR;
        setup.wValue = USB_DT_CONFIG << 8 | i;
        setup.wIndex = 0;
        setup.wLength = sizeof(struct usb_config_descriptor);

        result = usb_dev->hc_operation->usb_receive_control_transfer(usb_dev, usb_dev->usb_addr, 0, &setup, &usb_dev->config_info[i].config, sizeof(struct usb_config_descriptor));
        if (OS_SUCC != result) {
            usb_dbg(USB_ERROR, "get config descriptor, receive control msg fail");
            goto fail;
        }

        usb_dev->config_buffer[i] = kmalloc(usb_dev->config_info[i].config.wTotalLength);
        if (OS_NULL == usb_dev->config_buffer[i]) {
            usb_dbg(USB_ERROR, "get config descriptor, alloc descriptor fail");
            goto fail;
        }

        /* 请求全部描述符 */
        setup.wLength = usb_dev->config_info[i].config.wTotalLength;

        /* 获取所有设备的配置描述符 */
        result = usb_dev->hc_operation->usb_receive_control_transfer(usb_dev, usb_dev->usb_addr, 0, &setup, usb_dev->config_buffer[i], usb_dev->config_info[i].config.wTotalLength);
        if (OS_SUCC != result) {
            goto fail;
        }

        /* 解析描述符 */
        parse_usb_descriptor(usb_dev, i);
    }

    return OS_SUCC;
  fail:
    clear_config_descriptor(usb_dev);
    return OS_FAIL;
}

/***************************************************************
 * description : usb_20.pdf 8.3.2.1 address field
 *               地址是一个设备的标识, 范围从0-127
 *               一个host只能支持128个设备
 * history     :
 ***************************************************************/
LOCALC os_ret set_usb_dev_addr(struct usb_device *usb)
{
    struct usb_setup_data setup = { 0 };

    setup.bmRequestType = USB_TYPE_STANDARD | USB_RECIP_DEVICE | USB_DIR_OUT;
    setup.bRequest = USB_REQ_SET_ADDRESS;
    setup.wValue = usb->usb_addr;
    setup.wIndex = 0;
    setup.wLength = 0;

    if (OS_SUCC != usb->hc_operation->usb_send_control_transfer(usb, 0, 0, &setup, 0, 0)) {
        return OS_FAIL;
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret config_usb_device(struct usb_device *usb, os_u8 index)
{
    struct usb_setup_data setup = { 0 };

    setup.bmRequestType = USB_TYPE_STANDARD | USB_RECIP_DEVICE | USB_DIR_OUT;
    setup.bRequest = USB_REQ_SET_CONFIGURATION;
    setup.wValue = usb->config_info[index].config.bConfigurationValue;
    setup.wIndex = 0;
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
LOCALC os_ret set_usb_interface(struct usb_device *usb, struct usb_interface_descriptor *interface)
{
    struct usb_setup_data setup = { 0 };

    setup.bmRequestType = USB_TYPE_STANDARD | USB_RECIP_INTERFACE | USB_DIR_OUT;
    setup.bRequest = USB_REQ_SET_INTERFACE;
    setup.wValue = interface->bAlternateSetting;
    setup.wIndex = interface->bInterfaceNumber;
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
struct usb_device *alloc_usb_device(const os_void *controller, const enum usb_speed_type speed, const struct usb_host_controller_operations *operation)
{
    struct usb_device *usb;

    cassert(OS_NULL != operation);

    usb = kmalloc(sizeof(struct usb_device));
    if (OS_NULL == usb) {
        usb_dbg(USB_ERROR, "alloc_usb_device fail");
        return OS_NULL;
    }
    mem_set(usb, 0, sizeof(struct usb_device));
    usb->descriptor.bMaxPacketSize0 = DEFAULT_MAX_PACKET_SIZE;
    usb->hc_operation = operation;
    usb->host_controller = (os_void *) controller;
    usb->speed = speed;
    usb->multiple_function = OS_NULL;
    if (operation->alloc_usb_device_addr) {
        usb->usb_addr = operation->alloc_usb_device_addr(usb);
        if (0 == usb->usb_addr) {
            kfree(usb);
            return OS_NULL;
        }
    }
    /* alloc endpoint for this device addr */
    if (usb->hc_operation->add_usb_default_endpoint) {
        usb->hc_operation->add_usb_default_endpoint(usb);
    }
    return usb;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void free_usb_device(struct usb_device *device)
{
    if (device->hc_operation->del_usb_default_endpoint) {
        device->hc_operation->del_usb_default_endpoint(device);
    }
    if (device->hc_operation->free_usb_device_addr) {
        device->hc_operation->free_usb_device_addr(device);
    }
    kfree(device);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_u16 usb_endpoint_mps(struct usb_device *usb, os_u8 ep_num, enum usb_direction dir)
{
    os_u16 mps;
    os_u8 i, j;
    os_u8 num;
    struct usb_endpoint_descriptor **endpoint;

    cassert(OS_NULL != usb);

    mps = DEFAULT_MAX_PACKET_SIZE;
    if (0 == ep_num) {
        /* 端点0的mps从设备描述符中获取 */
        if (DEFAULT_MAX_PACKET_SIZE < usb->descriptor.bMaxPacketSize0) {
            mps = usb->descriptor.bMaxPacketSize0;
        }
    } else {
        /* 非端点0的mps从端点描述符中获取 */
        for (i = 0; i < usb->config_info[usb->config_index].config.bNumInterfaces; i++) {
            num = usb->multiple_function[i].itf->interface->bNumEndpoints;
            endpoint = usb->multiple_function[i].itf->endpoint;
            for (j = 0; j < num; j++) {
                if (((USB_IN == dir) && (endpoint_is_in(endpoint[j]->bEndpointAddress)))
                 || ((USB_OUT == dir) && (endpoint_is_out(endpoint[j]->bEndpointAddress)))) {
                    if (ep_num == endpoint_num(endpoint[j]->bEndpointAddress)) {
                        return endpoint_mps(endpoint[j]->wMaxPacketSize);
                    }
                }
            }
        }
    }
    return mps;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_u32 usb_endpoint_proportion(struct usb_device *usb, os_u8 ep_num, enum usb_direction dir)
{
    os_u8 i, j;
    os_u8 num;
    struct usb_endpoint_descriptor **endpoint;

    cassert(OS_NULL != usb);

    if (0 != ep_num) {
        /* 非端点0的mps从端点描述符中获取 */
        for (i = 0; i < usb->config_info[usb->config_index].config.bNumInterfaces; i++) {
            num = usb->multiple_function[i].itf->interface->bNumEndpoints;
            endpoint = usb->multiple_function[i].itf->endpoint;
            for (j = 0; j < num; j++) {
                if (((USB_IN == dir) && (endpoint_is_in(endpoint[j]->bEndpointAddress)))
                 || ((USB_OUT == dir) && (endpoint_is_out(endpoint[j]->bEndpointAddress)))) {
                    if (ep_num == endpoint_num(endpoint[j]->bEndpointAddress)) {
                        return usb->multiple_function[i].timeout_proportion;
                    }
                }
            }
        }
    }
    return 0;
}

/***************************************************************
 * description : usb驱动链表
 ***************************************************************/
struct usb_driver_list {
    struct list_node head;
    const struct usb_driver *driver;
};

LOCALD rwlock_t driver_lock;
LOCALD struct usb_driver_list drvier_list = { { &drvier_list.head, &drvier_list.head }, OS_NULL };

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void probe_usb_device(struct usb_hdevice *usb)
{
    struct list_node *node;
    struct usb_driver_list *driver;
    const struct usb_device_id *id;
    struct usb_interface_descriptor *interface;

    interface = usb->itf->interface;
    cassert(OS_NULL != interface);
    read_lock(&driver_lock);
    loop_list(node, &drvier_list.head) {
        driver = list_addr(node, struct usb_driver_list, head);
        /* hid code */
        id = driver->driver->id;
        if (((id->idVendor == usb->dev->descriptor.idVendor) || (USB_ANY_ID == id->idVendor))
         && ((id->idProduct == usb->dev->descriptor.idProduct) || (USB_ANY_ID == id->idProduct))
         && ((id->bInterfaceClass == interface->bInterfaceClass) || (USB_ANY_ID == id->bInterfaceClass))
         && ((id->bInterfaceSubClass == interface->bInterfaceSubClass) || (USB_ANY_ID == id->bInterfaceSubClass))
         && ((id->bInterfaceProtocol== interface->bInterfaceProtocol) || (USB_ANY_ID == id->bInterfaceProtocol))) {
            read_unlock(&driver_lock);
            if ((driver->driver->add_device) && (OS_FALSE == usb->load_driver)) {
                usb->load_driver = OS_TRUE;
                driver->driver->add_device(usb);
                return;
            }
            read_lock(&driver_lock);
        }
    }
    read_unlock(&driver_lock);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void remove_usb_device(struct usb_hdevice *usb)
{
    struct list_node *node;
    struct usb_driver_list *driver;
    const struct usb_device_id *id;
    struct usb_interface_descriptor *interface;

    if (OS_NULL != usb->itf) {
        interface = usb->itf->interface;
        cassert(OS_NULL != interface);
        read_lock(&driver_lock);
        loop_list(node, &drvier_list.head) {
            driver = list_addr(node, struct usb_driver_list, head);
            /* hid code */
            id = driver->driver->id;
            if (((id->idVendor == usb->dev->descriptor.idVendor) || (USB_ANY_ID == id->idVendor))
             && ((id->idProduct == usb->dev->descriptor.idProduct) || (USB_ANY_ID == id->idProduct))
             && ((id->bInterfaceClass == interface->bInterfaceClass) || (USB_ANY_ID == id->bInterfaceClass))
             && ((id->bInterfaceSubClass == interface->bInterfaceSubClass) || (USB_ANY_ID == id->bInterfaceSubClass))
             && ((id->bInterfaceProtocol== interface->bInterfaceProtocol) || (USB_ANY_ID == id->bInterfaceProtocol))) {
                read_unlock(&driver_lock);
                usb->load_driver = OS_FALSE;
                driver->driver->del_device(usb);
                return;
            }
        }
        read_unlock(&driver_lock);
    }
}

/***************************************************************
 * description : usb设备列表
 ***************************************************************/
struct usb_device_list {
    struct list_node head;
    /* not HDEVICE or interface */
    struct usb_device *device;
};

/* all device of ehci(s) will link to list */
LOCALD struct usb_device_list device_list = { { &device_list.head, &device_list.head }, OS_NULL };
LOCALD rwlock_t device_lock;

/***************************************************************
 * description : 纪录所有的usb设备, 以备后续驱动程序更新
 * history     :
 ***************************************************************/
LOCALC os_void record_usb_device(struct usb_device *usb)
{
    struct usb_device_list *node;

    node = kmalloc(sizeof(struct usb_device_list));
    if (OS_NULL == node) {
        usb_dbg(USB_ERROR, "alloc mem fail %d", __LINE__);
        return;
    }

    node->device = usb;
    write_lock(&device_lock);
    add_list_head(&device_list.head, &node->head);
    write_unlock(&device_lock);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void unrecord_usb_device(struct usb_device *usb)
{
    struct list_node *i, *_save;
    struct usb_device_list *node;

    read_lock(&device_lock);
    loop_del_list(i, _save, &device_list.head) {
        node = list_addr(i, struct usb_device_list, head);

        if (node->device == usb) {
            del_list(&node->head);
            kfree(node);
            break;
        }
    }
    read_unlock(&device_lock);
}

/***************************************************************
 * description : multiple host controller
 * history     :
 ***************************************************************/
struct usb_device *find_usb_device(os_void *hc, os_u32 hub_addr, os_u32 port)
{
    struct list_node *i;
    struct usb_device_list *node;

    cassert(OS_NULL != hc);

    read_lock(&device_lock);
    loop_list(i, &device_list.head) {
        node = list_addr(i, struct usb_device_list, head);

        if ((node->device->host_controller == hc) && (node->device->hub_addr == hub_addr) && (node->device->hub_port_num == port)) {
            read_unlock(&device_lock);
            return node->device;
        }
    }
    read_unlock(&device_lock);
    usb_dbg(USB_ERROR, "no usb device is found");
    return OS_NULL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret register_usb_driver(const struct usb_driver *driver)
{
    struct usb_driver_list *list;
    struct list_node *node;
    struct usb_device_list *device;
    const struct usb_device_id *id;
    struct usb_interface_descriptor *interface;
    os_u8 i;

    cassert((OS_NULL != driver) && (OS_NULL != driver->id));

    /* 添加到驱动列表中 */
    list = kmalloc(sizeof(struct usb_driver_list));
    if (OS_NULL == list) {
        usb_dbg(USB_ERROR, "alloc mem fail %d", __LINE__);
        return OS_FAIL;
    }
    list->driver = driver;
    write_lock(&driver_lock);
    add_list_head(&drvier_list.head, &list->head);
    write_unlock(&driver_lock);

    id = driver->id;
    read_lock(&device_lock);
    /* 查看是否有符合的usb设备, 更新驱动 */
    loop_list(node, &device_list.head) {
        device = list_addr(node, struct usb_device_list, head);

        for (i = 0; i < device->device->config_info[device->device->config_index].config.bNumInterfaces; i++) {
            cassert(OS_NULL != device->device->multiple_function);
            interface = device->device->multiple_function[i].itf->interface;
            if ((OS_FALSE == device->device->multiple_function[i].load_driver)
             && ((id->idVendor == device->device->descriptor.idVendor) || (USB_ANY_ID == id->idVendor))
             && ((id->idProduct == device->device->descriptor.idProduct) || (USB_ANY_ID == id->idProduct))
             && ((id->bInterfaceClass == interface->bInterfaceClass) || (USB_ANY_ID == id->bInterfaceClass))
             && ((id->bInterfaceSubClass == interface->bInterfaceSubClass) || (USB_ANY_ID == id->bInterfaceSubClass))
             && ((id->bInterfaceProtocol== interface->bInterfaceProtocol) || (USB_ANY_ID == id->bInterfaceProtocol))) {
                read_unlock(&device_lock);
                device->device->multiple_function[i].load_driver = OS_TRUE;
                driver->add_device(&device->device->multiple_function[i]);
                read_lock(&device_lock);
            }
        }
    }
    read_unlock(&device_lock);
    return OS_SUCC;
}

/***************************************************************
 * description : select one common configuration.
 * history     :
 ***************************************************************/
LOCALC os_u8 choose_usb_configuration(struct usb_device *udev)
{
    os_u8 i, j;
    os_u8 num_configs, cfg_index;
    struct usb_config_descriptor *c, *best;
    struct usb_interface_descriptor *desc;

    best = OS_NULL;
    cfg_index = 0;
    num_configs = udev->descriptor.bNumConfigurations;
    for (i = 0; i < num_configs; i++) {
        c = &udev->config_info[i].config;
        /* it's possible that a config has no interfaces! */
        for (j = 0; j < c->bNumInterfaces; j++) {
            /* from configs, choose the first one whose first interface is for a non-vendor-specific class. */
            desc = udev->config_info[i].intf_info[j].interface;
            /* bDeviceClass:
               If this field is reset to zero,
               each interface within a configuration specifies its own class information and the various interfaces operate independently.
               If this field is set to a value between 1 and FEH,
               the device supports different class specifications on different interfaces and the interfaces may not operate independently.
               This value identifies the class definition used for the aggregate interfaces.
               If this field is set to FFH,
               the device class is vendor-specific. */
            /* bInterfaceClass:
               A value of zero is reserved for future standardization.
               If this field is set to FFH, the interface class is vendor-specific.
               All other values are reserved for assignment by the USB-IF. */
            if ((USB_CLASS_VENDOR_SPEC != udev->descriptor.bDeviceClass)
             && (USB_CLASS_VENDOR_SPEC != desc->bInterfaceClass)) {
                best = c;
                cfg_index = i;
                break;
            }
        }
    }

    if (best) {
        usb_dbg(USB_INFO, "configuration %d chosen from %d", i, num_configs);
    } else {
        /* if all the remaining configs are vendor-specific, choose the first one. */
        cfg_index = 0;
    }
    return cfg_index;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret create_usb_composite_device(struct usb_device *usb)
{
    os_u8 bNumInterfaces;
    /* Zero-based value identifying the index in the array of concurrent interfaces supported by this configuration. */
    os_u8 bInterfaceNumber;
    os_u8 bAlternateSetting;
    struct usb_interface_descriptor *interface;
    struct usb_config_info *config_info;
    struct usb_itf_info *intf_info;
    os_u8 i;

    config_info = &usb->config_info[usb->config_index];
    bNumInterfaces = config_info->config.bNumInterfaces;
    usb->multiple_function = kmalloc(bNumInterfaces * sizeof(struct usb_hdevice));
    if (usb->multiple_function) {
        for (i = 0; i < bNumInterfaces; i++) {
            usb->multiple_function[i].check = HUSB_CHECK;
            usb->multiple_function[i].dev = usb;
            usb->multiple_function[i].itf = OS_NULL;
            usb->multiple_function[i].load_driver = OS_FALSE;
            usb->multiple_function[i].timeout_proportion = 0;
            usb->multiple_function[i].dedicated = OS_NULL;
        }

        bAlternateSetting = DEFAULT_INTERFACE_SETTING;
        for (bInterfaceNumber = 0; bInterfaceNumber < config_info->config.bNumInterfaces; bInterfaceNumber++) {
            interface = OS_NULL;
            for (i = 0; i < config_info->total_intf_count; i++) {
                intf_info = &config_info->intf_info[i];
                if ((bInterfaceNumber == intf_info->interface->bInterfaceNumber)
                 && (bAlternateSetting == intf_info->interface->bAlternateSetting)) {
                    interface = intf_info->interface;
                    break;
                }
            }
            cassert(OS_NULL != interface);
            usb->multiple_function[bInterfaceNumber].itf = intf_info;
        }
        return OS_SUCC;
    }
    usb_dbg(USB_ERROR, "create_usb_device_function fail");
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void delete_usb_composite_device(struct usb_device *device)
{
    if (device->multiple_function) kfree(device->multiple_function);
}

LOCALC os_ret WEAK_DEV_PARA update_usb_dev_proportion(os_u32 function, os_void *in, os_void *out);

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void update_usb_device_proportion(struct usb_hdevice *device)
{
    os_ret ret;
    os_u32 proportion;
    struct usb_interface_descriptor *interface;
    struct usb_dev_para para;

    para.vid = device->dev->descriptor.idVendor;
    para.pid = device->dev->descriptor.idProduct;
    interface = device->itf->interface;
    para.class = interface->bInterfaceClass;
    para.subclass = interface->bInterfaceSubClass;
    para.prot = interface->bInterfaceProtocol;
    ret = update_usb_dev_proportion(FUNCTION_USB_PARA, &para, &proportion);
    if (OS_SUCC == ret) {
        device->timeout_proportion = proportion;
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void usb_config_endpoint(struct usb_device *dev)
{
    os_uint i, j;
    struct usb_itf_info *intf;

    for (i = 0; i < dev->config_info[dev->config_index].total_intf_count; i++) {
        intf = &dev->config_info[dev->config_index].intf_info[i];
        for (j = 0; j < intf->interface->bNumEndpoints; j++) {
            if (dev->hc_operation->add_usb_endpoint) {
                dev->hc_operation->add_usb_endpoint(dev, intf->endpoint[j]);
            }
        }
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void usb_deconfig_endpoint(struct usb_device *dev)
{
    os_uint i, j;
    struct usb_itf_info *intf;

    for (i = 0; i < dev->config_info[dev->config_index].total_intf_count; i++) {
        intf = &dev->config_info[dev->config_index].intf_info[i];
        for (j = 0; j < intf->interface->bNumEndpoints; j++) {
            if (dev->hc_operation->del_usb_endpoint) {
                dev->hc_operation->del_usb_endpoint(dev, intf->endpoint[j]);
            }
        }
    }
}

/***************************************************************
 * description : 逐个枚举所有的设备, usb_20.pdf 9.1.2
 * history     :
 ***************************************************************/
os_ret enum_usb_device(struct usb_device *usb)
{
    os_u8 i;
    os_ret result;

    cassert((OS_NULL != usb) && (OS_NULL != usb->hc_operation));

    usb_dbg(USB_INFO, "enum usb device %d", usb->usb_addr);

    /* Before the USB device receives a unique address,
       it’s default pipe is still accessible via the default address.
       The host reads the device descriptor to determine what actual maximum data payload size this USB device’s default pipe can use. */
    /* 默认管道获取设备描述符 */
    result = get_default_device_descriptor(usb);
    if (OS_SUCC != result) {
        usb_dbg(USB_ERROR, "get default device descriptor fail");
        return OS_FAIL;
    }

    /* 使用地址0和端点0, 分配设备地址, 方向输出 */
    if (usb->hc_operation->assign_usb_device_addr) {
        usb->hc_operation->assign_usb_device_addr(usb);
    } else {
        result = set_usb_dev_addr(usb);
        if (OS_SUCC != result) {
            usb_dbg(USB_ERROR, "set usb addr fail");
            return OS_FAIL;
        }
    }

    /* update mps & address of control pipe */
    usb_dbg(USB_INFO, "addr %d max packet size %d", usb->usb_addr, usb->descriptor.bMaxPacketSize0);
    if (usb->hc_operation->update_usb_default_endpoint) { // NOTE: after in & out are used
        usb->hc_operation->update_usb_default_endpoint(usb);
    }

    /* 从新的设备地址和端点0获取设备描述符 */
    result = get_device_descriptor(usb, sizeof(struct usb_device_descriptor));
    if (OS_SUCC != result) {
        usb_dbg(USB_ERROR, "get device descriptor fail");
        return OS_FAIL;
    }
    usb_dbg(USB_INFO, "usb device vender id %x, product id %x", usb->descriptor.idVendor, usb->descriptor.idProduct);

    /* 更新该设备的控制描述符传输最大字节数 */
    //usb->hc_operation->update_usb_device_mps(usb);

    /* 从新的设备地址和端点0获取配置描述符 */
    result = get_config_descriptor(usb);
    if (OS_SUCC != result) {
        usb_dbg(USB_ERROR, "get config descriptor fail");
        return OS_FAIL;
    }

    usb_config_endpoint(usb);

    /* choose the proper index */
    usb->config_index = choose_usb_configuration(usb);
    result = config_usb_device(usb, usb->config_index);
    if (OS_SUCC != result) {
        usb_dbg(USB_ERROR, "config device fail");
        return OS_FAIL;
    }
    delay_ms(100);
    usb_dbg(USB_INFO, "config(%d %d) usb succ", usb->config_index, usb->descriptor.bNumConfigurations);

    create_usb_composite_device(usb);

    /* 纪录发现的usb设备 */
    record_usb_device(usb);

    usb_dbg(USB_INFO, "probe usb device (%d)", usb->config_info[usb->config_index].config.bNumInterfaces);
    for (i = 0; i < usb->config_info[usb->config_index].config.bNumInterfaces; i++) {
        /* config interface, it is optional, so some devices stall. */
        result = set_usb_interface(usb, usb->multiple_function[i].itf->interface);
        if (OS_SUCC != result) {
            usb_dbg(USB_ERROR, "set interface %d fail", usb->multiple_function[i].itf->interface->bInterfaceNumber);
            /* some devices will fail this message, ignore it */
            // return OS_FAIL;
        }
        delay_ms(10);

        update_usb_device_proportion(&usb->multiple_function[i]);

        /* 查找驱动处理该设备 */
        probe_usb_device(&usb->multiple_function[i]);
    }

    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret unenum_usb_device(struct usb_device *usb)
{
    os_u8 i;

    cassert(OS_NULL != usb);

    usb_deconfig_endpoint(usb);

    for (i = 0; i < usb->config_info[usb->config_index].config.bNumInterfaces; i++) {
        /* remove drivers that depends on special information */
        remove_usb_device(&usb->multiple_function[i]);
    }

    delete_usb_composite_device(usb);

    /* common information */
    unrecord_usb_device(usb);

    clear_config_descriptor(usb);

    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void dump_usb(os_void)
{
    struct list_node *node;
    struct usb_device_list *device;

    print("usb info (hc, addr, hub, port)\n");
    loop_list(node, &device_list.head) {
        device = list_addr(node, struct usb_device_list, head);
        print("%x %d %d %d\n", device->device->host_controller, device->device->usb_addr, device->device->hub_addr, device->device->hub_port_num);
    }
}

LOCALD os_u8 usb_debug_name[] = { "usb" };
LOCALD struct dump_info usb_debug = {
    usb_debug_name,
    dump_usb
};

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void init_usb(os_void)
{
    /* 初始化驱动列表 */
    init_rw_lock(&driver_lock);
    init_list_head(&drvier_list.head);
    drvier_list.driver = OS_NULL;

    /* 初始化设备列表 */
    init_rw_lock(&device_lock);
    init_list_head(&device_list.head);
    device_list.device = OS_NULL;

    /* init common hub thread */
    create_hub_monitor();

    if (OS_SUCC != register_dump(&usb_debug)) {
        usb_dbg(USB_ERROR, "usb register dump fail");
    }
}

bus_init_func(BUS_P0, init_usb);

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_u8 alloc_usb_addr(os_u8 addr_bitmap[USB_DEVICE_COUNT/8])
{
    os_uint i, j;
    os_u8 mask;
    os_u8 addr;

    addr_bitmap[0] |= 1; /* bypass address 0 */
    for (i = 0; i < USB_DEVICE_COUNT/8; i++) {
        mask = 0x1;
        for (j = 0; j < 8; j++) {
            if (0 == (mask & addr_bitmap[i])) {
                addr = i * 8 + j;
                addr_bitmap[i] |= mask;
                return addr;
            }
            mask = mask << 1;
        }
    }
    /* alloc addr fail */
    return 0;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void free_usb_addr(os_u8 addr_bitmap[USB_DEVICE_COUNT/8], os_u8 addr)
{
    os_u8 i;
    os_u8 mask;

    i = addr % 8;
    mask = 0x1;
    mask = ~(mask << i);
    i = addr / 8;
    addr_bitmap[i] &= mask;
}

/***************************************************************
 * description : 控制传输
 * history     :
 ***************************************************************/
os_ret recv_usb_control_data(HDEVICE usb, struct usb_setup_data *cmd, os_void *data, os_uint len)
{
    struct usb_device *device;

    cassert((OS_NULL != usb) && (HUSB_CHECK == ((struct usb_hdevice *) usb)->check) && (OS_NULL != cmd));
    device = ((struct usb_hdevice *) usb)->dev;
    /* 控制传输采用端点号0 */
    if (OS_SUCC != device->hc_operation->usb_receive_control_transfer(device, device->usb_addr, 0, cmd, data, len)) {
        return OS_FAIL;
    }
    return OS_SUCC;
}

/***************************************************************
 * description : 控制传输
 * history     :
 ***************************************************************/
os_ret send_usb_control_data(HDEVICE usb, struct usb_setup_data *cmd, os_void *data, os_uint len)
{
    struct usb_device *device;

    cassert((OS_NULL != usb) && (HUSB_CHECK == ((struct usb_hdevice *) usb)->check) && (OS_NULL != cmd));
    device = ((struct usb_hdevice *) usb)->dev;
    if (OS_SUCC != device->hc_operation->usb_send_control_transfer(device, device->usb_addr, 0, cmd, data, len)) {
        return OS_FAIL;
    }
    return OS_SUCC;
}

/***************************************************************
 * description : 批量传输
 * history     :
 ***************************************************************/
os_ret send_usb_bulk_data(HDEVICE usb, os_u8 endpoint, os_void *data, os_uint len)
{
    struct usb_device *device;

    cassert((OS_NULL != usb) && (HUSB_CHECK == ((struct usb_hdevice *) usb)->check));
    device = ((struct usb_hdevice *) usb)->dev;
    if (OS_SUCC != device->hc_operation->usb_send_bulk_transfer(device, endpoint, data, len)) {
        return OS_FAIL;
    }
    return OS_SUCC;
}

/***************************************************************
 * description : 批量传输
 * history     :
 ***************************************************************/
os_ret recv_usb_bulk_data(HDEVICE usb, os_u8 endpoint, os_void *data, os_uint len)
{
    struct usb_device *device;

    cassert((OS_NULL != usb) && (HUSB_CHECK == ((struct usb_hdevice *) usb)->check));
    device = ((struct usb_hdevice *) usb)->dev;
    if (OS_SUCC != device->hc_operation->usb_receive_bulk_transfer(device, endpoint, data, len)) {
        return OS_FAIL;
    }
    return OS_SUCC;
}

/***************************************************************
 * description : 中断传输
 * history     :
 ***************************************************************/
os_ret add_usb_int_pipe(HDEVICE usb, os_u8 endpoint, os_u8 *data, os_uint len, os_u8 bInterval, os_void (*recv)(os_u8 *data))
{
    struct usb_device *device;

    cassert((OS_NULL != usb) && (HUSB_CHECK == ((struct usb_hdevice *) usb)->check) && (OS_NULL != recv));
    device = ((struct usb_hdevice *) usb)->dev;
    if (OS_SUCC != device->hc_operation->usb_receive_interrupt_transfer(device, endpoint, data, len, bInterval, recv)) {
        return OS_FAIL;
    }
    return OS_SUCC;
}

/***************************************************************
 * description : 中断传输
 * history     :
 ***************************************************************/
os_ret del_usb_int_pipe(HDEVICE usb, os_u8 endpoint)
{
    struct usb_device *device;

    cassert((OS_NULL != usb) && (HUSB_CHECK == ((struct usb_hdevice *) usb)->check));
    device = ((struct usb_hdevice *) usb)->dev;
    if (OS_SUCC != device->hc_operation->usb_cancel_interrupt_transer(device, endpoint)) {
        return OS_FAIL;
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void set_usb_dedicated(HDEVICE usb, os_void *dedicated)
{
    cassert((OS_NULL != usb) && (HUSB_CHECK == ((struct usb_hdevice *) usb)->check));
    ((struct usb_hdevice *) usb)->dedicated = dedicated;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void *get_usb_dedicated(HDEVICE usb)
{
    cassert((OS_NULL != usb) && (HUSB_CHECK == ((struct usb_hdevice *) usb)->check));
    return ((struct usb_hdevice *) usb)->dedicated;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
struct usb_device_descriptor *usb_device_desc(HDEVICE usb)
{
    cassert((OS_NULL != usb) && (HUSB_CHECK == ((struct usb_hdevice *) usb)->check));
    return &((struct usb_hdevice *) usb)->dev->descriptor;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
struct usb_config_descriptor *usb_config_desc(HDEVICE usb)
{
    struct usb_hdevice *device;

    cassert((OS_NULL != usb) && (HUSB_CHECK == ((struct usb_hdevice *) usb)->check));
    device = usb;
    return &device->dev->config_info[device->dev->config_index].config;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
struct usb_itf_info *usb_interface_info(HDEVICE usb)
{
    cassert((OS_NULL != usb) && (HUSB_CHECK == ((struct usb_hdevice *) usb)->check));
    return ((struct usb_hdevice *) usb)->itf;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
enum usb_speed_type usb_device_speed(HDEVICE usb)
{
    cassert((OS_NULL != usb) && (HUSB_CHECK == ((struct usb_hdevice *) usb)->check));
    return ((struct usb_hdevice *) usb)->dev->speed;
}

