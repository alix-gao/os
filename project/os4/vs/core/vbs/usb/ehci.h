/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : ehci.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2011-04-22
 ***************************************************************/

#ifndef __EHCI_H__
#define __EHCI_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/

/* Register Space. Implementation-specific parameters and capabilities, plus operational control and
   status registers. This space, normally referred to as I/O space, must be implemented as memory-mapped
   I/O space.
   Memory-mapped USB Host Controller Registers. This block of registers is memory-mapped into
   non-cacheable memory (see Figure 1-3). This memory space must begin on a DWord (32-bit) boundary.
   This register space is divided into two sections: a set of read-only capability registers and a set of
   read/write operational registers. */
#define ehci_in(r) read_dword(r)
#define ehci_out(r, v) write_dword(r, v)

/* 不关心HCCPARAMS, USBCMD的启动默认值中Frame List Size = 0, 为1024. */
#define PERIODIC_FRAME_LIST_NUM UINT32_C(1024)

#define INTERRUPT_FRAME_LIST_NUM PERIODIC_FRAME_LIST_NUM
/* binary tree: INTERRUPT_FRAME_LIST_NUM * 2 - 1;
   4 uframe: 1
   2 uframe: 1
   1 uframe: 1
   qh terminate: 1 */
#define INT_ENDPOINT_NULL_NUM (INTERRUPT_FRAME_LIST_NUM * 2 - 1 + 3 + 1)

/* frame list link pointer type */
typedef os_u32 fllp_type;

/* capability parameters */
#define EHCI_CP_64AC (1 << 0)
#define ehci_cp_eecp(HCCPARAMS) ((UINT32_C(0x0000ff00) & (HCCPARAMS)) >> 8)

/* extended capibilites */
#define EHCI_EECP_CAP_ID UINT32_C(0x000000ff)
#define ehci_eecp_neecp(eecp) ((UINT32_C(0x0000ff00) & (eecp)) >> 8)
#define EHCI_EECP_HOOS (UINT32_C(0x1) << 24)
#define EHCI_EECP_HBOS (UINT32_C(0x1) << 16)

/* structural parameters, port power control */
#define EHCI_SP_PPC (UINT32_C(0x1) << 4)

#define EHCI_PORTSC_CCS (UINT32_C(0x1) << 0)
#define EHCI_PORTSC_CSC (UINT32_C(0x1) << 1)
#define EHCI_PORTSC_PED (UINT32_C(0x1) << 2)
#define EHCI_PORTSC_PEDC (UINT32_C(0x1) << 3)
#define EHCI_PORTSC_OCC (UINT32_C(0x1) << 5)
#define EHCI_PORTSC_PS (UINT32_C(0x1) << 7)
#define EHCI_PORTSC_PR (UINT32_C(0x1) << 8)
#define EHCI_PORTSC_PP (UINT32_C(0x1) << 12)
#define EHCI_PORTSC_PO (UINT32_C(0x1) << 13)
#define EHCI_PORTSC_LS (UINT32_C(0x3) << 10)

/* usb interrupt enable */
#define EHCI_INTR_UIE (UINT32_C(0x1) << 0)
/* usb error interrupt enable */
#define EHCI_INTR_UEIE (UINT32_C(0x1) << 1)
/* port change interrupt enable */
#define EHCI_INTR_PCIE (UINT32_C(0x1) << 2)
/* frame list rollover enable */
#define EHCI_INTR_FLRE (UINT32_C(0x1) << 3)
/* host system error enable */
#define EHCI_INTR_HSEE (UINT32_C(0x1) << 4)
/* interrupt on async advance enable */
#define EHCI_INTR_IOAAE (UINT32_C(0x1) << 5)

#define EHCI_INT_MASK (EHCI_INTR_UIE | EHCI_INTR_UEIE | EHCI_INTR_PCIE | EHCI_INTR_HSEE | EHCI_INTR_IOAAE)

/***************************************************************
 enum define
 ***************************************************************/

/***************************************************************
 struct define
 ***************************************************************/
/***************************************************************
 * description : ehci_r10.pdf 2.2 Host Controller Capability Registers
 ***************************************************************/
struct ehci_cap_regs {
    os_u8 CAPLENGTH; /* Capability Register Length */
    os_u8 Reserved;
    os_u16 HCIVERSION; /* Interface Version Number */
    os_u32 HCSPARAMS; /* Structural Parameters */
    os_u32 HCCPARAMS; /* Capability Parameters */
    os_u8 HCSP_PORTROUTE[8]; /* Companion Port Route Description */
} __attribute__((packed));

