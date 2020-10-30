/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : xhci.c
 * version     : 1.0
 * description : (key) host control and root hub
 * author      : gaocheng
 * date        : 2020-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include <vbs.h>
#include "usb.h"
#include "xhci.h"

/***************************************************************
 global variable declare
 ***************************************************************/
LOCALD HTASK xhci_enum_handle;

/***************************************************************
 function declare
 ***************************************************************/

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void xhci_flush_post_buffer(os_u64 reg)
{
    GLOBALDIF os_u64 tmp;
    tmp = reg;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void xhci_write_64(pointer reg, os_u64 value)
{
    os_u32 *addr;

    addr = (os_u32 *) reg;

    /* If the xHC supports 64-bit addressing (AC64 = ¡®1¡¯),
       then software should write registers containing 64-bit address fields using only Qword accesses.
       If a system is incapable of issuing Qword accesses,
       then writes to the 64-bit address fields shall be performed using 2 Dword accesses;
       low Dword-first, high-Dword second. */
    *addr = value & UINT32_MAX;
    wmb();
    *(addr + 1) = value >> 32;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void terminate_int_ep_trb(struct xhci_trb_wrap *ep)
{
    os_u8 cycle;

    /* get cycle from last trb */
    cassert(1 <= ep->index);
    cycle = ep->trb_ring[ep->index - 1].field[3] & TRB_C;

    /* build new link trb */
    *(os_u64 *)(ep->trb_ring[ep->index].field) = (os_u64) virt_to_phys((pointer) ep->trb_ring);
    ep->trb_ring[ep->index].field[2] = 0;
    ep->trb_ring[ep->index].field[3] = cycle | TRB_IOC | (TRB_LINK << 10) | TOGGLE_CYCLE;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void terminate_ep_trb(struct xhci_trb_wrap *ep)
{
    /* build link trb */
    *(os_u64 *)(ep->trb_ring[ep->index].field) = (os_u64) virt_to_phys((pointer) ep->trb_ring);
    ep->trb_ring[ep->index].field[2] = 0;
    ep->trb_ring[ep->index].field[3] = (ep->cycle) | (TRB_LINK << 10) | TOGGLE_CYCLE;
    wmb();
    /* update the first trb */
    ep->trb_ring[0].field[3] &= (~TRB_C);
    ep->trb_ring[0].field[3] |= ep->cycle;

    ep->cycle ^= 1;
    ep->index = 0;
}

#define INVALID_TRB_INDEX (-1)

/***************************************************************
 * description : solution 2
 * history     :
 ***************************************************************/
LOCALC os_uint produce_ep_trb(struct xhci_trb_wrap *ep)
{
    os_uint index;
    index = ep->index++;
    if (index > (ep->count - 2)) {
        return INVALID_TRB_INDEX;
    }
    return index;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void reset_ep_trb(struct xhci_trb_wrap *ep)
{
    os_uint i;

    ep->index = 0;
    ep->cycle = XHCI_INIT_CS;
    // init all cycles
    for (i = 0; i < ep->count; i++) {
        ep->trb_ring[i].field[3] = 0;
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_uint produce_cmd_trb(struct xhci *hc)
{
    struct xhci_trb_wrap *wrap;
    os_uint index;

    wrap = hc->crcr;

    index = wrap->index++;
    if ((wrap->count - 1) <= wrap->index) {
        // update link trb
        wrap->trb_ring[wrap->count - 1].field[3] &= (~TRB_C);
        wrap->trb_ring[wrap->count - 1].field[3] |= wrap->cycle;
        usb_dbg(USB_INFO, "cmd ring revers\n");
        wrap->index = 0;
        wrap->cycle ^= 1;
    }
    return index;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void send_xhci_int_msg(struct xhci *hc, struct xhci_trb *trb)
{
    struct xhci_cmd_msg *msg;

    msg = alloc_msg(sizeof(struct xhci_cmd_msg));
    msg->head.msg_name = trb_type(trb);
    msg->head.msg_len = sizeof(struct xhci_cmd_msg);
    msg->hc = hc;
    mem_cpy(&msg->cmd_rsp, trb, sizeof(struct xhci_trb));
    lock_schedule();
    post_thread_msg(xhci_enum_handle, msg);
    unlock_schedule();
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void handle_command_trb(struct xhci *hc, struct xhci_trb *trb)
{
    os_u8 slot_id;
    struct xhci_trb *req;
    os_uint trb_offset;

    slot_id = trb_slot_id(trb);

    req = (struct xhci_trb *) phys_to_virt((pointer) trb_req(trb));
    trb_offset = ((pointer) req - (pointer) hc->crcr->trb_ring) / sizeof(struct xhci_trb);
    cassert(hc->crcr->count > trb_offset);
    cassert(hc->crcr->shadow[trb_offset].func);
    hc->crcr->shadow[trb_offset].func(hc, trb, &hc->crcr->shadow[trb_offset].para);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void handle_transfer_trb(struct xhci *hc, struct xhci_trb *trb)
{
    struct xhci_dev *dev;
    os_u8 slot_id;
    os_u8 ep_id;
    os_uint trb_offset;
    struct xhci_trb *req;

    slot_id = trb_slot_id(trb);
    dev = hc->slot_to_dev[slot_id];

    /* Endpoint ID. The ID of the Endpoint that generated the event.
       This value is used as an index in the Device Context to select the Endpoint Context associated with this event. */
    ep_id = trb_endpoint_id(trb);
    if (dev->ep[ep_id]) { // endpoint exists
        req = (struct xhci_trb *) phys_to_virt((pointer) trb_req(trb));
        trb_offset = ((pointer) req - (pointer) dev->ep[ep_id]->trb_ring) / sizeof(struct xhci_trb);
        cassert_word(dev->ep[ep_id]->count > trb_offset, "ep id %d, %x %x\n", ep_id, trb_offset, dev->ep[ep_id]->count);
        if (dev->ep[ep_id]->shadow[trb_offset].func) {
            dev->ep[ep_id]->shadow[trb_offset].func(hc, trb, &dev->ep[ep_id]->shadow[trb_offset].para);
        } else {
            print("slot %d ep %d offset %d\n", slot_id, ep_id, trb_offset);
            cassert(OS_FALSE);
        }
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void handle_event_trb(struct xhci_trb *trb, struct xhci *hc)
{
    switch (trb_type(trb)) {
    case TRB_TRANSFER:
        handle_transfer_trb(hc, trb);
        break;
    case TRB_COMPLETION:
        handle_command_trb(hc, trb);
        break;
    case TRB_PORT_STATUS:
        send_xhci_int_msg(hc, trb);
        break;
    case TRB_CMD_NOOP:
        print("no op cmd\n");
        break;
    default:
        break;
    }
}

/***************************************************************
 * description : event ring does not use pindex
 * history     :
 ***************************************************************/
LOCALC os_void handle_xhci_event(struct xhci *hc)
{
    struct xhci_trb_wrap *er;

    er = hc->er;
    while (er->cycle == (er->trb_ring[er->index].field[3] & TRB_C)) {
        handle_event_trb(&er->trb_ring[er->index], hc);

        if (0) usb_dbg(USB_INFO, "trb [%x-%x]\n", er->trb_ring[er->index].field[0], er->trb_ring[er->index].field[3]);

        er->index++;
        if (EVENT_RING_TRB_COUNT <= er->index) {
            er->index = 0;
            er->cycle ^= 1;
        }
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret xhci_interrupt_1(HDEVICE pci)
{
    struct xhci *hc;
    os_u32 status;

    hc = get_pci_dedicated(pci);
    if (hc) {
        status = hc->operational->USBSTS;
        if (STS_EINT & status) {
            /* Software that uses EINT shall clear it prior to clearing any IP flags.
               A race condition may occur if software clears the IP flags then clears the EINT flag,
               and between the operations another IP ¡®0¡¯to '1' transition occurs.
               In this case the new IP transition shall be lost. */
            hc->operational->USBSTS |= STS_EINT;
            wmb();
            /* If PCI Message Signaled Interrupts (MSI or MSI-X) are enabled,
               then the assertion of the Interrupt Pending (IP) flag in Figure 30 generates a PCI Dword write.
               The IP flag is automatically cleared by the completion of the PCI write.
               If the PCI Interrupt Pin mechanism is enabled,
               then the assertion of Interrupt Pending (IP) asserts the appropriate PCI INTx# pin.
               And the IP flag is cleared by software writing the IMAN register. */
            hc->runtime->IR[0].IMAN |= 0x3;
            xhci_flush_post_buffer(hc->runtime->IR[0].IMAN);

            lock_schedule();
            handle_xhci_event(hc);
            unlock_schedule();

            hc->runtime->IR[0].ERDP = virt_to_phys((pointer) &hc->er->trb_ring[hc->er->index]) | ERDP_EHB;
            xhci_flush_post_buffer(hc->runtime->IR[0].ERDP);

            hc->int_occur = OS_TRUE;
            return OS_SUCC;
        }
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret xhci_interrupt_2(HDEVICE pci)
{
    struct xhci *hc;

    cassert(OS_NULL != pci);
    hc = get_pci_dedicated(pci);
    if (hc) {
        if (hc->int_occur) {
            hc->int_occur = OS_FALSE;
            return OS_SUCC;
        }
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC struct xhci *alloc_xhci(HDEVICE dev)
{
    struct xhci *hc;
    os_uint i;

    hc = kmalloc(sizeof(struct xhci));
    if (OS_NULL == hc) {
        usb_dbg(USB_ERROR, "alloc xhci fail");
        return OS_NULL;
    }
    mem_set(hc, 0, sizeof(struct xhci));
    hc->pci = dev;
    hc->int_occur = OS_FALSE;
    mem_set(hc->slot_to_usb, OS_NULL, MAX_HC_SLOTS * sizeof(struct usb_device *));
    mem_set(hc->slot_to_dev, OS_NULL, MAX_HC_SLOTS * sizeof(struct xhci_dev *));
    hc->enum_port_id = 0;

    return hc;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void *free_xhci(struct xhci *hc)
{
    kfree(hc);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void save_xhci_regs_addr(struct xhci *hc)
{
    os_u32 bar0, bar1;

    bar0 = get_pci_dev_bar(hc->pci, 0);
    bar1 = get_pci_dev_bar(hc->pci, 1);

    // TODO: bar1 is high 32 of 64 address
    usb_dbg(USB_INFO, "bar: %x %x\n", bar0, bar1);

    /* The offsets for these registers are all relative to the beginning of the host controller¡¯s MMIO address space. */
    hc->capability = (struct xhci_cap_reg *) (bar0 & 0xffffff00);

    usb_dbg(USB_WARNING, "slot: %d", HCS_MAX_SLOTS(hc->capability->HCSPARAMS_1));
    usb_dbg(USB_WARNING, "intr: %d", HCS_MAX_INTRS(hc->capability->HCSPARAMS_1));
    usb_dbg(USB_WARNING, "port: %d", HCS_MAX_PORTS(hc->capability->HCSPARAMS_1));

    hc->operational = (struct xhci_op_reg *)((pointer) hc->capability + hc->capability->CAPLENGTH);
#define RTSOFF_MASK (~0x1f)
    hc->runtime = (struct xhci_run_reg *)((pointer) hc->capability + (hc->capability->RTSOFF & RTSOFF_MASK));
#define DBOFF_MASK (~0x3)
    hc->doorbell = (struct xhci_db_reg *)((pointer) hc->capability + (hc->capability->DBOFF & DBOFF_MASK));
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void *free_single_segment_ring(struct xhci_trb_wrap *wraper)
{
    if (wraper) {
        destroy_critical_section(wraper->trb_lock);
        if (wraper->trb_ring) {
            cfree(wraper->trb_ring);
        }
        kfree(wraper);
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC struct xhci_trb_wrap *alloc_single_segment_ring(os_u32 cnt, os_u32 align)
{
    struct xhci_trb_wrap *wraper;
    os_uint size;
    os_ret ret;

    cassert(cnt > 0x10);

    /* All Transfer Request Blocks shall be aligned on a 16-byte boundary. */
    if (align < 0x10) {
        align = 0x10;
    }

    size = sizeof(struct xhci_trb_wrap) + cnt * sizeof(struct trb_shadow);
    wraper = kmalloc(size);
    if (wraper) {
        mem_set(wraper, 0, size);
        wraper->check = TRB_CHECK;
        wraper->count = cnt;
        wraper->index = 0;
        wraper->cycle = XHCI_INIT_CS;
        wraper->trb_ring = cmalloc(cnt * sizeof(struct xhci_trb), align);
        if (wraper->trb_ring) {
            struct xhci_trb *trb_ring;
            trb_ring = wraper->trb_ring;
            mem_set(trb_ring, 0, cnt * sizeof(struct xhci_trb));
            /* last trb is link */
            *(os_u64 *)(trb_ring[cnt - 1].field) = (os_u64) virt_to_phys((pointer) trb_ring);
            trb_ring[cnt - 1].field[2] = 0; /* interrupt target is 0 */
            trb_ring[cnt - 1].field[3] = (TRB_LINK << 10) | TOGGLE_CYCLE;
            ret = create_critical_section(&wraper->trb_lock, __LINE__);
            cassert(OS_SUCC == ret);
        } else {
            kfree(wraper);
            wraper = OS_NULL;
        }
    }

    return wraper;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void switch_ports_to_xhci(struct xhci *hc)
{
    os_u32 vd;
    os_u32 ports_available;

    return;

    vd = in_pci_reg(hc->pci, 0);
    if (PCI_VENDOR_ID_INTEL == (vd & 0x0000ffff)) {
        if (0x1e26 == ((vd & 0xffff0000) >> 16)) {
            usb_dbg(USB_WARNING, "intel xhci 0x1e26");
            ports_available = 0xffffffff;
#define USB_INTEL_USB3_PSSEN 0xD8
            out_pci_reg(hc->pci, USB_INTEL_USB3_PSSEN, ports_available);
            ports_available = in_pci_reg(hc->pci, USB_INTEL_USB3_PSSEN);
            ports_available = 0xffffffff;
#define USB_INTEL_XUSB2PR 0xD0
            out_pci_reg(hc->pci, USB_INTEL_XUSB2PR, ports_available);
            ports_available = in_pci_reg(hc->pci, USB_INTEL_XUSB2PR);
        }

        if (0x1e31 == ((vd & 0xffff0000) >> 16)) {
            usb_dbg(USB_WARNING, "intel xhci 0x1e31");
            out_pci_reg(hc->pci, 0xd4, 0xf);
            out_pci_reg(hc->pci, 0xd0, 0xf);
            out_pci_reg(hc->pci, 0xdc, 0xf);
            out_pci_reg(hc->pci, 0xd8, 0xf);
        }
    }
}

#define get_xECP(cap) (((cap) & 0xffff0000) >> 14)

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret xhci_supported_protocol(struct xhci *hc)
{
    os_u32 offset;
    os_u32 *addr;

    offset = get_xECP(hc->capability->HCCPARAMS);
    addr = (os_u32 *)((pointer) hc->capability + offset);
    while ((*addr) & 0xff00) {
        if (0x2 == ((*addr) & 0xff)) {
            usb_dbg(USB_INFO, "protocol: %x %x %x\n", *addr, *(addr+1), *(addr+2));
        }
        offset = (((*addr) & 0xff00) >> 6);
        addr = (os_u32 *)((pointer) addr + offset);
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret request_xhci_ownership(struct xhci *hc)
{
    os_u32 offset;
    os_u32 *addr;
    os_u8 *t;

    usb_dbg(USB_WARNING, "HCCPARAMS: %x\n", hc->capability->HCCPARAMS);

    offset = get_xECP(hc->capability->HCCPARAMS);
    addr = (os_u32 *)((pointer) hc->capability + offset);
    while ((*addr) & 0xff00) {
        if (0x1 == ((*addr) & 0xff)) {
            if ((*addr) & 0x10000) {
                usb_dbg(USB_WARNING, "init xhci is owned by bios\n");
            }
            t = (os_u8 *) addr;
            *(t + 3) = 0x1;
            while (*(t + 2)) {
                delay_task(1, __LINE__);
            }
            usb_dbg(USB_WARNING, "usglegsup: %x\n", *addr);
            return OS_SUCC;
        }
        offset = (((*addr) & 0xff00) >> 6);
        addr = (os_u32 *)((pointer) addr + offset);
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret free_scratchpad_buffer(struct xhci *hc)
{
    os_u64 *scratchpad_array;
    os_u32 cnt;
    os_uint i;
    pointer p;

    if (hc->dcbaap[0]) {
        scratchpad_array = (os_u64 *) phys_to_virt(hc->dcbaap[0]);
        cnt = (hc->capability->HCSPARAMS_2 >> 27) & 0x1f;
        for (i = 0; i < cnt; i++) {
            p = phys_to_virt(scratchpad_array[i]);
            cfree(p);
        }
        cfree(scratchpad_array);
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC pointer alloc_scratchpad_buffer(struct xhci *hc, os_u16 page_size)
{
    os_uint i;
    os_u64 *scratchpad_array;
    os_u32 cnt;

    /* !!! NOTE: max scratchpad bufs hi MUST be considered */
    cnt = (((hc->capability->HCSPARAMS_2 >> 21) & 0x1f) << 5) | ((hc->capability->HCSPARAMS_2 >> 27) & 0x1f);
    usb_dbg(USB_INFO, "scratchpad cnt: %d from %x\n", cnt, hc->capability->HCSPARAMS_2);
    if (cnt) {
        scratchpad_array = cmalloc(cnt * sizeof(os_u64), page_size);
        if (OS_NULL == scratchpad_array) {
            goto end;
        }
        mem_set(scratchpad_array, 0, cnt * sizeof(os_u64));
        for (i = 0; i < cnt; i++) {
            os_u8 *scratchpad;
            scratchpad = cmalloc(page_size, page_size);
            if (OS_NULL == scratchpad) {
                usb_dbg(USB_ERROR, "allocate scratchpad fail\n");
                cassert(OS_FALSE);
                goto end;
            }
            mem_set(scratchpad, 0, page_size);
            scratchpad_array[i] = virt_to_phys((pointer) scratchpad);
        }
    }
    return (pointer) scratchpad_array;
  end:
    free_scratchpad_buffer(hc);
    return (pointer) OS_NULL;
}

struct xhci *dbg_hc = OS_NULL;

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret xhci_probe(HDEVICE pci)
{
    struct xhci *hc;
    os_u32 tmp;
    os_ret result;

    usb_dbg(USB_INFO, "xhci probe");

    cassert(OS_NULL != pci);

    /* init for fail label */
    hc = OS_NULL;

    hc = alloc_xhci(pci);
    if (OS_NULL == hc) {
        usb_dbg(USB_ERROR, "alloc xhci instance fail");
        goto fail;
    }
    set_pci_dedicated(pci, hc);

    save_xhci_regs_addr(hc);

    /* here, the xhci is running... */
    usb_dbg(USB_ERROR, "init cmd & status: %x %x", hc->operational->USBCMD, hc->operational->USBSTS);
#if 0 // never stop xhci, this operation disables keyboard of thinkpad
    hc->operational->USBCMD &= (~CMD_RUN);
    while (!(hc->operational->USBSTS & STS_HALT)) {
        delay_task(1, __LINE__);
    }
    usb_dbg(USB_ERROR, "xhci stopped");
#endif

    result = request_xhci_ownership(hc);
    if (OS_FAIL == result) {
        usb_dbg(USB_ERROR, "maybe xhci is not used by smi!");
    }

    /* reset? maybe used by bios */
    hc->operational->USBCMD |= CMD_RESET;
    wmb();
    /* wait for xhci */
    while (hc->operational->USBCMD & CMD_RESET) { // !!!
        delay_task(1, __LINE__);
    }
    while (hc->operational->USBSTS & STS_CNR) {
        delay_task(1, __LINE__);
    }

    /* open dma capacity before write some address register!!! */
    enable_pci_dma(pci);

    /* hand off ports to xhci */
    switch_ports_to_xhci(hc);

    /* maxslotsen */
    tmp = hc->operational->CONFIG & (~HCS_SLOTS_MASK);
    hc->operational->CONFIG = tmp | HCS_MAX_SLOTS(hc->capability->HCSPARAMS_1);

    // Device Notification Control (only bit 1 is allowed)
    hc->operational->DNCTRL |= 1;

    hc->context_size = (hc->capability->HCCPARAMS & HCC_CSZ_MASK) ? 64 : 32;
    hc->page_size = (hc->operational->PAGESIZE & 0x0000ffff) << 12;

    /* dcbaap, The memory structure referenced by this physical memory pointer is assumed to be physically contiguous and 64-byte aligned. */
    hc->dcbaap = cmalloc(MAX_HC_SLOTS * sizeof(os_u64), hc->page_size);
    if (OS_NULL == hc->dcbaap) {
        usb_dbg(USB_ERROR, "alloc dcbaap fail");
        goto fail;
    }
    mem_set(hc->dcbaap, 0, MAX_HC_SLOTS * sizeof(os_u64));
    hc->dcbaap[0] = alloc_scratchpad_buffer(hc, hc->page_size);
    hc->dcbaap[0] = virt_to_phys(hc->dcbaap[0]);
    xhci_write_64((pointer) &hc->operational->DCBAAP, virt_to_phys((pointer) hc->dcbaap));

    /* command ring */
    hc->crcr = alloc_single_segment_ring(CMD_RING_TRB_COUNT, 64);
    if (OS_NULL == hc->crcr) {
        usb_dbg(USB_ERROR, "allocate cmd ring fail");
        goto fail;
    }
#define CMD_RING_RSVD_BITS (0x3f)
    cassert(0 == ((pointer) hc->crcr->trb_ring & CMD_RING_RSVD_BITS));
    xhci_write_64((pointer) &hc->operational->CRCR, XHCI_INIT_CS | virt_to_phys((pointer) hc->crcr->trb_ring));

    /* event ring do not use link trb, so extra trb is allocated.
     * Figure 20: Event Ring State Machine
     */
    hc->er = alloc_single_segment_ring(EVENT_RING_TRB_COUNT + 1, 64*1024);
    if (OS_NULL == hc->er) {
        usb_dbg(USB_ERROR, "allocate event ring fail");
        goto fail;
    }
#define ERST_SEGMENT_SIZE 1
    hc->erstba = cmalloc(ERST_SEGMENT_SIZE * sizeof(struct event_ring_segment_table_entry), 64);
    if (OS_NULL == hc->erstba) {
        usb_dbg(USB_ERROR, "allocate event ring segment table fail");
        goto fail;
    }
    hc->erstba->rsvdz = 0;
    hc->erstba->ring_segment_size = EVENT_RING_TRB_COUNT & 0xffff;
#define CMD_ER_RSVD_BITS (0x3f)
    cassert(0 == ((pointer) hc->er->trb_ring & CMD_ER_RSVD_BITS));
    hc->erstba->ring_segment_base_address = virt_to_phys((pointer) hc->er->trb_ring);

#define ERST_SIZE_MASK (0xffff)
    hc->runtime->IR[0].ERSTSZ = (hc->runtime->IR[0].ERSTSZ & (~ERST_SIZE_MASK)) | (ERST_SEGMENT_SIZE & ERST_SIZE_MASK);
    wmb();
#define ERST_DESI_MASK 0x7
#define ERST_ERDP_MASK (~0xf)
#define ERST_EHB_MASK (0x8)
    hc->runtime->IR[0].ERDP = ERST_EHB_MASK | (hc->runtime->IR[0].ERDP & (~ERST_ERDP_MASK)) | (virt_to_phys((pointer) hc->er->trb_ring) & ERST_ERDP_MASK);
    wmb();
#define ERST_PTR_MASK (~0x3f)
    hc->runtime->IR[0].ERSTBA = (hc->runtime->IR[0].ERSTBA & (~ERST_PTR_MASK)) | (virt_to_phys((pointer) hc->erstba) & ERST_PTR_MASK);

    /* only enable primary */
#define DEFAULT_IMODI 4000
#define DEFAULT_IMODC 4
    hc->runtime->IR[0].IMOD = DEFAULT_IMODI | (DEFAULT_IMODC << 16);
    wmb();
    hc->operational->USBCMD |= (CMD_EIE | CMD_HSEIE);
    wmb();
    /* Enable the Interrupter by writing a ¡®1¡¯ to the Interrupt Enable (IE) field of the Interrupter Management register (5.5.2.1). */
    hc->runtime->IR[0].IMAN |= 0x3; // generate interrupt
    /* Most systems have write buffers that minimize overhead,
       but this may require a read operation to guarantee that the write has been flushed from posted buffers. */
    xhci_flush_post_buffer(hc->runtime->IR[0].IMAN);

    hc->operational->USBSTS |= (STS_FATAL | STS_EINT | STS_PORT | STS_SRE);
    wmb();

    /* run */
    hc->operational->USBCMD |= CMD_RUN;
    while (STS_HALT & hc->operational->USBSTS) {
        delay_task(1, __LINE__);
    }

    xhci_supported_protocol(hc);

    /* register interrupt */
    result = enable_pci_int(pci);
    if (OS_SUCC != result) {
        usb_dbg(USB_ERROR, "pci device install interrupt fail");
        goto fail;
    }

    usb_dbg(USB_WARNING, "xhci probe done");
    dbg_hc = hc;

    return OS_SUCC;
  fail:
    usb_dbg(USB_ERROR, "xhci probe fail");
    disable_pci_int(pci);
    if (OS_NULL != hc) {
        if (hc->dcbaap) cfree(hc->dcbaap);
        if (hc->crcr) free_single_segment_ring(hc->crcr);
        if (hc->er) free_single_segment_ring(hc->er);
        if (hc->erstba) cfree(hc->erstba);
        free_scratchpad_buffer(hc);
        free_xhci(hc);
    }
    return OS_FAIL;
}

LOCALD const struct pci_device_id pci_xhci_ids = {
    /* no matter who makes it */
    PCI_ANY_ID, PCI_ANY_ID,
    /* handle any USB EHCI controller */
    PCI_CLASS_SERIAL_USB_XHCI, ~0
};

LOCALD const struct pci_driver pci_xhci_driver = {
    "usb-xhci",
    &pci_xhci_ids,
    xhci_probe,
    OS_NULL,
    xhci_interrupt_1,
    xhci_interrupt_2
};

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void xhci_cmd_trb_done(struct xhci *hc, struct xhci_trb *trb, os_void *para)
{
    struct xhci_dev *dev;
    os_u8 slot_id;
    os_u8 ed;
    struct xhci_trb *req;
    struct trb_para *p;

    usb_dbg(USB_INFO, "transfer done int %d %d slot %d\n", trb_result(trb), trb_type(trb), trb_slot_id(trb));

    if (TRB_CC_SUCCESS == trb_result(trb)) {
        ed = trb_event_data(trb);
        req = (struct xhci_trb *) phys_to_virt((pointer) trb_req(trb));
    } else {
        // TODO: fail case
        slot_id = trb_slot_id(trb);
        dev = hc->slot_to_dev[slot_id];
        print("!!! cmd trb fail %d, slot %d, offset %d\n", trb_result(trb), slot_id, trb_endpoint_id(trb));
    }
    p = para;
    mem_cpy(&p->event, trb, sizeof(struct xhci_trb));
    notify_event(p->handle, __LINE__);
}

#define DEFAULT_TRANSFER_TIMOUT 4000

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC struct xhci_trb *xhci_issue_cmd_msg(struct xhci *hc, struct xhci_trb *cmd)
{
    struct xhci_trb *trb;
    struct trb_shadow *done;
    os_uint i;
    HEVENT handle;
    os_ret result;

    result = OS_FAIL;
    handle = create_event_handle(EVENT_INVALID, "xhci cmd", __LINE__);
    if (OS_NULL == handle) {
        return OS_NULL;
    }
    enter_critical_section(hc->crcr->trb_lock);
    i = produce_cmd_trb(hc);
    if (hc->crcr->count <= i) {
        result = OS_FAIL;
        goto end;
    }
    trb = &hc->crcr->trb_ring[i];
    mem_cpy(trb, cmd, sizeof(struct xhci_trb));
    done = &hc->crcr->shadow[i];
    done->func = xhci_cmd_trb_done;
    done->para.handle = handle;
    wmb();
    /* To ring the Host Controller Doorbell software shall write the Host Controller Doorbell register (offset 0 in the Doorbell Register Array),
       asserting the Host Controller Command value in the DB Target field and '0' in the DB Stream ID field. */
    hc->doorbell->doorbell[0] = 0;
    xhci_flush_post_buffer(hc->doorbell->doorbell[0]);
    /* wait for */
    result = wait_event(handle, DEFAULT_TRANSFER_TIMOUT);
  end:
    destroy_event_handle(handle);
    leave_critical_section(hc->crcr->trb_lock);
    if ((OS_SUCC == result) && (TRB_CC_SUCCESS == trb_result(&done->para.event))) {
        return &done->para.event;
    }
    print("xhci cmd failed\n");
    cassert(OS_FALSE);
    return OS_NULL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_u32 *xhci_context_item(struct xhci *hc, os_u32 *context, os_u8 index)
{
    return (os_u32 *)((os_u32) context + hc->context_size * index);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void xhci_trb_done(struct xhci *hc, struct xhci_trb *trb, os_void *para)
{
    struct xhci_dev *dev;
    os_u8 slot_id;
    os_u8 ed;
    struct xhci_trb *req;
    struct trb_para *p;

    if (0) usb_dbg(USB_INFO, "transfer done int %d %d slot %d\n", trb_result(trb), trb_type(trb), trb_slot_id(trb));

    if ((TRB_CC_SUCCESS == trb_result(trb)) || (TRB_CC_SP == trb_result(trb))) {
        ed = trb_event_data(trb);
        req = (struct xhci_trb *) phys_to_virt((pointer) trb_req(trb));
    } else {
        // TODO: fail case
        slot_id = trb_slot_id(trb);
        dev = hc->slot_to_dev[slot_id];
        usb_dbg(USB_ERROR, "!!! trb fail %d, slot %d, endpoint %d\n", trb_result(trb), slot_id, trb_endpoint_id(trb));
    }
    p = para;
    mem_cpy(&p->event, trb, sizeof(struct xhci_trb));
    if (p->handle) {
        notify_event(p->handle, __LINE__);
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret xhci_recv_control_msg(struct usb_device *usb, os_u32 addr, os_u8 ed_num, struct usb_setup_data *cmd, os_void *buffer, os_uint len)
{
    struct xhci *hc;
    os_u8 slot_id;
    struct xhci_dev *dev;
    struct xhci_trb *trb;
    struct xhci_trb_wrap *ep;
    os_uint i;
    os_ret result;
    HEVENT handle;
    struct trb_shadow *s;

    cassert((OS_NULL != usb) && (EP_NUM > ed_num));

    handle = OS_NULL;

    if (OS_NULL != usb->host_controller) {
        hc = usb->host_controller;
        slot_id = usb->usb_addr;

        dev = hc->slot_to_dev[slot_id];
        /* endpoint 0 trb ring */
        ep = dev->ep[XHCI_EP0_ID];

        handle = create_event_handle(EVENT_INVALID, "xhci transfer", __LINE__);
        if (OS_NULL == handle) {
            goto end;
        }

        enter_critical_section(ep->trb_lock);
        /* setup stage */
        i = produce_ep_trb(ep);
        if (INVALID_TRB_INDEX == i) {
            goto end;
        }
        trb = &ep->trb_ring[i];
        cassert(8 == sizeof(struct usb_setup_data));
        mem_cpy(&trb->field[0], cmd, sizeof(struct usb_setup_data));
        trb->field[2] = sizeof(struct usb_setup_data); // TRB Transfer Length = 8.
        trb->field[3] = (!ep->cycle) | (TRB_SETUP << 10) | (1 << 6) | (TRT_IN_DATA_STAGE << 16); /* idt:1 */
        /* data stage */
        if (len) {
            i = produce_ep_trb(ep);
            if (INVALID_TRB_INDEX == i) {
                goto end;
            }
            trb = &ep->trb_ring[i];
            *(os_u64 *) &trb->field[0] = virt_to_phys((pointer) buffer);
            trb->field[2] = len;
            trb->field[3] = ep->cycle | (TRB_DATA << 10) | TRB_ISP | (1 << 16); /* idt:0 */
        }
        /* status stage */
        i = produce_ep_trb(ep);
        if (INVALID_TRB_INDEX == i) {
            goto end;
        }
        trb = &ep->trb_ring[i];
        trb->field[0] = 0;
        trb->field[1] = 0;
        trb->field[2] = 0;
        trb->field[3] = ep->cycle | (TRB_STATUS << 10) | (1 << 5); /* ioc:1 */

        s = &ep->shadow[i];
        s->func = xhci_trb_done;
        s->para.handle = handle;

        terminate_ep_trb(ep);
        wmb();
        hc->doorbell->doorbell[slot_id] = XHCI_EP0_ID;
        xhci_flush_post_buffer(hc->doorbell->doorbell[slot_id]);

        /* wait for */
        result = wait_event(handle, DEFAULT_TRANSFER_TIMOUT);
        destroy_event_handle(handle);
        leave_critical_section(ep->trb_lock);
        if ((OS_SUCC == result) && (TRB_CC_SUCCESS == trb_result(&s->para.event))) {
            return OS_SUCC;
        }
    }
    return OS_SUCC;
  end:
    if (handle) destroy_event_handle(handle);
    leave_critical_section(ep->trb_lock);
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret xhci_send_control_msg(struct usb_device *usb, os_u32 addr, os_u8 ed_num, struct usb_setup_data *cmd, os_void *buffer, os_uint len)
{
    struct xhci *hc;
    os_u8 slot_id;
    struct xhci_dev *dev;
    struct xhci_trb *trb;
    struct xhci_trb_wrap *ep;
    os_uint i;
    os_ret result;
    HEVENT handle;
    struct trb_shadow *s;

    cassert((OS_NULL != usb) && (EP_NUM > ed_num));

    handle = OS_NULL;

    if (OS_NULL != usb->host_controller) {
        hc = usb->host_controller;
        slot_id = usb->usb_addr;

        handle = create_event_handle(EVENT_INVALID, "xhci transfer", __LINE__);
        if (OS_NULL == handle) {
            goto end;
        }

        dev = hc->slot_to_dev[slot_id];
        /* endpoint 0 trb ring */
        ep = dev->ep[XHCI_EP0_ID];
        cassert((OS_NULL != ep) && (TRB_CHECK == ep->check));
        enter_critical_section(ep->trb_lock);
        /* setup stage */
        i = produce_ep_trb(ep);
        if (INVALID_TRB_INDEX == i) {
            goto end;
        }
        trb = &ep->trb_ring[i];
        cassert(8 == sizeof(struct usb_setup_data));
        mem_cpy(&trb->field[0], cmd, sizeof(struct usb_setup_data));
        trb->field[2] = sizeof(struct usb_setup_data); // TRB Transfer Length = 8.
        trb->field[3] = (!ep->cycle) | (TRB_SETUP << 10) | (1 << 6) | (TRT_OUT_DATA_STAGE << 16); /* idt:1 */
        /* data stage */
        if (len) {
            i = produce_ep_trb(ep);
            if (INVALID_TRB_INDEX == i) {
                goto end;
            }
            trb = &ep->trb_ring[i];
            *(os_u64 *) &trb->field[0] = virt_to_phys((pointer) buffer);
            trb->field[2] = len;
            trb->field[3] = ep->cycle | (TRB_DATA << 10) | (0 << 16); /* idt:0 */
        }
        /* status stage */
        i = produce_ep_trb(ep);
        if (INVALID_TRB_INDEX == i) {
            goto end;
        }
        trb = &ep->trb_ring[i];
        trb->field[0] = 0;
        trb->field[1] = 0;
        trb->field[2] = 0;
        trb->field[3] = ep->cycle | (TRB_STATUS << 10) | (1 << 5) | (1 << 16); /* ioc:1 */

        s = &ep->shadow[i];
        s->func = xhci_trb_done;
        s->para.handle = handle;
        terminate_ep_trb(ep);
        wmb();
        hc->doorbell->doorbell[slot_id] = XHCI_EP0_ID;
        xhci_flush_post_buffer(hc->doorbell->doorbell[slot_id]);

        /* wait for */
        result = wait_event(handle, DEFAULT_TRANSFER_TIMOUT); /* mb operation inside */
        destroy_event_handle(handle);
        leave_critical_section(ep->trb_lock);
        if ((OS_SUCC == result) && (TRB_CC_SUCCESS == trb_result(&s->para.event))) {
            return OS_SUCC;
        }
        return OS_FAIL;
    }
    return OS_SUCC;
  end:
    if (handle) destroy_event_handle(handle);
    leave_critical_section(ep->trb_lock);
    return OS_FAIL;
}

#define NORMAL_TRB_TRANSFER_LENGTH 0x1000
#define NORMAL_TRB_TRANSFER_TIMOUT 4000

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret xhci_normal_trb(struct usb_device *usb, os_u8 pipe, enum usb_direction dir, os_void *buffer, os_uint len)
{
    struct xhci *hc;
    os_u8 slot_id;
    struct xhci_dev *dev;
    struct xhci_trb *trb;
    struct xhci_trb_wrap *ep;
    os_uint i;
    os_ret result;
    HEVENT handle;
    struct trb_shadow *s;
    os_uint cnt, n;
    os_u32 td_packet_cnt, td_size;
    os_uint total;
    os_u16 mps;

    cassert((OS_NULL != usb) && (EP_NUM > pipe));

    handle = OS_NULL;

    if ((OS_NULL != usb->host_controller) && (len)) {
        hc = usb->host_controller;
        slot_id = usb->usb_addr;
        dev = hc->slot_to_dev[slot_id];
        /* endpoint trb ring */
        ep = dev->ep[xhci_dci(pipe, dir)];
        cassert(OS_NULL != ep);

        handle = create_event_handle(EVENT_INVALID, "xhci transfer", __LINE__);
        if (OS_NULL == handle) {
            goto end;
        }

        enter_critical_section(ep->trb_lock);
        total = 0;
        mps = usb_endpoint_mps(usb, pipe, dir);
        td_packet_cnt = divl_cell(len, mps);
        cnt = divl_cell(len, NORMAL_TRB_TRANSFER_LENGTH);
        for (n = 0; n < cnt; n++) {
            i = produce_ep_trb(ep);
            if (INVALID_TRB_INDEX == i) {
                goto end;
            }
            trb = &ep->trb_ring[i];
            *(os_u64 *)(&trb->field[0]) = virt_to_phys((pointer) buffer);
            buffer += NORMAL_TRB_TRANSFER_LENGTH;

            if (len >= NORMAL_TRB_TRANSFER_LENGTH) {
                len -= NORMAL_TRB_TRANSFER_LENGTH;
                trb->field[2] = (NORMAL_TRB_TRANSFER_LENGTH | DEFAULT_INTERRUPT_TARGET);
                total += NORMAL_TRB_TRANSFER_LENGTH;
            } else {
                trb->field[2] = (len | DEFAULT_INTERRUPT_TARGET);
                len = 0;
                total += len;
            }
            td_size = td_packet_cnt - divl_floor(total, mps);
            if (31 < td_size) {
                td_size = 31;
            }
            trb->field[2] |= trb_td_size(td_size);

            if (0 != i) {
                trb->field[3] = ep->cycle | TRB_ISP | TRB_CH | (TRB_NORMAL << 10);
            } else { /* first trb */
                trb->field[3] = (!ep->cycle) | TRB_ISP | TRB_CH | (TRB_NORMAL << 10);
            }

            s = &ep->shadow[i];
            s->func = xhci_trb_done;
            s->para.handle = handle;
        }
        /* update the last trb */
        trb->field[2] &= (~TRB_TD_SIZE_MASK); // the last Transfer TRB of a TD: TD Size (x) = 0.
        trb->field[3] &= (~TRB_CH);
        trb->field[3] &= (~TRB_ISP);
        trb->field[3] |= TRB_IOC;
        terminate_ep_trb(ep);
        wmb();
        hc->doorbell->doorbell[slot_id] = xhci_dci(pipe, dir);
        xhci_flush_post_buffer(hc->doorbell->doorbell[slot_id]);
        /* wait for */
        result = wait_event(handle, NORMAL_TRB_TRANSFER_TIMOUT); /* mb operation inside */
        destroy_event_handle(handle);
        leave_critical_section(ep->trb_lock);

        if ((OS_SUCC == result)
         && ((TRB_CC_SUCCESS == trb_result(&s->para.event)) || (TRB_CC_SP == trb_result(&s->para.event)))) {
            return OS_SUCC;
        }
    }
    return OS_FAIL;
  end:
    if (handle) destroy_event_handle(handle);
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret xhci_recv_bulk_msg(struct usb_device *usb, os_u8 pipe, os_u8 *buffer, os_uint len)
{
    return xhci_normal_trb(usb, pipe, USB_IN, buffer, len);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret xhci_send_bulk_msg(struct usb_device *usb, os_u8 pipe, os_u8 *buffer, os_uint len)
{
    return xhci_normal_trb(usb, pipe, USB_OUT, buffer, len);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void update_int_trb(struct xhci_trb *trb)
{
    os_u8 cycle;

    cycle = trb->field[3] & TRB_C;
    trb->field[3] &= (~TRB_C);
    trb->field[3] |= (!cycle);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void xhci_int_trb_done(struct xhci *hc, struct xhci_trb *trb, os_void *para)
{
    struct xhci_dev *dev;
    os_u8 slot_id;
    os_u8 ep_id;
    struct xhci_trb *req;
    struct trb_para *p;
    os_void (*complete)(os_u8 *data);

    if (0) usb_dbg(USB_INFO, "transfer done int %d %d slot %d\n", trb_result(trb), trb_type(trb), trb_slot_id(trb));

    if ((TRB_CC_SUCCESS == trb_result(trb)) || (TRB_CC_SP == trb_result(trb))) {
        p = para;
        complete = (os_void (*)(os_u8 *)) p->event.field[0];
        if (complete) {
            complete((os_u8 *) p->event.field[1]);
        }
        ep_id = trb_endpoint_id(trb);
        req = (struct xhci_trb *) phys_to_virt((pointer) trb_req(trb));
        slot_id = trb_slot_id(trb);
        update_int_trb(req);
        hc->doorbell->doorbell[slot_id] = ep_id;
        xhci_flush_post_buffer(hc->doorbell->doorbell[slot_id]);
    } else {
        // TODO: fail case
        slot_id = trb_slot_id(trb);
        dev = hc->slot_to_dev[slot_id];
        print("!!! int trb fail %d, slot %d, endpoint %d\n", trb_result(trb), slot_id, trb_endpoint_id(trb));
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret xhci_queue_normal_trb(struct usb_device *usb, os_u8 pipe, os_void *buffer, os_uint len, os_void (*complete)(os_u8 *data))
{
    struct xhci *hc;
    os_u8 slot_id;
    struct xhci_dev *dev;
    struct xhci_trb *trb;
    struct xhci_trb_wrap *ep;
    os_uint i;
    os_ret result;
    struct trb_shadow *s;
    spinlock_t lock;

    cassert((OS_NULL != usb) && (EP_NUM > pipe));
    cassert(len <= MAX_TRB_TRANSFER_LENGTH);

    if ((OS_NULL != usb->host_controller) && (len)) {
        hc = usb->host_controller;
        slot_id = usb->usb_addr;
        dev = hc->slot_to_dev[slot_id];
        /* endpoint trb ring */
        ep = dev->ep[xhci_dci(pipe, USB_IN)];
        cassert(OS_NULL != ep);

        init_spinlock(&lock);
        enter_critical_section(ep->trb_lock);
        spin_lock(&lock);
        i = produce_ep_trb(ep);
        if (INVALID_TRB_INDEX == i) {
            goto end;
        }
        s = &ep->shadow[i];
        s->func = xhci_int_trb_done;
        s->para.handle = OS_NULL;
        s->para.event.field[0] = (pointer) complete;
        s->para.event.field[1] = (pointer) buffer;

        trb = &ep->trb_ring[i];
        *(os_u64 *)(&trb->field[0]) = virt_to_phys((pointer) buffer);
        trb->field[2] = (len | DEFAULT_INTERRUPT_TARGET);
        wmb();
        trb->field[3] = ep->cycle | TRB_IOC | (TRB_NORMAL << 10);
        s = &ep->shadow[i+1];
        s->func = xhci_int_trb_done;
        s->para.handle = OS_NULL;
        s->para.event.field[0] = (pointer) OS_NULL;
        terminate_int_ep_trb(ep);
        spin_unlock(&lock);
        leave_critical_section(ep->trb_lock);
        hc->doorbell->doorbell[slot_id] = xhci_dci(pipe, USB_IN);
        xhci_flush_post_buffer(hc->doorbell->doorbell[slot_id]);
        return OS_SUCC;
    }
  end:
    print("normal interrupt trb (%d) failed\n", pipe);
    return OS_FAIL;
}

/***************************************************************
 * description : add interrupt periodic transfer
 * history     :
 ***************************************************************/
LOCALC os_ret xhci_add_interrupt_trans(struct usb_device *usb, os_u8 pipe, os_u8 *data, os_uint len, os_u8 bInterval, os_void (*complete)(os_u8 *data))
{
    return xhci_queue_normal_trb(usb, pipe, data, len, complete);
}

/***************************************************************
 * description : del interrupt periodic transfer
 * history     :
 ***************************************************************/
LOCALC os_ret xhci_del_interrupt_trans(struct usb_device *usb, os_u8 pipe)
{
    cassert(OS_FALSE);
    return OS_FAIL;
}


/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC enum xhci_ep_type xhci_ep_type_by(enum usb_transfer_type type, enum usb_direction dir)
{
    GLOBALDIF os_u8 ep_map[][3] = {
        { XHCI_EP_CONTROL_BID, USB_CTRL_TRANSFER, USB_OUT },
        { XHCI_EP_CONTROL_BID, USB_CTRL_TRANSFER, USB_IN },
        { XHCI_EP_ISO_OUT, USB_ISO_TRANSFER, USB_OUT },
        { XHCI_EP_ISO_IN, USB_ISO_TRANSFER, USB_IN },
        { XHCI_EP_BULK_OUT, USB_BULK_TRANSFER, USB_OUT },
        { XHCI_EP_BULK_IN, USB_BULK_TRANSFER, USB_IN },
        { XHCI_EP_INT_OUT, USB_INTERRUPT_TRANSFER, USB_OUT },
        { XHCI_EP_INT_IN, USB_INTERRUPT_TRANSFER, USB_IN }
    };
    os_uint i;

    for (i = 0; i < array_size(ep_map); i++) {
        if ((ep_map[i][1] == type) && (ep_map[i][2] == dir)) {
            return ep_map[i][0];
        }
    }
    return XHCI_EP_INVALID;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_u32 xhci_get_interval(struct usb_device *dev, struct usb_endpoint_descriptor *ep)
{
    os_u32 interval;

    interval = 0;

    if ((USB_CTRL_TRANSFER == get_usb_transfer_type(ep->bmAttributes))
     || (USB_BULK_TRANSFER == get_usb_transfer_type(ep->bmAttributes))) {
        return 0;
    }

    switch (dev->speed) {
    case USB_LOW_SPEED:
        do {
            os_u32 i;
            i = min(max(interval, 1), 255);
            for (interval = 0; i != 1; interval++) {
                i = i >> 1;
            }
            interval += 3;
        } while (0);
        break;
    case USB_FULL_SPEED:
        if (USB_ISO_TRANSFER == get_usb_transfer_type(ep->bmAttributes)) {
            interval = min(max(interval, 1), 16) + 2;
            break;
        }
        break;
    case USB_HIGH_SPEED:
    case USB_SUPER_SPEED:
    default:
        interval = min(max(ep->bInterval, 1), 16) - 1;
        break;
    }
    return interval;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_u32 xhci_get_max_esit_payload(struct usb_device *dev, struct usb_endpoint_descriptor *ep)
{
    int max_burst;
    int max_packet;

    if ((USB_CTRL_TRANSFER == get_usb_transfer_type(ep->bmAttributes))
     || (USB_BULK_TRANSFER == get_usb_transfer_type(ep->bmAttributes))) {
        return 0;
    }

    if (dev->speed == USB_SUPER_SPEED) {
        //if (ep->ss_ep_comp) return ep->ss_ep_comp->desc.wBytesPerInterval;
        return ep->wMaxPacketSize;
    }

    max_packet = ep->wMaxPacketSize & 0x3ff;
    max_burst = (ep->wMaxPacketSize & 0x1800) >> 11;
    /* A 0 in max burst means 1 transfer per ESIT */
    return max_packet * (max_burst + 1);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret xhci_add_endpoint(struct usb_device *usb, struct usb_endpoint_descriptor *ep)
{
    struct xhci *hc;
    struct xhci_trb *trb, cmd;
    os_u8 slot_id;
    struct xhci_dev *dev;
    struct usb_config_descriptor *config;
    struct usb_itf_info *intf;
    os_uint i, j;
    os_u32 *input_context;
    os_u32 *input_control, *ep_context, *slot;
    os_u8 ep_num;
    enum usb_direction dir;
    enum xhci_ep_type type;
    os_u32 max_burst;
    os_u32 max_esit_payload;
    os_u32 bInterval;

    cassert((OS_NULL != usb) && (OS_NULL != usb->host_controller));
    hc = usb->host_controller;
    slot_id = usb->usb_addr;
    dev = hc->slot_to_dev[slot_id];
    ep_num = endpoint_num(ep->bEndpointAddress);
    dir = endpoint_is_out(ep->bEndpointAddress) ? USB_OUT : USB_IN;
    type = xhci_ep_type_by(get_usb_transfer_type(ep->bmAttributes), dir);
    max_burst = (ep->wMaxPacketSize & 0x1800) >> 11;
    max_esit_payload = xhci_get_max_esit_payload(usb, ep);
    bInterval = xhci_get_interval(usb, ep);

    input_context = dev->input_context;
    input_control = xhci_context_item(hc, input_context, 0);
    input_control[0] = 0;
    input_control[1] = endpoint_flag(ep_num, dir);
    slot = xhci_context_item(hc, input_context, 1);
    slot[0] &= 0xf8000000;
    slot[0] |= (31 << 27);
    dev->ep[xhci_dci(ep_num, dir)] = alloc_single_segment_ring(EP_TRB_CNT, 0);
    if (OS_NULL == dev->ep[xhci_dci(ep_num, dir)]) {
        return OS_FAIL;
    }
    ep_context = xhci_context_item(hc, input_context, endpoint_index(ep_num, dir));
    ep_context[0] = bInterval << 16;
    ep_context[1] = (type << 3) | (max_burst << 8) | (endpoint_mps(ep->wMaxPacketSize) << 16) | EP_HID | (DEFAULT_ERR_CNT << 1);
    *(os_u64 *) &ep_context[2] = virt_to_phys((pointer) dev->ep[xhci_dci(ep_num, dir)]->trb_ring) | XHCI_INIT_CS;
    ep_context[4] = (max_esit_payload) | (max_esit_payload << 16);

    usb_dbg(USB_INFO, "add ep %d %d, %x %d, %d %x %d\n", ep_num,dir, input_control[1], xhci_dci(ep_num, dir), endpoint_index(ep_num, dir), input_context, type);

    *(os_u64 *) &cmd.field[0] = virt_to_phys((pointer) input_context);
    cmd.field[2] = 0;
    cmd.field[3] = hc->crcr->cycle | (TRB_CONFIG_EP << 10) | (slot_id << 24);
    trb = xhci_issue_cmd_msg(hc, &cmd);
    if (OS_NULL == trb) {
        return OS_FAIL;
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret xhci_del_endpoint(struct usb_device *usb, struct usb_endpoint_descriptor *ep)
{
    struct xhci *hc;
    struct xhci_trb *trb, cmd;
    os_u8 slot_id;
    os_u8 ep_num;
    enum usb_direction dir;
    os_u32 *input_context;
    os_u32 *input_control;
    struct xhci_dev *dev;

    cassert((OS_NULL != usb) && (OS_NULL != usb->host_controller));

    hc = usb->host_controller;
    slot_id = usb->usb_addr;
    dev = hc->slot_to_dev[slot_id];
    ep_num = endpoint_num(ep->bEndpointAddress);
    dir = endpoint_is_out(ep->bEndpointAddress) ? USB_OUT : USB_IN;

#if 0
    cmd.field[0] = 0;
    cmd.field[1] = 0;
    cmd.field[2] = 0;
    cmd.field[3] = hc->crcr->cycle | TRB_SP | (xhci_dci(ep_num, dir) << 16) | (TRB_STOP_RING << 10) | (slot_id << 24);
    trb = xhci_issue_cmd_msg(hc, &cmd);
    if (OS_NULL == trb) {
        print("stop endpoint failed\n");
    }
    delay_ms(10);
#endif

    input_context = dev->input_context;
    input_control = xhci_context_item(hc, input_context, 0);
    input_control[0] = endpoint_flag(ep_num, dir);
    input_control[1] = 0;
    *(os_u64 *) &cmd.field[0] = virt_to_phys((pointer) input_context);
    cmd.field[2] = 0;
    cmd.field[3] = hc->crcr->cycle | (TRB_CONFIG_EP << 10) | (slot_id << 24);
    trb = xhci_issue_cmd_msg(hc, &cmd);
    if (OS_NULL == trb) {
        usb_dbg(USB_ERROR, "delete endpoint failed\n");
    }

    if (dev->ep[xhci_dci(ep_num, dir)]) {
        free_single_segment_ring(dev->ep[xhci_dci(ep_num, dir)]);
        dev->ep[xhci_dci(ep_num, dir)] = OS_NULL;
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void free_xhci_dev(struct xhci_dev *dev)
{
    kfree(dev);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC struct xhci_dev *alloc_xhci_dev(struct xhci *hc, os_u8 port_id, os_u8 slot_id)
{
    struct xhci_dev *priv;
    os_uint i;

    priv = kmalloc(sizeof(struct xhci_dev));
    if (OS_NULL == priv) {
        return OS_NULL;
    }
    priv->hc = hc;
    priv->port_id = port_id;
    priv->slot_id = slot_id;

    for (i = 0; i < XHCI_EP_CNT; i++) {
        priv->ep[i] = OS_NULL;
    }
    priv->input_context = priv->output_context = OS_NULL;
    return priv;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_u8 xhci_enable_slot_cmd(struct xhci *hc, os_u8 port_id)
{
    struct xhci_trb *trb, cmd;

    cmd.field[0] = 0;
    cmd.field[1] = 0;
    cmd.field[2] = 0;
    cmd.field[3] = hc->crcr->cycle | (TRB_ENABLE_SLOT << 10);
    trb = xhci_issue_cmd_msg(hc, &cmd);
    if (OS_NULL == trb) {
        return 0;
    }
    return trb_slot_id(trb);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_u8 xhci_disable_slot_cmd(struct xhci *hc, os_u8 slot_id)
{
    struct xhci_trb *trb, cmd;

    cmd.field[0] = 0;
    cmd.field[1] = 0;
    cmd.field[2] = 0;
    cmd.field[3] = hc->crcr->cycle | (TRB_DISABLE_SLOT << 10) | (slot_id << 24);
    trb = xhci_issue_cmd_msg(hc, &cmd);
    if (OS_NULL == trb) {
        return OS_FAIL;
    }
    usb_dbg(USB_INFO, "disable slot %d succ\n", slot_id);
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_u8 alloc_xhci_device_addr(struct usb_device *usb)
{
    struct xhci *hc;
    os_u8 slot_id;

    cassert(OS_NULL != usb);
    hc = (struct xhci *) usb->host_controller;
    cassert(OS_NULL != hc);

    cassert(0 != hc->enum_port_id);
    slot_id = xhci_enable_slot_cmd(hc, hc->enum_port_id);
    usb_dbg(USB_INFO, "new slot id %d\n", slot_id);
    cassert(0 != slot_id);
    hc->slot_to_usb[slot_id] = usb;
    hc->slot_to_dev[slot_id] = alloc_xhci_dev(hc, hc->enum_port_id, slot_id);
    cassert(OS_NULL != hc->slot_to_dev[slot_id]);
    return slot_id;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret free_xhci_device_addr(struct usb_device *usb)
{
    struct xhci *hc;
    os_u8 slot_id;

    cassert(OS_NULL != usb);
    hc = (struct xhci *) usb->host_controller;
    cassert(OS_NULL != hc);

    slot_id = usb->usb_addr;
    xhci_disable_slot_cmd(hc, slot_id);
    free_xhci_dev(hc->slot_to_dev[slot_id]);
    hc->dcbaap[slot_id] = 0;
    hc->slot_to_dev[slot_id] = OS_NULL;
    return OS_SUCC;
}

LOCALD enum usb_speed_type st[] = {
    USB_INVALID_SPEED,
    USB_FULL_SPEED,
    USB_LOW_SPEED,
    USB_HIGH_SPEED,
    USB_SUPER_SPEED
};

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_u8 xhci_speed(enum usb_speed_type usb_speed)
{
    os_u8 i;

    for (i = 0; i < array_size(st); i++) {
        if (usb_speed == st[i]) {
            return i;
        }
    }
    return 0;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC enum usb_speed_type usb_speed(os_u8 xhci_speed)
{
    cassert(array_size(st) > xhci_speed);
    return st[xhci_speed];
}

/***************************************************************
 * description : 4.3.3 Device Slot Initialization
 * history     :
 ***************************************************************/
LOCALC os_void xhci_uninit_dev_slot(struct xhci *hc, struct usb_device *usb)
{
    os_u8 slot_id;
    struct xhci_dev *dev;

    slot_id = usb->usb_addr;
    dev = hc->slot_to_dev[slot_id];

    if (dev->input_context) {
        cfree(dev->input_context);
        dev->input_context = OS_NULL;
    }
    if (dev->ep[XHCI_EP0_ID]) {
        free_single_segment_ring(dev->ep[XHCI_EP0_ID]);
        dev->ep[XHCI_EP0_ID] = OS_NULL;
    }
    if (dev->output_context) {
        cfree(dev->output_context);
        dev->output_context = OS_NULL;
    }
}

/***************************************************************
 * description : 4.3.3 Device Slot Initialization
 * history     :
 ***************************************************************/
LOCALC os_void xhci_init_dev_slot(struct xhci *hc, struct usb_device *usb, os_u8 port_id)
{
    os_u8 slot_id;
    struct xhci_dev *dev;
    os_u32 *input_context, *output_context;
    os_u32 *input_control, *slot, *ep0;

    slot_id = usb->usb_addr;
    dev = hc->slot_to_dev[slot_id];

    input_context = output_context = OS_NULL;
    dev->ep[XHCI_EP0_ID] = OS_NULL;

    /* device context 64 align */
    input_context = cmalloc(0x420, hc->page_size);
    if (OS_NULL == input_context) {
        goto end;
    }
    mem_set(input_context, 0, 0x420);
    // setting the A0 and A1 flags
    input_control = xhci_context_item(hc, input_context, 0);
    input_control[1] |= 0x3;
    slot = xhci_context_item(hc, input_context, 1);
    slot[0] = (xhci_speed(usb->speed) << 20) | (1 << 27); // must be 1
    slot[1] = port_id << 16;
    dev->ep[XHCI_EP0_ID] = alloc_single_segment_ring(EP_TRB_CNT, 0);
    if (OS_NULL == dev->ep[XHCI_EP0_ID]) {
        goto end;
    }

    /* init ep0 context */
    ep0 = xhci_context_item(hc, input_context, 2);
    ep0[0] = 0;
    ep0[1] = (XHCI_EP_CONTROL_BID << 3) | (DEFAULT_MAX_PACKET_SIZE << 16) | EP_HID | (DEFAULT_ERR_CNT << 1);
    *(os_u64 *) &ep0[2] = virt_to_phys((pointer) dev->ep[XHCI_EP0_ID]->trb_ring) | XHCI_INIT_CS;
    ep0[4] = 8;
    output_context = cmalloc(0x400, hc->page_size);
    if (OS_NULL == output_context) {
        goto end;
    }
    mem_set(output_context, 0, 0x400);
    hc->dcbaap[slot_id] = virt_to_phys((pointer) output_context);
    dev->input_context = input_context;
    dev->output_context = output_context;
    return;
  end:
    if (input_context) cfree(input_context);
    if (output_context) cfree(output_context);
    if (dev->ep[XHCI_EP0_ID]) free_single_segment_ring(dev->ep[XHCI_EP0_ID]);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret xhci_add_ep0(struct xhci *hc, struct usb_device *usb)
{
    struct xhci_trb *trb, cmd;
    os_u8 slot_id;
    struct xhci_dev *dev;

    slot_id = usb->usb_addr;
    dev = hc->slot_to_dev[slot_id];

    *(os_u64 *) &cmd.field[0] = virt_to_phys((pointer) dev->input_context);
    cmd.field[2] = 0;
    cmd.field[3] = hc->crcr->cycle | (TRB_ADDR_DEV << 10) | ADC_BSR | (slot_id << 24);
    trb = xhci_issue_cmd_msg(hc, &cmd);
    if (OS_NULL == trb) {
        return OS_FAIL;
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret xhci_update_mps(struct usb_device *usb)
{
    struct xhci *hc;
    os_u8 slot_id;
    os_u32 *input_context;
    os_u32 *input_control;
    os_u32 *ep0;
    struct xhci_trb *trb, cmd;
    os_u16 mps;

    hc = usb->host_controller;
    slot_id = usb->usb_addr;
    mps = usb->descriptor.bMaxPacketSize0;

    input_context = hc->slot_to_dev[slot_id]->input_context;
    // setting the A1 flags
    input_control = xhci_context_item(hc, input_context, 0);
    input_control[1] = 0x2;
    ep0 = xhci_context_item(hc, input_context, 2);
    ep0[1] &= 0xffff; // erase max packet size
    ep0[1] |= (mps << 16);

    *(os_u64 *) &cmd.field[0] = virt_to_phys((pointer) input_context);
    cmd.field[2] = 0;
    cmd.field[3] = hc->crcr->cycle | (TRB_EVAL_CONTEXT << 10) | (slot_id << 24);
    trb = xhci_issue_cmd_msg(hc, &cmd);
    if (OS_NULL == trb) {
        return OS_FAIL;
    }
    usb_dbg(USB_INFO, "output context (for ep0 mps): %x\n", *(xhci_context_item(hc, hc->slot_to_dev[usb->usb_addr]->output_context, 1) + 1));
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret xhci_addr_dev(struct usb_device *usb)
{
    struct xhci_trb *trb, cmd;
    struct xhci_dev *dev;
    os_u8 slot_id;
    struct xhci *hc;

    hc = usb->host_controller;
    slot_id = usb->usb_addr;
    dev = hc->slot_to_dev[slot_id];

    *(os_u64 *) &cmd.field[0] = virt_to_phys((pointer) dev->input_context);
    cmd.field[2] = 0;
    cmd.field[3] = hc->crcr->cycle | (TRB_ADDR_DEV << 10) | (slot_id << 24);
    trb = xhci_issue_cmd_msg(hc, &cmd);
    if (OS_NULL == trb) {
        return OS_FAIL;
    }
    /* because slot is addressed??? */
    reset_ep_trb(dev->ep[XHCI_EP0_ID]);
    return OS_SUCC;
}

LOCALD const struct usb_host_controller_operations xhci_operation = {
    xhci_recv_control_msg,
    xhci_send_control_msg,
    xhci_recv_bulk_msg,
    xhci_send_bulk_msg,
    xhci_add_interrupt_trans,
    xhci_del_interrupt_trans,

    alloc_xhci_device_addr, // allocate usb address
    free_xhci_device_addr, // free usb address
    xhci_addr_dev,

    xhci_add_endpoint,
    xhci_del_endpoint,

    OS_NULL,
    OS_NULL,
    xhci_update_mps
};

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void start_xhci_dev(struct xhci *hc, os_u8 port_id)
{
    struct usb_device *usb;
    os_u8 speed;

    hc->enum_port_id = port_id;

    speed = port_speed(hc->operational->port[port_id - 1].PORTSC);
    usb = alloc_usb_device(hc, usb_speed(speed), &xhci_operation);
    if (OS_NULL != usb) {
        xhci_init_dev_slot(hc, usb, port_id);
        /* For some legacy USB devices it may be necessary to communicate with the device when it is in the Default state,
           before transitioning it to the Address state.
           To accomplish this system software shall issue an Address Device Command with the BSR flag set to ¡®1¡¯.
           Setting the BSR flag enables the operation of the Default Control Endpoint for the Device Slot but blocks the xHC from issuing a SET_ADDRESS request to the device,
           which would transition it to the Address state. */
        xhci_add_ep0(hc, usb);

        enum_usb_device(usb);
        usb_dbg(USB_INFO, "enum done\n");
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void stop_xhci_dev(struct xhci *hc, os_u8 port_id)
{
    struct usb_device *usb;
    os_uint slot_id;

    for (slot_id = 1; slot_id < MAX_HC_SLOTS; slot_id++) {
        if (hc->slot_to_dev[slot_id]->port_id == port_id) {
            break;
        }
    }
    if (MAX_HC_SLOTS <= slot_id) {
        usb_dbg(USB_INFO, "no slot id is found for port id %d\n", port_id);
        return;
    }
    usb_dbg(USB_WARNING, "stop slot %d, port id %d\n", slot_id, port_id);
    usb = hc->slot_to_usb[slot_id];
    /* the field4 of command trb in linux:
     * 0002400 trb(9) enable slot
     * 2002c00 trb(b) address device
     * 2003000 trb(c) configure endpoint
     * ---- device removed ----
     * 2003000 trb(c) configure endpoint
     * 2002800 trb(a) disable slot
     */
    unenum_usb_device(usb);
    xhci_uninit_dev_slot(hc, usb);
    free_usb_device(usb);
    hc->slot_to_usb[slot_id] = OS_NULL;
}

/***************************************************************
 * description : who can tell me why no-op cannot be executed after port enable
 * history     :
 ***************************************************************/
LOCALC os_void xhci_issue_noop_command(struct xhci *hc)
{
    os_uint i;

    for (i = 0; i < 0x1f; i++) {
        /* assert lots of no-op command */
        hc->crcr->trb_ring[0].field[0] = 0;
        hc->crcr->trb_ring[0].field[1] = 0;
        hc->crcr->trb_ring[0].field[2] = 0;
        hc->crcr->trb_ring[0].field[3] = 1 | (TRB_CMD_NOOP << 10);
    }
    wmb();
    /* To ring the Host Controller Doorbell software shall write the Host Controller Doorbell register (offset 0 in the Doorbell Register Array),
       asserting the Host Controller Command value in the DB Target field and ¡®0¡¯in the DB Stream ID field. */
    hc->doorbell->doorbell[0] = 0;
    xhci_flush_post_buffer(hc->doorbell->doorbell[0]);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void xhci_rh_status_change(struct xhci *hc, struct xhci_trb *rsp)
{
    struct usb_device *usb;
    os_uint i;
    volatile os_u32 *reg;
    os_u32 status;
    os_u8 port_id;

#define trb_port_id(trb) ((((trb)->field[0]) >> 24) & 0xff)
    port_id = trb_port_id(rsp);

    /* Whether an xHC implementation supports port power switches or not,
       it shall automatically enable VBus on all Root Hub ports after a Chip Hardware Reset or HCRST.
       The initial state of an xHCI Root Hub ports shall be the Disconnected state */
    reg = &hc->operational->port[port_id-1].PORTSC;
    status = *reg;
    usb_dbg(USB_WARNING, "port %d change %x\n", port_id, status);

    if (PORTSC_CSC & status) {
        /* clear CSC & PRC */
        *reg = status & PORTSC_RSVD;
        /* device exist */
        if (PORTSC_CCS & status) {
            if (PORTSC_PED & status) {
                usb_dbg(USB_WARNING, "port is enabled status, usb3");
            } else {
                switch (portsc_pls(status)) {
                case 5:
                    usb_dbg(USB_WARNING, "port is disconnected status, usb3");
                    break;
                case 7:
                    usb_dbg(USB_WARNING, "port is usb2");
//*reg |= PORTSC_PP;
//delay_task(0x100, __LINE__);
//*reg |= PORTSC_PED;
//delay_task(0x100, __LINE__);
                    *reg |= (PORTSC_PR | PORTSC_PP);
#define PORT_RESET_DELAY 0x100
                    for (i = 0; i < PORT_RESET_DELAY; i++) {
                        status = *reg;
                        if ((0 == (status & PORTSC_PR))
                          && (0 == portsc_pls(status))
                          && (0 != (status & PORTSC_PED))
                          && (0 != (status & PORTSC_PRC))) {
                            status = *reg;
                            *reg = status & (PORTSC_RSVD | PORTSC_PLC);
                            break;
                        }
                        delay_task(1, __LINE__);
                    }
                    if (PORT_RESET_DELAY <= i) {
                        usb_dbg(USB_ERROR, "port reset failed\n");
                        goto fail;
                    }
                    break;
                default:
                    goto fail;
                    break;
                }
            }
            /* here port is enabled state and device is in default state */
            usb_dbg(USB_WARNING, "port enabled (%x)\n", *reg);
            // TODO: NO-OP FAILED here!!!
            //xhci_issue_noop_command(hc);
            start_xhci_dev(hc, port_id);
        } else {
            usb_dbg(USB_WARNING, "device is removed %x\n", *reg);
            /* device removed */
            stop_xhci_dev(hc, port_id);
        }
    }
    return;
  fail:
    usb_dbg(USB_ERROR, "xhci_rh_status_change failed");
}

/***************************************************************
 * description : TASK_FUNC_PTR
 * history     :
 ***************************************************************/
LOCALC os_ret OS_CALLBACK xhci_enum_task(os_u32 arg1, os_u32 arg2, os_u32 arg3, os_u32 arg4, os_u32 arg5, os_u32 arg6, os_u32 arg7)
{
    struct xhci_cmd_msg *msg;

    while (OS_SUCC == get_message((void **) &msg)) {
        switch (msg->head.msg_name) {
        case TRB_PORT_STATUS:
            xhci_rh_status_change(msg->hc, &msg->cmd_rsp);
            break;
        default:
            break;
        }
        free_msg(msg);
    }
    cassert(OS_FALSE);
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void dump_xhci_info(os_void)
{
    if (dbg_hc) {
        print("status: %x\n", dbg_hc->operational->USBSTS);
        print("cmd: %x\n", dbg_hc->operational->USBCMD);
        print("interrupt status: %x\n", dbg_hc->runtime->IR[0].IMAN);
        print("erdp: %x\n", dbg_hc->runtime->IR[0].ERDP);
    }
}

LOCALD os_u8 xhci_debug_name[] = { "xhci" };
LOCALD struct dump_info xhci_debug = {
    xhci_debug_name,
    dump_xhci_info
};

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void init_xhci_driver(os_void)
{
    os_ret result;

    xhci_enum_handle = create_task("xhci rh", xhci_enum_task, TASK_PRIORITY_6, 0, 0, 0, 0, 0, 0, 0);
    cassert(OS_NULL != xhci_enum_handle);
    active_task_station(xhci_enum_handle);

    result = register_pci_driver(&pci_xhci_driver);
    cassert(OS_SUCC == result);

    result = register_dump(&xhci_debug);
    cassert(OS_SUCC == result);
}
/* because intel support ehci and xhci port switching,
 * in order to master port for xhci before ehci,
 * priority of xhci is higher than ehci.
 */
bus_init_func(BUS_P3, init_xhci_driver);

