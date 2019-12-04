/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : ohci.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __OHCI_H__
#define __OHCI_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/
/* The Host Controller (HC) contains a set of on-chip operational registers which are mapped into a
   noncacheable portion of the system addressable space. These registers are used by the Host
   Controller Driver (HCD). According to the function of these registers, they are divided into four
   partitions, specifically for Control and Status, Memory Pointer, Frame Counter and Root Hub. All
   of the registers should be read and written as Dwords. */
#define ohci_in(r) read_dword(r)
#define ohci_out(r, v) write_dword(r, v)

/* maximum OHCI root hub ports */
#define OHCI_RH_PORTS_CNT 15

/* part of the OHCI standard */
#define INT_ED_TABLE_CNT 32

/* OHCI CONTROL AND STATUS REGISTER MASKS */

/* HcControl (control) register masks */
#define OHCI_CTRL_CBSR  (UINT32_C(0x3) << 0)    /* control/bulk service ratio */
#define OHCI_CTRL_PLE   (UINT32_C(0x1) << 2)    /* periodic list enable */
#define OHCI_CTRL_IE    (UINT32_C(0x1) << 3)    /* isochronous enable */
#define OHCI_CTRL_CLE   (UINT32_C(0x1) << 4)    /* control list enable */
#define OHCI_CTRL_BLE   (UINT32_C(0x1) << 5)    /* bulk list enable */
#define OHCI_CTRL_HCFS  (UINT32_C(0x3) << 6)    /* host controller functional state */
#define OHCI_CTRL_IR    (UINT32_C(0x1) << 8)    /* interrupt routing */
#define OHCI_CTRL_RWC   (UINT32_C(0x1) << 9)    /* remote wakeup connected */
#define OHCI_CTRL_RWE   (UINT32_C(0x1) << 10)   /* remote wakeup enable */

/* pre-shifted values for HCFS */
#define OHCI_USB_RESET   (UINT32_C(0x0) << 6)
#define OHCI_USB_RESUME  (UINT32_C(0x1) << 6)
#define OHCI_USB_OPER    (UINT32_C(0x2) << 6)
#define OHCI_USB_SUSPEND (UINT32_C(0x3) << 6)

/* HcCommandStatus (cmdstatus) register masks */
#define OHCI_HCR (UINT32_C(0x1) << 0)    /* host controller reset */
#define OHCI_CLF (UINT32_C(0x1) << 1)    /* control list filled */
#define OHCI_BLF (UINT32_C(0x1) << 2)    /* bulk list filled */
#define OHCI_OCR (UINT32_C(0x1) << 3)    /* ownership change request */
#define OHCI_SOC (UINT32_C(0x3) << 16)   /* scheduling overrun count */

/* masks used with interrupt registers:
   HcInterruptStatus (intrstatus)
   HcInterruptEnable (intrenable)
   HcInterruptDisable (intrdisable) */
#define OHCI_INTR_SO    (UINT32_C(0x1) << 0)    /* scheduling overrun */
#define OHCI_INTR_WDH   (UINT32_C(0x1) << 1)    /* writeback of done_head */
#define OHCI_INTR_SF    (UINT32_C(0x1) << 2)    /* start frame */
#define OHCI_INTR_RD    (UINT32_C(0x1) << 3)    /* resume detect */
#define OHCI_INTR_UE    (UINT32_C(0x1) << 4)    /* unrecoverable error */
#define OHCI_INTR_FNO   (UINT32_C(0x1) << 5)    /* frame number overflow */
#define OHCI_INTR_RHSC  (UINT32_C(0x1) << 6)    /* root hub status change */
#define OHCI_INTR_OC    (UINT32_C(0x1) << 30)   /* ownership change */
#define OHCI_INTR_MIE   (UINT32_C(0x1) << 31)   /* master interrupt enable */

/* roothub.a masks */
#define RH_A_NDP    (UINT32_C(0xff) << 0)    /* number of downstream ports */
#define RH_A_PSM    (UINT32_C(0x1) << 8)     /* power switching mode */
#define RH_A_NPS    (UINT32_C(0x1) << 9)     /* no power switching */
#define RH_A_DT     (UINT32_C(0x1) << 10)    /* device type (mbz) */
#define RH_A_OCPM   (UINT32_C(0x1) << 11)    /* over current protection mode */
#define RH_A_NOCP   (UINT32_C(0x1) << 12)    /* no over current protection */
#define RH_A_POTPGT (UINT32_C(0xff) << 24)   /* power on to power good time */

/* roothub.b masks */
#define RH_B_DR     UINT32_C(0x0000ffff) /* device removable flags */
#define RH_B_PPCM   UINT32_C(0xffff0000) /* port power control mask */