/* cmd */
#define EHCI_CMD_RS (UINT32_C(0x1) << 0)
#define EHCI_CMD_RESET (UINT32_C(0x1) << 1)
#define EHCI_CMD_PSE (UINT32_C(0x1) << 4)
#define EHCI_CMD_ASE (UINT32_C(0x1) << 5)
#define EHCI_CMD_DOORBELL (UINT32_C(0x1) << 6)
#define EHCI_CMD_LHCR (UINT32_C(0x1) << 7)

/* status */
#define EHCI_STS_ASE (UINT32_C(0x1) << 15)
#define EHCI_STS_PSE (UINT32_C(0x1) << 14)
#define EHCI_STS_RECLAMATION (UINT32_C(0x1) << 13)
#define EHCI_STS_HALT (UINT32_C(0x1) << 12)
#define EHCI_STS_IAAD (UINT32_C(0x1) << 5)
#define EHCI_STS_HSE (UINT32_C(0x1) << 4)
#define EHCI_STS_FLR (UINT32_C(0x1) << 3)
#define EHCI_STS_PCD (UINT32_C(0x1) << 2)
#define EHCI_STS_UEI (UINT32_C(0x1) << 1)
#define EHCI_STS_UI (UINT32_C(0x1) << 0)

/***************************************************************
 * description : ehci_r10.pdf 2.3 Host Controller Operational Registers
 ***************************************************************/
struct ehci_operational_regs {
    os_u32 USBCMD; /* USB Command */
    os_u32 USBSTS; /* USB Status */
    os_u32 USBINTR; /* USB Interrupt Enable */
    os_u32 FRINDEX; /* USB Frame Index */
    os_u32 CTRLDSSEGMENT; /* 4G Segment Selector */
    os_u32 PERIODICLISTBASE; /* Frame List Base Address */
    os_u32 ASYNCLISTADDR; /* Next Asynchronous List Address */
    os_u8 Reserved[36];
    os_u32 CONFIGFLAG; /* Configured Flag Register */
    os_u32 PORTSC[1]; /* 1-N_PORTS, Port Status/Control */
} __attribute__((packed));

/***************************************************************
 * description : host controller operational registers
 *               see ehci_r10.pdf 2. Register Interface
 *               pci地址空间要求内存256字节对齐
 ***************************************************************/
struct ehci_regs {
    struct ehci_cap_regs *caps;
    struct ehci_operational_regs *operation;
};

#define get_qtd_addr(pointer) (UINT32_C(0xffffffe0) & (os_u32)(pointer))
#define QTD_TERMINATE 1

#define QTD_PID_OUT 0
#define QTD_PID_IN 1
#define QTD_PID_SETUP 2

#define QTD_TOGGLE_0 0
#define QTD_TOGGLE_1 1

/* common macro */
#define clear_and_set(data, linfo, lmask, offset) ((data) = ((~((lmask) << (offset))) & (data)) | (((lmask) & (linfo)) << (offset)))
#define get_info(data, lmask, offset) (((data) >> (offset)) & (lmask))

#define QTD_CTRL_DT (UINT32_C(0x1) << 31)
#define set_qtd_toggle(ctrl, t) (clear_and_set(ctrl, t, 1, 31))
#define set_qtd_tbt(ctrl, len) (clear_and_set(ctrl, len, UINT32_C(0x7fff), 16))
#define get_qtd_tbt(ctrl) (get_info(ctrl, UINT32_C(0x7fff), 16))
#define QTD_CTRL_IOC (UINT32_C(0x1) << 15)
#define set_qtd_cp(ctrl, cp) (clear_and_set(ctrl, cp, UINT32_C(0x7), 12))
#define set_qtd_cerr(ctrl, cnt) (clear_and_set(ctrl, cnt, UINT32_C(0x3), 10))
#define get_qtd_pid(ctrl) (get_info(ctrl, UINT32_C(0x3), 8))
#define set_qtd_pid(ctrl, pid) (clear_and_set(ctrl, pid, UINT32_C(0x3), 8))

#define qtd_pid_is_in(tok) (1 == (((tok)>>8) & 0x3))

#define QTD_STS_MASK UINT32_C(0xff)

