/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : ohci.c
 * version     : 1.0
 * description : (key) host control and root hub
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include <vbs.h>
#include "usb.h"
#include "ohci.h"

/***************************************************************
 global variable declare
 ***************************************************************/

#define ohci_dbg(level, fmt, arg...) do { if (USB_INFO <= (level)) { flog(fmt"\n", ##arg); } } while (0)

/***************************************************************
 function declare
 ***************************************************************/

/***************************************************************
 * description : 16字节对齐
 * history     :
 ***************************************************************/
LOCALC struct ohci_td *alloc_td(os_void)
{
    struct ohci_td *td;

    td = cmalloc(sizeof(struct ohci_td), OHCI_TD_ALIGN);
    if (OS_NULL == td) {
        return OS_NULL;
    }

    /* init hardware info */
    td->hwBE = 0;
    td->hwCBP = 0;
    td->hwINFO = 0;
    td->hwNextTD = 0;

    td->complete = OS_NULL;
    td->app_complete = OS_NULL;
    td->td_type = USB_INVALID_TRANSFER;
    td->ed = OS_NULL;
    td->data = OS_NULL;
    td->data_len = 0;
    td->urb = OS_NULL;
    init_list_head(&td->urb_node);
    td->next_done_td = OS_NULL;

    return td;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void free_td(struct ohci_td *td, os_u32 line)
{
    cfree(td);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC struct ohci_urb *alloc_ohci_urb(os_void)
{
    struct ohci_urb *urb;

    urb = kmalloc(sizeof(struct ohci_urb));
    if (OS_NULL == urb) {
        ohci_dbg(USB_ERROR, "alloc ohci urb failed");
        return OS_NULL;
    }

    /* 初始化信号量 */
    urb->wait = create_event_handle(EVENT_INVALID, "ohci urb", __LINE__);
    if (OS_NULL == urb->wait) {
        kfree(urb);
        return OS_NULL;
    }
    /* 初始化头节点 */
    init_list_head(&urb->list);
    urb->td_count = 0;

    return urb;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void *free_ohci_urb(struct ohci_urb *urb)
{
    destroy_event_handle(urb->wait);
    kfree(urb);
    return OS_NULL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void unuse_ohci_ed(struct ohci_ed *ed)
{
    if (ed->ed_lock) {
        destroy_critical_section(ed->ed_lock);
        ed->ed_lock = OS_NULL;
    }
    ed->hwINFO = OHCI_ED_SKIP;
    ed->hwHeadP = ed->hwTailP = 0;
    ed->hwNextED = 0;

    ed->type = USB_INVALID_TRANSFER;
    ed->state = OHCI_ED_UNUSED;
}

/***************************************************************
 * description : 方向统一从td中获取
 *               生成的td_null会始终在td链表的尾部
 * history     :
 ***************************************************************/
LOCALC os_ret use_ohci_ed(struct usb_device *usb, os_u8 addr, os_u8 pipe, struct ohci_ed *ed, enum usb_direction direction, enum usb_transfer_type type)
{
    os_u32 MaxPacketSize;
    struct ohci_td *td;
    os_ret result;

    if (OHCI_ED_UNUSED != ed->state) {
        /* ed不需要修改 */
        return OS_SUCC;
    }

    ohci_dbg(USB_INFO, "use new ed, addr %d, pipe %d", addr, pipe);

    ed->proportion = usb_endpoint_proportion(usb, pipe, direction);

    MaxPacketSize = usb_endpoint_mps(usb, pipe, direction);
    ohci_dbg(USB_INFO, "MaxPacketSize %d", MaxPacketSize);

    result = create_critical_section(&ed->ed_lock, __LINE__);
    if (OS_SUCC != result) {
        ohci_dbg(USB_ERROR, "create ohci lock failed");
        goto error;
    }

    /* 设置ed的null td */
    td = alloc_td();
    if (OS_NULL == td) {
        ohci_dbg(USB_ERROR, "alloc td failed: %d", __LINE__);
        goto error;
    }
    ed->hwNextED = 0;
    ed->hwINFO = OHCI_ED_SKIP | (addr & ED_FA) | (pipe << 7) | (MaxPacketSize << 16) & (~ED_C); /* skip this ed */
    ed->hwTailP = ed->hwHeadP = virt_to_phys((pointer) td) & TD_ADDR_MASK; /* clear halt and toggle */
    ed->hwINFO &= ~OHCI_ED_SPEED; /* clear */
    /* set speed info */
    if (USB_LOW_SPEED == usb->speed) {
        ed->hwINFO |= OHCI_ED_SPEED;
    }

    /* ed不设置传输方向 */
#if 0
    switch (direction) {
    case USB_IN:
        ed->hwINFO |= OHCI_ED_IN;
        break;
    case USB_OUT:
        ed->hwINFO |= OHCI_ED_OUT;
        break;
    default:
        break; /* 从td中获取传输方向 */
    }
#endif

    ed->type = type;
    ed->state = OHCI_ED_USED;
    return OS_SUCC;
  error:
    unuse_ohci_ed(ed);
    return OS_FAIL;
}

/***************************************************************
 * description : 5.2.7.1.3 Pause
                 When the upper layers of software initiate a cancel of a request,
                 Host Controller Driver must set the HC_ENDPOINT_DESCRIPTOR.Control.sKip bit and then ensure that the Host Controller is not processing that endpoint.
                 After setting the bit, Host Controller Driver must wait for the next frame before the endpoint is paused.
 * history     :
 ***************************************************************/
LOCALC os_void ohci_pause_ed(struct ohci *hc, struct ohci_ed *ed, enum usb_transfer_type type)
{
    os_ret ret;
    struct ohci_regs *regs;
    os_u16 frame_no;

    regs = hc->regs;
    frame_no = ohci_in(&hc->hcca.frame_no);
    ed->hwINFO |= OHCI_ED_SKIP;

    switch (type) {
    case USB_BULK_TRANSFER:
        ohci_out(&hc->regs->control, ohci_in(&hc->regs->control) & ~OHCI_CTRL_BLE);
        break;
    case USB_CTRL_TRANSFER:
        ohci_out(&hc->regs->control, ohci_in(&hc->regs->control) & ~OHCI_CTRL_CLE);
        break;
    case USB_INTERRUPT_TRANSFER:
    case USB_ISO_TRANSFER:
    default:
        cassert(OS_FALSE);
        break;
    }

    do {
        /* enable sof */
        ohci_out(&regs->intrstatus, OHCI_INTR_SF);
        ohci_out(&regs->intrenable, OHCI_INTR_SF);
        ohci_in(&regs->intrdisable); /* PCI posting flush */

        /* clear the ED.Halt in ohci_interrupt_2() */
        /* wait for 2 frames. actually, it should be two ticks becasue of time boundary.
           A frame begins with a Start of Frame (SOF) token and is 1.0 ms ±0.25% in length. */
        ret = wait_event(hc->wait_sof, 2*(1000/OS_HZ)); /* mb inside */
        if (OS_SUCC != ret) {
            cassert(OS_FALSE);
        }

        /* disable sof */
        ohci_out(&hc->regs->intrdisable, OHCI_INTR_SF);
        ohci_in(&hc->regs->intrdisable); /* PCI posting flush */
    } while (ohci_in(&hc->hcca.frame_no) == frame_no);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void ohci_resume_ed(struct ohci_ed *ed)
{
    ed->hwINFO &= ~OHCI_ED_SKIP;
}

/***************************************************************
 * description : OHCI_COMPLETE
 * history     :
 ***************************************************************/
LOCALC os_bool ohci_td_complete(struct ohci_td *td, os_u32 no)
{
    struct ohci_urb *urb;

    if (OS_NULL != td) {
        /* 分片传输完成 */
        del_list(&td->urb_node);
        free_td(td, __LINE__);
        urb = td->urb;
        cassert(0 < urb->td_count);
        urb->td_count--;
        /* bugfix: add data underun handle */
        if ((OS_NULL != urb)
         && ((OS_TRUE == list_empty(&urb->list))
          || (TD_DATAUNDERRUN == no))) {
            /* 所有分片传输完毕, 释放信号量 */
            notify_event(urb->wait, __LINE__);
            return OS_TRUE;
        }
    }
    return OS_FALSE;
}

/* 需要考虑usb设备的处理时间, 例如u盘处理器读取的时间. */
#define OHCI_DEFAULT_PROPORTION 8
#define OHCI_WAIT_MS_GRANULARITY 0x100

#define MAX_TD_TRANS 4096

/***************************************************************
 * description : ed_num, 管道号(0 is control), token表示传输方向
 * history     :
 ***************************************************************/
LOCALC os_ret ohci_async_trans(struct ohci *hc, struct ohci_ed *ed, enum usb_transfer_type type, os_u32 token, os_u32 toggle, os_u8 *buffer, os_u32 len)
{
    struct list_node *i, *_save;
    struct ohci_urb *urb;
    struct ohci_ed *head;
    os_u32 td_cnt;
    struct ohci_td *td, *td_null;
    os_ret result;
    os_u32 proportion;
    os_u32 time;

    result = OS_FAIL;

    urb = alloc_ohci_urb();
    if (OS_NULL == urb) {
        ohci_dbg(USB_ERROR, "alloc urb failed");
        goto error;
    }

    /* 计算td数量 */
    td_cnt = 0;
    switch (type) {
    case USB_BULK_TRANSFER:
    case USB_CTRL_TRANSFER:
        /* one TD for every 4096 Byte */
        if ((0 != len) && (OS_NULL != buffer)) {
            td_cnt = (len - 1) / MAX_TD_TRANS + 1;
        } else {
            td_cnt = 1;
        }
        break;
    case USB_ISO_TRANSFER:
    case USB_INTERRUPT_TRANSFER:
    default:
        cassert(OS_FALSE);
        goto error;
        break;
    }
    urb->td_count = td_cnt;

    proportion = ed->proportion;
    if (0 == proportion) {
        /* proportion is not set, use the default value. */
        proportion = OHCI_DEFAULT_PROPORTION;
    }
    time = td_cnt * proportion * OHCI_WAIT_MS_GRANULARITY;

    td = (struct ohci_td *)(phys_to_virt(ed->hwTailP) & TD_ADDR_MASK);
    while (td_cnt) {
        td_cnt--;

        add_list_tail(&urb->list, &td->urb_node);
        td->urb = urb;

        /* 设置td完成函数 */
        td->complete = ohci_td_complete;
        /* 纪录传输类型 */
        td->td_type = type;

        td->hwINFO = token | TD_DI_SET(0) | toggle | TD_CC;
        if ((0 != len) && (OS_NULL != buffer)) {
            if (MAX_TD_TRANS < len) {
                td->hwCBP = virt_to_phys((pointer) buffer);
                td->hwBE = virt_to_phys((pointer) buffer) + MAX_TD_TRANS - 1;
                buffer += MAX_TD_TRANS;
                len -= MAX_TD_TRANS;
            } else {
                td->hwCBP = virt_to_phys((pointer) buffer);
                td->hwBE = virt_to_phys((pointer) buffer) + len - 1;
                td->hwINFO |= TD_R;
            }
        } else {
            td->hwCBP = 0;
            td->hwBE = 0;
        }

        /* add new td and save togglecarry */
        td_null = alloc_td();
        if (OS_NULL == td_null) {
            /* 无尾状态, 释放资源 */
            destroy_critical_section(&ed->ed_lock);
            unuse_ohci_ed(ed);
            ohci_dbg(USB_ERROR, "alloc td failed");
            goto error;
        }
        td->hwNextTD = virt_to_phys((pointer) td_null) & TD_ADDR_MASK;
        td = td_null;
    }
    wmb(); /* sync */
    /* If the bufferRounding bit in the General TD is not set,
       then this condition is treated as an error and the Host Controller sets the ConditionCode field to DATAUNDERRUN and the Halted bit of the ED is set as the General TD is retired. */
    ed->hwHeadP = ed->hwHeadP & (~ED_H);
    ed->hwTailP = virt_to_phys((pointer) td_null) & TD_ADDR_MASK;
    ed->hwINFO &= ~OHCI_ED_SKIP;

    do { /* one operation */
        os_u32 *reg_ed_async_head;
        os_u32 cmdstatus_bits;
        os_u32 control_bits;
        lock_t eflag;

        /* 通知硬件新的ctrl队列生效 */
        switch (type) {
        case USB_BULK_TRANSFER:
            reg_ed_async_head = &hc->regs->ed_bulkhead;
            cmdstatus_bits = OHCI_BLF;
            control_bits = OHCI_CTRL_BLE;
            break;
        case USB_CTRL_TRANSFER:
            reg_ed_async_head = &hc->regs->ed_controlhead;
            cmdstatus_bits = OHCI_CLF;
            control_bits = OHCI_CTRL_CLE;
            break;
        case USB_INTERRUPT_TRANSFER:
        case USB_ISO_TRANSFER:
        default:
            cassert(OS_FALSE);
            break;
        }
        lock_int(eflag);
        if (OHCI_ED_LINKED != ed->state) {
            ed->state = OHCI_ED_LINKED;
            spin_lock(&hc->ohci_lock);
            if (ohci_in(reg_ed_async_head)) {
                head = (struct ohci_ed *) phys_to_virt(ohci_in(reg_ed_async_head));
                while (head->hwNextED) {
                    head = (struct ohci_ed *) phys_to_virt(head->hwNextED);
                }
                head->hwNextED = virt_to_phys((os_u32) ed);
                /* get head again */
                head = (struct ohci_ed *) phys_to_virt(ohci_in(reg_ed_async_head));
                add_list_tail(&head->ed_node, &ed->ed_node);
            } else {
                ohci_out(reg_ed_async_head, virt_to_phys((pointer) ed));
                init_list_head(&ed->ed_node);
            }
            spin_unlock(&hc->ohci_lock);
        }
        wmb();
        ohci_out(&hc->regs->cmdstatus, cmdstatus_bits | ohci_in(&hc->regs->cmdstatus));
        ohci_out(&hc->regs->control, control_bits | ohci_in(&hc->regs->control));

        result = wait_event(urb->wait, time); /* mb operation inside */
        if ((OS_SUCC == result) && (0 == urb->td_count)) {
            free_ohci_urb(urb);
        } else {
            /* pause ed */
            ohci_pause_ed(hc, ed, type);
            /* del all tds */
            loop_del_list(i, _save, &urb->list) {
                td = list_addr(i, struct ohci_td, urb_node);
                del_list(i);
                /* remove hardware */
                if ((ed->hwHeadP & TD_ADDR_MASK) == virt_to_phys((pointer) td)) {
                    /* If the bufferRounding bit in the General TD is not set,
                       then this condition is treated as an error and the Host Controller sets the ConditionCode field to DATAUNDERRUN and the Halted bit of the ED is set as the General TD is retired. */
                    /* clear ED.Halt because Halt is in hwHeadP. */
                    ed->hwHeadP = (td->hwNextTD & TD_ADDR_MASK) | (ed->hwHeadP & ED_C);
                } else {
                    list_addr(td->urb_node.prev, struct ohci_td, urb_node)->hwNextTD = virt_to_phys((pointer) list_addr(td->urb_node.next, struct ohci_td, urb_node)) & TD_ADDR_MASK;
                }
                del_list(&td->urb_node);
                free_td(td, __LINE__);
                urb->td_count--;
            }
            cassert(0 == urb->td_count);
            free_ohci_urb(urb);
        }
        /* resume ed */
        ohci_resume_ed(ed);
        unlock_int(eflag);
    } while (0);

    return result;
  error:
    if (urb) {
        loop_del_list(i, _save, &urb->list) {
            td = list_addr(i, struct ohci_td, urb_node);
            del_list(&td->urb_node);
            free_td(td, __LINE__);
        }
        free_ohci_urb(urb);
    }
    return result;
}

/***************************************************************
 * description : usb_20.pdf 8.5.3 control transfers
 *               addr: usb设备地址
 *               ednum: usb设备端点号
 *               input:
 *               output:
 * history     :
 ***************************************************************/
LOCALC os_ret ohci_recv_control_transfer(struct usb_device *usb, os_u32 addr, os_u8 ed_num, struct usb_setup_data *cmd, os_void *buffer, os_uint len)
{
    struct ohci *hc;
    struct ohci_ed *ed;
    os_ret result;

    cassert((OS_NULL != usb) && (EP_NUM > ed_num));

    if (OS_NULL != usb->host_controller) {
        hc = usb->host_controller;
        ed = &hc->ed[addr][ed_num][USB_IN];
        result = OS_SUCC;
        /* 初始化ed, 传输方向从td中获取, 此时要使用指定的usb设备地址 */
        use_ohci_ed(usb, addr, ed_num, ed, USB_IN, USB_CTRL_TRANSFER);
        enter_critical_section(ed->ed_lock);
        /* setup stage */
        if (OS_SUCC == ohci_async_trans(hc, ed, USB_CTRL_TRANSFER, TD_DP_SETUP, TD_T_DATA0, (os_u8 *) cmd, sizeof(struct usb_setup_data))) {
            if (0 != len) {
                /* data stage */
                if (OS_SUCC != ohci_async_trans(hc, ed, USB_CTRL_TRANSFER, TD_DP_IN, TD_T_DATA1, buffer, len)) {
                    ohci_dbg(USB_ERROR, "ohci recv ctrl msg failed");
                    result = OS_FAIL;
                    goto end;
                }
            }
            /* status stage */
            if (OS_SUCC != ohci_async_trans(hc, ed, USB_CTRL_TRANSFER, TD_DP_OUT, TD_T_DATA1, OS_NULL, 0)) {
                result = OS_FAIL;
            }
        }
      end:
        leave_critical_section(ed->ed_lock);
        return result;
    }
    ohci_dbg(USB_ERROR, "ohci recv ctrl msg failed");
    return OS_FAIL;
}

/***************************************************************
 * description : usb_20.pdf 8.5.3 control transfers
 * history     :
 ***************************************************************/
LOCALC os_ret ohci_send_control_transfer(struct usb_device *usb, os_u32 addr, os_u8 ed_num, struct usb_setup_data *cmd, os_void *buffer, os_uint len)
{
    struct ohci *hc;
    struct ohci_ed *ed;
    os_ret result;

    cassert((OS_NULL != usb) && (EP_NUM > ed_num));

    if (OS_NULL != usb->host_controller) {
        hc = usb->host_controller;
        ed = &hc->ed[addr][ed_num][USB_OUT];
        result = OS_SUCC;
        /* 初始化ed, 传输方向从td中获取, 此时要使用指定的usb设备地址 */
        use_ohci_ed(usb, addr, ed_num, ed, USB_OUT, USB_CTRL_TRANSFER);
        enter_critical_section(ed->ed_lock);
        /* setup stage */
        if (OS_SUCC == ohci_async_trans(hc, ed, USB_CTRL_TRANSFER, TD_DP_SETUP, TD_T_DATA0, (os_u8 *)cmd, sizeof(struct usb_setup_data))) {
            if (0 != len) {
                /* data stage */
                if (OS_SUCC != ohci_async_trans(hc, ed, USB_CTRL_TRANSFER, TD_DP_OUT, TD_T_DATA1, buffer, len)) {
                    ohci_dbg(USB_ERROR, "ohci send ctrl msg failed");
                    result = OS_FAIL;
                    goto end;
                }
            }
            /* status stage */
            if (OS_SUCC != ohci_async_trans(hc, ed, USB_CTRL_TRANSFER, TD_DP_IN, TD_T_DATA1, OS_NULL, 0)) {
                result = OS_FAIL;
            }
        }
      end:
        leave_critical_section(ed->ed_lock);
        return result;
    }
    ohci_dbg(USB_ERROR, "ohci send ctrl msg failed");
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret ohci_recv_bulk_transfer(struct usb_device *usb, os_u8 pipe, os_u8 *buffer, os_uint len)
{
    struct ohci *hc;
    struct ohci_ed *ed;
    os_ret result;

    cassert((OS_NULL != usb) && (EP_NUM > pipe));

    if (OS_NULL != usb->host_controller) {
        hc = usb->host_controller;
        ed = &hc->ed[usb->usb_addr][pipe][USB_IN];
        result = OS_SUCC;
        /* 初始化ed, 传输方向从td中获取, 此时要使用指定的usb设备地址 */
        use_ohci_ed(usb, usb->usb_addr, pipe, ed, USB_IN, USB_BULK_TRANSFER);
        enter_critical_section(ed->ed_lock);
        if (OS_SUCC != ohci_async_trans(hc, ed, USB_BULK_TRANSFER, TD_DP_IN, TD_T_TOGGLE, buffer, len)) {
            result = OS_FAIL;
            ohci_dbg(USB_ERROR, "ohci device 0x%x %d receive bulk msg on pipe %d failed", usb, usb->usb_addr, pipe);
        }
        leave_critical_section(ed->ed_lock);
        return result;
    }
    ohci_dbg(USB_ERROR, "ohci device 0x%x %d receive bulk msg on pipe %d failed", usb, usb->usb_addr, pipe);
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret ohci_send_bulk_transfer(struct usb_device *usb, os_u8 pipe, os_u8 *buffer, os_uint len)
{
    struct ohci *hc;
    struct ohci_ed *ed;
    os_ret result;

    cassert((OS_NULL != usb) && (EP_NUM > pipe));

    if (OS_NULL != usb->host_controller) {
        hc = usb->host_controller;
        ed = &hc->ed[usb->usb_addr][pipe][USB_OUT];
        result = OS_SUCC;
        /* 初始化ed, 传输方向从td中获取, 此时要使用指定的usb设备地址 */
        use_ohci_ed(usb, usb->usb_addr, pipe, ed, USB_OUT, USB_BULK_TRANSFER);
        enter_critical_section(ed->ed_lock);
        if (OS_SUCC != ohci_async_trans(hc, ed, USB_BULK_TRANSFER, TD_DP_OUT, TD_T_TOGGLE, buffer, len)) {
            result = OS_FAIL;
            ohci_dbg(USB_ERROR, "ohci device 0x%x %d send bulk msg on pipe %d failed", usb, usb->usb_addr, pipe);
        }
        leave_critical_section(ed->ed_lock);
        return result;
    }
    ohci_dbg(USB_ERROR, "ohci device 0x%x %d send bulk msg on pipe %d failed", usb, usb->usb_addr, pipe);
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void reconnect_interrupt_td(struct ohci_td *td)
{
    struct ohci_ed *ed;
    struct ohci_td *td_null;

    ed = td->ed;
    cassert(OS_NULL != ed);

    td_null = (struct ohci_td *)(phys_to_virt(ed->hwTailP) & TD_ADDR_MASK);
    cassert(OS_NULL != td_null);

    /* get toggle from ed, initial toggle is data0 */
    td_null->hwINFO = (TD_R | TD_DP_IN | TD_T_TOGGLE | TD_DI_SET(0) | TD_CC);
    td_null->hwCBP = virt_to_phys((pointer) td->data);
    td_null->hwBE = virt_to_phys((pointer) td->data) + td->data_len - 1;
    td_null->data = td->data;
    td_null->data_len = td->data_len;
    td_null->hwNextTD = virt_to_phys((pointer) td) & TD_ADDR_MASK;
    td_null->urb = OS_NULL; /* 中断传输, 不需要等待 */
    td_null->complete = td->complete;
    td_null->app_complete = td->app_complete;
    td_null->td_type = USB_INTERRUPT_TRANSFER;
    td_null->ed = ed;
    wmb();
    ed->hwTailP = virt_to_phys((pointer) td);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_bool ohci_int_td_complete(struct ohci_td *td, os_u32 no)
{
    if (USB_INTERRUPT_TRANSFER == td->td_type) {
        if (td->app_complete) {
            td->app_complete(td->data);
        }
        /* 重新挂接td */
        reconnect_interrupt_td(td);
    }
}

/***************************************************************
 * description : 插入控制器的中断调度队列
 * history     :
 ***************************************************************/
LOCALC os_void add_int_ed_list(struct ohci *hc, struct ohci_ed *new, os_u8 bInterval)
{
    struct ohci_ed *ed, *select;
    os_uint i, j;

    select = OS_NULL;

    ed = hc->int_ed;

    for (i = 0; i < STATIC_INT_ED_COUNT; i++) {
        if (bInterval >= ed[i].int_interval) {
            select = &ed[i];
            /* 负载均衡 */
            for (j = i + 1; j < STATIC_INT_ED_COUNT; j++) {
                if (ed[i].int_interval == ed[j].int_interval) {
                    if (ed[j].count < select->count) {
                        select = &ed[j];
                    }
                } else {
                    break;
                }
            }
            break;
        }
    }
    cassert(OS_NULL != select);
    select->count++;
    new->hwNextED = select->hwNextED;
    wmb();
    select->hwNextED = virt_to_phys((pointer) new);
    new->int_interval = select->int_interval;
    add_list_tail(&select->ed_node, &new->ed_node);
}

/***************************************************************
 * description : 删除控制器的中断调度队列
 * history     :
 ***************************************************************/
LOCALC os_void del_int_ed_list(struct ohci *hc, struct ohci_ed *ed)
{
    os_uint i;
    struct ohci_ed *prev;

    prev = list_addr(ed->ed_node.prev, struct ohci_ed, ed_node);
    prev->hwNextED = ed->hwNextED;
    del_list(&ed->ed_node);
    for (i = 0; i < STATIC_INT_ED_COUNT; i++) {
        if (&hc->int_ed[i] == ed) {
            cassert(0 != hc->int_ed[i].count);
            hc->int_ed[i].count--;
        }
    }
}

/***************************************************************
 * description : 添加ed到int table中, 新分配一个td用于中断传输
 * history     :
 ***************************************************************/
LOCALC os_ret ohci_add_interrupt_pipe(struct usb_device *usb, os_u8 pipe, os_u8 *data, os_uint len, os_u8 bInterval, os_void (*complete)(os_u8 *data))
{
    struct ohci *hc;
    struct ohci_ed *ed;
    struct ohci_td *td, *td_null;

    cassert(OS_NULL != usb);
    hc = usb->host_controller;
    cassert(OS_NULL != hc);
    ed = &hc->ed[usb->usb_addr][pipe][USB_IN]; /* 中断传输只支持in类型 */

    /* 初始化ed信息 */
    use_ohci_ed(usb, usb->usb_addr, pipe, ed, USB_IN, USB_INTERRUPT_TRANSFER);

    /* 新的尾节点 */
    td_null = alloc_td();
    if (OS_NULL == td_null) {
        ohci_dbg(USB_ERROR, "allocate interrupt td null failed");
        return OS_FAIL;
    }

    td = (struct ohci_td *)(phys_to_virt(ed->hwTailP) & TD_ADDR_MASK);

    /* get toggle from ed, initial toggle is data0 */
    td->hwINFO = (TD_R | TD_DP_IN | TD_T_TOGGLE | TD_DI_SET(0) | TD_CC);
    if ((0 != len) && (OS_NULL != data)) {
        td->hwCBP = virt_to_phys((pointer) data);
        td->hwBE = virt_to_phys((pointer) data) + len - 1;
    } else {
        td->hwCBP = 0;
        td->hwBE = 0;
    }
    td->hwNextTD = virt_to_phys((pointer) td_null) & TD_ADDR_MASK;
    td->urb = OS_NULL; /* 中断传输, 不需要等待 */
    td->complete = ohci_int_td_complete;
    td->app_complete = complete;
    td->td_type = USB_INTERRUPT_TRANSFER;
    td->data = data;
    cassert(MAX_TD_TRANS >= len);
    td->data_len = len;
    td->ed = ed;

    wmb();
    ed->hwTailP = virt_to_phys((pointer) td_null);
    ed->hwINFO &= ~OHCI_ED_SKIP;

    if (ed->state != OHCI_ED_LINKED) {
        ed->state = OHCI_ED_LINKED;
        add_int_ed_list(hc, ed, bInterval);
    }

    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret ohci_del_interrupt_pipe(struct usb_device *usb, os_u8 pipe)
{
    struct ohci *hc;
    struct ohci_ed *ed;
    struct ohci_td *td, *td_null;

    cassert(OS_NULL != usb);
    hc = usb->host_controller;
    cassert(OS_NULL != hc);
    ed = &hc->ed[usb->usb_addr][pipe][USB_IN]; /* 中断传输只支持in类型 */

    ed->hwHeadP = ed->hwTailP | (ed->hwHeadP & ED_C);
    del_int_ed_list(hc, ed);

    /* ohci cannot handles TD accrossing 1 frame */
    ed->hwINFO = OHCI_ED_SKIP;
    wmb(); // delay cannot guarantee registers are written.
    delay_frame();

    /* delete td */
    td = (struct ohci_td *) phys_to_virt((pointer) ed->hwHeadP & TD_ADDR_MASK);
    td_null = (struct ohci_td *) phys_to_virt((pointer) ed->hwTailP & TD_ADDR_MASK);
    while (td != td_null) {
        free_td(td, __LINE__);
        td = (struct ohci_td *) phys_to_virt((pointer) td->hwNextTD  & TD_ADDR_MASK);
    }
    unuse_ohci_ed(ed);
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_u32 alloc_ohci_dev_addr(struct usb_device *usb)
{
    struct ohci *hc;

    cassert(OS_NULL != usb);
    hc = usb->host_controller;
    cassert(OS_NULL != hc);
    return alloc_usb_addr(hc->addr_bitmap);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret free_ohci_dev_addr(struct usb_device *usb)
{
    struct ohci *hc;

    cassert(OS_NULL != usb);
    hc = usb->host_controller;
    cassert(OS_NULL != hc);
    free_usb_addr(hc->addr_bitmap, usb->usb_addr);
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret create_ohci_device_endpoint(struct usb_device *usb)
{
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret clear_ohci_device_endpoint(struct usb_device *usb)
{
    struct ohci *hc;
    os_uint i, j;
    os_u32 reg;
    struct ohci_ed *ed, *h;

    cassert(OS_NULL != usb);
    hc = usb->host_controller;
    cassert(OS_NULL != hc);

    for (i = 0; i < EP_NUM; i++) {
        for (j = 0; j < USB_DIR_CNT; j++) {
            ed = &hc->ed[usb->usb_addr][i][j];
            if (OHCI_ED_UNUSED != ed->state) {
                ed->hwINFO = OHCI_ED_SKIP;
                switch (ed->type) {
                case USB_CTRL_TRANSFER:
                    if ((pointer) ed == phys_to_virt(ohci_in(&hc->regs->ed_controlhead))) {
                        ohci_out(&hc->regs->ed_controlhead, (pointer) ed->hwNextED);
                    } else {
                        h = list_addr(ed->ed_node.prev, struct ohci_ed, ed_node);
                        h->hwNextED = ed->hwNextED;
                    }
                    del_list(&ed->ed_node);
                    break;
                case USB_BULK_TRANSFER:
                    if ((pointer) ed == phys_to_virt(ohci_in(&hc->regs->ed_bulkhead))) {
                        ohci_out(&hc->regs->ed_bulkhead, (pointer) ed->hwNextED);
                    } else {
                        h = list_addr(ed->ed_node.prev, struct ohci_ed, ed_node);
                        h->hwNextED = ed->hwNextED;
                    }
                    del_list(&ed->ed_node);
                    break;
                case USB_INTERRUPT_TRANSFER:
                    del_int_ed_list(hc, ed);
                    break;
                case USB_ISO_TRANSFER:
                case USB_INVALID_TRANSFER:
                    cassert(OS_FALSE);
                    break;
                }
            }
        }
    }

    /* flush async endpoint */
    mb();
    reg = ohci_in(&hc->regs->control);
    ohci_out(&hc->regs->control, reg & ~(OHCI_CTRL_BLE | OHCI_CTRL_CLE));
    mb(); // delay cannot guarantee registers are written.
    delay_frame(); // wait for 1 frame
    ohci_out(&hc->regs->ed_controlcurrent, 0); // delete multi-eds, refer to 6.4.2.2, If the “CurrentED” register contains a value of ‘0,’ the Host Controller has reached the end of the list.
    ohci_out(&hc->regs->ed_bulkcurrent, 0); // delete multi-eds, refer to 6.4.2.2, If the “CurrentED” register contains a value of ‘0,’ the Host Controller has reached the end of the list.
    mb();
    ohci_out(&hc->regs->control, reg);

    /* clear ed and td, td_null */
    for (i = 0; i < EP_NUM; i++) {
        for (j = 0; j < USB_DIR_CNT; j++) {
            ed = &hc->ed[usb->usb_addr][i][j];
            if (OHCI_ED_UNUSED != ed->state) {
                struct ohci_td *h, *t;

                h = (struct ohci_td *)(phys_to_virt(ed->hwHeadP) & TD_ADDR_MASK);
                while ((pointer) h != phys_to_virt(ed->hwTailP)) {
                    t = h;
                    h = (struct ohci_td *)(phys_to_virt(h->hwNextTD) & TD_ADDR_MASK);
                    del_list(&t->urb_node);
                    if (t->urb->td_count && (0 == --t->urb->td_count)) {
                        free_ohci_urb(t->urb);
                    }
                    free_td(t, __LINE__);
                }
                /* td null */
                cassert(0 == phys_to_virt(h->hwNextTD));
                free_td(h, __LINE__);
                unuse_ohci_ed(ed);
            }
        }
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC inline os_void init_device_ed(struct ohci *hc, os_u8 addr)
{
    os_uint i, j;
    struct ohci_ed *ed;

    for (i = 0; i < EP_NUM; i++) {
        for (j = 0; j < USB_DIR_CNT; j++) {
            ed = &hc->ed[addr][i][j];

            ed->hwINFO = (DEFAULT_MAX_PACKET_SIZE << 16) | OHCI_ED_SPEED | OHCI_ED_SKIP;
            ed->hwHeadP = ed->hwTailP = 0;
            ed->hwNextED = 0;

            ed->type = USB_INVALID_TRANSFER;
            ed->state = OHCI_ED_UNUSED;
            ed->ed_lock = OS_NULL;
            init_list_head(&ed->ed_node);
            ed->int_interval = 0;
            ed->count = 0;
            ed->proportion = 0;
        }
    }
}

/***************************************************************
 * description : delete td only
 * history     :
 ***************************************************************/
LOCALC inline os_void clear_device_td(struct ohci *hc, os_u32 addr)
{
    struct ohci_ed *ed;
    struct ohci_td *td;
    os_uint i, j;

    for (i = 0; i < EP_NUM; i++) {
        for (j = 0; j < USB_DIR_CNT; j++) {
            ed = &hc->ed[addr][i][j];
            if (OHCI_ED_UNUSED != ed->state) {
                td = (struct ohci_td *)(phys_to_virt(ed->hwHeadP) & TD_ADDR_MASK);
                while ((pointer) td != phys_to_virt(ed->hwTailP)) {
                    if (td->urb->td_count && (0 == --td->urb->td_count)) {
                        free_ohci_urb(td->urb);
                    }
                    free_td(td, __LINE__);
                }
            }
        }
    }
}

/***************************************************************
 * description : addr 0
 * history     :
 ***************************************************************/
LOCALC os_ret reset_ohci_default_endpoint(struct usb_device *usb)
{
    struct ohci *hc;

    cassert(OS_NULL != usb);
    hc = usb->host_controller;
    cassert(OS_NULL != hc);
    ohci_pause_ed(hc, &hc->ed[0][0][USB_IN], USB_CTRL_TRANSFER);
    ohci_pause_ed(hc, &hc->ed[0][0][USB_OUT], USB_CTRL_TRANSFER);
    clear_device_td(usb->host_controller, 0);
    ohci_resume_ed(&hc->ed[0][0][USB_IN]);
    ohci_resume_ed(&hc->ed[0][0][USB_OUT]);

    init_device_ed(usb->host_controller, 0);
    return OS_SUCC;
}

/* ohci的操作集合 */
LOCALD const struct usb_host_controller_operations ohci_operation = {
    ohci_recv_control_transfer,
    ohci_send_control_transfer,
    ohci_recv_bulk_transfer,
    ohci_send_bulk_transfer,
    ohci_add_interrupt_pipe,
    ohci_del_interrupt_pipe,

    alloc_ohci_dev_addr,
    free_ohci_dev_addr,

    create_ohci_device_endpoint,
    clear_ohci_device_endpoint,

    reset_ohci_default_endpoint
};

/***************************************************************
 * description : 256字节内存对齐
 * history     :
 ***************************************************************/
LOCALC struct ohci *alloc_ohci(os_void)
{
    struct ohci *hc;

#define OHCI_ALIGN 0x100 /* ohci.hcca, 256字节对齐 */
    hc = cmalloc(sizeof(struct ohci), OHCI_ALIGN);
    if (OS_NULL == hc) {
        return OS_NULL;
    }
    mem_set(hc, 0, sizeof(struct ohci)); /* 此处需要清空内存 */
    hc->regs = OS_NULL;
    hc->interrupt = OS_FALSE;
    hc->sof = OS_FALSE;

    /* 初始化信号量 */
    hc->wait_sof = create_event_handle(EVENT_INVALID, "ohci sof", __LINE__);
    cassert(OS_NULL != hc->wait_sof);

    init_spinlock(&hc->ohci_lock);

    hc->done = 0;

    mem_set(hc->rh_dev, OS_NULL, OHCI_RH_PORTS_CNT * sizeof(struct usb_device *));

    return hc;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void free_ohci(struct ohci *hc)
{
    destroy_event_handle(hc->wait_sof);
    cfree(hc);
}

/***************************************************************
 * description : hcir1_0a.pdf table a-4: BAR_OHCI register
 * history     :
 ***************************************************************/
LOCALC os_void save_ohci_regs_addr(struct ohci *usb, os_u32 addr)
{
    usb->regs = (struct ohci_regs *)(addr & 0xfffff000);
}

/***************************************************************
 * description : 获取NDP, NumberDownstreamPorts
 * history     :
 ***************************************************************/
LOCALC inline os_u32 get_ohci_ndp(struct ohci *usb)
{
    return ohci_in(&usb->regs->roothub.a) & RH_A_NDP;
}

/***************************************************************
 * description : 重启hc和bus
 * history     :
 ***************************************************************/
LOCALC os_ret reset_ohci(struct ohci *hc)
{
    struct ohci_regs *regs;
    os_uint timeout;

    regs = hc->regs;

    /* smm owns the hc */
    if (ohci_in(&regs->control) & OHCI_CTRL_IR) {
        ohci_out(&regs->cmdstatus, OHCI_OCR);
        timeout = 50;
        while (ohci_in(&regs->control) & OHCI_CTRL_IR) {
            delay_ms(10);
            if (1 > timeout--) {
                ohci_dbg(USB_ERROR, "usb hc takeover failed");
                return OS_FAIL;
            }
        }
    }

    /* disable hc interrupt */
    ohci_out(&regs->intrdisable, OHCI_INTR_MIE);

    /* reset usb */
    ohci_out(&regs->control, 0);

    timeout = 10;
    ohci_out(&regs->cmdstatus, OHCI_HCR);
    while (0 != (ohci_in(&regs->cmdstatus) & OHCI_HCR)) {
        delay_ms(10);
        if (1 > timeout--) {
            ohci_dbg(USB_ERROR, "usb hc reset timeout");
            return OS_FAIL;
        }
    }
    return OS_SUCC;
}

/***************************************************************
 * description : 中断传输的方向总是从设备到主机
 * history     :
 ***************************************************************/
LOCALC os_void init_interrupt_list(struct ohci *hc)
{
    LOCALD const os_u8 alloc_table[INT_ED_TABLE_CNT] = {
        0x00, 0x10, 0x08, 0x18, 0x04, 0x14, 0x0c, 0x1c,
        0x02, 0x12, 0x0a, 0x1a, 0x06, 0x16, 0x0e, 0x1e,
        0x01, 0x11, 0x09, 0x19, 0x05, 0x15, 0x0d, 0x1d,
        0x03, 0x13, 0x0b, 0x1b, 0x07, 0x17, 0x0f, 0x1f
    };
    os_u32 interval;
    struct ohci_ed *int_ed;
    os_uint i, start;

    /* 初始化中断调度列表 */
    int_ed = hc->int_ed;
    start = 0;
    interval = INT_ED_TABLE_CNT;
    for (i = 0; i < STATIC_INT_ED_COUNT; i++) {
        if ((i - start) >= interval) {
            interval /= 2;
            start = i;
        }
        int_ed[i].int_interval = interval;
        int_ed[i].hwNextED = virt_to_phys((pointer) &int_ed[((i - start) / 2) + start + interval]); /* 最后一个读越界 */
        int_ed[i].hwHeadP = int_ed[i].hwTailP = 0;
        int_ed[i].hwINFO = (DEFAULT_MAX_PACKET_SIZE << 16) | OHCI_ED_SKIP | OHCI_ED_SPEED;
        int_ed[i].count = 0;
        init_list_head(&int_ed[i].ed_node);
    }
    int_ed[STATIC_INT_ED_COUNT - 1].hwNextED = 0;

    /* 初始化硬件寄存器 */
    for (i = 0; i < INT_ED_TABLE_CNT; i++) {
        hc->hcca.int_table[i] = virt_to_phys((pointer) &int_ed[alloc_table[i]]);
    }
}

/***************************************************************
 * description : 启动ohci, 设置总线可操作, 开中断, 连接hub
 * history     :
 ***************************************************************/
LOCALC os_void init_ohci(struct ohci *hc)
{
    struct ohci_regs *regs;
    os_u32 fminterval = 0x2edf;
    os_u32 mask;
    os_uint i;

    regs = hc->regs;

    /* 清空control和bulk的endpoint descriptor列表 */
    ohci_out(&regs->ed_controlhead, 0);
    ohci_out(&regs->ed_controlcurrent, 0);
    ohci_out(&regs->ed_bulkhead, 0);
    ohci_out(&regs->ed_bulkcurrent, 0);

    /* 初始化设备ed */
    for (i = 0; i < USB_DEVICE_COUNT; i++) {
        init_device_ed(hc, i);
    }

    init_interrupt_list(hc);
    // init_iso_list();

    /* 初始化hcca指针 */
    ohci_out(&regs->hcca, virt_to_phys((pointer) &hc->hcca));

    ohci_out(&regs->periodicstart, (fminterval * 9) / 10);
    fminterval |= ((((fminterval - 210) * 6) / 7) << 16);
    ohci_out(&regs->fminterval, fminterval);
    ohci_out(&regs->lsthresh, 0x628);

    /* InterruptRouting
       This bit determines the routing of interrupts generated by events registered in HcInterruptStatus.
       If clear, all interrupts are routed to the normal host bus interrupt mechanism.
       If set, interrupts are routed to the System Management Interrupt.
       HCD clears this bit upon a hardware reset, but it does not alter this bit upon a software reset.
       HCD uses this bit as a tag to indicate the ownership of HC. */
    wmb();
/* for initializing controller (mask in an HCFS mode too) */
#define OHCI_CONTROL_INIT ((OHCI_CTRL_CBSR & 0x3) | OHCI_CTRL_IE | OHCI_CTRL_PLE)
    ohci_out(&regs->control, OHCI_CONTROL_INIT | OHCI_USB_OPER); /* 设置控制器为oper, 才可以进行后续的操作. */

    /* choose some interrupts */
    mask = OHCI_INTR_MIE | OHCI_INTR_UE | OHCI_INTR_WDH | OHCI_INTR_RHSC | OHCI_INTR_SO;
    ohci_out(&regs->intrenable, mask);
    ohci_out(&regs->intrstatus, mask);

    /* root hub, set global power */
    wmb();
    ohci_out(&regs->roothub.a, (ohci_in(&regs->roothub.a) | RH_A_NPS) & ~RH_A_PSM);
    ohci_out(&regs->roothub.status, RH_HS_LPSC); /* turn on power */
    delay_ms((ohci_in(&regs->roothub.a) >> 23) & 0x1fe); /* ms */
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret ohci_probe(HDEVICE pci)
{
    struct ohci *hc;
    os_ret result;

    cassert(OS_NULL != pci);

    ohci_dbg(USB_INFO, "ohci probe");

    /* new instance */
    hc = alloc_ohci();
    if (OS_NULL == hc) {
        ohci_dbg(USB_ERROR, "alloc ohci instance failed");
        return OS_FAIL;
    }
    set_pci_dedicated(pci, hc);

    /* 获取ohci第0个内存映射基地址, 保存ohci寄存器地址. */
    save_ohci_regs_addr(hc, get_pci_dev_bar(pci, 0));

    /* 开启总线竞争能力, 使得pci设备可以读写内存 */
    enable_pci_dma(pci);

    /* 安装pci设备中断 */
    result = enable_pci_int(pci);
    if (OS_SUCC != result) {
        ohci_dbg(USB_ERROR, "install pci device (ohci) interrupt failed");
        goto fail;
    }

    /* 复位控制器, 断连设备 */
    result = reset_ohci(hc);
    if (OS_SUCC != result) {
        ohci_dbg(USB_ERROR, "reset ohci failed");
        goto fail;
    }
    delay_ms(10);

    /* 初始化控制器 */
    init_ohci(hc);

    return OS_SUCC;
  fail:
    if (hc) free_ohci(hc);
    disable_pci_int(pci);
    ohci_dbg(USB_ERROR, "ohci probe failed.\n");
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void ohci_remove(HDEVICE pci)
{
    struct ohci *hc;
    os_u32 i, ndp;
    os_ret result;
    lock_t eflag;

    cassert(OS_NULL != pci);

    lock_int(eflag);

    /* 删除pci中断 */
    result = disable_pci_int(pci);
    if (OS_FAIL == result) {
        ohci_dbg(USB_ERROR, "ohci delete pci interrupt failed.\n");
    }

    hc = get_pci_dedicated(pci);
    cassert(OS_NULL != hc);

    if (OS_NULL != hc->regs) {
        ndp = get_ohci_ndp(hc);
        for (i = 0; i < ndp; i++) {
            ohci_out(&hc->regs->roothub.portstatus[i], RH_PS_CCS); /* 断连 */
            free_usb_device(hc->rh_dev[i]);
            hc->rh_dev[i] = OS_NULL;
        }
        ohci_out(&hc->regs->roothub.status, RH_HS_LPS);

        reset_ohci(hc);

        ohci_out(&hc->regs->intrdisable, 0xffffffff);
        ohci_out(&hc->regs->donehead, 0);
        ohci_out(&hc->regs->ed_controlhead, 0);
        ohci_out(&hc->regs->ed_controlcurrent, 0);

        /* 清除done, 防止重启后不断产生done中断 */
        hc->hcca.done_head = 0;

        /* clear all, so that it could use 'vesa' command */
        mem_set(&hc->hcca, 0, sizeof(struct ohci_hcca));

        for (i = 0; i < USB_DEVICE_COUNT; i++) {
            clear_device_td(hc, i);
        }
    }

    unlock_int(eflag);

    /* 释放ohci */
    free_ohci(hc);
}

LOCALD HTASK rh_handle;

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret ohci_interrupt_1(HDEVICE pci)
{
    struct ohci *hc;
    struct ohci_regs *regs;
    struct ohci_hcca *hcca;
    os_u32 ints;

    cassert(OS_NULL != pci);
    hc = get_pci_dedicated(pci);

    /* clear events */
    hc->interrupt = OS_FALSE;
    hc->sof = OS_FALSE;

    regs = hc->regs;
    hcca = &hc->hcca;

    /* is our interrupt? */
    if (0 == (ints = (ohci_in(&regs->intrstatus) & ohci_in(&regs->intrenable)))) {
        return OS_FAIL;
    } else if ((OHCI_INTR_WDH & ints) && (0 != hcca->done_head) && !(0x01 & hcca->done_head)) {
        ints = OHCI_INTR_WDH; /* td处理完成 */
    }
    hc->interrupt = OS_TRUE;

    /* it is our interrupt, prevent hc from doing it again util finished */
    ohci_out(&regs->intrdisable, OHCI_INTR_MIE);
    wmb();

    hc->done = 0;
    if (OHCI_INTR_WDH & ints) {
        // regs->intrdisable = OHCI_INTR_WDH;
        hc->done = ohci_in(&hcca->done_head) & TD_ADDR_MASK;
        ohci_out(&hcca->done_head, 0);
        // regs->intrenable = OHCI_INTR_WDH;
    }

    if (OHCI_INTR_RHSC & ints) {
        struct ohci_rh_msg *msg;

        ohci_dbg(USB_INFO, "ohci root hub status change");
        msg = alloc_msg(sizeof(struct ohci_rh_msg));
        msg->head.msg_name = OHCI_RH_MSG_ID;
        msg->head.msg_len = sizeof(struct ohci_rh_msg);
        msg->hc = hc;
        post_thread_msg(rh_handle, msg);
    }

    if (OHCI_INTR_UE & ints) {
        ohci_dbg(USB_ERROR, "unrecoverable error");
    }

    if (OHCI_INTR_SO & ints) {
        ohci_dbg(USB_ERROR, "schedule overrun");
        // regs->intrenable = OHCI_INTR_SO;
    }

    /* start frame */
    if (OHCI_INTR_SF & ints) {
        // regs->intrdisable = OHCI_INTR_SF;
        // regs->intrenable = OHCI_INTR_SF;
        ohci_dbg(USB_INFO, "start of frame");
        hc->sof = OS_TRUE;
    }

    /* hcir1_0a.pdf 7.1.4 */
    ohci_out(&regs->intrstatus, ints);
    ohci_out(&regs->intrenable, OHCI_INTR_MIE);
    wmb();

    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC struct ohci_td *ohci_reverse_donelist(os_u32 done)
{
    struct ohci_td *td;
    struct ohci_td *list;
    struct ohci_td *prev;

    list = OS_NULL;
    prev = OS_NULL;
    td = (struct ohci_td *)(phys_to_virt(done) & TD_ADDR_MASK);
    while (td) {
        list = td;
        td = (struct ohci_td *)(phys_to_virt(td->hwNextTD) & TD_ADDR_MASK);
        list->next_done_td = prev;
        prev = list;
    }
    return list;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret ohci_interrupt_2(HDEVICE pci)
{
    struct ohci *hc;
    struct ohci_td *td, *next;
    os_uint sch; /* 调度补偿 */

    cassert(OS_NULL != pci);
    hc = get_pci_dedicated(pci);

    if (OS_TRUE != hc->interrupt) {
        return OS_FAIL;
    }

    sch = OS_FALSE;

    spin_lock(&hc->ohci_lock);
    lock_schedule();
    do {
        /* hardware remove (finish) */
        if (hc->done) {
            /* td链表反转 */
            td = ohci_reverse_donelist(hc->done);
            hc->done = 0; /* clear done list */
            while (td) {
                next = td->next_done_td;
                /* 回调td完成函数 */
                if (td->complete) {
                    if (OS_TRUE == td->complete(td, TD_CC_GET(td->hwINFO))) {
                        sch = OS_TRUE;
                    }
                }
                td = next;
            }
        }

        /* software remove (cancel) */
        if (hc->sof) {
            sch = OS_TRUE;
            notify_event(hc->wait_sof, __LINE__);
        }
    } while (0);
    unlock_schedule();
    spin_unlock(&hc->ohci_lock);

    if (sch) {
        schedule();
    }

    return OS_SUCC;
}

LOCALD const struct pci_device_id pci_ohci_id = {
    /* no matter who makes it */
    PCI_ANY_ID, PCI_ANY_ID,
    /* handle any USB OHCI controller */
    PCI_CLASS_SERIAL_USB_OHCI, ~0
};

LOCALD const struct pci_driver pci_ohci_driver = {
    "usb-ohci",
    &pci_ohci_id,
    ohci_probe,
    ohci_remove,
    ohci_interrupt_1, /* ohci中断上半部处理 */
    ohci_interrupt_2 /* ohci中断下半部处理 */
};

/***************************************************************
 * description : 复位端口设备
 * history     :
 ***************************************************************/
LOCALC os_ret reset_hub_port_device(struct ohci *hc, os_u32 port)
{
    struct ohci_roothub_regs *rh_regs;
    os_uint timeout;

    rh_regs = &hc->regs->roothub;

    /* 清除复位状态位 */
    if (ohci_in(&rh_regs->portstatus[port]) & RH_PS_PRSC) {
        ohci_out(&rh_regs->portstatus[port], RH_PS_PRSC);
    }

    /* 复位端口 */
    ohci_out(&rh_regs->portstatus[port], ohci_in(&rh_regs->portstatus[port]) | RH_PS_PRS);
    timeout = 50;
    while (0 == (ohci_in(&rh_regs->portstatus[port]) & RH_PS_PRSC)) {
        delay_ms(10);
        if (1 > timeout--) {
            ohci_dbg(USB_ERROR, "wait ohci port reset timeout");
            return OS_FAIL;
        }
    }
    return OS_SUCC;
}

/***************************************************************
 * description : 从端口状态寄存器中获取设备的速度类型
 * history     :
 ***************************************************************/
LOCALC enum usb_speed_type get_device_speed(struct ohci *hc, os_u32 port)
{
    struct ohci_roothub_regs *rh_regs;

    rh_regs = &hc->regs->roothub;
    /* This field is valid only when the CurrentConnectStatus is set. */
    if (ohci_in(&rh_regs->portstatus[port]) & RH_PS_LSDA) {
        return USB_LOW_SPEED;
    } else {
        return USB_FULL_SPEED;
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void ohci_rh_status_change(struct ohci *hc)
{
    os_u32 i, ndp;
    struct ohci_roothub_regs *rh_regs;
    struct usb_device *usb;
    enum usb_speed_type speed;
    os_ret result;

    /* Now that the host knows the port to which the new device has been attached,
       the host then waits for at least 100 ms to allow completion of an insertion process and for power at the device to become stable. */
    delay_ms(100);

    rh_regs = &hc->regs->roothub;
    ndp = get_ohci_ndp(hc);
    for (i = 0; i < ndp; i++) {
        if (RH_PS_CSC & ohci_in(&rh_regs->portstatus[i])) { // ConnectStatusChange
            ohci_out(&rh_regs->portstatus[i], RH_PS_CSC); // write 1 to clear status
            if (RH_PS_CCS & ohci_in(&rh_regs->portstatus[i])) { // CurrentConnectStatus
                ohci_dbg(USB_INFO, "new ohci device");
                /* 复位设备 */
                result = reset_hub_port_device(hc, i);
                if (OS_SUCC != result) {
                    ohci_dbg(USB_ERROR, "reset ohci device failed");
                    continue; // check next port
                }

                /* 检测设备速度类型 */
                speed = get_device_speed(hc, i);
                ohci_dbg(USB_INFO, "new usb device (speed %d) is on port %d", speed, i);

                /* 分配usb设备实体 */
                usb = alloc_usb_device(hc, speed, &ohci_operation);
                if (OS_NULL == usb) {
                    ohci_dbg(USB_ERROR, "create ohci instance failed");
                    break; // resource is not enough, exit
                }
                ohci_dbg(USB_INFO, "usb device addr: %d", usb->usb_addr);

                /* 使用默认管道和地址0进行枚举 */
                result = enum_usb_device(usb);
                if (OS_SUCC != result) {
                    ohci_dbg(USB_WARNING, "enum device failed");
                }

                /* 默认地址0的默认ed和td null一直保留 */

                /* 记录端口对应的设备 */
                hc->rh_dev[i] = usb;
            } else {
                ohci_dbg(USB_INFO, "ohci device disconnected");

                if (hc->rh_dev[i]) {
                    unenum_usb_device(hc->rh_dev[i]);
                    free_usb_device(hc->rh_dev[i]);
                    hc->rh_dev[i] = OS_NULL;
                }
            }
        }
    }
}

/***************************************************************
 * description : TASK_FUNC_PTR
 * history     :
 ***************************************************************/
LOCALC os_ret OS_CALLBACK ohci_roothub_msg_proc(os_u32 arg1, os_u32 arg2, os_u32 arg3, os_u32 arg4, os_u32 arg5, os_u32 arg6, os_u32 arg7)
{
    os_void *msg;

    while (OS_SUCC == get_message(&msg)) {
        ohci_rh_status_change(((struct ohci_rh_msg *) msg)->hc);
        free_msg(msg);
    }
    cassert(OS_FALSE);
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void init_ohci_driver(os_void)
{
    os_ret result;

    ohci_dbg(USB_INFO, "init ohci driver");

    rh_handle = create_task("ohci-rh", ohci_roothub_msg_proc, TASK_PRIORITY_6, 0, 0, 0, 0, 0, 0, 0);
    cassert(OS_NULL != rh_handle);

    result = active_task_station(rh_handle);
    cassert(OS_SUCC == result);

    result = register_pci_driver(&pci_ohci_driver);
    cassert(OS_SUCC == result);
}
bus_init_func(BUS_P1, init_ohci_driver);