/* roothub.status bits */
#define RH_HS_LPS   UINT32_C(0x00000001) /* local power status */
#define RH_HS_OCI   UINT32_C(0x00000002) /* over current indicator */
#define RH_HS_DRWE  UINT32_C(0x00008000) /* device remote wakeup enable */
#define RH_HS_LPSC  UINT32_C(0x00010000) /* local power status change */
#define RH_HS_OCIC  UINT32_C(0x00020000) /* over current indicator change */
#define RH_HS_CRWE  UINT32_C(0x80000000) /* clear remote wakeup enable */

/* roothub.portstatus[] bits */
#define RH_PS_CCS   UINT32_C(0x00000001) /* current connect status */
#define RH_PS_PES   UINT32_C(0x00000002) /* port enable status*/
#define RH_PS_PSS   UINT32_C(0x00000004) /* port suspend status */
#define RH_PS_POCI  UINT32_C(0x00000008) /* port over current indicator */
#define RH_PS_PRS   UINT32_C(0x00000010) /* port reset status */
#define RH_PS_PPS   UINT32_C(0x00000100) /* port power status */
#define RH_PS_LSDA  UINT32_C(0x00000200) /* low speed device attached */
#define RH_PS_CSC   UINT32_C(0x00010000) /* connect status change */
#define RH_PS_PESC  UINT32_C(0x00020000) /* port enable status change */
#define RH_PS_PSSC  UINT32_C(0x00040000) /* port suspend status change */
#define RH_PS_OCIC  UINT32_C(0x00080000) /* over current indicator change */
#define RH_PS_PRSC  UINT32_C(0x00100000) /* port reset status change */

#define OHCI_ED_SKIP (UINT32_C(0x1) << 14)
#define OHCI_ED_SPEED (UINT32_C(0x1) << 13)
#define OHCI_ED_IN (UINT32_C(0x2) << 11)
#define OHCI_ED_OUT (UINT32_C(0x1) << 11)

/***************************************************************
 enum define
 ***************************************************************/
/***************************************************************
 * enum name   :
 * description :
 ***************************************************************/
enum ohci_ed_state {
    OHCI_ED_UNUSED,
    OHCI_ED_USED,
    OHCI_ED_LINKED
};

/***************************************************************
 struct define
 ***************************************************************/
/***************************************************************
 * description :
 ***************************************************************/
struct ohci_urb {
    struct list_node list; /* 分片td的头节点 */
    os_u32 td_count;
    HEVENT wait;
};

/* TD info field */
#define TD_CC UINT32_C(0xf0000000)
#define TD_CC_GET(td_p) (((td_p) >> 28) & UINT32_C(0x0f))
#define TD_CC_SET(td_p, cc) (td_p) = ((td_p) & UINT32_C(0x0fffffff)) | (((cc) & UINT32_C(0x0f)) << 28)
#define TD_EC       UINT32_C(0x0C000000)
#define TD_T        UINT32_C(0x03000000)
#define TD_T_DATA0  UINT32_C(0x02000000)
#define TD_T_DATA1  UINT32_C(0x03000000)
#define TD_T_TOGGLE UINT32_C(0x00000000) /* get toggle from ed */
#define TD_R        UINT32_C(0x00040000)
#define TD_DI       UINT32_C(0x00E00000)
#define TD_DI_SET(X) (((X) & UINT32_C(0x07))<< 21)
#define TD_DP       UINT32_C(0x00180000)
#define TD_DP_SETUP UINT32_C(0x00000000)
#define TD_DP_IN    UINT32_C(0x00100000)
#define TD_DP_OUT   UINT32_C(0x00080000)

#define TD_ISO      UINT32_C(0x00010000)
#define TD_DEL      UINT32_C(0x00020000)

#define TD_ADDR_MASK UINT32_C(0xfffffff0)

/*
 * CC 4.3.3 Completion Codes
 */
#define TD_CC_NOERROR      0x00
#define TD_CC_CRC          0x01
#define TD_CC_BITSTUFFING  0x02
#define TD_CC_DATATOGGLEM  0x03
#define TD_CC_STALL        0x04
#define TD_DEVNOTRESP      0x05
#define TD_PIDCHECKFAIL    0x06
#define TD_UNEXPECTEDPID   0x07
#define TD_DATAOVERRUN     0x08
#define TD_DATAUNDERRUN    0x09
/* 0x0A, 0x0B reserved for hardware */
#define TD_BUFFEROVERRUN   0x0C
#define TD_BUFFERUNDERRUN  0x0D
/* 0x0E, 0x0F reserved for HCD */
#define TD_NOTACCESSED     0x0F

/***************************************************************
 * description :
 ***************************************************************/
#define OHCI_TD_ALIGN 0x10
struct ohci_td {
    /* hardware */
    os_u32 hwINFO;
    os_u32 hwCBP;    /* Current Buffer Pointer */
    os_u32 hwNextTD; /* Next TD Pointer */
    os_u32 hwBE;     /* Memory Buffer End Pointer */