#define QTD_STS_ACTIVE (UINT32_C(0x1) << 7)
/* e.g., stall */
#define QTD_STS_HALT (UINT32_C(0x1) << 6)
/* data buffer error */
#define QTD_STS_DBE (UINT32_C(0x1) << 5)
/* babble detected */
#define QTD_STS_BD (UINT32_C(0x1) << 4)
/* transaction error(XactErr) */
#define QTD_STS_TE (UINT32_C(0x1) << 3)
/* missed micro-frame */
#define QTD_STS_MMF (UINT32_C(0x1) << 2)
#define QTD_STS_SPLITXSTATE (UINT32_C(0x1) << 1)
#define QTD_STS_PING (UINT32_C(0x1) << 0)

#define EHCI_TRANS_PAGE_NUM 5

#define QTD_CERR_NUM 3

/* Buffer Pointer List. Each element in the list is a 4K page aligned, physical memory address. */
#define QTD_BPL_MASK (0x1000 - 1)

/* max Total Bytes to Transfer. */
#define MAX_QTD_TBT 0x5000

/***************************************************************
 * description : ehci queue element transfer descriptor
 *               32-byte (cache line) aligned
 ***************************************************************/
struct ehci_qtd_hw {
    os_u32 next_qtd_pointer;
    os_u32 alternate_next_qtd_pointer;
    os_u32 buffer_control;
    os_u32 buffer_pointer[EHCI_TRANS_PAGE_NUM];
    /* ehci 64-bit data structure */
    os_u32 appendix_64[EHCI_TRANS_PAGE_NUM];
} _aligned_(32);

struct ehci;

/***************************************************************
 * description : ehci queue element transfer descriptor
 *               32-byte (cache line) aligned
 ***************************************************************/
struct ehci_qtd {
    /* hardware part */
    struct ehci_qtd_hw hwinfo;

    struct ehci_qh *qh;
    struct list_node qh_list; /* endpoint list */

    /* software part */
    struct list_node doing_list; /* doing list */

    struct ehci_urb *urb;
    struct list_node urb_list;

    os_bool (*complete)(struct ehci *hc, struct ehci_qtd *qtd);

    /* record for reuse of qtd */
    os_u8 *data;
    os_u32 data_len;
} _aligned_(32);

/***************************************************************
 * description : encapsulation of multi transactions
 ***************************************************************/
struct ehci_urb {
    struct list_node qtd; /* urb list */
    os_u32 count; /* count of qtd in urb */

    HEVENT urb_sem;
    HTIMER thandle;
    os_bool timeout;

    os_void (*app_complete)(os_u8 *data);
};

/***************************************************************
 * enum name   :
 * description :
 ***************************************************************/
enum ehci_ed_state {
    EHCI_ED_NEW,
    EHCI_ED_OLD,
    EHCI_ED_BUTT
};

/***************************************************************
 * enum name   : transfer descriptor type
 * description : refer to table 3-18
 ***************************************************************/
enum td_type {
    EHCI_ITD,
    EHCI_QH,
    EHCI_SITD,
    EHCI_FSTN
};
#define QH_INVALID 1
#define QH_VALID 0
#define set_qhhlp(qhhlp, pointer, type, t) ((qhhlp) = (UINT32_C(0xffffffe0) & (os_u32)(pointer)) | ((UINT32_C(0x3) & (type)) << 1) | (UINT32_C(0x1) & (t)))
#define mod_qhhlp_addr(qhhlp, pointer) ((qhhlp) = (UINT32_C(0xffffffe0) & (os_u32)(pointer)) | (UINT32_C(0x1f) & (qhhlp)))

/* endpoint characteristics */
/* nak count reload */
#define ep_ncr(cha, rl) (clear_and_set(cha, rl, UINT32_C(0xf), 28))
/* control endpoint flag */
#define ep_cef(cha, c) (clear_and_set(cha, c, UINT32_C(0x1), 27))
/* maximum packet length */
#define ep_mpl(cha, mpl) (clear_and_set(cha, mpl, UINT32_C(0x7ff), 16))
#define get_ep_mps(cha) (get_info(cha, UINT32_C(0x7ff), 16))
/* head of reclamation list flag */
#define ep_hrlf(cha, h) (clear_and_set(cha, h, UINT32_C(0x1), 15))
/* data toggle control */
#define ep_dtc(cha, d) (clear_and_set(cha, d, UINT32_C(0x1), 14))
/* endpoint speed */
#define EHCI_CHA_FULL_SPEED 0
#define EHCI_CHA_LOW_SPEED 1
#define EHCI_CHA_HIGH_SPEED 2
#define ep_eps(cha, eps) (clear_and_set(cha, eps, UINT32_C(0x3), 12))
#define get_eps(cha) (get_info(cha, UINT32_C(0x3), 12))
/* endpoint number */
#define ep_endpt(cha, endpt) (clear_and_set(cha, endpt, UINT32_C(0xf), 8))
/* inactivate on next transaction */
#define ep_iont(cha, i) (clear_and_set(cha, i, UINT32_C(0x1), 7))
/* device address */
#define ep_da(cha, da) (clear_and_set(cha, da, UINT32_C(0x7f), 0))

