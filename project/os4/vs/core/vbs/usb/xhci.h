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

/***************************************************************
 * enum name   :
 * description :
 ***************************************************************/
enum trb_complete_code {
    TRB_CC_INVALID = 0, // invalid
    TRB_CC_SUCCESS = 1, // success
    TRB_CC_DBE = 2, // data buffer error
    TRB_CC_BDE = 3, // babble detected error
    TRB_CC_UTE = 4, // usb transaction error
    TRB_CC_TE = 5, // trb error
    TRB_CC_SE = 6, // stall error
    TRB_CC_RE = 7, // resource error
    TRB_CC_BE = 8, // bandwidth error
    TRB_CC_NSA = 9, // no slots available
    TRB_CC_ISTE = 10, // invalid stream type error
    TRB_CC_SNEE = 11, // slot not enabled error
    TRB_CC_ENEE = 12, // endpoint not enabled error
    TRB_CC_SP = 13, // short packet
    TRB_CC_RU = 14, // ring underrun
    TRB_CC_RO = 15, // ring overrun
    TRB_CC_VERFE = 16, // vf event ring full error
    TRB_CC_PE = 17, // parameter error
    TRB_CC_BOE = 18, // bandwidth overrun error
    TRB_CC_CSE = 19, // context state error
    TRB_CC_NPRE = 20, // no ping response error
    TRB_CC_ERFE = 21, // event ring full error
    TRB_CC_IDE = 22, // incomatible device error
    TRB_CC_MSE = 23, // missed service error
    TRB_CC_CRS = 24, // command ring stopped
    TRB_CC_CA = 25, // command aborted
    TRB_CC_STOPPED = 26, // stopped
    TRB_CC_SLI = 27, // stopped - length invalid
    TRB_CC_RSVD1 = 28, // reserved
    TRB_CC_MELTLE = 29, // max exit latency too large error
    TRB_CC_RSVD2 = 30, // reserved
    TRB_CC_IBO = 31, // isoch buffer overrun
    TRB_CC_ELE = 32, // event lost error
    TRB_CC_UE = 33, // undefined error
    TRB_CC_ISIE = 34, // invalid stream id error
    TRB_CC_SBE = 35, // secondary bandwith error
    TRB_CC_STE = 36, // split transaction error
    TRB_CC_RSVD3 = 37 // reserved
    // 192-223 vendor defined error
    // 224-255 vendor defined info
};

#define TRB_C 0x1
#define TRB_ISP (1 << 2)
#define TRB_CH (1 << 4)
#define TRB_IOC (1 << 5)
#define TRB_TD_SIZE_MASK (0x1f << 17)
#define trb_td_size(size) (((size) & 0x1f) << 17)
#define trb_type(trb) ((((trb)->field[3]) >> 10) & 0x3f)
#define trb_result(trb) (((trb)->field[2] >> 24) & 0xff)
#define trb_slot_id(trb) (((trb)->field[3] >> 24) & 0xff)
#define trb_endpoint_id(trb) (((trb)->field[3] >> 16) & 0x1f)
#define trb_event_data(trb) (((trb)->field[3] >> 2) & 0x1)
#define trb_c(trb) ((trb)->field[3] & TRB_C)
#define get_trb_ioc(trb) (((trb)->field[3] >> 5) & 0x1)
#define TOGGLE_CYCLE (0x2)

#define trb_req(rsp) ((*(os_u64 *)&(rsp)->field[0]) & (~0xf))

#define TRB_SP (1 << 23)

#define DEFAULT_INTERRUPT_TARGET (0 << 22)

enum xhci_trt {
    TRT_NO_DATA_STAGE = 0,
    TRT_RESERVED = 1,
    TRT_OUT_DATA_STAGE = 2,
    TRT_IN_DATA_STAGE = 3
};

#define MAX_TRB_TRANSFER_LENGTH 0x10000

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

#define HCC_CSZ_MASK (1 << 2)

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

#define PORTSC_CCS (1 << 0)
#define PORTSC_CSC (1 << 17)
#define PORTSC_PED (1 << 1)
#define portsc_pls(s) (((s) >> 5) & 0xf)
#define PORTSC_PR (1 << 4)
#define PORTSC_PRC (1 << 21)
#define PORTSC_PLC (1 << 22)
#define port_speed(portsc) (((portsc) >> 10) & 0xf)
#define PORTSC_PP (1 << 9)

