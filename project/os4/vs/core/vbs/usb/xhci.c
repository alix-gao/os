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

/***************************************************************
 function declare
 ***************************************************************/

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void xhci_flush_post_buffer(os_u64 reg)
{
    os_u64 tmp;
    tmp = reg;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret xhci_interrupt_1(HDEVICE pci)
{
    struct xhci *hc;
    os_u32 status;
    os_u32 temp;

    hc = get_pci_dedicated(pci);
    if (hc) {
        print("iman %x", hc->runtime->IR[0].IMAN);
        status = hc->operational->USBSTS;
        print("s:%x\n", status);
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
            temp = hc->runtime->IR[0].ERDP;
            temp = (temp + sizeof(struct xhci_trb)) | ERDP_EHB;
            hc->runtime->IR[0].ERDP = temp;
            xhci_flush_post_buffer(hc->runtime->IR[0].ERDP);
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
    cassert(OS_NULL != pci);

    return OS_FAIL;
}


/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC struct xhci *alloc_xhci(HDEVICE dev)
{
    struct xhci *hc;
    os_ret result;

    hc = kmalloc(sizeof(struct xhci));
    if (OS_NULL == hc) {
        usb_dbg(USB_ERROR, "alloc xhci fail");
        return OS_NULL;
    }
    mem_set(hc, 0, sizeof(struct xhci));
    hc->pci = dev; /* ¼ÇÂ¼pciÐÅÏ¢ */

    return hc;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void *free_xhci(struct xhci *hc)
{
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
    print("bar: %x %x\n", bar0, bar1);

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
LOCALC os_void *alloc_single_segment_ring(os_u32 cnt)
{
    struct xhci_trb *trb;

    cassert(cnt > 0x10);

    /* All Transfer Request Blocks shall be aligned on a 16-byte boundary. */
    trb = cmalloc(cnt * sizeof(struct xhci_trb), 32);
    if (trb) {
        mem_set(trb, 0, cnt * sizeof(struct xhci_trb));
        /* last trb is link */
        *(os_u64 *)(trb[cnt - 1].field) = (os_u64)(pointer) trb;
        trb[cnt - 1].field[2] = 0; /* interrupt target is 0 */
#define TOGGLE_CYCLE (0x2)
        trb[cnt - 1].field[3] = (TRB_LINK << 10) | TOGGLE_CYCLE;
    }
    return trb;
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
            print("switch oooooooooooooooooooo\n");
            out_pci_reg(hc->pci, 0xd4, 0xf);
            out_pci_reg(hc->pci, 0xd0, 0xf);
            out_pci_reg(hc->pci, 0xdc, 0xf);
            out_pci_reg(hc->pci, 0xd8, 0xf);
        }
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret request_xhci_ownership(struct xhci *hc)
{
    os_u32 cap;
    os_u32 offset;
    os_u8 *addr;

    cap = hc->capability->HCCPARAMS;
    print("HCCPARAMS: %x\n", cap);

#define get_xECP(cap) (((cap) & 0xffff0000) >> 14)
    offset = get_xECP(cap);

    cap = *(os_u32 *)((pointer) hc->capability + offset);
    while (cap & 0xff00) {
        offset += ((cap & 0xff00) >> 6);
        cap = *(os_u32 *)((pointer) hc->capability + offset);

        if (0x1 == (cap & 0xff)) {
            if (cap & 0x10000) {
                print("init xhci is owned by bios\n");
            }
            addr = (os_u8 *)((pointer) hc->capability + offset);
            *(addr + 3) = 0x1;
            while (*(addr + 2)) {
                delay_task(1, __LINE__);
            }
            print("usglegsup: %x\n", *(os_u32 *) addr);
            return OS_SUCC;
        }
    }
    return OS_FAIL;
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
#if 0
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
    while (hc->operational->USBSTS & STS_CNR) {
        delay_task(1, __LINE__);
    }

    /* hand off ports to xhci */
    switch_ports_to_xhci(hc);

    /* maxslotsen */
    tmp = hc->operational->CONFIG & (~HCS_SLOTS_MASK);
    hc->operational->CONFIG = tmp | HCS_MAX_SLOTS(hc->capability->HCSPARAMS_1);

    // Device Notification Control (only bit 1 is allowed)
    hc->operational->DNCTRL |= 1;

    /* dcbaap, The memory structure referenced by this physical memory pointer is assumed to be physically contiguous and 64-byte aligned. */
    hc->dcbaap = cmalloc(MAX_HC_SLOTS * sizeof(os_u64), 64);
    if (OS_NULL == hc->dcbaap) {
        usb_dbg(USB_ERROR, "alloc dcbaap fail");
        goto fail;
    }
    mem_set(hc->dcbaap, 0, MAX_HC_SLOTS * sizeof(os_u64));
    hc->operational->DCBAAP = virt_to_phys((pointer) hc->dcbaap);

    /* command ring */
    hc->crcr = alloc_single_segment_ring(0x20);
    if (OS_NULL == hc->crcr) {
        usb_dbg(USB_ERROR, "allocate cmd ring fail");
        goto fail;
    }
#define CMD_RING_RSVD_BITS (0x3f)
    hc->operational->CRCR = (hc->operational->CRCR & CMD_RING_RSVD_BITS) | (virt_to_phys((pointer) hc->crcr) & (~CMD_RING_RSVD_BITS));

    /* interrupt data structure, event ring */
#define EVENT_RING_TRB_COUNT 0x1f
    /* event ring do not use link trb, so extra trb is allocated.
     * Figure 20: Event Ring State Machine
     */
    hc->er = alloc_single_segment_ring(EVENT_RING_TRB_COUNT + 1);
    if (OS_NULL == hc->er) {
        usb_dbg(USB_ERROR, "allocate event ring fail");
        goto fail;
    }
#define ERST_SEGMENT_SIZE 1
    hc->erstba = cmalloc(ERST_SEGMENT_SIZE * sizeof(struct event_ring_segment_table_entry), 32);
    if (OS_NULL == hc->erstba) {
        usb_dbg(USB_ERROR, "allocate event ring segment table fail");
        goto fail;
    }
    hc->erstba->rsvdz = 0;
    hc->erstba->ring_segment_size = EVENT_RING_TRB_COUNT & 0xffff;
    hc->erstba->ring_segment_base_address = virt_to_phys((pointer) hc->er) & (~0x3f);

#define ERST_SIZE_MASK (0xffff)
    hc->runtime->IR[0].ERSTSZ = (hc->runtime->IR[0].ERSTSZ & (~ERST_SIZE_MASK)) | (ERST_SEGMENT_SIZE & ERST_SIZE_MASK);
    wmb();
#define ERST_DESI_MASK 0x7
#define ERST_ERDP_MASK (~0xf)
#define ERST_EHB_MASK (0x8)
    hc->runtime->IR[0].ERDP = ERST_EHB_MASK | (hc->runtime->IR[0].ERDP & (~ERST_ERDP_MASK)) | (virt_to_phys((pointer) hc->er) & ERST_ERDP_MASK);
    wmb();
#define ERST_PTR_MASK (~0x3f)
    hc->runtime->IR[0].ERSTBA = (hc->runtime->IR[0].ERSTBA & (~ERST_PTR_MASK)) | (virt_to_phys((pointer) hc->erstba) & ERST_PTR_MASK);

    /* dma & interrupt */
    enable_pci_dma(pci);
    result = enable_pci_int(pci);
    if (OS_SUCC != result) {
        usb_dbg(USB_ERROR, "pci device install interrupt fail");
        goto fail;
    }
    /* only enable primary */
#define DEFAULT_IMODI 4000
#define DEFAULT_IMODC 4
    //hc->runtime->IR[0].IMOD = DEFAULT_IMODI | (DEFAULT_IMODC << 16);
    print("imod: %x\n", hc->runtime->IR[0].IMOD);
    hc->runtime->IR[0].IMOD = 0;
    wmb();
    hc->operational->USBCMD |= (CMD_EIE | CMD_HSEIE);
    wmb();
    /* Enable the Interrupter by writing a ¡®1¡¯ to the Interrupt Enable (IE) field of the Interrupter Management register (5.5.2.1). */
    //hc->runtime->IR[0].IMAN = (hc->runtime->IR[0].IMAN & (~0x3)) | 0x3;
    //hc->runtime->IR[0].IMAN = (hc->runtime->IR[0].IMAN & (~0x3)) | 0x2;
    //hc->runtime->IR[0].IMAN |= 0x2; // no interrupt is generated
    hc->runtime->IR[0].IMAN |= 0x3; // generate interrupt
    /* Most systems have write buffers that minimize overhead,
    but this may require a read operation to guarantee that the write has been flushed from posted buffers. */
    tmp = hc->runtime->IR[0].IMAN;
    wmb();

    hc->operational->USBSTS |= (STS_FATAL | STS_EINT | STS_PORT | STS_SRE);
    wmb();

    /* run */
    hc->operational->USBCMD |= CMD_RUN;

    usb_dbg(USB_WARNING, "xhci probe done");
    dbg_hc = hc;
    print("hc: %x\n", hc);

    return OS_SUCC;
  fail:
    usb_dbg(USB_ERROR, "xhci probe fail");
    disable_pci_int(pci);
    if (OS_NULL != hc) {
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
os_u64 woccc = 9;
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
        dbg_hc->runtime->IR[0].ERDP = 0x8 | (dbg_hc->runtime->IR[0].ERDP);
        print("erdp: %x\n", dbg_hc->runtime->IR[0].ERDP);
        print("port status: %x\n", dbg_hc->operational->PORTSC);
#if 1
        print("0: %x\n", dbg_hc->er[0].field[0]);
        print("1: %x\n", dbg_hc->er[0].field[1]);
        print("2: %x\n", dbg_hc->er[0].field[2]);
        print("3: %x\n", dbg_hc->er[0].field[3]);
        print("0: %x\n", dbg_hc->er[1].field[0]);
        print("1: %x\n", dbg_hc->er[1].field[1]);
        print("2: %x\n", dbg_hc->er[1].field[2]);
        print("3: %x\n", dbg_hc->er[1].field[3]);
#endif
        /* assert no-op command */
        dbg_hc->crcr[0].field[0] = 0;
        dbg_hc->crcr[0].field[1] = 0;
        dbg_hc->crcr[0].field[2] = 0;
        dbg_hc->crcr[0].field[3] = 1 | (0x8 << 10);
        /* To ring the Host Controller Doorbell software shall write the Host Controller Doorbell register (offset 0 in the
Doorbell Register Array), asserting the Host Controller Command value in the DB Target field and ¡®0¡¯ in the
DB Stream ID field. */
        dbg_hc->doorbell->doorbell[0] = 0;
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

    result = register_pci_driver(&pci_xhci_driver);
    cassert(OS_SUCC == result);

    result = register_dump(&xhci_debug);
    cassert(OS_SUCC == result);
}
/* because intel support ehci and xhci port switching,
 * in order to master port for xhci before ehci,
 * priority of xhci is higher than ehci.
 */
//bus_init_func(BUS_P3, init_xhci_driver);

