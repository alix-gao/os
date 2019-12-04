/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : hid.c
 * version     : 1.0
 * description : HID1-11.pdf
 * author      : gaocheng
 * date        : 2011-01-01
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include <vbs.h>
#include <vds.h>
#include "hid.h"

/***************************************************************
 global variable declare
 ***************************************************************/

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description :
 ***************************************************************/
enum hid_mouse_status_type {
    HID_MOUSE_EXIST,
    HID_MOUSE_NO_EXIST
};
LOCALD volatile os_u32 hid_mouse_status _CPU_ALIGNED_ = HID_MOUSE_NO_EXIST;

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC inline os_s32 axes_value(os_u8 *data, os_u8 offset, os_u8 size)
{
    os_u32 i;

    data += (offset / 8);
    i = *(os_u32 *) data;
    i = i >> (offset % 8);
    cassert((0 < size) && (32 >= size));
    i &= ~(UINT32_MAX << size);
    if (i & (1 << (size - 1))) {
        i |= (UINT32_MAX << size);
    }

    return i;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void hid_mouse_complete(os_u8 *buffer)
{
    struct hid_mouse_report_wrap *mouse_buffer;
    os_u8 *data;
    os_s32 x, y, wheel;

    if (HID_MOUSE_EXIST == hid_mouse_status) {
        rmb();
        mouse_buffer = (struct hid_mouse_report_wrap *)(buffer - (pointer)(&((struct hid_mouse_report_wrap *) 0)->data));
        data = mouse_buffer->data;
        /* If the Report ID tag was not used,
           all values are returned in a single report and a prefix ID is not included in that report. */
        x = axes_value(data, mouse_buffer->x_offset, mouse_buffer->x_size);
        y = axes_value(data, mouse_buffer->y_offset, mouse_buffer->y_size);
        wheel = axes_value(data, mouse_buffer->wheel_offset, mouse_buffer->wheel_size);
        send_mouse_msg(!!(mouse_buffer->left_mask & data[mouse_buffer->left_offset]),
                       !!(mouse_buffer->right_mask & data[mouse_buffer->right_offset]),
                       !!(mouse_buffer->mid_mask & data[mouse_buffer->mid_offset]),
                       x, y, wheel);
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret find_hid_mouse_para(struct hid_mouse_report_wrap *mouse_buffer, struct hid_report_info *report_info)
{
    struct list_node *node;
    struct hid_report_info *t;
    os_u32 i;
    os_u32 bit_offset, offset;
    os_u32 report_id;

    /* 6.2.2.7
       Report ID 1000 01 nn Unsigned value that specifies the Report ID.
       If a Report ID tag is used anywhere in Report descriptor, all data reports for the device are preceded by a single byte ID field.
       All items succeeding the first Report ID tag but preceding a second Report ID tag are included in a report prefixed by a 1-byte ID.
       All items succeeding the second but preceding a third Report ID tag are included in a second report prefixed by a second ID, and so on.
       This Report ID value indicates the prefix added to a particular report.
       For example, a Report descriptor could define a 3-byte report with a Report ID of 01.
       This device would generate a 4-byte data report in which the first byte is 01. */
    report_id = list_addr(report_info->node.next, struct hid_report_info, node)->report_id;
    flog("report id: %d\n", report_id);
    if (report_id) {
        bit_offset = 8;
    } else {
        bit_offset = 0;
    }
    loop_list(node, report_info->node.prev) {
        t = list_addr(node, struct hid_report_info, node);
        hid_dbg(HID_INFO, "usage %d %d, %d\n", t->usage_page, t->usage_cnt, t->report_id);
        switch (t->usage_page) {
        /* all the report id is the same? */
        case HID_UP_BUTTON:
            offset = bit_offset;
            mouse_buffer->left_offset = offset / 8;
            mouse_buffer->left_mask = 1 << (offset % 8);
            offset += t->report_size;
            mouse_buffer->right_offset = offset / 8;
            mouse_buffer->right_mask = 1 << (offset % 8);
            offset += t->report_size;
            mouse_buffer->mid_offset = offset / 8;
            mouse_buffer->mid_mask = 1 << (offset % 8);
            break;
        case HID_UP_GENDESK:
        case HID_UP_MSVENDOR:
        default:
            offset = bit_offset;
            for (i = 0; i < t->usage_cnt; i++)
            switch (t->usage[i]) {
            case HID_GD_X:
                mouse_buffer->x_offset = offset;
                mouse_buffer->x_size = t->report_size;
                offset += t->report_size;
                break;
            case HID_GD_Y:
                mouse_buffer->y_offset = offset;
                mouse_buffer->y_size = t->report_size;
                offset += t->report_size;
                break;
            case HID_GD_Z:
                mouse_buffer->z_offset = offset;
                mouse_buffer->z_size = t->report_size;
                offset += t->report_size;
                break;
            case HID_GD_WHEEL:
                mouse_buffer->wheel_offset = offset;
                mouse_buffer->wheel_size = t->report_size;
                offset += t->report_size;
                break;
            default:
                break;
            }
            break;
        case HID_UP_UNDEFINED:
            break;
        }
        bit_offset += (t->report_size * t->report_count);
        hid_dbg(HID_INFO, "bit_offset: %d\n", bit_offset);
    }
    hid_dbg(HID_INFO, "--------------\n");
    hid_dbg(HID_INFO, "x_offset %d\n", mouse_buffer->x_offset);
    hid_dbg(HID_INFO, "y_offset %d\n", mouse_buffer->y_offset);
    hid_dbg(HID_INFO, "z_offset %d\n", mouse_buffer->z_offset);
    hid_dbg(HID_INFO, "wheel_offset %d\n", mouse_buffer->wheel_offset);
    hid_dbg(HID_INFO, "left_offset %x\n", mouse_buffer->left_offset);
    hid_dbg(HID_INFO, "left_mask %x\n", mouse_buffer->left_mask);
    hid_dbg(HID_INFO, "right_offset %x\n", mouse_buffer->right_offset);
    hid_dbg(HID_INFO, "right_mask %x\n", mouse_buffer->right_mask);
    hid_dbg(HID_INFO, "mid_offset %x\n", mouse_buffer->mid_offset);
    hid_dbg(HID_INFO, "mid_mask %x\n", mouse_buffer->mid_mask);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret hid_mouse(HDEVICE usb, struct usb_itf_info *intf_info, struct hid_report_info *report_info)
{
    os_u8 pipe_num;
    struct usb_endpoint_descriptor *endpoint;
    struct hid_mouse_report_wrap *mouse_buffer;
    struct hid_device *dedicated;
    os_u32 i;

    dedicated = get_usb_dedicated(usb);

    /* 从端点描述符中找到ms需要的个端点信息, 选择端点号小的. */
    endpoint = OS_NULL;
    if (1 != intf_info->interface->bNumEndpoints) {
        hid_dbg(HID_ERROR, "hid mouse endpoint is not 1\n");
        /* return OS_FAIL; */
    }
    pipe_num = intf_info->interface->bNumEndpoints;
    for (i = 0; i < pipe_num; i++) {
        if (USB_INTERRUPT_TRANSFER == get_usb_transfer_type(intf_info->endpoint[i]->bmAttributes)) {
            if (endpoint_is_in(intf_info->endpoint[i]->bEndpointAddress)) {
                if ((OS_NULL == endpoint)
                 || (endpoint->bEndpointAddress > intf_info->endpoint[i]->bEndpointAddress)) {
                    endpoint = intf_info->endpoint[i];
                }
            }
        }
    }
    if (OS_NULL == endpoint) {
        hid_dbg(HID_ERROR, "hid mouse lookup endpoint fail\n");
        return OS_FAIL;
    }

    /* 设置描述符, 从端点描述符中获取endpoint num, 即管道号 */
    pipe_num = endpoint_num(endpoint->bEndpointAddress); /* usb_20.pdf 9.6.6 endpoint */
    hid_dbg(HID_INFO, "endpoint addr %d %d (%d)\n", intf_info->interface->bNumEndpoints, pipe_num, endpoint->bInterval);
    dedicated->pipe = pipe_num;

    mouse_buffer = kmalloc(endpoint->wMaxPacketSize + sizeof(struct hid_mouse_report_wrap));
    if (OS_NULL == mouse_buffer) {
        hid_dbg(HID_ERROR, "alloc mouse buffer fail\n");
        return OS_FAIL;
    }
    mem_set(mouse_buffer, 0, sizeof(struct hid_mouse_report_wrap));
    dedicated->int_buffer = mouse_buffer;

    find_hid_mouse_para(mouse_buffer, report_info);

    add_usb_int_pipe(usb, pipe_num,
                     mouse_buffer->data, endpoint->wMaxPacketSize,
                     endpoint->bInterval, hid_mouse_complete);
    wmb();
    hid_mouse_status = HID_MOUSE_EXIST;

    return OS_SUCC;
}

/***************************************************************
 * description : refer to 7.1.1 Get_Descriptor Request
 * history     :
 ***************************************************************/
LOCALC os_ret get_report_descriptor(HDEVICE usb, os_u8 itfnum, os_u8 *buffer, os_u16 len)
{
    struct usb_setup_data setup;

    setup.bmRequestType = USB_TYPE_STANDARD | USB_RECIP_INTERFACE | USB_DIR_IN;
    setup.bRequest = USB_REQ_GET_DESCRIPTOR;
    /* The low byte is the Descriptor Index used to specify the set for Physical
       Descriptors, and is reset to zero for other HID class descriptors. */
    setup.wValue = (HID_DESCRIPTOR_TYPE_REPORT << 8) | 0;
    setup.wIndex = itfnum;
    setup.wLength = len;

    if (OS_SUCC != recv_usb_control_data(usb, &setup, buffer, len)) {
        return OS_FAIL;
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_u32 item_udata(struct hid_item *item)
{
    switch (item->size) {
    case 1: return item->data.u8d;
    case 2: return item->data.u16d;
    case 4: return item->data.u32d;
    }
    return 0;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_s32 item_sdata(struct hid_item *item)
{
    switch (item->size) {
    case 1: return item->data.s8d;
    case 2: return item->data.s16d;
    case 4: return item->data.s32d;
    }
    return 0;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret hid_parser_main(struct hid_report_info *report_info, struct hid_item *item)
{
    os_u32 ud;
    struct hid_report_info *info, *last;
    os_ret ret;

    ret = OS_SUCC;
    ud = item_udata(item); flog("main %d %d\n", item->tag, ud);
    switch (item->tag) {
    case HID_MAIN_ITEM_TAG_BEGIN_COLLECTION:
        report_info->usage_cnt = 0;
        switch (ud) {
        case HID_COLLECTION_PHYSICAL:
        case HID_COLLECTION_APPLICATION:
        case HID_COLLECTION_LOGICAL:
            hid_dbg(HID_INFO, "start collection %d\n", ud);
            break;
        }
        break;
    case HID_MAIN_ITEM_TAG_END_COLLECTION:
        hid_dbg(HID_INFO, "end collection\n");
        break;
    case HID_MAIN_ITEM_TAG_INPUT:
        info = kmalloc(sizeof(struct hid_report_info));
        if (OS_NULL == info) {
            hid_dbg(HID_INFO, "alloc hid report node fail\n");
            ret = OS_FAIL;
            goto end;
        }
        mem_set(info, 0, sizeof(struct hid_report_info));
        add_list_head(&report_info->node, &info->node);
        /* 6.2.2.7 Global Items
           Global items describe rather than define data from a control.
           A new Main item assumes the characteristics of the item state table.
           Global items can change the state table.
           As a result Global item tags apply to all subsequently defined items unless overridden by another Global item. */
        last = list_addr(info->node.prev, struct hid_report_info, node);
        info->usage_page = last->usage_page;
        info->logical_min = last->logical_min;
        info->logical_max = last->logical_max;
        info->report_id = last->report_id;
        info->report_size = last->report_size;
        info->report_count = last->report_count;
        hid_dbg(HID_ERROR, "<main input %d>\n", ud);
        break;
    case HID_MAIN_ITEM_TAG_OUTPUT:
        hid_dbg(HID_ERROR, "main output %d\n", ud);
        break;
    case HID_MAIN_ITEM_TAG_FEATURE:
        break;
    default:
        hid_dbg(HID_ERROR, "unknown main item tag 0x%x\n", item->tag);
    }
  end:
    return ret;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret hid_parser_global(struct hid_report_info *report_info, struct hid_item *item)
{
    os_u32 ud;

    ud = item_udata(item); flog("global %d %d\n", item->tag, ud);
    switch (item->tag) {
    case HID_GLOBAL_ITEM_TAG_USAGE_PAGE:
        report_info->usage_page = ud;
        switch (ud) {
        case HID_UP_GENDESK:
            hid_dbg(HID_INFO, "generic desktop\n");
            break;
        case HID_UP_SIMULATION:
        case HID_UP_KEYBOARD:
        case HID_UP_LED:
        case HID_UP_BUTTON:
            hid_dbg(HID_INFO, "usage button\n");
            break;
        case HID_UP_ORDINAL:
        case HID_UP_CONSUMER:
        case HID_UP_DIGITIZER:
        case HID_UP_PID:
        case HID_UP_HPVENDOR:
        case HID_UP_MSVENDOR: /* microsoft */
            hid_dbg(HID_INFO, "microsoft\n");
            break;
        case HID_UP_CUSTOM:
        case HID_UP_LOGIVENDOR:
            break;
        case HID_UP_UNDEFINED:
        default:
            break;
        }
        break;
    case HID_GLOBAL_ITEM_TAG_LOGICAL_MINIMUM:
        report_info->logical_min = ud;
        hid_dbg(HID_INFO, "logical min %d\n", ud);
        break;
    case HID_GLOBAL_ITEM_TAG_LOGICAL_MAXIMUM:
        report_info->logical_max = ud;
        hid_dbg(HID_INFO, "logical max %d\n", ud);
        break;
    case HID_GLOBAL_ITEM_TAG_PHYSICAL_MINIMUM:
    case HID_GLOBAL_ITEM_TAG_PHYSICAL_MAXIMUM:
    case HID_GLOBAL_ITEM_TAG_UNIT_EXPONENT:
    case HID_GLOBAL_ITEM_TAG_UNIT:
        break;
    case HID_GLOBAL_ITEM_TAG_REPORT_SIZE:
        report_info->report_size = ud;
        hid_dbg(HID_INFO, "report size %d\n", ud);
        break;
    case HID_GLOBAL_ITEM_TAG_REPORT_COUNT:
        report_info->report_count = ud;
        hid_dbg(HID_INFO, "report count %d\n", ud);
        break;
    case HID_GLOBAL_ITEM_TAG_REPORT_ID:
        report_info->report_id = ud;
        if (0 == report_info->report_id) {
            hid_dbg(HID_ERROR, "report id 0\n");
            return OS_FAIL;
        }
        hid_dbg(HID_INFO, "report id %d\n", ud);
        break;
    case HID_GLOBAL_ITEM_TAG_PUSH:
    case HID_GLOBAL_ITEM_TAG_POP:
        break;
    default:
        hid_dbg(HID_ERROR, "unknown global tag 0x%x\n", item->tag);
        return OS_FAIL;
        break;
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret hid_parser_local(struct hid_report_info *report_info, struct hid_item *item)
{
    os_u32 ud;

    ud = item_udata(item); flog("local %d %d\n", item->tag, ud);
    switch (item->tag) {
    case HID_LOCAL_ITEM_TAG_USAGE:
        if (LOCAL_USAGE_CNT <= report_info->usage_cnt) {
            hid_dbg(HID_ERROR, "usage cnt full\n");
            return OS_FAIL;
        }
        report_info->usage[report_info->usage_cnt++] = ud;
        hid_dbg(HID_INFO, "usage %d\n", ud);
        switch (ud) {
        case HID_GD_POINTER:
        case HID_GD_MOUSE:
            hid_dbg(HID_INFO, "usage pointer or mouse\n");
            break;
        case HID_GD_JOYSTICK:
        case HID_GD_GAMEPAD:
        case HID_GD_KEYBOARD:
        case HID_GD_KEYPAD:
        case HID_GD_MULTIAXIS:
            break;
        case HID_GD_X:
            hid_dbg(HID_INFO, "x\n");
            break;
        case HID_GD_Y:
            hid_dbg(HID_INFO, "y\n");
            break;
        case HID_GD_Z:
        case HID_GD_RX:
        case HID_GD_RY:
        case HID_GD_RZ:
        case HID_GD_SLIDER:
        case HID_GD_DIAL:
            break;
        case HID_GD_WHEEL:
            hid_dbg(HID_INFO, "wheel\n");
            break;
        case HID_GD_HATSWITCH:
        case HID_GD_BUFFER:
        case HID_GD_BYTECOUNT:
        case HID_GD_MOTION:
        case HID_GD_START:
        case HID_GD_SELECT:
        case HID_GD_VX:
        case HID_GD_VY:
        case HID_GD_VZ:
        case HID_GD_VBRX:
        case HID_GD_VBRY:
        case HID_GD_VBRZ:
        case HID_GD_VNO:
        case HID_GD_FEATURE:
        case HID_GD_UP:
        case HID_GD_DOWN:
        case HID_GD_RIGHT:
        case HID_GD_LEFT:
        default:
            break;
        }
        break;
    case HID_LOCAL_ITEM_TAG_DELIMITER:
        break;
    case HID_LOCAL_ITEM_TAG_USAGE_MINIMUM:
        report_info->usage_min = ud;
        hid_dbg(HID_INFO, "usage min %d\n", ud);
        break;
    case HID_LOCAL_ITEM_TAG_USAGE_MAXIMUM:
        report_info->usage_max = ud;
        hid_dbg(HID_INFO, "usage max %d\n", ud);
        break;
    default:
        hid_dbg(HID_ERROR, "unknown local item tag 0x%x\n", item->tag);
        break;
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret hid_parser_reserved(struct hid_report_info *report_info, struct hid_item *item)
{
    hid_dbg(HID_ERROR, "hid_parser_reserved\n");
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_u8 *fetch_item(os_u8 *start, os_u8 *end, struct hid_item *item)
{
    os_u8 b;

    if ((end - start) <= 0) {
        return OS_NULL;
    }

    b = *start++;
    item->type = (b >> 2) & 3;
    item->tag  = (b >> 4) & 15;
    if (item->tag == HID_ITEM_TAG_LONG) {
        item->format = HID_ITEM_FORMAT_LONG;

        if ((end - start) < 2) {
            return OS_NULL;
        }
        item->size = *start++;
        item->tag = *start++;
        if ((end - start) < item->size) {
            return OS_NULL;
        }
        item->data.longdata = start;
        start += item->size;
        return start;
    }

    item->format = HID_ITEM_FORMAT_SHORT;
    item->size = b & 3;
    switch (item->size) {
    case 0:
        return start;
    case 1:
        if ((end - start) < 1)
            return OS_NULL;
        item->data.u8d = *start++;
        return start;
    case 2:
        if ((end - start) < 2)
            return OS_NULL;
        item->data.u16d = *(os_u16 *) start; // get_unaligned_le16(start);
        start = (os_u8 *)((os_u16 *) start + 1);
        return start;
    case 3:
        item->size++;
        if ((end - start) < 4)
            return OS_NULL;
        item->data.u32d = *(os_u32 *) start; // get_unaligned_le32(start);
        start = (os_u8 *)((os_u32 *) start + 1);
        return start;
    }

    return OS_NULL;
}

/***************************************************************
 * description : One or more fields of data from controls are defined by a Main item and further described by the preceding Global and Local items.
 *               Local items only describe the data fields defined by the next Main item.
 *               Global items become the default attributes for all subsequent data fields in that descriptor.
 * history     :
 ***************************************************************/
LOCALC os_void parse_report_descriptor(struct hid_report_info *report_info, os_u8 *desc, os_u32 len)
{
    /* function is not void, so use function table */
    GLOBALDIF os_ret (*dispatch_type[])(struct hid_report_info *report_info, struct hid_item *item) = {
        hid_parser_main,
        hid_parser_global,
        hid_parser_local,
        hid_parser_reserved
    };

    struct hid_item item;
    os_u8 *start;
    os_u8 *end;
    struct hid_report_info *tmp;

    start = desc;
    end = start + len;
    while (OS_NULL != (start = fetch_item(start, end, &item))) {
        if (item.format != HID_ITEM_FORMAT_SHORT) {
            hid_dbg(HID_ERROR, "not short item\n");
            break;
        }
        tmp = list_addr(report_info->node.prev, struct hid_report_info, node);
        if (OS_SUCC != dispatch_type[item.type](tmp, &item)) {
            hid_dbg(HID_ERROR, "parse error.\n");
            break;
        }
        if (start == end) {
            hid_dbg(HID_INFO, "parse report descriptor succ\n");
            break;
        }
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC struct hid_report_info *prepare_report_descriptor(HDEVICE usb, struct usb_itf_info *intf_info)
{
    struct hid_device *dedicated;
    struct hid_descriptor *hid;
    struct hid_report_info *report_info;
    os_u16 report_desc_len;
    os_u8 *report_descriptor;
    os_ret result;
    os_u32 i;

    /* for cleanup */
    dedicated = OS_NULL;
    report_descriptor = OS_NULL;
    report_info = OS_NULL;

    dedicated = kmalloc(sizeof(struct hid_device));
    if (OS_NULL == dedicated) {
        goto cleanup;
    }
    set_usb_dedicated(usb, dedicated);

    /* The HID class uses the standard request Get_Descriptor as described in the USB
       Specification. When a Get_Descriptor(Configuration) request is issued, it
       returns the Configuration descriptor, all Interface descriptors, all Endpoint
       descriptors, and the HID descriptor for each interface. It shall not return the
       String descriptor, HID Report descriptor or any of the optional HID class
       descriptors. */
    hid = intf_info->hid;
    if (OS_NULL == hid) {
        hid_dbg(HID_ERROR, "hid probe, no hid descriptor\n");
        goto cleanup;
    }
    hid_dbg(HID_INFO, "hid num: %d\n", hid->bNumDescriptors);
    /* get length of report descriptor */
    report_desc_len = 0;
    for (i = 0; i < hid->bNumDescriptors; i++) {
        if (HID_DESCRIPTOR_TYPE_REPORT == hid->desc[i].bDescriptorType) {
            report_desc_len = hid->desc[i].wDescriptorLength;
            break; // there is only one report descriptor
        }
    }
    if (0 == report_desc_len) {
        hid_dbg(HID_ERROR, "no report descriptor info in hid descriptor\n");
        goto cleanup;
    }
    report_descriptor = kmalloc(report_desc_len);
    if (OS_NULL == report_descriptor) {
        hid_dbg(HID_ERROR, "alloc report descriptor fail\n");
        goto cleanup;
    }
    dedicated->report_descriptor = report_descriptor;

    result = get_report_descriptor(usb, intf_info->interface->bInterfaceNumber, report_descriptor, report_desc_len);
    if (OS_SUCC != result) {
        hid_dbg(HID_ERROR, "get report descriptor fail\n");
    }
    hid_dbg(HID_INFO, "report descriptor %x %d\n", report_descriptor, report_desc_len);

    report_info = kmalloc(sizeof(struct hid_report_info));
    if (OS_NULL == report_info) {
        hid_dbg(HID_ERROR, "alloc hid report info fail\n");
        goto cleanup;
    }
    mem_set(report_info, 0, sizeof(struct hid_report_info));
    init_list_head(&report_info->node);
    /* The bit length of an item’s data is obtained through the Report descriptor (Report Size * Report Count).
       Item data is ordered just as items are ordered in the Report descriptor.
       If a Report ID tag was used in the Report descriptor, all reports include a single byte ID prefix.
       If the Report ID tag was not used, all values are returned in a single report and a prefix ID is not included in that report. */
    report_info->report_id = 0;
    parse_report_descriptor(report_info, report_descriptor, report_desc_len);

    do {
        struct list_node *node;
        struct hid_report_info *t;
        flog("----------show report--------------------\n");
        loop_list(node, report_info->node.prev) {
            t = list_addr(node, struct hid_report_info, node);
            flog("usage page %x\n", t->usage_page);
            for (i = 0; i < t->usage_cnt; i++) {
                flog("usage %x\n", t->usage[i]);
            }
            flog("usage min %d\n", t->usage_min);
            flog("usage max %d\n", t->usage_max);
            flog("id %d\n", t->report_id);
            flog("size %d\n", t->report_size);
            flog("count %d\n", t->report_count);
            flog("min %d\n", t->logical_min);
            flog("max %d\n~~~~~~~\n", t->logical_max);
        }
    } while (0);

    return report_info;
  cleanup:
    if (report_info) { kfree(report_info); }
    if (report_descriptor) { kfree(report_descriptor); }
    if (dedicated) { kfree(dedicated); }
    return OS_NULL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret usb_hid_probe(HDEVICE usb)
{
    struct usb_itf_info *intf_info;
    struct hid_report_info *report_info;
    os_ret result;

    cassert(OS_NULL != usb);
    result = OS_FAIL;

    hid_dbg(HID_INFO, "usb hid probe.\n");

    intf_info = usb_interface_info(usb);
    if (OS_NULL == intf_info->interface) {
        hid_dbg(HID_ERROR, "hid probe, interface is null\n");
        return OS_FAIL;
    }
    hid_dbg(HID_INFO, "hid protocol %d %d %d\n", intf_info->interface->bInterfaceClass, intf_info->interface->bInterfaceSubClass, intf_info->interface->bInterfaceProtocol);
    switch (intf_info->interface->bInterfaceProtocol) {
    case 0: /* not boot device */
        break;
    case 1: /* keyboard */
        hid_dbg(HID_INFO, "hid kb\n");
        set_usb_dedicated(usb, OS_NULL);
        break;
    case 2: /* mouse */
        hid_dbg(HID_INFO, "hid mouse\n");
        report_info = prepare_report_descriptor(usb, intf_info);
        if (OS_NULL != report_info) {
            result = hid_mouse(usb, intf_info, report_info);
        }
        break;
    default: /* reserved */
        break;
    }

    return result;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void usb_hid_remove(HDEVICE usb)
{
    struct hid_device *dedicated;

    cassert(OS_NULL != usb);

    hid_dbg(HID_INFO, "remove usb hid\n");

    wmb();
    hid_mouse_status = HID_MOUSE_NO_EXIST;

    if (usb) {
        dedicated = get_usb_dedicated(usb);
        if (dedicated) {
            del_usb_int_pipe(usb, dedicated->pipe);
            if (dedicated->report_descriptor) {
                kfree(dedicated->report_descriptor);
            }
            if (dedicated->int_buffer) {
                kfree(dedicated->int_buffer);
            }
            kfree(dedicated);
        }
    }
}

LOCALD const struct usb_device_id usb_hid_id = {
    USB_ANY_ID, USB_ANY_ID, /* any vender, any product id. */
    USB_CLASS_HID,
    USB_ANY_ID,
    USB_ANY_ID /* 包括键盘, 鼠标 */
};

/* 通用usb设备驱动表 */
LOCALD const struct usb_driver usb_hid_driver = {
    "usb-hid",
    &usb_hid_id,
    usb_hid_probe,
    usb_hid_remove
};

/***************************************************************
 * description : usb hid
 * history     :
 ***************************************************************/
LOCALC os_void init_usb_hid_driver(os_void)
{
    os_ret result;

    hid_dbg(HID_INFO, "install hid driver\n");

    hid_mouse_status = HID_MOUSE_NO_EXIST;

    /* 注册的时序无法保证, 因此要求注册了即可使用设备 */
    result = register_usb_driver(&usb_hid_driver);
    cassert(OS_SUCC == result);
}
device_init_func(init_usb_hid_driver);

