/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : xhci.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2020-04-22
 ***************************************************************/

#ifndef __XHCI_H__
#define __XHCI_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/

/***************************************************************
 enum define
 ***************************************************************/

/***************************************************************
 struct define
 ***************************************************************/

/***************************************************************
 * enum name   :
 * description :
 ***************************************************************/
enum trb_type {
    /* bulk, interrupt, isoc scatter/gather, and control data stage */
    TRB_NORMAL = 1,
    /* setup stage for control transfers */
    TRB_SETUP = 2,
    /* data stage for control transfers */
    TRB_DATA = 3,
    /* status stage for control transfers */
    TRB_STATUS = 4,
    /* isoc transfers */
    TRB_ISOC = 5,
    /* TRB for linking ring segments */
    TRB_LINK = 6,
    TRB_EVENT_DATA = 7,
    /* Transfer Ring No-op (not for the command ring) */
    TRB_TR_NOOP = 8,
    /* Command TRBs */
    /* Enable Slot Command */
    TRB_ENABLE_SLOT = 9,
    /* Disable Slot Command */
    TRB_DISABLE_SLOT = 10,
    /* Address Device Command */
    TRB_ADDR_DEV = 11,
    /* Configure Endpoint Command */
    TRB_CONFIG_EP = 12,
    /* Evaluate Context Command */
    TRB_EVAL_CONTEXT = 13,
    /* Reset Endpoint Command */
    TRB_RESET_EP = 14,
    /* Stop Transfer Ring Command */
    TRB_STOP_RING = 15,
    /* Set Transfer Ring Dequeue Pointer Command */
    TRB_SET_DEQ = 16,
    /* Reset Device Command */
    TRB_RESET_DEV = 17,
    /* Force Event Command (opt) */
    TRB_FORCE_EVENT = 18,
    /* Negotiate Bandwidth Command (opt) */
    TRB_NEG_BANDWIDTH = 19,
    /* Set Latency Tolerance Value Command (opt) */
    TRB_SET_LT = 20,
    /* Get port bandwidth Command */
    TRB_GET_BW = 21,
    /* Force Header Command - generate a transaction or link management packet */
    TRB_FORCE_HEADER = 22,
    /* No-op Command - not for transfer rings */
    TRB_CMD_NOOP = 23,
    /* TRB IDs 24-31 reserved */
    /* Event TRBS */
    /* Transfer Event */
    TRB_TRANSFER = 32,
    /* Command Completion Event */
    TRB_COMPLETION = 33,
    /* Port Status Change Event */
    TRB_PORT_STATUS = 34,
    /* Bandwidth Request Event (opt) */
    TRB_BANDWIDTH_EVENT = 35,
    /* Doorbell Event (opt) */
    TRB_DOORBELL = 36,
    /* Host Controller Event */
    TRB_HC_EVENT = 37,
    /* Device Notification Event - device sent function wake notification */
    TRB_DEV_NOTE = 38,
    /* MFINDEX Wrap Event - microframe counter wrapped */
    TRB_MFINDEX_WRAP = 39
    /* TRB IDs 40-47 reserved, 48-63 is vendor-defined */
};

struct xhci_trb {
    os_u32 field[4];
} __attribute__((packed));

#define MAX_HC_SLOTS 256

/* HCSPARAMS1 - hcs_params1 - bitmasks */
/* bits 0:7, Max Device Slots */
#define HCS_MAX_SLOTS(p) (((p) >> 0) & 0xff)
#define HCS_SLOTS_MASK 0xff
/* bits 8:18, Max Interrupters */
#define HCS_MAX_INTRS(p) (((p) >> 8) & 0x7ff)
/* bits 24:31, Max Ports - max value is 0x7F = 127 ports */
#define HCS_MAX_PORTS(p) (((p) >> 24) & 0x7f)

struct xhci_cap_reg {
    os_u8 CAPLENGTH;
    os_u8 Rsvd_1;
    os_u16 HCIVERSION;
    os_u32 HCSPARAMS_1;
    os_u32 HCSPARAMS_2;
    os_u32 HCSPARAMS_3;
    os_u32 HCCPARAMS;
    os_u32 DBOFF;
    os_u32 RTSOFF;
    os_u8 Rsvd_2[0];
} __attribute__((packed));

