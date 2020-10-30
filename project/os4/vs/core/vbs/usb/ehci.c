/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : ehci.c
 * version     : 1.0
 * description : (key) host control and root hub
 * author      : gaocheng
 * date        : 2011-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include <vbs.h>
#include "usb.h"
#include "ehci.h"

#ifdef usb_dbg
//#undef usb_dbg
//#define usb_dbg(level, fmt, arg...) do { if (USB_ERROR <= level) print(fmt"\n", ##arg); } while (0)
#endif

/***************************************************************
 global variable declare
 ***************************************************************/
LOCALD HTASK ehci_rh_handle;

/***************************************************************
 function declare
 ***************************************************************/

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void init_ehci_qtd(struct ehci_qtd *qtd)
{
    mem_set(qtd, 0, sizeof(struct ehci_qtd));

    qtd->hwinfo.alternate_next_qtd_pointer = QTD_TERMINATE;
    qtd->hwinfo.next_qtd_pointer = QTD_TERMINATE;
    qtd->hwinfo.buffer_control = QTD_STS_HALT; /* default status */

    qtd->qh = OS_NULL;
    init_list_head(&qtd->qh_list);
    init_list_head(&qtd->doing_list);
    qtd->urb = OS_NULL;
    init_list_head(&qtd->urb_list);
    qtd->complete = OS_NULL;
    qtd->data = OS_NULL;
    qtd->data_len = 0;
}

/***************************************************************
 * description : Using the selected pointer the host controller fetches the referenced qTD.
                 If the fetched qTD has it’s Active bit set to a one,
                 the host controller moves the pointer value used to reach the qTD (Next or Alternate Next) to the Current qTD Pointer field,
                 then performs the overlay.
                 If the fetched qTD has its Active bit set to a zero,
                 the host controller aborts the queue advance and follows the queue head's horizontal pointer to the next schedule data structure.
 * history     :
 ***************************************************************/