    /* software */
    os_bool (*complete)(struct ohci_td *td, os_u32 error); /* complete function */
    os_void (*app_complete)(os_u8 *data); /* 用户级完成功能 */

    enum usb_transfer_type td_type; /* td的传输类型 */
    struct ohci_ed *ed; /* 自动重连接 */

    /* 传输数据缓存 */
    os_u8 *data;
    os_u32 data_len;

    /* 分片传输的信息 */
    struct ohci_urb *urb;
    struct list_node urb_node;
    struct ohci_td *next_done_td;
} _aligned_(OHCI_TD_ALIGN);

/***************************************************************
 * description :
 ***************************************************************/
struct ohci_iso_td {
    /* hardware */
    os_u32 hwINFO;
    os_u32 hwCBP;    /* Current Buffer Pointer */
    os_u32 hwNextTD; /* Next TD Pointer */
    os_u32 hwBE;     /* Memory Buffer End Pointer */
} _aligned_(32);

#define ED_FA UINT32_C(0x7f)
#define ED_MPS UINT32_C(0xf800ffff)
#define ED_C UINT32_C(0x2)
#define ED_H UINT32_C(0x1)

/***************************************************************
 * description : hcir1_0a.pdf 4.2.1 endpoint descriptor format
 ***************************************************************/
struct ohci_ed {
    /* hardware */
    os_u32 hwINFO;
    os_u32 hwTailP;
    os_u32 hwHeadP;
    os_u32 hwNextED;

    /* software */
    enum usb_transfer_type type;
    enum ohci_ed_state state;
    HEVENT ed_lock;
    struct list_node ed_node;

    /* interrupt trans */
    os_u32 int_interval; /* 中断间隔 */
    os_u32 count; /* 负载计数 */

    os_u32 proportion;
} _aligned_(16);

/***************************************************************
 * description : The HCCA (Host Controller Communications Area) is a 256 byte structure defined in the OHCI spec.
 *               that the host controller is told the base address of. It must be 256-byte aligned.
 ***************************************************************/
struct ohci_hcca {
    os_u32 int_table[INT_ED_TABLE_CNT]; /* Interrupt ED table */
    os_u16 frame_no; /* current frame number */
    os_u16 pad1; /* set to 0 on each frame_no change */
    os_u32 done_head; /* info returned for an interrupt */
    os_u8 reserved_for_hc[116];
} _aligned_(256);

/***************************************************************
 * description : host controller operational registers
 *               see hcir_0a.pdf section 7 operational registers
 *               pci地址空间要求内存32位对齐
 ***************************************************************/
struct ohci_regs {
    /* control and status registers */
    os_u32 revision;
    os_u32 control;
    os_u32 cmdstatus;
    os_u32 intrstatus;
    os_u32 intrenable;
    os_u32 intrdisable;
    /* memory pointers */
    os_u32 hcca;
    os_u32 ed_periodcurrent;
    os_u32 ed_controlhead;
    os_u32 ed_controlcurrent;
    os_u32 ed_bulkhead;
    os_u32 ed_bulkcurrent;
    os_u32 donehead;
    /* frame counters */
    os_u32 fminterval;
    os_u32 fmremaining;
    os_u32 fmnumber;
    os_u32 periodicstart;
    os_u32 lsthresh;
    /* Root hub ports */
    struct ohci_roothub_regs {
        os_u32 a;
        os_u32 b;
        os_u32 status;
        os_u32 portstatus[OHCI_RH_PORTS_CNT];
    } roothub;
} _aligned_(32);

#define STATIC_INT_ED_COUNT 63

/***************************************************************
 * description : host controller operational registers
 *               see hcir_0a.pdf section 7 operational registers
 *               256字节对齐
 ***************************************************************/
struct ohci {
    struct ohci_hcca hcca; /* 256字节对齐 */
    /* endpoint descriptor resource */
    struct ohci_ed ed[USB_DEVICE_COUNT][EP_NUM][2]; /* 16字节对齐 */
    /* 中断静态ed描述符 */
    struct ohci_ed int_ed[STATIC_INT_ED_COUNT]; /* 16字节对齐 */

    os_u8 addr_bitmap[USB_DEVICE_COUNT/8];

    /* 硬件寄存器地址 */
    struct ohci_regs *regs;

    /* message between int1 & int2 */
    os_bool interrupt;
    os_bool sof;
    HEVENT wait_sof;

    spinlock_t ohci_lock;

    /* td完成队列 */
    os_u32 done;

    struct usb_device *rh_dev[OHCI_RH_PORTS_CNT];
};

#define OHCI_RH_MSG_ID 0x29
/***************************************************************
 * description :
 ***************************************************************/
struct ohci_rh_msg {
    struct message head;
    struct ohci *hc;
};

/***************************************************************
 extern function
 ***************************************************************/

#pragma pack()

#endif /* end of header */