#define CMD_RUN (1 << 0)
#define CMD_RESET (1 << 1)
/* Event Interrupt Enable - a '1' allows interrupts from the host controller */
#define CMD_EIE (1 << 2)
/* Host System Error Interrupt Enable - get out-of-band signal for HC errors */
#define CMD_HSEIE (1 << 3)
/* bits 4:6 are reserved (and should be preserved on writes). */
/* light reset (port status stays unchanged) - reset completed when this is 0 */
#define CMD_LRESET (1 << 7)
/* FIXME: ignoring host controller save/restore state for now. */
#define CMD_CSS (1 << 8)
#define CMD_CRS (1 << 9)
/* Enable Wrap Event - '1' means xHC generates an event when MFINDEX wraps. */
#define CMD_EWE (1 << 10)
/* MFINDEX power management - '1' means xHC can stop MFINDEX counter if all root
 * hubs are in U3 (selective suspend), disconnect, disabled, or powered-off.
 * '0' means the xHC can power it off if all ports are in the disconnect,
 * disabled, or powered-off state.
 */
#define CMD_PM_INDEX (1 << 11)
/* bits 12:31 are reserved (and should be preserved on writes). */

/* HC not running - set to 1 when run/stop bit is cleared. */
#define STS_HALT (1 << 0)
/* serious error, e.g. PCI parity error.  The HC will clear the run/stop bit. */
#define STS_FATAL (1 << 2)
/* event interrupt - clear this prior to clearing any IP flags in IR set*/
#define STS_EINT (1 << 3)
/* port change detect */
#define STS_PORT (1 << 4)
/* bits 5:7 reserved and zeroed */
/* save state status - '1' means xHC is saving state */
#define STS_SAVE (1 << 8)
/* restore state status - '1' means xHC is restoring state */
#define STS_RESTORE (1 << 9)
/* true: save or restore error */
#define STS_SRE (1 << 10)
/* true: Controller Not Ready to accept doorbell or op reg writes after reset */
#define STS_CNR (1 << 11)
/* true: internal Host Controller Error - SW needs to reset and reinitialize */
#define STS_HCE (1 << 12)

struct xhci_op_reg {
    os_u32 USBCMD;
    os_u32 USBSTS;
    os_u32 PAGESIZE;
    os_u32 RsvdZ_1;
    os_u32 RsvdZ_2;
    os_u32 DNCTRL;
    os_u64 CRCR;
    /* rsvd: offset 0x20-2F */
    os_u32 RsvdZ_3[4];
    os_u64 DCBAAP;
    os_u32 CONFIG;
    /* rsvd: offset 0x3C-3FF */
    os_u32 RsvdZ_4[241];
    /* port 1 registers, which serve as a base address for other ports */
    os_u32 PORTSC;
    os_u32 PORTPMSC;
    os_u32 PORTLI;
    os_u32 rsvd;
    /* registers for ports 2-255 */
#define NUM_PORT_REGS 4
    os_u32 reserved6[NUM_PORT_REGS*254];
} __attribute__((packed));

#define ERDP_EHB (1 << 3)

struct xhci_intr_reg {
    os_u32 IMAN;
    os_u32 IMOD;
    os_u32 ERSTSZ;
    os_u32 RsvdP;
    os_u64 ERSTBA;
    os_u64 ERDP;
} __attribute__((packed));

struct xhci_run_reg {
    os_u32 MFINDEX;
    os_u32 RsvdZ[7];
    struct xhci_intr_reg IR[128];
} __attribute__((packed));

struct xhci_db_reg {
    os_u32 doorbell[256];
} __attribute__((packed));

struct event_ring_segment_table_entry {
    os_u64 ring_segment_base_address;
    os_u32 ring_segment_size;
    os_u32 rsvdz;
};

/***************************************************************
 * description :
 ***************************************************************/
struct xhci {
    HDEVICE pci; // for common

    volatile struct xhci_cap_reg *capability;
    volatile struct xhci_op_reg *operational;
    volatile struct xhci_run_reg *runtime;
    volatile struct xhci_db_reg *doorbell;

    /* bak virtual address */
    os_u32 *dcbaap;
    struct xhci_trb *crcr;
    struct event_ring_segment_table_entry *erstba; // event ring segment table
    struct xhci_trb *er; // event ring

    /* . */
};

/***************************************************************
 extern function
 ***************************************************************/

#pragma pack()

#endif