LOCALC struct ehci_qtd *alloc_qtd_tail(os_void)
{
    struct ehci_qtd *qtd;

#define EHCI_QTD_ALIGN 32
    qtd = cmalloc(sizeof(struct ehci_qtd), EHCI_QTD_ALIGN);
    if (OS_NULL == qtd) {
        usb_dbg(USB_ERROR, "alloc qtd fail");
        return OS_NULL;
    }
    /* init qtd as null */
    init_ehci_qtd(qtd);
    return qtd;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void free_qtd(struct ehci_qtd *qtd)
{
    cfree(qtd);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC struct ehci_urb *alloc_urb(os_void)
{
    struct ehci_urb *urb;

    urb = kmalloc(sizeof(struct ehci_urb));
    if (OS_NULL == urb) {
        usb_dbg(USB_ERROR, "alloc mem(0x%x) fail", sizeof(struct ehci_urb));
        return OS_NULL;
    }
    init_list_head(&urb->qtd);
    urb->app_complete = OS_NULL;
    urb->count = 0;
    urb->urb_sem = create_event_handle(EVENT_INVALID, "ehci urb", __LINE__);
    if (OS_NULL == urb->urb_sem) {
        usb_dbg(USB_ERROR, "create sem fail");
        kfree(urb);
        return OS_NULL;
    }
    urb->thandle = OS_NULL;
    urb->timeout = OS_FALSE;
    return urb;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void free_urb(struct ehci_urb *urb)
{
    destroy_event_handle(urb->urb_sem);
    kfree(urb);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret delete_qtd(struct ehci_qtd *qtd)
{
    os_u32 token;

    token = qtd->hwinfo.buffer_control;

    del_list(&qtd->doing_list);
    del_list(&qtd->urb_list);
    del_list(&qtd->qh_list);
    free_qtd(qtd);

    /* QTD_STS_SPLITXSTATE and QTD_STS_PING maybe normal. */
    if ((QTD_STS_DBE | QTD_STS_BD | QTD_STS_TE | QTD_STS_MMF) & token) {
        usb_dbg(USB_ERROR, "<qtd stat 0x%x>", token);
        return OS_FAIL;
    }

    /* The device responds to the transaction with a STALL PID.
       When this occurs, the Halted bit is set to a one and the Active bit is set to a zero.
       This results in the hardware not advancing the queue and the pipe halts.
       Software must intercede to recover. */
    if (QTD_STS_HALT & token) {
        usb_dbg(USB_ERROR, "<stall>");
        return OS_FAIL;
    }

    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret free_ehci_qh(struct ehci *hc, struct ehci_qh *ep)
{
    struct ehci_qtd *qtd;

    if (ep->qh_lock) {
        destroy_critical_section(ep->qh_lock);
        ep->qh_lock = OS_NULL;
    }
    /* delete qtd null */
    cassert(ep->qtd.next == ep->qtd.prev);
    if (ep->qtd.next != &ep->qtd) {
        qtd = list_addr(ep->qtd.next, struct ehci_qtd, qh_list);
        del_list(&qtd->qh_list);
        spin_lock(&hc->ehci_lock);
        del_list(&qtd->doing_list);
        spin_unlock(&hc->ehci_lock);
        del_list(&qtd->urb_list);
        free_qtd(qtd);
    }
    cassert(list_empty(&ep->qtd));
    del_init_list(&ep->qh);
    ep->state = EHCI_ED_NEW;
}

/* dir: enum usb_direction */
#define ehci_ed_index(pipe, dir) (USB_DIR_CNT * (pipe) + (dir))

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC struct ehci_qh *alloc_ehci_qh(struct usb_device *usb, os_u32 addr, os_u8 pipe, enum usb_transfer_type type, enum usb_direction dir)
{
    struct ehci *hc;
    struct ehci_qh *ed;
    os_u32 MaxPacketSize; /* 32bit type, because of ep_mpl(hwinfo->characteristics, MaxPacketSize); */
    struct ehci_qh_hw *hwinfo;
    os_ret result;

    /* 4.10.2 Advance Queue
       To advance the queue, the host controller must find the next qTD, adjust pointers, perform the overlay and
       write back the results to the queue head.
       This state is entered from the FetchQHD state if the overlay Active and Halt bits are set to zero. On entry to
       this state, the host controller determines which next pointer to use to fetch a qTD, fetches a qTD and
       determines whether or not to perform an overlay. Note that if the I-bit is a one and the Active bit is a zero,
       the host controller immediately skips processing of this queue head, exits this state and uses the horizontal
       pointer to the next schedule data structure. */

    hc = usb->host_controller;
    ed = &hc->endpoint[usb->usb_addr][ehci_ed_index(pipe, dir)];
    if (EHCI_ED_NEW != ed->state) {
        /* no modify */
        return ed;
    }
    ed->state = EHCI_ED_OLD;

    ed->qh_lock = OS_NULL;

    ed->proportion = usb_endpoint_proportion(usb, pipe, dir);

    /* default mps is 8 */
    MaxPacketSize = usb_endpoint_mps(usb, pipe, dir);
    usb_dbg(USB_INFO, "mps %d %d", pipe, MaxPacketSize);

    init_list_head(&ed->qh);
    init_list_head(&ed->qtd);

    result = create_critical_section(&ed->qh_lock, __LINE__);
    if (OS_SUCC != result) {
        usb_dbg(USB_ERROR, "create ehci lock fail");
        cassert(OS_FALSE);
        goto error;
    }

    ed->type = type;

    /* set hardware part of endpoint */
    hwinfo = &ed->hwinfo;

    set_qhhlp(hwinfo->qhhlp, virt_to_phys((pointer) hwinfo), EHCI_QH, QH_VALID);

    /* Using the selected pointer the host controller fetches the referenced qTD.
       If the fetched qTD has it’s Active bit set to a one,
       the host controller moves the pointer value used to reach the qTD (Next or Alternate Next) to the Current qTD Pointer field,
       then performs the overlay */
    hwinfo->curr_qtd_pointer = QTD_TERMINATE;

    /* characteristics */
    ep_ncr(hwinfo->characteristics, 0);
    /* If the QH.EPS field indicates the endpoint is not a high-speed device,
       and the endpoint is an control endpoint, then software must set this bit to a one.
       Otherwise it should always set this bit to a zero. */
    if ((USB_CTRL_TRANSFER == type) && (USB_HIGH_SPEED != usb->speed)) {
        ep_cef(hwinfo->characteristics, 1);
    } else {
        ep_cef(hwinfo->characteristics, 0);
    }
    ep_mpl(hwinfo->characteristics, MaxPacketSize);
    /* head of relamation, don't need again except in alloc_ehci */
    ep_hrlf(hwinfo->characteristics, 0);
    switch (usb->speed) {
    case USB_LOW_SPEED:
        ep_eps(hwinfo->characteristics, EHCI_CHA_LOW_SPEED);
        break;
    case USB_FULL_SPEED:
        ep_eps(hwinfo->characteristics, EHCI_CHA_FULL_SPEED);
        break;
    case USB_HIGH_SPEED:
        ep_eps(hwinfo->characteristics, EHCI_CHA_HIGH_SPEED);
        break;
    default:
        cassert(OS_FALSE);
        break;
    }
    ep_endpt(hwinfo->characteristics, pipe);
    /* This field is only valid when the queue head is in the Periodic Schedule and the EPS field
       indicates a Full or Low-speed endpoint. Setting this bit to a one when the queue head is in
       the Asynchronous Schedule or the EPS field indicates a high-speed device yields
       undefined results. */
    ep_iont(hwinfo->characteristics, 0);
    switch (type) {
    /* refer to 8.5.3 Control Transfers, usb_20.pdf */
    case USB_CTRL_TRANSFER:
    default:
        /* 1b Initial data toggle comes from incoming qTD DT bit.
           Host controller replaces DT bit in the queue head from the DT bit in the qTD. */
        ep_dtc(hwinfo->characteristics, 1);
        break;
    /* refer to 8.5.2, 8.5.4, 8.5.5 usb_20.pdf */
    case USB_BULK_TRANSFER:
    case USB_ISO_TRANSFER:
    case USB_INTERRUPT_TRANSFER:
        ep_dtc(hwinfo->characteristics, 0);
        break;
    }
    ep_da(hwinfo->characteristics, addr);

    /* capabilities */
    ep_hbpm(hwinfo->capabilities, EP_MULT_ONE);

    ep_pn(hwinfo->capabilities, usb->hub_port_num);
    ep_ha(hwinfo->capabilities, usb->hub_addr);

    switch (type) {
    /* async schedule */
    case USB_CTRL_TRANSFER:
    case USB_BULK_TRANSFER:
    default:
        /* This field is ignored by the host controller unless the EPS field indicates this device is a low- or full-speed device and this queue head is in the periodic list. */
        ep_scm(hwinfo->capabilities, 0);
        /* This field is used for all endpoint speeds.
           Software should set this field to a zero when the queue head is on the asynchronous schedule. */
        ep_ism(hwinfo->capabilities, 0);
        break;

    /* period schedule */
    case USB_ISO_TRANSFER:
        ep_ism(hwinfo->capabilities, 0);
    case USB_INTERRUPT_TRANSFER:
        /* do not set ism here, ep_ism(hwinfo->capabilities, 0x1); */
        if ((USB_LOW_SPEED == usb->speed) || (USB_FULL_SPEED == usb->speed)) {
            /* use the example of 4.12.2.1 Split Transaction Scheduling Mechanisms for Interrupt */
            ep_scm(hwinfo->capabilities, 0x1c);
        } else {
            ep_scm(hwinfo->capabilities, 0);
        }
        break;
    }

    /* The general
       operational model is that the host controller can detect whether the overlay area contains a description of an active transfer.
       4.10.3 The host controller enters this state from the Fetch Queue Head state only if the Active bit in Status field of the queue head is set to a one. */
    mem_set(&hwinfo->overlay, 0, sizeof(struct ehci_qtd_hw));

    hwinfo->overlay.next_qtd_pointer |= QTD_TERMINATE;
    hwinfo->overlay.alternate_next_qtd_pointer |= QTD_TERMINATE;
    hwinfo->overlay.buffer_control |= QTD_STS_HALT;

    /* add null qtd to ed, null qtd is the last qtd of endpoint */
    do {
        struct ehci_qtd *qtd_null;
        /* Using the selected pointer the host controller fetches the referenced qTD.
           If the fetched qTD has it’s Active bit set to a one,
           the host controller moves the pointer value used to reach the qTD (Next or Alternate Next) to the Current qTD Pointer field,
           then performs the overlay.
           If the fetched qTD has its Active bit set to a zero,
           the host controller aborts the queue advance and follows the queue head's horizontal pointer to the next schedule data structure. */
        qtd_null = alloc_qtd_tail();
        if (OS_NULL == qtd_null) {
            usb_dbg(USB_ERROR, "alloc null qtd fail");
            cassert(OS_FALSE);
            goto error;
        }
        add_list_tail(&ed->qtd, &qtd_null->qh_list);
        hwinfo->overlay.next_qtd_pointer = virt_to_phys((pointer) &qtd_null->hwinfo);
    } while (0);

    return ed;
  error:
    if (ed->qh_lock) destroy_critical_section(ed->qh_lock);
    free_ehci_qh(hc, ed);
    return OS_NULL;
}

/***************************************************************
 * description : reenable endpoint because of null qtd
 *               refer to Figure 4-14. Host Controller Queue Head Traversal State Machine
 * history     :
 ***************************************************************/
LOCALC os_void enable_qh(struct ehci_qh *ed)
{
    set_qtd_tbt(ed->hwinfo.overlay.buffer_control, 0);
    wmb();
    /* clear halt and active to start advance queue, not execute translation */
    ed->hwinfo.overlay.buffer_control &= (~(QTD_STS_HALT | QTD_STS_ACTIVE));
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void disable_qh(struct ehci_qh *ed)
{
    ed->hwinfo.overlay.buffer_control |= QTD_STS_HALT;
    ed->hwinfo.overlay.alternate_next_qtd_pointer |= QTD_TERMINATE;
    wmb();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void enable_ehci_schedule(struct ehci *hc, enum usb_transfer_type type)
{
    wmb();
    switch (type) {
    case USB_CTRL_TRANSFER:
    case USB_BULK_TRANSFER:
        ehci_out(&hc->regs.operation->USBCMD, EHCI_CMD_ASE | ehci_in(&hc->regs.operation->USBCMD));
        break;
    case USB_ISO_TRANSFER:
    case USB_INTERRUPT_TRANSFER:
        ehci_out(&hc->regs.operation->USBCMD, EHCI_CMD_PSE | ehci_in(&hc->regs.operation->USBCMD));
        break;
    default:
        cassert(OS_FALSE);
        break;
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void disable_ehci_schedule(struct ehci *hc, enum usb_transfer_type type)
{
    wmb();
    switch (type) {
    case USB_CTRL_TRANSFER:
    case USB_BULK_TRANSFER:
        ehci_out(&hc->regs.operation->USBCMD, (~EHCI_CMD_ASE) & ehci_in(&hc->regs.operation->USBCMD));
        break;
    case USB_ISO_TRANSFER:
    case USB_INTERRUPT_TRANSFER:
        ehci_out(&hc->regs.operation->USBCMD, (~EHCI_CMD_PSE) & ehci_in(&hc->regs.operation->USBCMD));
        break;
    default:
        cassert(OS_FALSE);
        break;
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret OS_CALLBACK ehci_qtd_timer(os_u32 event_id)
{
    struct ehci_urb *urb;

    cassert(OS_NULL != (struct ehci_urb *) event_id);
    urb = (struct ehci_urb *) event_id;
    usb_dbg(USB_ERROR, "qtd timeout");
    urb->timeout = OS_TRUE;
    notify_event(urb->urb_sem, __LINE__);
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_bool qtd_complete(struct ehci *hc, struct ehci_qtd *qtd)
{
    os_u32 token;
    os_u32 urb_done;

    token = qtd->hwinfo.buffer_control;

    if (QTD_STS_DBE & token) {
        usb_dbg(USB_INFO, "<dbe>");
    } else if (QTD_STS_BD & token) {
        /* 4.15.1.1.1 Serial Bus Babble
           The USBERRINT bit in the USBSTS register is set to a one
           and if the USB Error Interrupt Enable bit in the USBINTR register is a one,
           then a hardware interrupt is signaled to the system at the next interrupt threshold.
           The host controller must never start an OUT transaction that will babble across a micro-frame EOF.*/
        usb_dbg(USB_INFO, "<bd>");
    } else if (QTD_STS_TE & token) {
        usb_dbg(USB_INFO, "<te>");
    } else if (QTD_STS_MMF & token) {
        usb_dbg(USB_INFO, "<mmf>");
    } else if (QTD_STS_SPLITXSTATE & token) {
        usb_dbg(USB_INFO, "<split>");
    } else if (QTD_STS_PING & token) {
        //usb_dbg(USB_INFO, "<ping>");
    } else {
        //usb_dbg(USB_INFO, "i");
    }

    urb_done = 0;

    qtd->urb->count--;

    if (0 == qtd->urb->count) {
        urb_done = 1;
    } else if (qtd_pid_is_in(token) && get_qtd_tbt(token)) {
        /* To advance queue head’s transfer state,
           the Total Bytes to Transfer field is decremented by the number of bytes moved in the transaction */
        /* bugfix: short read handle */
        urb_done = 1;

/* bugfix: loop_del_list() cannot delete more than one */
#if 0
        /* clear doing list for stopping scan */
        do {
            struct list_node *i;
            struct ehci_qtd *p;

            loop_list(i, &qtd->urb->qtd) {
                p = list_addr(i, struct ehci_qtd, urb_list);
                del_init_list(&p->doing_list);
            }
        } while (0);
#endif
    }

    reset_timer(qtd->urb->thandle);
    /* bugfix, delete first, and then give sem. */
    /* if (list_empty(&qtd->urb->qtd)) { */
    if (urb_done) {
        qtd->urb->timeout = OS_FALSE;
        notify_event(qtd->urb->urb_sem, __LINE__);
        return OS_TRUE;
    }
    return OS_FALSE;
}

#define EHCI_DEFAULT_PROPORTION 8
#define EHCI_WAIT_MS_GRANULARITY 0x80

/***************************************************************
 * description : ed_num, 管道号(0 is control), token表示传输方向
 * history     :
 ***************************************************************/
LOCALC os_ret ehci_add_async_qtd(struct ehci *hc, struct ehci_qh *ed, os_u32 token, os_u32 toggle, os_u8 *buffer, os_u32 len)
{
    lock_t eflag;
    struct ehci_urb *urb;
    struct ehci_qtd *qtd, *first;
    struct ehci_qtd *qtd_null;
    os_u32 mps;
    os_u32 qtd_data_len;
    os_u32 addr;
    os_u32 qtd_num;
    os_uint i;
    struct list_node *li, *_save;
    os_bool doorbell;
    os_u32 proportion;
    os_ret result;

    /* null qtd -> current qtd */
    cassert(OS_FALSE == list_empty(&ed->qtd));

    urb = alloc_urb();
    if (OS_NULL == urb) {
        usb_dbg(USB_ERROR, "alloc urb fail");
        return OS_FAIL;
    }

    /* get max packet size from endpoint */
    mps = get_ep_mps(ed->hwinfo.characteristics);
    qtd_num = 0;
    first = qtd = OS_NULL;
    do {
        /* bugfix, alloc qtd null first. */
        qtd_null = alloc_qtd_tail();
        if (OS_NULL == qtd_null) {
            usb_dbg(USB_ERROR, "alloc qtd fail");
            result = OS_FAIL;
            goto clear;
        }

        lock_int(eflag);

        do {
            qtd = list_addr(ed->qtd.prev, struct ehci_qtd, qh_list);
            /* insert qtd by transaction order */
            add_list_tail(&ed->qtd, &qtd_null->qh_list); /* null qtd always is the last */
        } while (0);

        if (first) {
            qtd->hwinfo.buffer_control |= QTD_STS_ACTIVE;
            qtd->hwinfo.buffer_control &= (~QTD_STS_HALT);
            qtd->complete = qtd_complete;
            spin_lock(&hc->ehci_lock);
            add_list_tail(&hc->doing_list, &qtd->doing_list);
            spin_unlock(&hc->ehci_lock);
        } else {
            first = qtd;
        }

        unlock_int(eflag);

        add_list_tail(&urb->qtd, &qtd->urb_list);
        qtd->urb = urb;
        qtd->qh = ed;

        qtd->hwinfo.alternate_next_qtd_pointer |= QTD_TERMINATE;
        set_qtd_toggle(qtd->hwinfo.buffer_control, toggle);
        qtd->hwinfo.buffer_control |= QTD_CTRL_IOC; /* use interrupt */
        set_qtd_cp(qtd->hwinfo.buffer_control, 0);
        set_qtd_cerr(qtd->hwinfo.buffer_control, QTD_CERR_NUM);
        set_qtd_pid(qtd->hwinfo.buffer_control, token);

        qtd_data_len = MAX_QTD_TBT - ((os_u32) buffer & QTD_BPL_MASK);
        if (qtd_data_len >= len) {
            qtd_data_len = len;
        } else {
            /* bugfix, short packets may only terminate transfers!!!
               5.7.3 Interrupt Transfer Packet Size Constraints of usb_20.pdf
               An interrupt transfer is complete when the endpoint does one of the following:
               Has transferred exactly the amount of data expected
               Transfers a packet with a payload size less than wMaxPacketSize or transfers a zero-length packet */
            qtd_data_len -= (qtd_data_len % mps);
        }
        set_qtd_tbt(qtd->hwinfo.buffer_control, qtd_data_len);
        len -= qtd_data_len;

        addr = (os_u32) buffer;
        qtd->hwinfo.buffer_pointer[0] = virt_to_phys(addr);
        qtd->hwinfo.appendix_64[0] = hc->appendix_64;
        addr = (addr & ~QTD_BPL_MASK) + 0x1000;
        for (i = 1; i < EHCI_TRANS_PAGE_NUM; i++) {
            qtd->hwinfo.buffer_pointer[i] = virt_to_phys(addr) & ~QTD_BPL_MASK;
            qtd->hwinfo.appendix_64[i] = hc->appendix_64;
            addr += 0x1000;
        }
        buffer += qtd_data_len;

        qtd->hwinfo.next_qtd_pointer = virt_to_phys((pointer) &qtd_null->hwinfo);
        /* bugfix: not short read, no terminate */
        if (QTD_PID_IN == token) {
            /* If the field Bytes to Transfer is not zero and the T-bit in the Alternate Next qTD Pointer is set to zero,
               then the host controller uses the Alternate Next qTD Pointer.
               Otherwise, the host controller uses the Next qTD Pointer.
               If Next qTD Pointer’s T-bit is set to a one,
               then the host controller exits this state and uses the horizontal pointer to the next schedule data structure. */
            qtd->hwinfo.alternate_next_qtd_pointer = virt_to_phys((pointer) &hc->qtd_null->hwinfo);
        }

        qtd_num++;
    } while (0 < len);
    urb->count = qtd_num;

    do { /* one operation */
        lock_int(eflag);

        if (first) {
            spin_lock(&hc->ehci_lock);
            add_list_tail(&hc->doing_list, &first->doing_list);
            spin_unlock(&hc->ehci_lock);
            first->complete = qtd_complete;
            wmb();
            first->hwinfo.buffer_control &= (~QTD_STS_HALT);
            first->hwinfo.buffer_control |= QTD_STS_ACTIVE;
        }
        /* clear halt & active in the overlay */
        enable_qh(ed);
        enable_ehci_schedule(hc, ed->type);

        doorbell = OS_FALSE;
        proportion = ed->proportion;
        /* wait for interrupt, interrupt may be occur between regs and sem, use EHCI_WAIT_QTD_DONE */
        if (0 == proportion) {
            /* proportion is not set, use the default value. */
            proportion = EHCI_DEFAULT_PROPORTION;
        }
        urb->thandle = set_timer_callback((os_u32) urb, qtd_num * proportion * EHCI_WAIT_MS_GRANULARITY, ehci_qtd_timer, TIMER_MODE_NOT_LOOP);
        if (urb->thandle) {
            result = wait_event(urb->urb_sem, 0);
            kill_timer(urb->thandle);
            if (OS_SUCC != result) {
                result = OS_FAIL;
                doorbell = OS_TRUE;
                usb_dbg(USB_ERROR, "ehci take urb sem fail");
            } else {
                if ((!urb->timeout) && (urb->count)) {
                    /* short read */
                    doorbell = OS_TRUE;
                }
            }
            if (urb->timeout) {
                result = OS_FAIL;
                doorbell = OS_TRUE;
            }
        }
        unlock_int(eflag);
    } while (0);

    if (doorbell) {
        os_ret r;

        disable_qh(ed);

        /* start doorbell */
        ehci_out(&hc->regs.operation->USBCMD, EHCI_CMD_DOORBELL | ehci_in(&hc->regs.operation->USBCMD));

        /* wait for interrupt */
#define EHCI_WAIT_DB_MS 20
        r = wait_event(hc->doorbell_sem, EHCI_WAIT_DB_MS);
        if (OS_SUCC != r) {
            usb_dbg(USB_ERROR, "\nwait doorbell timeout");
        }
        usb_dbg(USB_INFO, "wait doorbell success");

        /* makes ed points to qtd null */
        qtd = list_addr(ed->qtd.prev, struct ehci_qtd, qh_list);
        cassert(qtd == qtd_null);
        ed->hwinfo.overlay.next_qtd_pointer = virt_to_phys((pointer) &qtd->hwinfo);
        ed->hwinfo.overlay.alternate_next_qtd_pointer = QTD_TERMINATE;
        /* clear length for next transfer's alternative next pointer */
        set_qtd_tbt(ed->hwinfo.overlay.buffer_control, 0);
        wmb();
    }

  clear:
    /* delete urb doing list */
    loop_del_list(li, _save, &urb->qtd) {
        qtd = list_addr(li, struct ehci_qtd, urb_list);

        spin_lock(&hc->ehci_lock);
        if (OS_SUCC != delete_qtd(qtd)) {
            result = OS_FAIL;
        }
        spin_unlock(&hc->ehci_lock);
        qtd_num--;
    }
    cassert(0 == qtd_num);
    cassert(list_empty(&urb->qtd));
    cassert(ed->qtd.prev = ed->qtd.next);
    free_urb(urb);

    return result;
}

/***************************************************************
 * description : 4.8.2 Removing Queue Heads from Asynchronous Schedule
 *               The handshake is implemented with three bits in the host controller.
 *               The first bit is a command bit (Interrupt on Async Advance Doorbell bit in the USBCMD register) that allows software to inform the host controller that something has been removed from its asynchronous schedule.
 *               The second bit is a status bit (Interrupt on Async Advance bit in the USBSTS register) that the host controller sets after it has released all on-chip state that may potentially reference one of the data structures just removed.
 *               When the host controller sets this status bit to a one,
 *               it also sets the command bit to a zero.
 *               The third bit is an interrupt enable (Interrupt on Async Advance bit in the USBINTR register) that is matched with the status bit.
 *               If the status bit is a one and the interrupt enable bit is a one,
 *               then the host controller will assert a hardware interrupt.
 * history     :
 ***************************************************************/
LOCALC os_ret ehci_remove_async_qh(struct ehci *hc, struct ehci_qh *qh)
{
    struct ehci_qh *prev;
    struct ehci_qtd *qtd;
    struct ehci_qtd_hw *info;
    struct list_node *inode, *_save;
    os_u32 times;
    os_ret result;

    if (EHCI_ED_OLD != qh->state) {
        return OS_SUCC;
    }

    /* The normal mode of operation is that software removes queue heads from the asynchronous schedule without shutting it down.
       Software must not remove an active queue head from the schedule.
       Software should first deactivate all active qTDs,
       wait for the queue head to go inactive,
       then remove the queue head from the asynchronous list. */

    /* deactive all qtd, refer to 4.10.5 Follow Queue Head Horizontal Pointer */
    info = &qh->hwinfo.overlay;
    while (0 == (QTD_TERMINATE & info->next_qtd_pointer)) {
        info = (struct ehci_qtd_hw *) get_qtd_addr(phys_to_virt(info->next_qtd_pointer));
        info->buffer_control &= ~QTD_STS_ACTIVE;
    }

    /* wait queue head deactive */
    times = 10;
    while ((0 != (QTD_STS_ACTIVE & qh->hwinfo.overlay.buffer_control)) && (0 < times)) {
        times--;
        delay_ms(1);
    }
    if (0 == times) {
        /* wait timeout */
        usb_dbg(USB_ERROR, "wait deactive all qtd timeout");
        return OS_FAIL;
    }

    /* del endpoint, but partly */
    prev = list_addr(qh->qh.prev, struct ehci_qh, qh);
    mod_qhhlp_addr(prev->hwinfo.qhhlp, qh->hwinfo.qhhlp);

    wmb();

    /* doorbell */
    ehci_out(&hc->regs.operation->USBCMD, EHCI_CMD_DOORBELL | ehci_in(&hc->regs.operation->USBCMD));

    /* wait for interrupt */
#define EHCI_WAIT_DB_MS 20
    result = wait_event(hc->doorbell_sem, EHCI_WAIT_DB_MS);
    if (OS_SUCC != result) {
        usb_dbg(USB_ERROR, "wait doorbell timeout");
    }

    /* delete ed and free qtd safely */
    loop_del_list(inode, _save, &qh->qtd) {
        qtd = list_addr(inode, struct ehci_qtd, qh_list);

        /* delete from doing, done list, urb list. including null qtd. */
        spin_lock(&hc->ehci_lock);
        del_list(&qtd->doing_list);
        spin_unlock(&hc->ehci_lock);
        del_list(&qtd->urb_list);
        if ((qtd->urb) && list_empty(&qtd->urb->qtd)) {
            free_urb(qtd->urb);
        }
        del_list(&qtd->qh_list);
        free_qtd(qtd);
    }

    return OS_SUCC;
}

/***************************************************************
 * description : refer to 4.8.1
 *               insert do not care hardware cache
 * history     :
 ***************************************************************/
LOCALC os_ret ehci_insert_async_qh(struct ehci *hc, struct ehci_qh *qh)
{
    /* the list of new qh is empty */
    if (list_empty(&qh->qh)) {
        spin_lock(&hc->ehci_lock);
        /* Queue head data structures are the only valid data structures that may be linked into the asynchronous schedule. */
        add_list_head(&hc->async_head->qh, &qh->qh);
        set_qhhlp(qh->hwinfo.qhhlp, hc->async_head->hwinfo.qhhlp, EHCI_QH, QH_VALID);
        wmb();
        set_qhhlp(hc->async_head->hwinfo.qhhlp, virt_to_phys((pointer) &qh->hwinfo), EHCI_QH, QH_VALID);
        spin_unlock(&hc->ehci_lock);
    }

    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret ehci_remove_periodic_qh(struct ehci *hc, struct ehci_qh *qh)
{
    struct ehci_qh *prev;
    struct ehci_qtd *qtd;
    struct ehci_qtd_hw *info;
    struct list_node *inode, *_save;
    os_u32 times;

    if (EHCI_ED_OLD != qh->state) {
        return OS_SUCC;
    }

    /* deactive all qtd, refer to 4.10.5 Follow Queue Head Horizontal Pointer */
    info = &qh->hwinfo.overlay;
    while (0 == (QTD_TERMINATE & info->next_qtd_pointer)) {
        info = (struct ehci_qtd_hw *) get_qtd_addr(phys_to_virt(info->next_qtd_pointer));

        info->buffer_control &= ~QTD_STS_ACTIVE;
    }

    /* wait queue head deactive */
    times = 10;
    while ((0 != (QTD_STS_ACTIVE & qh->hwinfo.overlay.buffer_control)) && (0 < times)) {
        times--;
        delay_ms(1);
    }
    if (0 == times) {
        /* wait timeout */
        usb_dbg(USB_ERROR, "wait deactive all qtd timeout");
        return OS_FAIL;
    }

    /* del endpoint, but partly */
    prev = list_addr(qh->qh.prev, struct ehci_qh, qh);
    mod_qhhlp_addr(prev->hwinfo.qhhlp, qh->hwinfo.qhhlp);

    /* free qtd and delete ed safely */
    loop_del_list(inode, _save, &qh->qtd) {
        qtd = list_addr(inode, struct ehci_qtd, qh_list);

        /* delete from doing, done list, urb list. including null qtd. */
        spin_lock(&hc->ehci_lock);
        del_list(&qtd->doing_list);
        spin_unlock(&hc->ehci_lock);
        del_list(&qtd->urb_list);
        if ((OS_NULL != qtd->urb) && list_empty(&qtd->urb->qtd)) {
            free_urb(qtd->urb);
        }
        del_list(&qtd->qh_list);
        free_qtd(qtd);
    }

    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void ehci_insert_periodic_qh(struct ehci *hc, struct ehci_qh *new, os_u32 uframe)
{
    struct ehci_qh *ed;
    struct ehci_qh *select;
    os_uint i, j;

    ed = hc->qh_int;
    select = OS_NULL;
    for (i = 0; i < INT_ENDPOINT_NULL_NUM; i++) {
        if (uframe >= ed[i].int_interval) {
            select = &ed[i];
            /* balance */
            for (j = i + 1; j < INT_ENDPOINT_NULL_NUM; j++) {
                if (ed[i].int_interval == ed[j].int_interval) {
                    if (ed[j].load_cnt < select->load_cnt) {
                        select = &ed[j];
                    }
                } else {
                    break;
                }
            }
            break;
        }
    }

    if (OS_NULL != select) {
        select->load_cnt++;
        /* only link to the tail of ed_null */
        add_list_tail(&select->qh, &new->qh);

        /* preserve type and terminate of the qh */ //new->hwinfo.qhhlp = select->hwinfo.qhhlp;
        ed = list_addr(new->qh.prev, struct ehci_qh, qh); // use the last qh
        new->hwinfo.qhhlp = ed->hwinfo.qhhlp;
        if ((USB_INTERRUPT_TRANSFER == new->type) && (EHCI_CHA_HIGH_SPEED != get_eps(new->hwinfo.capabilities))) {
            /* do not use FSTN */
            ep_ism(new->hwinfo.capabilities, 0x01); // start uframe no 0.
            ep_scm(new->hwinfo.capabilities, 0x1c); // complete uframe at 4, 3 or 2.
        } else {
            ep_ism(new->hwinfo.capabilities, get_ep_ism(select->hwinfo.capabilities));
        }
        /* clear terminate and modify address */
        set_qhhlp(ed->hwinfo.qhhlp, ed->hwinfo.qhhlp, EHCI_QH, QH_VALID);
        wmb();
        mod_qhhlp_addr(ed->hwinfo.qhhlp, virt_to_phys((pointer) &new->hwinfo));
    }
}

/***************************************************************
 * description : usb_20.pdf 8.5.3 control transfers
 *               addr: usb设备地址
 *               ednum: usb设备端点号
 *               input:
 *               output:
 * history     :
 ***************************************************************/
LOCALC os_ret ehci_recv_control_msg(struct usb_device *usb, os_u32 addr, os_u8 ed_num, struct usb_setup_data *cmd, os_void *buffer, os_uint len)
{
    struct ehci *hc;
    struct ehci_qh *ed;
    os_ret result;

    cassert((OS_NULL != usb) && (EP_NUM > ed_num));

    if (OS_NULL != usb->host_controller) {
        hc = usb->host_controller;
        if (OS_NULL != hc->endpoint[usb->usb_addr]) {
            /* 4.10 Managing Control/Bulk/Interrupt Transfers via Queue Heads */
            result = OS_SUCC;
            ed = alloc_ehci_qh(usb, addr, ed_num, USB_CTRL_TRANSFER, USB_IN);
            cassert(OS_NULL != ed);
            ehci_insert_async_qh(hc, ed);

            enter_critical_section(ed->qh_lock);
            /* setup stage */
            if (OS_SUCC == ehci_add_async_qtd(hc, ed, QTD_PID_SETUP, QTD_TOGGLE_0, (os_u8 *) cmd, sizeof(struct usb_setup_data))) {
                if (0 != len) {
                    /* data stage */
                    if (OS_SUCC != ehci_add_async_qtd(hc, ed, QTD_PID_IN, QTD_TOGGLE_1, buffer, len)) {
                        usb_dbg(USB_ERROR, "in ehci recv ctrl data fail");
                        result = OS_FAIL;
                        goto end;
                    }
                }
                /* status stage */
                if (OS_SUCC != ehci_add_async_qtd(hc, ed, QTD_PID_OUT, QTD_TOGGLE_1, OS_NULL, 0)) {
                    usb_dbg(USB_ERROR, "in ehci recv ctrl status fail");
                    result = OS_FAIL;
                    goto end;
                }
            } else {
                result = OS_FAIL;
            }
          end:
            leave_critical_section(ed->qh_lock);
            return result;
        }
    }
    usb_dbg(USB_ERROR, "ehci recv ctrl msg, input para error");
    return OS_FAIL;
}

/***************************************************************
 * description : usb_20.pdf 8.5.3 control transfers
 * history     :
 ***************************************************************/
LOCALC os_ret ehci_send_control_msg(struct usb_device *usb, os_u32 addr, os_u8 ed_num, struct usb_setup_data *cmd, os_void *buffer, os_uint len)
{
    struct ehci *hc;
    struct ehci_qh *ed;
    os_ret result;

    cassert((OS_NULL != usb) && (EP_NUM > ed_num));

    if (OS_NULL != usb->host_controller) {
        hc = usb->host_controller;
        if (OS_NULL != hc->endpoint[usb->usb_addr]) {
            /* 4.10 Managing Control/Bulk/Interrupt Transfers via Queue Heads */
            result = OS_SUCC;
            ed = alloc_ehci_qh(usb, addr, ed_num, USB_CTRL_TRANSFER, USB_OUT);
            cassert(OS_NULL != ed);
            ehci_insert_async_qh(hc, ed);

            enter_critical_section(ed->qh_lock);
            /* setup stage */
            if (OS_SUCC == ehci_add_async_qtd(hc, ed, QTD_PID_SETUP, QTD_TOGGLE_0, (os_u8 *) cmd, sizeof(struct usb_setup_data))) {
                if (0 != len) {
                    /* data stage */
                    if (OS_SUCC != ehci_add_async_qtd(hc, ed, QTD_PID_OUT, QTD_TOGGLE_1, buffer, len)) {
                        usb_dbg(USB_ERROR, "out ehci send ctrl data fail");
                        result = OS_FAIL;
                        goto end;
                    }
                }
                /* status stage */
                if (OS_SUCC != ehci_add_async_qtd(hc, ed, QTD_PID_IN, QTD_TOGGLE_1, OS_NULL, 0)) {
                    usb_dbg(USB_ERROR, "out ehci recv ctrl status fail");
                    result = OS_FAIL;
                    goto end;
                }
            } else {
                usb_dbg(USB_ERROR, "out ehci setup status fail");
                result = OS_FAIL;
            }
          end:
            leave_critical_section(ed->qh_lock);
            return result;
        }
    }
    usb_dbg(USB_ERROR, "ehci send ctrl msg, input para error");
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret ehci_recv_bulk_msg(struct usb_device *usb, os_u8 pipe, os_u8 *buffer, os_uint len)
{
    struct ehci *hc;
    struct ehci_qh *ed;
    os_ret result;

    cassert((OS_NULL != usb) && (EP_NUM > pipe));

    if (OS_NULL != usb->host_controller) {
        hc = usb->host_controller;
        if ((OS_NULL != hc) && (OS_NULL != hc->endpoint[usb->usb_addr])) {
            /* 4.10 Managing Control/Bulk/Interrupt Transfers via Queue Heads */
            result = OS_SUCC;
            ed = alloc_ehci_qh(usb, usb->usb_addr, pipe, USB_BULK_TRANSFER, USB_IN);
            cassert(OS_NULL != ed);
            ehci_insert_async_qh(hc, ed);

            enter_critical_section(ed->qh_lock);
            /* QTD_TOGGLE_0 is unused */
            if (OS_SUCC != ehci_add_async_qtd(hc, ed, QTD_PID_IN, QTD_TOGGLE_0, buffer, len)) {
                result = OS_FAIL;
            }
            leave_critical_section(ed->qh_lock);
            return result;
        }
    }
    usb_dbg(USB_ERROR, "ehci recv bulk msg fail");
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret ehci_send_bulk_msg(struct usb_device *usb, os_u8 pipe, os_u8 *buffer, os_uint len)
{
    struct ehci *hc;
    struct ehci_qh *ed;
    os_ret result;

    cassert((OS_NULL != usb) && (EP_NUM > pipe));

    if (OS_NULL != usb->host_controller) {
        hc = usb->host_controller;
        if ((OS_NULL != hc) && (OS_NULL != hc->endpoint[usb->usb_addr])) {
            /* 4.10 Managing Control/Bulk/Interrupt Transfers via Queue Heads */
            result = OS_SUCC;
            ed = alloc_ehci_qh(usb, usb->usb_addr, pipe, USB_BULK_TRANSFER, USB_OUT);
            cassert(OS_NULL != ed);
            ehci_insert_async_qh(hc, ed);

            enter_critical_section(ed->qh_lock);
            /* QTD_TOGGLE_0 is unused */
            if (OS_SUCC != ehci_add_async_qtd(hc, ed, QTD_PID_OUT, QTD_TOGGLE_0, buffer, len)) {
                result = OS_FAIL;
            }
            leave_critical_section(ed->qh_lock);
            return result;
        }
    }
    usb_dbg(USB_ERROR, "ehci send bulk msg fail");
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_bool int_qtd_complete(struct ehci *hc, struct ehci_qtd *qtd)
{
    struct ehci_qtd *qtd_null;

    if (qtd->urb->app_complete) {
        qtd->urb->app_complete(qtd->data);
    }

    del_init_list(&qtd->urb_list);
    do {
        qtd_null = list_addr(qtd->qh->qtd.prev, struct ehci_qtd, qh_list);
        del_list(&qtd->qh_list);
    } while (0);

    /* copy the old info to new */
    qtd_null->hwinfo.alternate_next_qtd_pointer |= QTD_TERMINATE;
    //set_qtd_toggle(qtd_null->hwinfo.buffer_control, toggle);
    qtd_null->hwinfo.buffer_control |= QTD_CTRL_IOC;
    set_qtd_cp(qtd_null->hwinfo.buffer_control, 0);
    set_qtd_cerr(qtd_null->hwinfo.buffer_control, QTD_CERR_NUM);
    set_qtd_pid(qtd_null->hwinfo.buffer_control, get_qtd_pid(qtd->hwinfo.buffer_control));
    set_qtd_tbt(qtd_null->hwinfo.buffer_control, qtd->data_len);
    qtd_null->hwinfo.buffer_pointer[0] = virt_to_phys((pointer) qtd->data);
    qtd_null->hwinfo.appendix_64[0] = hc->appendix_64;
    /* ehci lock is used out of callback. */
    /* spin_lock(&hc->ehci_lock); */
    add_list_head(&hc->doing_list, &qtd_null->doing_list);
    /* spin_unlock(&hc->ehci_lock); */
    qtd_null->complete = int_qtd_complete;
    add_list_head(&qtd->urb->qtd, &qtd_null->urb_list);
    qtd_null->urb = qtd->urb;
    // add_list_tail(&qtd->qh->qtd, &qtd_null->qh_list);
    qtd_null->data = qtd->data;
    qtd_null->data_len = qtd->data_len;
    qtd_null->qh = qtd->qh;

    /* init new qtd null */
    qtd_null->hwinfo.next_qtd_pointer = virt_to_phys((pointer) &qtd->hwinfo);
    /* Using the selected pointer the host controller fetches the referenced qTD.
       If the fetched qTD has it’s Active bit set to a one,
       the host controller moves the pointer value used to reach the qTD (Next or Alternate Next) to the Current qTD Pointer field,
       then performs the overlay.
       If the fetched qTD has its Active bit set to a zero,
       the host controller aborts the queue advance and follows the queue head's horizontal pointer to the next schedule data structure. */
    init_ehci_qtd(qtd);
    add_list_tail(&qtd_null->qh->qtd, &qtd->qh_list); /* add null to tail */

    wmb();
    qtd_null->hwinfo.buffer_control &= (~QTD_STS_MASK);
    qtd_null->hwinfo.buffer_control |= QTD_STS_ACTIVE;

    return OS_TRUE;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret ehci_add_int_qtd(struct ehci *hc, struct ehci_qh *ed, os_u32 token, os_u32 toggle, os_u8 *buffer, os_u32 len, os_void (*complete)(os_u8 *data))
{
    struct ehci_urb *urb;
    struct ehci_qtd *qtd, *qtd_null;

    urb = alloc_urb();
    if (OS_NULL == urb) {
        usb_dbg(USB_ERROR, "alloc urb fail");
        return OS_FAIL;
    }
    urb->app_complete = complete;

    qtd_null = alloc_qtd_tail();
    if (OS_NULL == qtd_null) {
        usb_dbg(USB_ERROR, "alloc qtd fail");
        free_urb(urb);
        return OS_FAIL;
    }
    do {
        /* null qtd -> current qtd */
        qtd = list_addr(ed->qtd.prev, struct ehci_qtd, qh_list);
        /* insert qtd by transaction order */
        add_list_tail(&ed->qtd, &qtd_null->qh_list);
    } while (0);

    /* add to urb list */
    add_list_head(&urb->qtd, &qtd->urb_list);
    qtd->urb = urb;

    /* add to doing list */
    spin_lock(&hc->ehci_lock);
    add_list_head(&hc->doing_list, &qtd->doing_list);
    spin_unlock(&hc->ehci_lock);
    qtd->complete = int_qtd_complete;

    qtd->hwinfo.next_qtd_pointer = virt_to_phys((pointer) &qtd_null->hwinfo);
    qtd->hwinfo.alternate_next_qtd_pointer |= QTD_TERMINATE;
    set_qtd_toggle(qtd->hwinfo.buffer_control, toggle);
    qtd->hwinfo.buffer_control |= QTD_CTRL_IOC; /* use interrupt */
    set_qtd_cp(qtd->hwinfo.buffer_control, 0);
    set_qtd_cerr(qtd->hwinfo.buffer_control, QTD_CERR_NUM);
    set_qtd_pid(qtd->hwinfo.buffer_control, token);
    set_qtd_tbt(qtd->hwinfo.buffer_control, len);
    qtd->hwinfo.buffer_pointer[0] = virt_to_phys((pointer) buffer);
    qtd->hwinfo.appendix_64[0] = hc->appendix_64;

    cassert(MAX_QTD_TBT >= len); /* one qtd should be enough */
    urb->count = 1;
    qtd->data = buffer;
    qtd->data_len = len;
    qtd->qh = ed;

    /* start transfer */
    ed->hwinfo.overlay.next_qtd_pointer = virt_to_phys((pointer) &qtd->hwinfo);
    enable_qh(ed);
    qtd->hwinfo.buffer_control &= (~QTD_STS_HALT);
    qtd->hwinfo.buffer_control |= QTD_STS_ACTIVE;
    enable_ehci_schedule(hc, ed->type);
    return OS_SUCC;
}

/***************************************************************
 * description : add interrupt periodic transfer
 * history     :
 ***************************************************************/
LOCALC os_ret ehci_add_interrupt_trans(struct usb_device *usb, os_u8 pipe, os_u8 *data, os_uint len, os_u8 bInterval, os_void (*complete)(os_u8 *data))
{
    struct ehci *hc;
    struct ehci_qh *ed;
    os_u32 uframe;
    os_ret result;

    cassert((OS_NULL != usb) && (OS_NULL != data) && (0 != len) && (EP_NUM > pipe));

    if (OS_NULL != usb->host_controller) {
        hc = usb->host_controller;
        if ((OS_NULL != hc) && (OS_NULL != hc->endpoint[usb->usb_addr])) {
            /* 4.10 Managing Control/Bulk/Interrupt Transfers via Queue Heads */
            result = OS_SUCC;
            ed = alloc_ehci_qh(usb, usb->usb_addr, pipe, USB_INTERRUPT_TRANSFER, USB_IN);
            cassert(OS_NULL != ed);
            /* An endpoint for an interrupt pipe specifies its desired bus access period.
               A full-speed endpoint can specify a desired period from 1 ms to 255 ms.
               Low-speed endpoints are limited to specifying only 10 ms to 255 ms.
               High-speed endpoints can specify a desired period (2bInterval-1)x125 μs,
               where bInterval is in the range 1 to (including) 16. */
            if (USB_HIGH_SPEED == usb->speed) {
                cassert((1 <= bInterval) && (16 >= bInterval));
                uframe = power_of_2(bInterval - 1);
            } else {
                uframe = bInterval * 8;
            }
            ehci_insert_periodic_qh(hc, ed, uframe);
            cassert(0 != get_ep_ism(ed->hwinfo.capabilities));
            usb_dbg(USB_INFO, "ism: %d 0x%x", uframe, get_ep_ism(ed->hwinfo.capabilities));

            enter_critical_section(ed->qh_lock);
            if (OS_SUCC != ehci_add_int_qtd(hc, ed, QTD_PID_IN, QTD_TOGGLE_0, data, len, complete)) {
                usb_dbg(USB_ERROR, "ehci add interrupt pipe fail");
                result = OS_FAIL;
            }
            leave_critical_section(ed->qh_lock);
            return result;
        }
    }
    usb_dbg(USB_ERROR, "ehci add interrupt pipe, input para error");
    return OS_FAIL;
}

/***************************************************************
 * description : del interrupt periodic transfer
 * history     :
 ***************************************************************/
LOCALC os_ret ehci_del_interrupt_trans(struct usb_device *usb, os_u8 pipe)
{
    struct ehci *hc;
    struct ehci_qh *ed, *p;
    struct ehci_qtd *qtd;
    struct list_node *li, *_save;

    cassert(OS_NULL != usb);

    hc = usb->host_controller;
    ed = &hc->endpoint[usb->usb_addr][ehci_ed_index(pipe, USB_IN)]; /* 中断传输只支持in类型 */

    disable_qh(ed);
    p = list_addr(ed->qh.prev, struct ehci_qh, qh);
    p->hwinfo.qhhlp = ed->hwinfo.qhhlp;
    del_list(&ed->qh);
    wmb();
    delay_ms(1); // delay 1 micro-frame

    /* free qtd and delete ed safely */
    loop_del_list(li, _save, &ed->qtd) {
        qtd = list_addr(li, struct ehci_qtd, qh_list);

        /* delete from doing, done list, urb list. including null qtd. */
        spin_lock(&hc->ehci_lock);
        del_list(&qtd->doing_list);
        spin_unlock(&hc->ehci_lock);
        del_list(&qtd->urb_list);
        if ((OS_NULL != qtd->urb) && list_empty(&qtd->urb->qtd)) {
            free_urb(qtd->urb);
        }
        del_list(&qtd->qh_list);
        free_qtd(qtd);
    }
    free_ehci_qh(hc, ed);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_u8 alloc_ehci_device_addr(struct usb_device *usb)
{
    struct ehci *hc;

    cassert(OS_NULL != usb);
    hc = (struct ehci *) usb->host_controller;
    cassert(OS_NULL != hc);
    return alloc_usb_addr(hc->addr_bitmap);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret free_ehci_device_addr(struct usb_device *usb)
{
    struct ehci *hc;

    cassert(OS_NULL != usb);
    hc = (struct ehci *) usb->host_controller;
    cassert(OS_NULL != hc);
    free_usb_addr(hc->addr_bitmap, usb->usb_addr);
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void init_ehci_endpoint(struct ehci *hc, os_u32 addr)
{
    os_uint i;
    struct ehci_qh *ep;

    for (i = 0; i < EP_NUM * USB_DIR_CNT; i++) {
        ep = &hc->endpoint[addr][i];

        ep->qh_lock = OS_NULL;
        init_list_head(&ep->qh);
        init_list_head(&ep->qtd);
        ep->state = EHCI_ED_NEW;
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret alloc_ehci_endpoint(struct ehci *hc, os_u32 addr)
{
    if (OS_NULL == hc->endpoint[addr]) {
        hc->endpoint[addr] = cmalloc(EP_NUM * USB_DIR_CNT * sizeof(struct ehci_qh), EHCI_QH_ALIGN);
        if (OS_NULL == hc->endpoint[addr]) {
            return OS_FAIL;
        }
        mem_set(hc->endpoint[addr], 0, EP_NUM * USB_DIR_CNT * sizeof(struct ehci_qh));
        /* init */
        init_ehci_endpoint(hc, addr);
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret free_ehci_endpoint(struct ehci *hc, os_u32 addr)
{
    os_uint i, j;
    struct ehci_qh *endpoint, *qh;

    endpoint = hc->endpoint[addr];
    if (OS_NULL == endpoint) {
        return OS_SUCC;
    }

    /* unlink endpoint and free qtd */
    for (i = 0; i < USB_DIR_CNT; i++) {
        for (j = 0; j < EP_NUM; j++) {
            os_u32 convert[] = { USB_IN, USB_OUT };
            qh = &endpoint[ehci_ed_index(j, convert[i])];
            if (EHCI_ED_NEW != qh->state) {
                enter_critical_section(qh->qh_lock);
                switch (qh->type) {
                case USB_CTRL_TRANSFER:
                case USB_BULK_TRANSFER:
                default:
                    /* unlink qh, free qtd */
                    ehci_remove_async_qh(hc, qh);
                    break;
                case USB_ISO_TRANSFER:
                case USB_INTERRUPT_TRANSFER:
                    ehci_remove_periodic_qh(hc, qh);
                    break;
                }
                leave_critical_section(qh->qh_lock);
                free_ehci_qh(hc, qh);
            }
        }
    }

    cfree(endpoint);
    hc->endpoint[addr] = OS_NULL;

    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret create_device_endpoint(struct usb_device *usb)
{
    cassert((OS_NULL != usb) && (OS_NULL != usb->host_controller));
    return alloc_ehci_endpoint(usb->host_controller, usb->usb_addr);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret clear_device_endpoint(struct usb_device *usb)
{
    cassert((OS_NULL != usb) && (OS_NULL != usb->host_controller));
    return free_ehci_endpoint(usb->host_controller, usb->usb_addr);
}

/***************************************************************
 * description : update mps of control pipe
 * history     :
 ***************************************************************/
LOCALC os_ret update_ehci_default_endpoint(struct usb_device *usb)
{
    struct ehci *hc;
    struct ehci_qh *qh;

    cassert(OS_NULL != usb);
    hc = usb->host_controller;
    cassert(OS_NULL != hc);

    /* addr 0 endpoint 0 is in async schedule */
    qh = &hc->endpoint[usb->usb_addr][ehci_ed_index(0, USB_IN)];
    ep_mpl(qh->hwinfo.characteristics, usb->descriptor.bMaxPacketSize0);
    ep_da(qh->hwinfo.characteristics, usb->usb_addr);

    qh = &hc->endpoint[usb->usb_addr][ehci_ed_index(0, USB_OUT)];
    ep_mpl(qh->hwinfo.characteristics, usb->descriptor.bMaxPacketSize0);
    ep_da(qh->hwinfo.characteristics, usb->usb_addr);
    return OS_SUCC;
}

/***************************************************************
 * description : addr 0
 * history     :
 ***************************************************************/
LOCALC os_ret reset_ehci_default_endpoint(struct usb_device *usb)
{
    struct ehci *hc;
    struct ehci_qh *qh;

    cassert(OS_NULL != usb);
    hc = usb->host_controller;
    cassert(OS_NULL != hc);

    /* addr 0 endpoint 0 is in async schedule */
    qh = &hc->endpoint[0][ehci_ed_index(0, USB_IN)];
    if (EHCI_ED_NEW != qh->state) {
        enter_critical_section(qh->qh_lock);
        ehci_remove_async_qh(hc, qh);
        leave_critical_section(qh->qh_lock);
        free_ehci_qh(hc, qh);
    }

    qh = &hc->endpoint[0][ehci_ed_index(0, USB_OUT)];
    if (EHCI_ED_NEW != qh->state) {
        enter_critical_section(qh->qh_lock);
        ehci_remove_async_qh(hc, qh);
        leave_critical_section(qh->qh_lock);
        free_ehci_qh(hc, qh);
    }

    /* reset control pipe status */
    init_ehci_endpoint(usb->host_controller, 0);
    return OS_SUCC;
}

LOCALD const struct usb_host_controller_operations ehci_operation = {
    ehci_recv_control_msg,
    ehci_send_control_msg,
    ehci_recv_bulk_msg,
    ehci_send_bulk_msg,
    ehci_add_interrupt_trans,
    ehci_del_interrupt_trans,

    alloc_ehci_device_addr,
    free_ehci_device_addr,
    OS_NULL,

    OS_NULL, // endpoints are allocated in add_usb_default_endpoint()
    OS_NULL, // endpoints are freed in del_usb_default_endpoint()

    create_device_endpoint,
    clear_device_endpoint,
    update_ehci_default_endpoint
};

/***************************************************************
 * description : 4.10.2 Advance Queue
 *               If Next qTD Pointer’s T-bit is set to a one, then the host controller exits this state and uses the
 *               horizontal pointer to the next schedule data structure.
 * history     :
 ***************************************************************/
LOCALC os_void init_ehci_interrupt_list(struct ehci_qh *qh_int)
{
    os_uint i, start;
    os_u32 interval; /* how many uframes */
    os_u8 uframe_mask;

    uframe_mask = 0x1;
    interval = INTERRUPT_FRAME_LIST_NUM;
    start = 0;
    for (i = 0; i < (INTERRUPT_FRAME_LIST_NUM * 2 - 1); i++) {
        if ((i - start) >= interval) {
            if (uframe_mask) {
                uframe_mask <<= 1;
            } else {
                uframe_mask = 0x1;
            }
            interval /= 2;
            start = i;
        }

        qh_int[i].int_interval = interval * 8; /* 8 uframes in every frame */
        set_qhhlp(qh_int[i].hwinfo.qhhlp, virt_to_phys((pointer) &qh_int[((i - start) / 2) + start + interval].hwinfo), EHCI_QH, QH_VALID);
        qh_int[i].hwinfo.overlay.next_qtd_pointer = QTD_TERMINATE;
        qh_int[i].hwinfo.characteristics = 0;
        ep_hbpm(qh_int[i].hwinfo.capabilities, EP_MULT_ONE);
        ep_ism(qh_int[i].hwinfo.capabilities, uframe_mask);
        qh_int[i].load_cnt = 0;
        init_list_head(&qh_int[i].qh);
        qh_int[i].state = EHCI_ED_BUTT;
    }
    usb_dbg(USB_INFO, "init_ehci_interrupt_list %d %d %d", i, start, interval);

    interval = 4;
    for (; i < INT_ENDPOINT_NULL_NUM - 1; i++) {
        qh_int[i].int_interval = interval; /* x uframes in every frame */
        interval /= 2;
        set_qhhlp(qh_int[i].hwinfo.qhhlp, virt_to_phys((pointer) &qh_int[i + 1].hwinfo), EHCI_QH, QH_VALID);
        qh_int[i].hwinfo.overlay.next_qtd_pointer = QTD_TERMINATE;
        qh_int[i].hwinfo.characteristics = 0;
        ep_hbpm(qh_int[i].hwinfo.capabilities, EP_MULT_ONE);
        ep_ism(qh_int[i].hwinfo.capabilities, uframe_mask);
        qh_int[i].load_cnt = 0;
        init_list_head(&qh_int[i].qh);
        qh_int[i].state = EHCI_ED_BUTT;
    }
    ep_ism(qh_int[i-4].hwinfo.capabilities, 0x40); /* 01000000b, 8 uframe */
    ep_ism(qh_int[i-3].hwinfo.capabilities, 0x11); /* 00010001b, 4 uframe */
    ep_ism(qh_int[i-2].hwinfo.capabilities, 0xaa); /* 10101010b, 2 uframe */
    ep_ism(qh_int[i-1].hwinfo.capabilities, 0xff); /* 11111111b, 1 uframe */

    /* the last one, i = INT_ENDPOINT_NULL_NUM - 1 */
    qh_int[i].int_interval = UINT32_MAX;
    qh_int[i].hwinfo.qhhlp = QH_INVALID; /* end */
    qh_int[i].hwinfo.overlay.next_qtd_pointer = QTD_TERMINATE;
    qh_int[i].hwinfo.characteristics = 0;
    ep_hbpm(qh_int[i].hwinfo.capabilities, EP_MULT_ONE);
    qh_int[i].load_cnt = 0;
}

LOCALD os_u32 ehci_int_table[INTERRUPT_FRAME_LIST_NUM] = { 0 };
LOCALD os_u32 ehci_int_table_index = 0;

/***************************************************************
 * description : The periodic frame list is a 4K-page aligned array of Frame List Link pointers. The length of the frame list
                 may be programmable. The programmability of the periodic frame list is exported to system software via the
                 HCCPARAMS register. If non-programmable, the length is 1024 elements. If programmable, the length can
                 be selected by system software as one of 256, 512, or 1024 elements.
 * history     :
 ***************************************************************/
LOCALC os_ret alloc_ehci_periodic_schedule_resource(struct ehci *hc)
{
    os_u32 *periodic_frame_list_base;
    struct ehci_qh *qh_int;
    os_uint i;

    periodic_frame_list_base = cmalloc(PERIODIC_FRAME_LIST_NUM * sizeof(fllp_type), 4*1024);
    if (OS_NULL == periodic_frame_list_base) {
        usb_dbg(USB_ERROR, "alloc ehci periodic fame list fail");
        return OS_FAIL;
    }
    /* The least significant bit is the T-Bit (bit 0).
       When this bit is set to a one,
       the host controller will never use the value of the frame list pointer as a physical memory pointer. */
    for (i = 0; i < PERIODIC_FRAME_LIST_NUM; i++) {
        periodic_frame_list_base[i] = QH_INVALID;
    }

    hc->PERIODICLISTBASE = periodic_frame_list_base;
    hc->periodic_list_size = PERIODIC_FRAME_LIST_NUM;

    /* alloc schedule null endpoint, number is 1024 * 2 - 1. */
    qh_int = cmalloc(INT_ENDPOINT_NULL_NUM * sizeof(struct ehci_qh), EHCI_QH_ALIGN);
    if (OS_NULL == qh_int) {
        usb_dbg(USB_ERROR, "alloc interrupt endpoint fail");
        cfree(periodic_frame_list_base);
        return OS_FAIL;
    }
    mem_set(qh_int, 0, INT_ENDPOINT_NULL_NUM * sizeof(struct ehci_qh));

    /* init int endpoint */
    init_ehci_interrupt_list(qh_int);

    hc->qh_int = qh_int;

    /* link to int list */
    for (i = 0; i < INTERRUPT_FRAME_LIST_NUM; i++) {
        set_qhhlp(periodic_frame_list_base[i * (PERIODIC_FRAME_LIST_NUM / INTERRUPT_FRAME_LIST_NUM)], virt_to_phys((pointer) &qh_int[ehci_int_table[i]]), EHCI_QH, QH_VALID); // the same as qhhlp
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret alloc_ehci_async_schedule_resource(struct ehci *hc)
{
    hc->qtd_null= OS_NULL;
    hc->async_head = OS_NULL;
    hc->doorbell_sem = OS_NULL;

    hc->qtd_null = alloc_qtd_tail();
    if (OS_NULL == hc->qtd_null) {
        usb_dbg(USB_ERROR, "alloc ehci qtd_null fail");
        goto fail;
    }
    /* use qtd_null qtd default status. */
    hc->qtd_null->hwinfo.buffer_control = QTD_STS_HALT;

    /* async tail node, refer to 4.8.3 Empty Asynchronous Schedule Detection */
    hc->async_head = cmalloc(sizeof(struct ehci_qh), EHCI_QH_ALIGN);
    if (OS_NULL == hc->async_head) {
        usb_dbg(USB_ERROR, "alloc ehci async head fail");
        goto fail;
    }
    mem_set(hc->async_head, 0, sizeof(struct ehci_qh));

    init_list_head(&hc->async_head->qh);
    init_list_head(&hc->async_head->qtd);

    /* terminate is ignored by hc when the queue head is in the async schedule */
    set_qhhlp(hc->async_head->hwinfo.qhhlp, virt_to_phys((pointer) &hc->async_head->hwinfo), EHCI_QH, QH_VALID);

    /* characteristics */
    ep_hrlf(hc->async_head->hwinfo.characteristics, 1); /* head of relamation */
    ep_ncr(hc->async_head->hwinfo.characteristics, 0);
    ep_cef(hc->async_head->hwinfo.characteristics, 0);
    ep_mpl(hc->async_head->hwinfo.characteristics, DEFAULT_MAX_PACKET_SIZE);
    ep_dtc(hc->async_head->hwinfo.characteristics, 1); /* 1b Initial data toggle comes from incoming qTD DT bit. Host controller replaces DT bit in the queue head from the DT bit in the qTD. */
    ep_eps(hc->async_head->hwinfo.characteristics, EHCI_CHA_HIGH_SPEED);
    ep_endpt(hc->async_head->hwinfo.characteristics, 0);
    /* This field is only valid when the queue head is in the Periodic Schedule and the EPS field
       indicates a Full or Low-speed endpoint. Setting this bit to a one when the queue head is in
       the Asynchronous Schedule or the EPS field indicates a high-speed device yields
       undefined results. */
    ep_iont(hc->async_head->hwinfo.characteristics, 0);
    ep_da(hc->async_head->hwinfo.characteristics, 0); /* addr is invalid */

    /* capabilities */
    ep_hbpm(hc->async_head->hwinfo.capabilities, EP_MULT_ONE);
    ep_pn(hc->async_head->hwinfo.capabilities, 0);
    ep_ha(hc->async_head->hwinfo.capabilities, 0);
    /* This field is ignored by the host controller
       unless the EPS field indicates this device is a low- or full-speed device and this queue
       head is in the periodic list. */
    ep_scm(hc->async_head->hwinfo.capabilities, 0);
    /* This field is used for all endpoint speeds.
       Software should set this field to a zero when the queue head is on the asynchronous schedule. */
    ep_ism(hc->async_head->hwinfo.capabilities, 0);

    /* QTD_STS_ACTIVE is zero */
    hc->async_head->hwinfo.overlay.buffer_control |= QTD_STS_HALT;
    hc->async_head->hwinfo.overlay.next_qtd_pointer |= QTD_TERMINATE;

    hc->doorbell_flag = 0;
    hc->doorbell_sem = create_event_handle(EVENT_INVALID, "ehci db", __LINE__);
    if (OS_NULL == hc->doorbell_sem) {
        usb_dbg(USB_ERROR, "alloc ehci doorbell sem fail");
        goto fail;
    }

    return OS_SUCC;
  fail:
    if (hc->async_head) cfree(hc->async_head);
    if (hc->qtd_null) free_qtd(hc->qtd_null);
    if (hc->doorbell_sem) destroy_event_handle(hc->doorbell_sem);
    return OS_FAIL;
}

/***************************************************************
 * description : 释放ehci分配的资源
 * history     :
 ***************************************************************/
LOCALC os_void *free_ehci(struct ehci *hc)
{
    os_uint i;

    for (i = 0; i < USB_DEVICE_COUNT; i++) {
        if (OS_NULL != hc->endpoint[i]) {
            cfree(hc->endpoint[i]);
            hc->endpoint[i] = OS_NULL;
        }
    }
    if (hc->PERIODICLISTBASE) cfree(hc->PERIODICLISTBASE);
    if (hc->qh_int) cfree(hc->qh_int);
    if (hc->async_head) cfree(hc->async_head);
    if (hc->qtd_null) free_qtd(hc->qtd_null);
    if (hc->doorbell_sem) destroy_event_handle(hc->doorbell_sem);

    kfree(hc);
    return OS_NULL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC struct ehci *alloc_ehci(HDEVICE dev)
{
    struct ehci *hc;
    os_ret result;

    hc = kmalloc(sizeof(struct ehci));
    if (OS_NULL == hc) {
        usb_dbg(USB_ERROR, "alloc ehci fail");
        return OS_NULL;
    }
    mem_set(hc, 0, sizeof(struct ehci));
    hc->pci = dev; /* 记录pci信息 */

    hc->done_flag = 0;

    init_spinlock(&hc->ehci_lock);

    /* alloc resource for period schedule */
    result = alloc_ehci_periodic_schedule_resource(hc);
    if (OS_SUCC != result) {
        usb_dbg(USB_ERROR, "alloc ehci periodic resource fail");
        goto cleanup;
    }

    /* alloc resource for async schedule */
    result = alloc_ehci_async_schedule_resource(hc);
    if (OS_SUCC != result) {
        usb_dbg(USB_ERROR, "alloc ehci asnyc resource fail");
        goto cleanup;
    }

    hc->done_flag = OS_FALSE;
    init_list_head(&hc->doing_list);
    mem_set(hc->rh_dev, OS_NULL, EHCI_RH_PORTS_CNT * sizeof(struct usb_device *));

    return hc;
  cleanup:
    free_ehci_endpoint(hc, 0);
    if (hc) free_ehci(hc);
    return OS_NULL;
}

/***************************************************************
 * description : ehci_0a.pdf 2.1.3 USBBASE. Register Space Base Address Register
 * history     :
 ***************************************************************/
LOCALC os_ret save_ehci_regs_addr(struct ehci *hc, os_u32 addr)
{
    os_u32 temp;

    ehci_out(&hc->regs.caps, addr & 0xffffff00);

    /* The operational register base must be DWord aligned and is
       calculated by adding the value in the first capabilities register (CAPLENGTH, Section 2.2.1) to the base
       address of the enhanced host controller register address space. All registers are 32 bits in length. Software
       should read and write these registers using only DWord accesses. */
    temp = (addr & 0xffffff00) + hc->regs.caps->CAPLENGTH;
    ehci_out(&hc->regs.operation, (temp & 0xfffffffc) + ((temp % 4)?(4):(0)));

    if (addr & 0x4) {
        /* May be mapped into 64-bit addressing space. */
        usb_dbg(USB_ERROR, "do not support 64 bit ehci");
        cassert(OS_FALSE);
        return OS_FAIL;
    } else {
        /* May only be mapped into 32-bit addressing space (Recommended). */
        return OS_SUCC;
    }
}

/***************************************************************
 * description : Number of Companion Controller
 * history     :
 ***************************************************************/
LOCALC os_u8 get_ehci_ncc(struct ehci *hc)
{
    return (ehci_in(&hc->regs.caps->HCSPARAMS) >> 12) & 0x0f;
}

/***************************************************************
 * description : Number of Ports per Companion Controller
 * history     :
 ***************************************************************/
LOCALC os_u8 get_ehci_npcc(struct ehci *hc)
{
    /* For example, if N_PORTS has a value of 6 and N_CC has a value of 2 then N_PCC could have a value of 3.
       The convention is that the first N_PCC ports are assumed to be routed to companion controller 1,
       the next N_PCC ports to companion controller 2, etc.
       In the previous example, the N_PCC could have been 4,
       where the first 4 are routed to companion controller 1 and the last two are routed to companion controller 2. */
    return (ehci_in(&hc->regs.caps->HCSPARAMS) >> 8) & 0x0f;
}

/***************************************************************
 * description : 4.1 Host Controller Initialization
 * history     :
 ***************************************************************/
LOCALC os_ret request_ehci_ownership(struct ehci *hc)
{
    os_u32 offset; /* offset of extended capibilities */
    os_u32 eecp;
    os_u32 times;

    offset = ehci_cp_eecp(ehci_in(&hc->regs.caps->HCCPARAMS));
    if (0 == offset) {
        /* A non-zero value in this register indicates the
           offset in PCI configuration space of the first EHCI extended capability. */
        return OS_SUCC;
    } else if (0x40 <= offset) {
        /* A non-zero value in this register indicates the offset in PCI configuration space of the first EHCI extended capability.
           The pointer value must be 40h or greater if implemented to maintain the consistency of the PCI header defined for this class of device. */
        do {
            eecp = in_pci_reg(hc->pci, offset);
            switch (EHCI_EECP_CAP_ID & eecp) {
            case 1:
                out_pci_reg(hc->pci, offset, EHCI_EECP_HOOS | eecp);
                times = 10;
                while (times) {
                    times--;
                    if (0 == (EHCI_EECP_HBOS & in_pci_reg(hc->pci, offset))) {
                        return OS_SUCC;
                    }
                    delay_ms(100);
                }
                break;

            case 0:
            default:
                usb_dbg(USB_ERROR, "error cap id = %d", EHCI_EECP_CAP_ID & eecp);
                break;
            }
            offset = ehci_eecp_neecp(eecp);
        } while (0 != offset);
        return OS_FAIL;
    } else {
        return OS_FAIL;
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret reset_ehci(struct ehci *hc)
{
    os_uint i;
    struct ehci_operational_regs *operation;

    operation = hc->regs.operation;

    /* stop and halt hc */
    ehci_out(&operation->USBCMD, (~EHCI_CMD_RS) & ehci_in(&operation->USBCMD));
    delay_ms(100);
    /* The Host Controller must halt within 16 micro-frames after software clears the Run bit. */
    if (0 != (EHCI_STS_HALT & ehci_in(&operation->USBSTS))) {
        /* refer to table 2-9.
           Software should not set this bit to a one when the HCHalted bit in the USBSTS register
           is a zero. Attempting to reset an actively running host controller will result in undefined
           behavior. */
        ehci_out(&operation->USBCMD, EHCI_CMD_RESET);
        i = 10;
        while (i) {
            if (0 == (EHCI_CMD_RESET & ehci_in(&operation->USBCMD))) {
                return OS_SUCC;
            }
            delay_ms(10);
        }
        return OS_FAIL;
    }
    return OS_FAIL;
}

/***************************************************************
 * description : 4.1 Host Controller Initialization
 * history     :
 ***************************************************************/
LOCALC os_ret init_ehci(struct ehci *hc)
{
    /* 1. Program the CTRLDSSEGMENT register with 4-Gigabyte segment where all of the interface data structures are allocated */
    if (0 != (EHCI_CP_64AC & ehci_in(&hc->regs.caps->HCCPARAMS))) {
#define EHCI_64_ADDR UINT32_C(0)
        hc->appendix_64 = EHCI_64_ADDR;
        /* This 32-bit register corresponds to the most significant address bits [63:32] for all EHCI data structures.
           This register allows the host software to locate all control data structures within the same 4 Gigabyte
           memory segment.*/
        ehci_out(&hc->regs.operation->CTRLDSSEGMENT, 0);
    } else {
        hc->appendix_64 = 0;
        /* If the 64-bit Addressing Capability field in HCCPARAMS is a zero, then this register is not used. */
    }

    /* 2. Write the appropriate value to the USBINTR register to enable the appropriate interrupts. */
    ehci_out(&hc->regs.operation->USBINTR, EHCI_INT_MASK);

    /* 3. Write the base address of the Periodic Frame List to the PERIODICLIST BASE register. If there are no work items in the periodic schedule, all elements of the Periodic Frame List should have their T-Bits set to a one. */
    ehci_out(&hc->regs.operation->PERIODICLISTBASE, virt_to_phys((os_u32) hc->PERIODICLISTBASE));

    ehci_out(&hc->regs.operation->ASYNCLISTADDR, virt_to_phys((os_u32) &hc->async_head->hwinfo));

    /* 4. Write the USBCMD register to set the desired interrupt threshold, frame list size (if applicable) and turn the host controller ON via setting the Run/Stop bit. */
    ehci_out(&hc->regs.operation->USBCMD, EHCI_CMD_RS | ehci_in(&hc->regs.operation->USBCMD)); // 1ms interrupt, 1024 frame

    /* 5. Write a 1 to CONFIGFLAG register to route all ports to the EHCI controller (see Section 4.2). */
    ehci_out(&hc->regs.operation->CONFIGFLAG, 0x1);

    /* 6. unblock post write */
    ehci_in(&hc->regs.operation->USBCMD);

    /* linux: After we set CF, a short delay lets the hardware catch up */
    delay_ms(10);

    return OS_SUCC;
}

/***************************************************************
 * description : N_PORTS
 * history     :
 ***************************************************************/
LOCALC inline os_u8 get_ehci_nports(struct ehci *hc)
{
    return (0x0f & ehci_in(&hc->regs.caps->HCSPARAMS));
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret enable_ehci_port_power(struct ehci *hc, os_u32 port)
{
    /* If the port has port power control, software cannot change the state of the port until after it applies power to
       the port by setting port power to a 1. Software must not attempt to change the state of the port until after
       power is stable on the port. The host is required to have power stable to the port within 20 milliseconds of
       the zero to one transition. */

    /* hcsparams is ro */
    if (0 == (EHCI_SP_PPC & ehci_in(&hc->regs.caps->HCSPARAMS))) {
        /* Port Power Control (PPC). This field indicates whether the host controller
           implementation includes port power control. A one in this bit indicates the ports have
           port power switches. A zero in this bit indicates the port do not have port power
           switches. The value of this field affects the functionality of the Port Power field in each
           port status and control register (see Section 2.3.8). */
        /* host controller does not have port power control switches. */
        /* each port is hard-wired to power. */
    } else {
        ehci_out(&hc->regs.operation->PORTSC[port], EHCI_PORTSC_PP | ehci_in(&hc->regs.operation->PORTSC[port]));
        delay_ms(20);
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret ehci_probe(HDEVICE pci)
{
    struct ehci *hc;
    os_u8 port_num, i;
    os_ret result;

    usb_dbg(USB_INFO, "ehci probe");

    cassert(OS_NULL != pci);

    /* init for fail label */
    hc = OS_NULL;

    hc = alloc_ehci(pci);
    if (OS_NULL == hc) {
        usb_dbg(USB_ERROR, "alloc ehci instance fail");
        goto fail;
    }
    set_pci_dedicated(pci, hc);

    /* 获取ehci第0个内存映射基地址, 保存ehci寄存器地址. */
    result = save_ehci_regs_addr(hc, get_pci_dev_bar(pci, 0));
    if (OS_SUCC != result) {
        usb_dbg(USB_ERROR, "save ehci register address fail");
        goto fail;
    }

    if (0 != get_ehci_ncc(hc)) {
        usb_dbg(USB_INFO, "ehci companion host controller exist, port count %d", get_ehci_npcc(hc));
        /* 如果存在兼容控制器, roothub优先使用兼容控制器.
           如果没有兼容控制器, 只能使用ehci的hub来兼容1.1设备.
        goto fail; */
    }

    /* 开启总线竞争能力, 使得ehci可以读写内存 */
    enable_pci_dma(pci);

    /* 安装pci设备中断 */
    result = enable_pci_int(pci);
    if (OS_SUCC != result) {
        usb_dbg(USB_ERROR, "pci device install interrupt fail");
        goto fail;
    }

    /* bugfix. refer to 5. ehci extended capabilities */
    result = request_ehci_ownership(hc);
    if (OS_SUCC != result) {
        usb_dbg(USB_ERROR, "ehci request ownership fail");
        goto fail;
    }

    /* 复位控制器 */
    result = reset_ehci(hc);
    if (OS_SUCC != result) {
        usb_dbg(USB_ERROR, "reset ehci fail");
        goto fail;
    }

    /* 初始化控制器 */
    result = init_ehci(hc);
    if (OS_SUCC != result) {
        usb_dbg(USB_ERROR, "init ehci fail");
        goto fail;
    }

    /* 端口电源 */
    port_num = get_ehci_nports(hc);
    usb_dbg(USB_INFO, "ehci port num: %d", port_num);
    for (i = 0; i < port_num; i++) {
        enable_ehci_port_power(hc, i);
    }

    return OS_SUCC;
  fail:
    usb_dbg(USB_ERROR, "ehci probe fail");
    disable_pci_int(pci);
    if (OS_NULL != hc) {
        free_ehci(hc);
    }
    return OS_FAIL;
}

/***************************************************************
 * description : the qh without qtd is head
 * history     :
 ***************************************************************/
LOCALC os_void dump_async_info(struct ehci *hc)
{
    struct ehci_qh *ed, *tmp;
    struct ehci_qtd *qtd;

    print("async info (0x%x):", hc->regs.operation->USBSTS);
    tmp = ed = (struct ehci_qh *) phys_to_virt(ehci_in(&hc->regs.operation->ASYNCLISTADDR));
    do {
        print("(0x%x)\n0x%x 0x%x)", tmp, tmp->hwinfo.curr_qtd_pointer, ((struct ehci_qtd *) tmp->hwinfo.curr_qtd_pointer)->hwinfo.buffer_control);
        qtd = (struct ehci_qtd *) phys_to_virt(tmp->hwinfo.overlay.next_qtd_pointer);
        while (!((os_u32) qtd & QTD_TERMINATE)) {
            print("\n0x%x 0x%x", qtd, qtd->hwinfo.buffer_control);
            qtd = (struct ehci_qtd *) phys_to_virt(qtd->hwinfo.next_qtd_pointer);
        } print("\n");

        tmp = (struct ehci_qh *)(0xffffffe0L & phys_to_virt(tmp->hwinfo.qhhlp));
    } while (tmp != ed);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void dump_periodic_info(struct ehci *hc)
{
    os_uint i;

    /* travel */
    flog("periodic info:");
    for (i = 0; i < PERIODIC_FRAME_LIST_NUM; i++) {
        struct ehci_qh *qh;
        os_u32 cnt;
        cnt = 0;
        qh = (struct ehci_qh *)(phys_to_virt(hc->PERIODICLISTBASE[i]) & 0xffffffe1L);
        while (QH_INVALID != (0x1 & (pointer) qh)) {
            if (EHCI_ED_NEW == qh->state) {
                flog("%d ", qh->int_interval);
            }
            qh = (struct ehci_qh *)(phys_to_virt(qh->hwinfo.qhhlp) & 0xffffffe1L);
            cnt++;
        }
        if (11 != cnt) {
            flog("[%d %d]", i, cnt);
        }
    } flog("\n");
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void proc_ehci_hw_interrupt(struct ehci *hc, os_u32 mask)
{
    if ((EHCI_STS_UEI | EHCI_STS_UI) & mask) { /* normal [4.15.1.2] or error [4.15.1.1] completion, ehci-r10.pdf */
        hc->done_flag = 1;
    }

    if (EHCI_STS_PCD & mask) {
        struct ehci_rh_msg *msg;
        os_ret ret;

        print("ehci port change\n");
        msg = alloc_msg(sizeof(struct ehci_rh_msg));
        msg->head.msg_name = EHCI_RH_MSG_ID;
        msg->head.msg_len = sizeof(struct ehci_rh_msg);
        msg->hc = hc;
        lock_schedule();
        ret = post_thread_msg(ehci_rh_handle, msg);
        unlock_schedule();
    }

    if (EHCI_STS_FLR & mask) {
        usb_dbg(USB_INFO, "ehci frame list rollover");
    }

    if (EHCI_STS_HSE & mask) {
        print("ehci host sys error!\n");
        dump_async_info(hc);
        dump_periodic_info(hc);
    }

    if (EHCI_STS_IAAD & mask) {
        hc->doorbell_flag = 1;
        usb_dbg(USB_INFO, "ehci async advance");
    }
}

/***************************************************************
 * description : pci共享中断的上半部
 * history     :
 ***************************************************************/
LOCALC os_ret ehci_interrupt_1(HDEVICE pci)
{
    struct ehci *hc;
    struct ehci_operational_regs *operation;
    os_u32 int_mask;
    os_u32 int_bak;

    cassert(OS_NULL != pci);

    hc = get_pci_dedicated(pci);
    if (OS_NULL != hc) {
        /* 判断共享中断是否属于current ehci hc */
        operation = hc->regs.operation;
        if (0 == (int_mask = ehci_in(&operation->USBSTS) & EHCI_INT_MASK)) {
            return OS_FAIL;
        }

        /* 关闭usb中断 */
        int_bak = ehci_in(&operation->USBINTR);
        ehci_out(&operation->USBINTR, 0);

        /* 清除usb中断状态 */
        ehci_out(&operation->USBSTS, int_mask);
        wmb();

        /* 处理中断 */
        proc_ehci_hw_interrupt(hc, int_mask);

        /* 打开usb中断 */
        ehci_out(&operation->USBINTR, int_bak);

        // iret also makes sequence coherency
        return OS_SUCC;
    }

    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret proc_ehci_sw_interrupt(struct ehci *hc)
{
    struct list_node *list, *_save;
    struct ehci_qtd *qtd;
    os_u32 token;
    os_bool need_sch;
    os_ret ret;

    ret = OS_FAIL;
    need_sch = OS_FALSE;

    spin_lock(&hc->ehci_lock);
    lock_schedule();
    do {
        if (1 == hc->done_flag) {
            hc->done_flag = 0;
            ret = OS_SUCC;

            loop_del_list(list, _save, &hc->doing_list) {
                qtd = list_addr(list, struct ehci_qtd, doing_list);
                /* Halted.
                   Set to a 1 by the Host Controller during status updates to indicate that a serious error has occurred at the device/endpoint addressed by this qTD.
                   This can be caused by babble, the error counter counting down to zero,
                   or reception of the STALL handshake from the device during a transaction.
                   Any time that a transaction results in the Halted bit being set to a one,
                   the Active bit is also set to 0. */
                token = qtd->hwinfo.buffer_control;
                if ((0 == (QTD_STS_ACTIVE & token))
                 || (0 != (QTD_STS_HALT & token))) {
                    del_init_list(&qtd->doing_list);

                    if (qtd->complete) {
                        if (qtd->complete(hc, qtd)) {
                            need_sch = OS_TRUE;
                        }
                    } else {
                        cassert(OS_FALSE);
                    }
                }
            }
        }

        /* doorbell handle should after done list handle! */
        if (1 == hc->doorbell_flag) {
            hc->doorbell_flag = 0;
            notify_event(hc->doorbell_sem, __LINE__);
            need_sch = OS_FALSE;
        }
    } while (0);
    unlock_schedule();
    spin_unlock(&hc->ehci_lock);

    if (need_sch) {
        schedule();
    }
    return ret;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret ehci_interrupt_2(HDEVICE pci)
{
    struct ehci *hc;

    cassert(OS_NULL != pci);

    hc = get_pci_dedicated(pci);
    if (OS_NULL != hc) {
        return proc_ehci_sw_interrupt(hc);
    }
    return OS_FAIL;
}

/* 通用ehci设备信息 */
LOCALD const struct pci_device_id pci_ehci_ids = {
    /* no matter who makes it */
    PCI_ANY_ID, PCI_ANY_ID,
    /* handle any USB EHCI controller */
    PCI_CLASS_SERIAL_USB_EHCI, ~0
};

/* 通用ehci设备驱动表 */
LOCALD const struct pci_driver pci_ehci_driver = {
    "usb-ehci",
    &pci_ehci_ids,
    ehci_probe,
    OS_NULL,
    ehci_interrupt_1,
    ehci_interrupt_2
};

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_bool ehci_port_device_exist(struct ehci *hc, os_u32 index)
{
    /* 1=Device is present on port. 0=No device is present. */
    if (0 == (EHCI_PORTSC_CCS & ehci_in(&hc->regs.operation->PORTSC[index]))) {
        return OS_FALSE;
    } else {
        return OS_TRUE;
    }
}

/***************************************************************
 * description : refer to 4.2.2 Port Routing Control via PortOwner and Disconnect Event
 * history     :
 ***************************************************************/
LOCALC enum usb_speed_type check_device_speed(struct ehci *hc, os_u32 index)
{
    enum usb_speed_type speed;
    os_uint times;

    /* clear the connect change, reset and enable the port. */
    ehci_out(&hc->regs.operation->PORTSC[index], EHCI_PORTSC_CSC | ehci_in(&hc->regs.operation->PORTSC[index]));

    /* When the EHCI Driver receives the request to reset and enable the port,
       it first checks the value reported by the LineStatus bits in the PORTSC register. */
    switch (EHCI_PORTSC_LS & ehci_in(&hc->regs.operation->PORTSC[index])) {
    case 0x1: /* K-state, Low-speed device, release ownership of port */
        speed = USB_LOW_SPEED;
        goto end;
        break;

    case 0x0: /* SE0 */
    case 0x2: /* J-state */
    default: /* 0x3, Undefined */
        ehci_out(&hc->regs.operation->PORTSC[index], (~EHCI_PORTSC_PED) & (EHCI_PORTSC_PR | ehci_in(&hc->regs.operation->PORTSC[index])));
        delay_ms(50);
        ehci_out(&hc->regs.operation->PORTSC[index], (~EHCI_PORTSC_PR) & ehci_in(&hc->regs.operation->PORTSC[index]));

        times = 10;
        while ((0 != (EHCI_PORTSC_PR & ehci_in(&hc->regs.operation->PORTSC[index]))) && (0 != times)) {
            times--;
            delay_ms(2);
        }
        if (0 == times) {
            usb_dbg(USB_ERROR, "reset port fail");
            speed = USB_FULL_SPEED;
            goto end;
        }

        /* reset complete */
        if (0 != (EHCI_PORTSC_PED & ehci_in(&hc->regs.operation->PORTSC[index]))) {
            /* high speed device */
            return USB_HIGH_SPEED;
        } else {
            speed = USB_FULL_SPEED;
            goto end;
        }

        break;
    }

  end:
    times = 10;
    while (!(EHCI_PORTSC_PO & ehci_in(&hc->regs.operation->PORTSC[index])) && (0 != times)) {
        times--;
        /* release ownership of port */
        ehci_out(&hc->regs.operation->PORTSC[index], (~(EHCI_PORTSC_PED | EHCI_PORTSC_CSC | EHCI_PORTSC_PEDC)) & (EHCI_PORTSC_PO | ehci_in(&hc->regs.operation->PORTSC[index])));
        delay_ms(5);
    }
    return speed;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void ehci_rh_status_change(struct ehci *hc)
{
    struct usb_device *usb;
    enum usb_speed_type speed;
    os_u8 port_cnt, i;
    os_u32 status;
    os_ret result;

    /* Now that the host knows the port to which the new device has been attached,
       the host then waits for at least 100 ms to allow completion of an insertion process and for power at the device to become stable. */
    delay_ms(100);

    port_cnt = get_ehci_nports(hc);
    usb_dbg(USB_INFO, "ehci root hub port count %d", port_cnt);
    for (i = 0; i < port_cnt; i++) {
        status = ehci_in(&hc->regs.operation->PORTSC[i]);
        if (0 != (EHCI_PORTSC_CSC & status)) {
            ehci_out(&hc->regs.operation->PORTSC[i], EHCI_PORTSC_CSC | status);

            /* 查看端口是否有设备连接 */
            if (ehci_port_device_exist(hc, i)) {
                usb_dbg(USB_INFO, "device is present on port %d", i);

                if (status & EHCI_PORTSC_PO) {
                    continue;
                }

                /* 复位设备, 检测速度类型 */
                speed = check_device_speed(hc, i);
                if (USB_HIGH_SPEED != speed) {
                    /* roothub中不支持低速设备和全速设备 */
                    usb_dbg(USB_INFO, "usb device speed %d", speed);
                    continue;
                }

                /* 分配usb设备实体 */
                usb = alloc_usb_device(hc, speed, &ehci_operation);
                if (OS_NULL == usb) {
                    usb_dbg(USB_ERROR, "create ehci instance fail");
                    break;
                }

                /* 使用默认管道和地址0进行枚举, endpoint 0 has been alloced in alloc_ehci */
                result = enum_usb_device(usb);
                if (OS_SUCC != result) {
                    usb_dbg(USB_ERROR, "enum device fail");
                }

                hc->rh_dev[i] = usb;
            } else {
                /* note: owner is chcs */
                usb_dbg(USB_WARNING, "ehci device disconnected - %d", i);

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
LOCALC os_ret OS_CALLBACK ehci_roothub_msg_proc(os_u32 arg1, os_u32 arg2, os_u32 arg3, os_u32 arg4, os_u32 arg5, os_u32 arg6, os_u32 arg7)
{
    os_void *msg;

    while (OS_SUCC == get_message(&msg)) {
        ehci_rh_status_change(((struct ehci_rh_msg *) msg)->hc);
        free_msg(msg);
    }
    cassert(OS_FALSE);
    return OS_SUCC;
}

/***************************************************************
 * description : binary tree node
 ***************************************************************/
struct bt_node {
    os_u32 no; // load
    struct bt_node *left;
    struct bt_node *right;
};

/***************************************************************
 * description : create banlance binary tree for interrupt schedule
 * history     :
 ***************************************************************/
LOCALC struct bt_node *alloc_bt_node(os_void)
{
    GLOBALDIF struct bt_node bt_nodes[INTERRUPT_FRAME_LIST_NUM * 2 - 1] = {0};
    GLOBALDIF os_u32 count = 0;

    if ((INTERRUPT_FRAME_LIST_NUM * 2 - 1) > count) {
        return bt_nodes + count++;
    } else {
        return OS_NULL;
    }
}

/***************************************************************
 * description : create banlance binary tree for interrupt schedule
 * history     :
 ***************************************************************/
LOCALC os_ret create_ehci_bt(struct bt_node *node, os_u32 load, os_u32 level, os_u32 depth)
{
    if (level <= depth) {
        node->left = alloc_bt_node();
        if (OS_NULL == node->left) {
            usb_dbg(USB_ERROR, "alloc left sched bt fail");
            return OS_FAIL;
        }
        node->left->left = OS_NULL;
        node->left->right = OS_NULL;
        node->left->no = load + power_of_2(level);
        create_ehci_bt(node->left, node->left->no, level + 1, depth);

        node->right = alloc_bt_node();
        if (OS_NULL == node->right) {
            usb_dbg(USB_ERROR, "alloc right sched bt fail");
            return OS_FAIL;
        }
        node->right->left = OS_NULL;
        node->right->right = OS_NULL;
        node->right->no = load + power_of_2(level + 1);
        create_ehci_bt(node->right, node->right->no, level + 1, depth);
    }
    return OS_SUCC;
}

/***************************************************************
 * description : 中序遍历二叉树, inorder traversal
 * history     :
 ***************************************************************/
LOCALC os_void travel_ehci_bt(struct bt_node *node)
{
    if (node) {
        travel_ehci_bt(node->left);
        ehci_int_table[ehci_int_table_index++] = node->no;
        travel_ehci_bt(node->right);
    }
}

/***************************************************************
 * description : for all ehci
 * history     :
 ***************************************************************/
LOCALC os_ret init_ehci_int_table(os_void)
{
    struct bt_node *head;

    head = alloc_bt_node();
    if (OS_NULL == head) {
        return OS_FAIL;
    }
    head->left = head->right = OS_NULL;
    head->no = 1;

    create_ehci_bt(head, head->no, 0, 8);

    ehci_int_table[0] = 0;
    ehci_int_table_index = 1;
    travel_ehci_bt(head);
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void dump_ehci_info(os_void)
{
    lock_t eflag;

    lock_int(eflag);

    print("%d %d\n", sizeof(struct ehci_cap_regs), sizeof(struct ehci_operational_regs));

    /* test case */
    do {
        struct ehci hc;
        struct ehci_qh *new1, *new2;

        mem_set(&hc, 0, sizeof(struct ehci));

        alloc_ehci_periodic_schedule_resource(&hc);

        new1 = cmalloc(sizeof(struct ehci_qh), EHCI_QH_ALIGN);
        mem_set(new1, 0, sizeof(struct ehci_qh));
        new1->state = EHCI_ED_OLD;
        init_list_head(&new1->qtd);

        new2 = cmalloc(sizeof(struct ehci_qh), EHCI_QH_ALIGN);
        mem_set(new2, 0, sizeof(struct ehci_qh));
        new2->state = EHCI_ED_OLD;
        init_list_head(&new2->qtd);

        ehci_insert_periodic_qh(&hc, new1, 128); // 1
        dump_periodic_info(&hc);
        ehci_insert_periodic_qh(&hc, new2, 128); // 1
        dump_periodic_info(&hc);
        ehci_remove_periodic_qh(&hc, new2);

        dump_periodic_info(&hc);
    } while (0);

    do {
        os_u32 i;
        print("int table index: %d\n", ehci_int_table_index);
        for (i = 0; i < PERIODIC_FRAME_LIST_NUM; i++) {
            print("%d ", ehci_int_table[i]);
        } print("\n");
    } while (0);

    unlock_int(eflag);
}

LOCALD os_u8 ehci_debug_name[] = { "ehci" };
LOCALD struct dump_info ehci_debug = {
    ehci_debug_name,
    dump_ehci_info
};

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void init_ehci_driver(os_void)
{
    os_ret result;

    ehci_rh_handle = create_task("ehci rh", ehci_roothub_msg_proc, TASK_PRIORITY_6, 0, 0, 0, 0, 0, 0, 0);
    cassert(OS_NULL != ehci_rh_handle);

    active_task_station(ehci_rh_handle);

    result = init_ehci_int_table();
    cassert(OS_SUCC == result);

    result = register_pci_driver(&pci_ehci_driver);
    cassert(OS_SUCC == result);

    result = register_dump(&ehci_debug);
    cassert(OS_SUCC == result);
}

//#define _DISABLE_EHCI_ // test 1.x host controller only
#ifdef _DISABLE_EHCI_
bus_init_func(BUS_Px, init_ehci_driver);
#else
bus_init_func(BUS_P2, init_ehci_driver);
#endif