#define PORTSC_RSVD (~PORTSC_PED)

struct xhci_port_reg_set {
    os_u32 PORTSC;
    os_u32 PORTPMSC;
    os_u32 PORTLI;
    os_u32 rsvd;
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

#define MAX_RH_PORT_CNT 255

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
    struct xhci_port_reg_set port[MAX_RH_PORT_CNT];
} __attribute__((packed));

#define ERDP_EHB (1 << 3)
#define ERDP_MASK (0xfffffffffffffff0ULL)

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
    os_u32 doorbell[MAX_RH_PORT_CNT + 1]; // 0 is hc
} __attribute__((packed));

struct event_ring_segment_table_entry {
    os_u64 ring_segment_base_address;
    os_u32 ring_segment_size;
    os_u32 rsvdz;
};

// Consumer Cycle State (CCS) & Producer Cycle State (PCS)
#define XHCI_INIT_CS 1

/* interrupt data structure, event ring */
#define EVENT_RING_TRB_COUNT 0x1f
#define CMD_RING_TRB_COUNT 0x20

struct xhci;
typedef os_void (*trb_finish)(struct xhci *hc, struct xhci_trb *trb, os_void *para);

struct trb_para {
    HEVENT handle;
    struct xhci_trb event; // event trb
};

struct trb_shadow {
    trb_finish func;
    struct trb_para para;
};

#define TRB_CHECK 0xc1c11c1c
struct xhci_trb_wrap {
    os_uint count;
    os_uint index;
    os_u8 cycle; // toggle cycle of ring
    struct xhci_trb *trb_ring;
    HEVENT trb_lock;
    os_u32 check;
    struct trb_shadow shadow[0];
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
    os_u64 *dcbaap;
    struct xhci_trb_wrap *crcr;
    struct event_ring_segment_table_entry *erstba; // event ring segment table
    struct xhci_trb_wrap *er; // event ring

    os_u16 page_size;
    os_u8 context_size;

    os_bool int_occur;

    os_u8 enum_port_id;

    struct usb_device *slot_to_usb[MAX_HC_SLOTS];
    struct xhci_dev *slot_to_dev[MAX_HC_SLOTS];
};

enum xhci_ep_type {
    XHCI_EP_INVALID = 0,
    XHCI_EP_ISO_OUT = 1,
    XHCI_EP_BULK_OUT = 2,
    XHCI_EP_INT_OUT = 3,
    XHCI_EP_CONTROL_BID = 4,
    XHCI_EP_ISO_IN = 5,
    XHCI_EP_BULK_IN = 6,
    XHCI_EP_INT_IN = 7
};

#define ADC_BSR (1 << 9)

/* Host Initiate Disable */
#define EP_HID (1 << 7)

#define EP_TRB_CNT 0x20

#define XHCI_EP_CNT (EP_NUM * USB_DIR_CNT)

#define XHCI_EP0_ID 1
#define xhci_dci(n, dir) (2*(n) + (((dir) == USB_OUT)?0:1))

#define endpoint_index(n, dir) (1 + xhci_dci(n, dir))
#define endpoint_flag(n, dir) (1 << xhci_dci(n, dir))

#define EP0_INDEX endpoint_index(XHCI_EP0_ID, USB_OUT)

#define DEFAULT_ERR_CNT 3

/***************************************************************
 * description :
 ***************************************************************/
struct xhci_dev {
    struct xhci *hc;
    os_u8 port_id;
    os_u8 slot_id; // slot id is usb address for xhci driver

    os_u32 *input_context, *output_context;
    struct xhci_trb_wrap *ep[XHCI_EP_CNT];
};

/***************************************************************
 * description : universal xhci command message
 ***************************************************************/
struct xhci_cmd_msg {
    struct message head;
    struct xhci *hc;
    struct xhci_trb cmd_rsp;
};

/***************************************************************
 * description : save port change event
 ***************************************************************/
struct pcs_trb {
    struct list_node node;
    struct xhci_trb pcs;
};

/***************************************************************
 extern function
 ***************************************************************/

#pragma pack()

#endif