/* endpoint capabilities */
/* high-bandwidth pipe multiplier */
#define EP_MULT_ONE 1
#define EP_MULT_TWO 2
#define EP_MULT_THREE 3
#define ep_hbpm(cap, h) (clear_and_set(cap, h, UINT32_C(0x3), 30))
/* port number */
#define ep_pn(cap, p) (clear_and_set(cap, p, UINT32_C(0x7f), 23))
/* hub addr */
#define ep_ha(cap, h) (clear_and_set(cap, h, UINT32_C(0x7f), 16))
/* split completion mask */
#define ep_scm(cap, s) (clear_and_set(cap, s, UINT32_C(0xff), 8))
/* interrupt schedule mask */
#define ep_ism(cap, i) (clear_and_set(cap, i, UINT32_C(0xff), 0))
#define get_ep_ism(cap) (get_info(cap, UINT32_C(0xff), 0))

#define EHCI_QH_ALIGN 32

/***************************************************************
 * description : ehci queue head
 *               32-byte (cache line) aligned
 *               相当于端点描述符
 ***************************************************************/
struct ehci_qh_hw {
    /* hardware part */
    os_u32 qhhlp; /* queue head horizontal link pointer */
    os_u32 characteristics; /* endpoint characteristics */
    os_u32 capabilities; /* endpoint capabilities */
    os_u32 curr_qtd_pointer;
    /* The DWords 4-11 of a queue head are the transaction overlay area. This area has the same base structure as
       a Queue Element Transfer Descriptor */
    struct ehci_qtd_hw overlay;
} _aligned_(EHCI_QH_ALIGN);

/***************************************************************
 * description : ehci queue head
 *               32-byte (cache line) aligned
 *               相当于端点描述符
 ***************************************************************/
struct ehci_qh {
    /* hardware part */
    struct ehci_qh_hw hwinfo;

    /* software part */
    HEVENT qh_lock;
    struct list_node qh; /* for struct ehci_qh */

    enum ehci_ed_state state;
    enum usb_transfer_type type;

    /* qtd list, notice the order */
    struct list_node qtd;

    os_u32 int_interval;
    os_u32 load_cnt;

    os_u32 proportion;
} _aligned_(EHCI_QH_ALIGN); // for continuous

#define EHCI_RH_PORTS_CNT 15 /* maximum EHCI root hub ports */

/***************************************************************
 * description : ehci info
 *               refer to ehci_r10.pdf
 ***************************************************************/
struct ehci {
    /* 硬件寄存器地址 */
    struct ehci_regs regs;

    /* pci信息 */
    HDEVICE pci; // for common

    /* 控制器上usb设备数量最大128 */
    os_u8 addr_bitmap[USB_DEVICE_COUNT/8];

    /* 64位内存高位 */
    os_u32 appendix_64;

    //struct ehci_qh endpoint[USB_DEVICE_COUNT][EP_NUM][USB_DIR_CNT];
    struct ehci_qh *endpoint[USB_DEVICE_COUNT];

    /* 资源列表 */
    os_u32 *PERIODICLISTBASE; /* 周期列表的内存地址 */
    os_u32 periodic_list_size; /* 1024 */
    struct ehci_qh *qh_int;

    struct ehci_qh *async_head; /* 异步头节点, reclamation */
    struct ehci_qtd *qtd_null; /* for short read */

    os_u32 doorbell_flag;
    HEVENT doorbell_sem;

    spinlock_t ehci_lock;
    struct list_node doing_list; // for qtd
    os_u32 done_flag;

    struct usb_device *rh_dev[EHCI_RH_PORTS_CNT];
};

#define EHCI_RH_MSG_ID 0x27

/***************************************************************
 * description :
 ***************************************************************/
struct ehci_rh_msg {
    struct message head;
    struct ehci *hc;
};

/***************************************************************
 extern function
 ***************************************************************/

#pragma pack()

#endif /* end of header */

