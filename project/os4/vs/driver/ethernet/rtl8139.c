/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : rtl8139.c
 * version     : 1.0
 * description : realtek 8139
 * author      : sicui
 * date        : 2014-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <lib.h>
#include <os.h>
#include <core.h>
#include <vbs.h>

//#define rtl_dbg(fmt, arg...) print(fmt, ##arg)
#define rtl_dbg(fmt, arg...) flog(fmt, ##arg)

/***************************************************************
 global variable declare
 ***************************************************************/
enum RTL8139_registers {
    MAC0 = 0, /* Ethernet hardware address. */
    MAR0 = 8, /* Multicast filter. */
    TxStatus0 = 0x10, /* Transmit status (Four 32bit registers). */
    TxAddr0 = 0x20, /* Tx descriptors (also four 32bit). */
    RxBuf = 0x30,
    ChipCmd = 0x37,
    RxBufPtr = 0x38,
    RxBufAddr = 0x3A,
    IntrMask = 0x3C,
    IntrStatus = 0x3E,
    TxConfig = 0x40,
    RxConfig = 0x44,
    Timer = 0x48, /* A general-purpose counter. */
    RxMissed = 0x4C, /* 24 bits valid, write clears. */
    Cfg9346 = 0x50,
    Config0 = 0x51,
    Config1 = 0x52,
    FlashReg = 0x54,
    MediaStatus = 0x58,
    Config3 = 0x59,
    Config4 = 0x5A, /* absent on RTL-8139A */
    HltClk = 0x5B,
    MultiIntr = 0x5C,
    TxSummary = 0x60,
    BasicModeCtrl = 0x62,
    BasicModeStatus = 0x64,
    NWayAdvert = 0x66,
    NWayLPAR = 0x68,
    NWayExpansion = 0x6A,
    /* Undocumented registers, but required for proper operation. */
    FIFOTMS = 0x70, /* FIFO Control and test. */
    CSCR = 0x74, /* Chip Status and Configuration Register. */
    PARA78 = 0x78,
    PARA7c = 0x7c, /* Magic transceiver parameter register. */
    Config5 = 0xD8, /* absent on RTL-8139A */
};

enum ClearBitMasks {
    MultiIntrClear = 0xF000,
    ChipCmdClear = 0xE2,
    Config1Clear = (1<<7)|(1<<6)|(1<<3)|(1<<2)|(1<<1),
};

enum ChipCmdBits {
    CmdReset = 0x10,
    CmdRxEnb = 0x08,
    CmdTxEnb = 0x04,
    RxBufEmpty = 0x01,
};

/* Interrupt register bits, using my own meaningful names. */
enum IntrStatusBits {
    PCIErr = 0x8000,
    PCSTimeout = 0x4000,
    RxFIFOOver = 0x40,
    RxUnderrun = 0x20,
    RxOverflow = 0x10,
    TxErr = 0x08,
    TxOK = 0x04,
    RxErr = 0x02,
    RxOK = 0x01,

    RxAckBits = RxFIFOOver | RxOverflow | RxOK,
};

enum TxStatusBits {
    TxHostOwns = 0x2000,
    TxUnderrun = 0x4000,
    TxStatOK = 0x8000,
    TxOutOfWindow = 0x20000000,
    TxAborted = 0x40000000,
    TxCarrierLost = 0x80000000,
};
enum RxStatusBits {
    RxMulticast = 0x8000,
    RxPhysical = 0x4000,
    RxBroadcast = 0x2000,
    RxBadSymbol = 0x0020,
    RxRunt = 0x0010,
    RxTooLong = 0x0008,
    RxCRCErr = 0x0004,
    RxBadAlign = 0x0002,
    RxStatusOK = 0x0001,
};

/* Bits in RxConfig. */
enum rx_mode_bits {
    AcceptErr = 0x20,
    AcceptRunt = 0x10,
    AcceptBroadcast = 0x08,
    AcceptMulticast = 0x04,
    AcceptMyPhys = 0x02,
    AcceptAllPhys = 0x01,
};

/* Bits in TxConfig. */
enum tx_config_bits {
    TxIFG1 = (1 << 25), /* Interframe Gap Time */
    TxIFG0 = (1 << 24), /* Enabling these bits violates IEEE 802.3 */
    TxLoopBack = (1 << 18) | (1 << 17), /* enable loopback test mode */
    TxCRC = (1 << 16), /* DISABLE appending CRC to end of Tx packets */
    TxClearAbt = (1 << 0), /* Clear abort (WO) */
    TxDMAShift = 8, /* DMA burst value (0-7) is shifted this many bits */
    TxRetryShift = 4, /* TXRR value (0-15) is shifted this many bits */

    TxVersionMask = 0x7C800000, /* mask out version bits 30-26, 23 */
};

/* Bits in Config1 */
enum Config1Bits {
    Cfg1_PM_Enable = 0x01,
    Cfg1_VPD_Enable = 0x02,
    Cfg1_PIO = 0x04,
    Cfg1_MMIO = 0x08,
    LWAKE = 0x10, /* not on 8139, 8139A */
    Cfg1_Driver_Load = 0x20,
    Cfg1_LED0 = 0x40,
    Cfg1_LED1 = 0x80,
    SLEEP = (1 << 1), /* only on 8139, 8139A */
    PWRDN = (1 << 0), /* only on 8139, 8139A */
};

/* Bits in Config3 */
enum Config3Bits {
    Cfg3_FBtBEn    = (1 << 0), /* 1 = Fast Back to Back */
    Cfg3_FuncRegEn = (1 << 1), /* 1 = enable CardBus Function registers */
    Cfg3_CLKRUN_En = (1 << 2), /* 1 = enable CLKRUN */
    Cfg3_CardB_En  = (1 << 3), /* 1 = enable CardBus registers */
    Cfg3_LinkUp    = (1 << 4), /* 1 = wake up on link up */
    Cfg3_Magic     = (1 << 5), /* 1 = wake up on Magic Packet (tm) */
    Cfg3_PARM_En   = (1 << 6), /* 0 = software can set twister parameters */
    Cfg3_GNTSel    = (1 << 7), /* 1 = delay 1 clock from PCI GNT signal */
};

/* Bits in Config4 */
enum Config4Bits {
    LWPTN = (1 << 2), /* not on 8139, 8139A */
};

/* Bits in Config5 */
enum Config5Bits {
    Cfg5_PME_STS = (1 << 0), /* 1 = PCI reset resets PME_Status */
    Cfg5_LANWake = (1 << 1), /* 1 = enable LANWake signal */
    Cfg5_LDPS = (1 << 2), /* 0 = save power when link is down */
    Cfg5_FIFOAddrPtr = (1 << 3), /* Realtek internal SRAM testing */
    Cfg5_UWF = (1 << 4), /* 1 = accept unicast wakeup frame */
    Cfg5_MWF = (1 << 5), /* 1 = accept multicast wakeup frame */
    Cfg5_BWF = (1 << 6), /* 1 = accept broadcast wakeup frame */
};

enum RxConfigBits {
    /* rx fifo threshold */
    RxCfgFIFOShift = 13,
    RxCfgFIFONone = (7 << RxCfgFIFOShift),

    /* Max DMA burst */
    RxCfgDMAShift = 8,
    RxCfgDMAUnlimited = (7 << RxCfgDMAShift),

    /* rx ring buffer length */
    RxCfgRcv8K = 0,
    RxCfgRcv16K = (1 << 11),
    RxCfgRcv32K = (1 << 12),
    RxCfgRcv64K = (1 << 11) | (1 << 12),

    /* Disable packet wrap at end of Rx buffer */
    RxNoWrap = (1 << 7),
};

/* Twister tuning parameters from RealTek.
   Completely undocumented, but required to tune bad links on some boards. */
enum CSCRBits {
    CSCR_LinkOKBit = 0x0400,
    CSCR_LinkChangeBit = 0x0800,
    CSCR_LinkStatusBits = 0x0f000,
    CSCR_LinkDownOffCmd = 0x003c0,
    CSCR_LinkDownCmd = 0x0f3c0,
};

enum Cfg9346Bits {
    Cfg9346_Lock = 0x00,
    Cfg9346_Unlock = 0xC0,
};

typedef enum {
    CH_8139 = 0,
    CH_8139_K,
    CH_8139A,
    CH_8139A_G,
    CH_8139B,
    CH_8130,
    CH_8139C,
    CH_8100,
    CH_8100B_8139D,
    CH_8101,
} chip_t;

enum chip_flags {
    HasHltClk = (1 << 0),
    HasLWake = (1 << 1),
};

#define IMR_SERR (1 << 15)
#define IMR_TIMEOUT (1 << 14)
#define IMR_LENCHG (1 << 13)
#define IMR_FOVW (1 << 6)
#define IMR_PUN_LINKCHG (1 << 5)
#define IMR_RXOVW (1 << 4)
#define IMR_TER (1 << 3)
#define IMR_TOK (1 << 2)
#define IMR_RER (1 << 1)
#define IMR_ROK (1 << 0)

#define RTL_INT (IMR_ROK | IMR_RER | IMR_TOK | IMR_TER | IMR_RXOVW | IMR_PUN_LINKCHG | IMR_FOVW | IMR_LENCHG | IMR_TIMEOUT | IMR_SERR)

#define EEPROM_LEN 0x40

#define RTL8139_MEM 1
#define RTL8139_IO 0

struct rtl8139_info {
    os_u32 operational_register;
    os_u8 mem_io; /* type */
    os_u8 mac_addr[8];
    os_u16 eeprom[EEPROM_LEN];

    os_u8 *rx_buff;
};

/***************************************************************
 function declare
 ***************************************************************/

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC inline os_u8 rtl_read_byte(struct rtl8139_info *rtl, os_u32 offset)
{
    u8 data;

    switch (rtl->mem_io) {
    case RTL8139_IO:
        inb_p(rtl->operational_register + offset, data);
        return data;
        break;
    case RTL8139_MEM:
        return read_byte(rtl->operational_register + offset);
        break;
    default:
        cassert(OS_FALSE);
        break;
    }
    return 0;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC inline os_ret rtl_write_byte(struct rtl8139_info *rtl, os_u32 offset, os_u8 data)
{
    switch (rtl->mem_io) {
    case RTL8139_IO:
        outb_p(rtl->operational_register + offset, data);
        break;
    case RTL8139_MEM:
        write_byte(rtl->operational_register + offset, data);
        break;
    default:
        cassert(OS_FALSE);
        break;
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC inline os_u16 rtl_read_word(struct rtl8139_info *rtl, os_u32 offset)
{
    u16 data;

    switch (rtl->mem_io) {
    case RTL8139_IO:
        inw_p(rtl->operational_register + offset, data);
        return data;
        break;
    case RTL8139_MEM:
        return read_word(rtl->operational_register + offset);
        break;
    default:
        cassert(OS_FALSE);
        break;
    }
    return 0;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC inline os_ret rtl_write_word(struct rtl8139_info *rtl, os_u32 offset, os_u16 data)
{
    switch (rtl->mem_io) {
    case RTL8139_IO:
        outw_p(rtl->operational_register + offset, data);
        break;
    case RTL8139_MEM:
        write_word(rtl->operational_register + offset, data);
        break;
    default:
        cassert(OS_FALSE);
        break;
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC inline os_u32 rtl_read_dword(struct rtl8139_info *rtl, os_u32 offset)
{
    u32 data;

    switch (rtl->mem_io) {
    case RTL8139_IO:
        inl_p(rtl->operational_register + offset, data);
        return data;
        break;
    case RTL8139_MEM:
        return read_dword(rtl->operational_register + offset);
        break;
    default:
        cassert(OS_FALSE);
        break;
    }
    return 0;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC inline os_ret rtl_write_dword(struct rtl8139_info *rtl, os_u32 offset, os_u32 data)
{
    switch (rtl->mem_io) {
    case RTL8139_IO:
        outl_p(rtl->operational_register + offset, data);
        break;
    case RTL8139_MEM:
        write_dword(rtl->operational_register + offset, data);
        break;
    default:
        cassert(OS_FALSE);
        break;
    }
    return OS_SUCC;
}

LOCALD struct rtl8139_info *dump_rtl = OS_NULL;

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void dump_rtl8139_info(os_void)
{
    os_u32 reg;

    if (dump_rtl) {
        rtl_dbg("dump\n");

        reg = rtl_read_byte(dump_rtl, ChipCmd);
        rtl_dbg("cmd: %x\n", reg);
        reg = rtl_read_word(dump_rtl, IntrStatus);
        rtl_dbg("intstatus: %x\n", reg);
        reg = rtl_read_word(dump_rtl, NWayLPAR);
        rtl_dbg("auto-neg: %x\n", reg);
        rtl_dbg("mac: %x:%x:%x:%x:%x:%x\n", dump_rtl->mac_addr[0],dump_rtl->mac_addr[1],dump_rtl->mac_addr[2],dump_rtl->mac_addr[3],dump_rtl->mac_addr[4],dump_rtl->mac_addr[5]);
        rtl_dbg("ee: %x %x %x %x\n", dump_rtl->eeprom[0], dump_rtl->eeprom[1], dump_rtl->eeprom[2], dump_rtl->eeprom[3]);
    }
}

LOCALD os_u8 rtl8139_debug_name[] = { "rtl8139" };
LOCALD struct dump_info rtl8139_debug = {
    rtl8139_debug_name,
    dump_rtl8139_info
};

/***************************************************************
 * description : setting to 1 forces the rtl8139c(l) to a software reset state which disables the transmitter and receiver,
 *               reininitializes the fifos, resets the system buffer pointer to the initial value (tx buffer is at tsad0, rx buffer is empty).
 *               the value of idr0-5 and mar0-7 and pci configuration space will have no changes.
 *               this bit is 1 during the reset operation, and is cleared to 0 by the rtl8139c(l) when the reset operation is complete.
 * history     :
 ***************************************************************/
LOCALC os_ret rtl8139_reset(struct rtl8139_info *rtl)
{
    int i;

    /* Soft reset the chip. */
    rtl_write_byte(rtl, ChipCmd, CmdReset);

    /* Check that the chip has finished the reset. */
    for (i = 0x100; i > 0; i--) {
        barrier();
        if (0 == (rtl_read_byte(rtl, ChipCmd) & CmdReset)) {
            return OS_SUCC;
        }
        delay_task(10, __LINE__);
    }
    return OS_FAIL;
}

#define RX_BUF_LEN_IDX 3 /* 0==8K, 1==16K, 2==32K, 3==64K */
#define RX_BUF_LEN (8192 << RX_BUF_LEN_IDX)
#define RX_BUF_PAD 16
#define RX_BUF_WRAP_PAD 0 /* spare padding to handle lack of packet wrap */
#define RX_BUF_TOT_LEN (RX_BUF_LEN + RX_BUF_PAD + RX_BUF_WRAP_PAD)

/* without the following, the compiler will make a mistake which cause panic.
os_void OS_API *alloc_coherent_mem(os_u32 size, os_u32 align, os_u32 line);
os_void OS_API *free_coherent_mem(os_void **addr, os_u32 line);
   also we could add the declaration to base.h.
   with the function declaration, the instructions are right.
 0101c1e7 <_init_rtl8139_buffer>:
 101c1e7:    55                       push   %ebp
 101c1e8:    89 e5                    mov    %esp,%ebp
 101c1ea:    53                       push   %ebx
 101c1eb:    83 ec 14                 sub    $0x14,%esp
 101c1ee:    8b 5d 08                 mov    0x8(%ebp),%ebx
 101c1f1:    c7 44 24 08 a9 01 00     movl   $0x1a9,0x8(%esp)
 101c1f8:    00
 101c1f9:    c7 44 24 04 04 00 00     movl   $0x4,0x4(%esp)
 101c200:    00
 101c201:    c7 04 24 10 00 01 00     movl   $0x10010,(%esp)
 101c208:    e8 a3 03 ff ff           call   100c5b0 <_alloc_coherent_mem@12>
 101c20d:    83 ec 0c                 sub    $0xc,%esp
 101c210:    89 43 08                 mov    %eax,0x8(%ebx)
 101c213:    85 c0                    test   %eax,%eax
 101c215:    75 13                    jne    101c22a <_init_rtl8139_buffer+0x43>
 101c217:    c7 04 24 f0 39 02 01     movl   $0x10239f0,(%esp)
 101c21e:    e8 9d e3 ff ff           call   101a5c0 <_print>
 101c223:    b8 01 00 00 00           mov    $0x1,%eax
 101c228:    eb 44                    jmp    101c26e <_init_rtl8139_buffer+0x87>
 101c22a:    8b 43 08                 mov    0x8(%ebx),%eax
 101c22d:    89 44 24 04              mov    %eax,0x4(%esp)
 101c231:    c7 04 24 0a 3a 02 01     movl   $0x1023a0a,(%esp)
 101c238:    e8 83 e3 ff ff           call   101a5c0 <_print>
 101c23d:    89 d8                    mov    %ebx,%eax
 101c23f:    b2 30                    mov    $0x30,%dl
 101c241:    8b 5b 08                 mov    0x8(%ebx),%ebx
 101c244:    0f b6 48 04              movzbl 0x4(%eax),%ecx
 101c248:    85 c9                    test   %ecx,%ecx
 101c24a:    74 07                    je     101c253 <_init_rtl8139_buffer+0x6c>
 101c24c:    83 f9 01                 cmp    $0x1,%ecx
 101c24f:    74 10                    je     101c261 <_init_rtl8139_buffer+0x7a>
 101c251:    eb 16                    jmp    101c269 <_init_rtl8139_buffer+0x82>
 101c253:    0f b6 d2                 movzbl %dl,%edx
 101c256:    03 10                    add    (%eax),%edx
 101c258:    89 d8                    mov    %ebx,%eax
 101c25a:    ef                       out    %eax,(%dx)
 101c25b:    eb 00                    jmp    101c25d <_init_rtl8139_buffer+0x76>
 101c25d:    eb 00                    jmp    101c25f <_init_rtl8139_buffer+0x78>
 101c25f:    eb 08                    jmp    101c269 <_init_rtl8139_buffer+0x82>
 101c261:    0f b6 d2                 movzbl %dl,%edx
 101c264:    8b 00                    mov    (%eax),%eax
 101c266:    89 1c 02                 mov    %ebx,(%edx,%eax,1)
 101c269:    b8 00 00 00 00           mov    $0x0,%eax
 101c26e:    8b 5d fc                 mov    0xfffffffc(%ebp),%ebx
 101c271:    c9                       leave
 101c272:    c3                       ret
   without the function declearation, the compiler do not know _alloc_coherent_mem is __stdcall, the instructions are wrong.
 0101c1e7 <_init_rtl8139_buffer>:
 101c1e7:    55                       push   %ebp
 101c1e8:    89 e5                    mov    %esp,%ebp
 101c1ea:    53                       push   %ebx
 101c1eb:    83 ec 14                 sub    $0x14,%esp
 101c1ee:    8b 5d 08                 mov    0x8(%ebp),%ebx
 101c1f1:    c7 44 24 08 aa 01 00     movl   $0x1aa,0x8(%esp)
 101c1f8:    00
 101c1f9:    c7 44 24 04 04 00 00     movl   $0x4,0x4(%esp)
 101c200:    00
 101c201:    c7 04 24 10 00 01 00     movl   $0x10010,(%esp)
 101c208:    e8 a3 03 ff ff           call   100c5b0 <_alloc_coherent_mem>
 101c20d:    89 43 08                 mov    %eax,0x8(%ebx)
 101c210:    85 c0                    test   %eax,%eax
 101c212:    75 13                    jne    101c227 <_init_rtl8139_buffer+0x40>
 101c214:    c7 04 24 f0 39 02 01     movl   $0x10239f0,(%esp)
 101c21b:    e8 a0 e3 ff ff           call   101a5c0 <_print>
 101c220:    b8 01 00 00 00           mov    $0x1,%eax
 101c225:    eb 44                    jmp    101c26b <_init_rtl8139_buffer+0x84>
 101c227:    8b 43 08                 mov    0x8(%ebx),%eax
 101c22a:    89 44 24 04              mov    %eax,0x4(%esp)
 101c22e:    c7 04 24 0a 3a 02 01     movl   $0x1023a0a,(%esp)
 101c235:    e8 86 e3 ff ff           call   101a5c0 <_print>
 101c23a:    89 d8                    mov    %ebx,%eax
 101c23c:    b2 30                    mov    $0x30,%dl
 101c23e:    8b 5b 08                 mov    0x8(%ebx),%ebx
 101c241:    0f b6 48 04              movzbl 0x4(%eax),%ecx
 101c245:    85 c9                    test   %ecx,%ecx
 101c247:    74 07                    je     101c250 <_init_rtl8139_buffer+0x69>
 101c249:    83 f9 01                 cmp    $0x1,%ecx
 101c24c:    74 10                    je     101c25e <_init_rtl8139_buffer+0x77>
 101c24e:    eb 16                    jmp    101c266 <_init_rtl8139_buffer+0x7f>
 101c250:    0f b6 d2                 movzbl %dl,%edx
 101c253:    03 10                    add    (%eax),%edx
 101c255:    89 d8                    mov    %ebx,%eax
 101c257:    ef                       out    %eax,(%dx)
 101c258:    eb 00                    jmp    101c25a <_init_rtl8139_buffer+0x73>
 101c25a:    eb 00                    jmp    101c25c <_init_rtl8139_buffer+0x75>
 101c25c:    eb 08                    jmp    101c266 <_init_rtl8139_buffer+0x7f>
 101c25e:    0f b6 d2                 movzbl %dl,%edx
 101c261:    8b 00                    mov    (%eax),%eax
 101c263:    89 1c 02                 mov    %ebx,(%edx,%eax,1)
 101c266:    b8 00 00 00 00           mov    $0x0,%eax
 101c26b:    83 c4 14                 add    $0x14,%esp
 101c26e:    5b                       pop    %ebx
 101c26f:    5d                       pop    %ebp
 101c270:    c3                       ret
*/

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret alloc_rtl8139_buffer(struct rtl8139_info *rtl)
{
    rtl->rx_buff = cmalloc(RX_BUF_TOT_LEN, 0x4);
    if (NULL == rtl->rx_buff) {
        rtl_dbg("alloc rtl rx buffer fail\n");
        return OS_FAIL;
    }
    rtl_write_dword(rtl, RxBuf, virt_to_phys((pointer) rtl->rx_buff));
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret free_rtl8139_buffer(struct rtl8139_info *rtl)
{
    if (rtl->rx_buff) {
        cfree(rtl->rx_buff);
        return OS_SUCC;
    }
    return OS_FAIL;
}

/*  EEPROM_Ctrl bits. */
#define EE_SHIFT_CLK 0x04 /* EEPROM shift clock. */
#define EE_CS 0x08 /* EEPROM chip select. */
#define EE_DATA_WRITE 0x02 /* EEPROM chip data in. */
#define EE_WRITE_0 0x00
#define EE_WRITE_1 0x02
#define EE_DATA_READ 0x01 /* EEPROM chip data out. */
#define EE_ENB (0x80 | EE_CS)

#define EE_WRITE_CMD (5)
#define EE_READ_CMD (6)
#define EE_ERASE_CMD (7)

/***************************************************************
 * description : read eeprom, addressed by words!
 * history     :
 ***************************************************************/
LOCALC os_u16 read_rtl8139_eeprom(struct rtl8139_info *rtl, os_u16 addr, os_u8 addr_len)
{
    os_sint i;
    os_u16 data;
    os_u16 ret; /* D15 - D0 */
    os_u16 read_cmd;

    read_cmd = addr | (EE_READ_CMD << addr_len);

    rtl_write_byte(rtl, Cfg9346, EE_ENB & ~EE_CS);
    rtl_write_byte(rtl, Cfg9346, EE_ENB);

    /* shift the read command bits out */
    for (i = 4 + addr_len; i >= 0; i--) {
        data = (read_cmd & (1 << i)) ? EE_DATA_WRITE : 0;
        rtl_write_byte(rtl, Cfg9346, EE_ENB | data);
        delay_us(1);
        rtl_write_byte(rtl, Cfg9346, EE_ENB | data | EE_SHIFT_CLK);
        delay_us(1);
    }
    rtl_write_byte(rtl, Cfg9346, EE_ENB);
    delay_ms(10);

    ret = 0;
    for (i = 16; i > 0; i--) {
        rtl_write_byte(rtl, Cfg9346, EE_ENB | EE_SHIFT_CLK);
        delay_us(1);
        ret = (ret << 1) | ((rtl_read_byte(rtl, Cfg9346) & EE_DATA_READ) ? 1 : 0);
        rtl_write_byte(rtl, Cfg9346, EE_ENB);
        delay_us(1);
    }

    /* terminate the EEPROM access */
    rtl_write_byte(rtl, Cfg9346, ~EE_CS);
    return ret;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret save_eeprom_context(struct rtl8139_info *rtl)
{
    os_u8 i;
    os_u8 addr_len;

#define EE_9356_SEL (1 << 6)
    if (rtl_read_dword(rtl, RxConfig) & EE_9356_SEL) {
        /* the eeprom used is 9356 */
        addr_len = 8;
    } else {
        /* the eeprom used is 9346 */
        addr_len = 6;
    }

    if (0x8129 != read_rtl8139_eeprom(rtl, 0, addr_len)) {
        rtl_dbg("test eeprom fail\n");
        addr_len = (8 == addr_len) ? 6 : 8;
        if (0x8129 != read_rtl8139_eeprom(rtl, 0, addr_len)) {
            rtl_dbg("test eeprom fail again\n");
            return OS_FAIL;
        }
    }

    for (i = 0; i < EEPROM_LEN; i++) {
        rtl->eeprom[i] = read_rtl8139_eeprom(rtl, i, addr_len);
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret rtl8139_probe(HDEVICE pci)
{
    struct rtl8139_info *rtl;
    os_u32 tmp;

    rtl = OS_NULL;

    /* allocate rtl8139 entity */
    rtl = kmalloc(sizeof(struct rtl8139_info));
    if (OS_NULL == rtl) {
        goto end;
    }
    mem_set(rtl, 0, sizeof(struct rtl8139_info));
    set_pci_dedicated(pci, rtl);
    dump_rtl = rtl;

#define OP_REG_MASK 0xffffff00
    /* check io */
    if (0x1 & get_pci_dev_cmd(pci)) {
        tmp = get_pci_dev_bar(pci, 0);
        if ((tmp & 0x1) && (OP_REG_MASK & tmp)) {
            rtl->operational_register = tmp;
            rtl->mem_io = RTL8139_IO;
            //rtl_dbg("iotype: %d, addr: %x\n", rtl->mem_io, rtl->operational_register);
        }
    }

    /* check mem */
    if (0x2 & get_pci_dev_cmd(pci)) {
        tmp = get_pci_dev_bar(pci, 1);
        /* rtl set mem31-8 as 0 */
        if ((0 == (tmp & 0x1)) && (0 != (OP_REG_MASK & tmp))) {
            rtl->operational_register = tmp;
            rtl->mem_io = RTL8139_MEM;
            //rtl_dbg("memtype: %d, addr: %x\n", rtl->mem_io, rtl->operational_register);
        }
    }

    cassert(!add_u32_overflow(rtl->operational_register, UINT8_MAX));

    rtl_dbg("rtl8139 type: %d, addr: %x\n", rtl->mem_io, rtl->operational_register);

    /* turn on the rtl8139 */
    rtl_write_byte(rtl, Cfg9346, Cfg9346_Unlock);
#define LED_MODE (0x0 << 6) /* refer to 5.5 LED Interface */
    rtl_write_byte(rtl, Config1, 0 | LED_MODE);
    rtl_write_byte(rtl, Cfg9346, Cfg9346_Lock);

    enable_pci_dma(pci);

    tmp = enable_pci_int(pci);
    if (OS_SUCC != tmp) {
        rtl_dbg("install rtl8139 pci device interrupt fail\n");
        goto end;
    }

    /* reset rtl8139 */
    tmp = rtl8139_reset(rtl);
    if (OS_SUCC != tmp) {
        goto end;
    }

    /* read mac */
    *(os_u32 *)(&rtl->mac_addr[0]) = rtl_read_dword(rtl, MAC0);
    *(os_u32 *)(&rtl->mac_addr[4]) = rtl_read_dword(rtl, MAC0 + 4);

    /* read eeprom */
    save_eeprom_context(rtl);

    /* unlock config0-4 */
    rtl_write_byte(rtl, Cfg9346, Cfg9346_Unlock);

    /* set mac address */

    /* must enable Tx/Rx before setting transfer thresholds! */
    rtl_write_byte(rtl, ChipCmd, CmdRxEnb); /* receiver enable: when set to 1, and the receive state machine is idle, the receive machine became active. */

    /* configure tcr/rcr */
#define WRAP (0 << 7) /* the rtl8139 will transfre the rest of the packet data into the beginning of the rx buffer if this packet has not been completely moved into the rx buffer and the transfer arrived at the end of the rx buffer. */
#define MXDMA (6 << 8) /* b110 = 1024 bytes, b111 = unlimited */
#define RBLEN (RX_BUF_LEN_IDX << 11) /* rx buffer length, b11 = 64k + 16 byte */
#define RXFTH (6 << 13) /* rx fifo threshold */
#define RER8 (0 << 16) /* the rtl8139 receives the error packet larger than 64-byte long when the rer8 bit is cleared. */
#define MULERINT (0 << 17) /* multiple early interrupt select */
#define ERTH (0 << 24) /* early rx threshold bit */
    tmp = AcceptErr | AcceptRunt | AcceptBroadcast | AcceptMulticast | AcceptMyPhys | AcceptAllPhys
        | WRAP | MXDMA | RBLEN | RXFTH | RER8 | MULERINT | ERTH;
    rtl_write_dword(rtl, RxConfig, tmp);

    /* lock config0-4 */
    rtl_write_byte(rtl, Cfg9346, Cfg9346_Lock);

    tmp = alloc_rtl8139_buffer(rtl);
    if (OS_SUCC != tmp) {
        goto end;
    }
    rtl_dbg("rtl rx buffer addr %x\n", rtl->rx_buff);

    rtl_write_dword(rtl, RxMissed, 0);

    /* enable transimit/receive (re/te in command register) */
    rtl_write_byte(rtl, ChipCmd, CmdRxEnb);

    /* enable imr */
    rtl_write_word(rtl, IntrMask, RTL_INT);

    return OS_SUCC;
  end:
    disable_pci_int(pci);
    free_rtl8139_buffer(rtl);
    if (rtl) { kfree(rtl); }
    return OS_FAIL;
}

/***************************************************************
 * description : pci共享中断的上半部
 * history     :
 ***************************************************************/
LOCALC os_ret ethernet_interrupt_1(HDEVICE pci)
{
    struct rtl8139_info *rtl;
    os_u16 isr;

    cassert(OS_NULL != pci);
    rtl = get_pci_dedicated(pci);
    if (NULL != rtl) {
        isr = rtl_read_word(rtl, IntrStatus);
        if (isr & RTL_INT) {
            rtl_write_word(rtl, IntrStatus, isr);
            print("rtl\n");
            return OS_SUCC;
        }
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret ethernet_interrupt_2(HDEVICE pci)
{
}

/***************************************************************
 * description : refer to 12.1.2 DID—Device Identification Register of 6-chipset-c200-chipset-datasheet.pdf
 *               and 10.3.1.5 Device ID (Word 0x0D) of 82579-gbe-phy-datasheet-vol-2-1.pdf
 * history     :
 ***************************************************************/
LOCALD const struct pci_device_id ethernet_ids = {
    /* vendor: realtek */
    PCI_VENDOR_ID_REALTEK, PCI_DEVICE_ID_REALTEK_8139,
    /* class: ethernet 0x20000 */
    PCI_CLASS_NETWORK_ETHERNET << 8, ~0
};

LOCALD const struct pci_driver rtl8139_driver = {
    "realtek 8139 Gigabit Network Connection",
    &ethernet_ids,
    rtl8139_probe,
    OS_NULL,
    ethernet_interrupt_1,
    ethernet_interrupt_2
};

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void init_ethernet(os_void)
{
    os_ret result;

    result = register_pci_driver(&rtl8139_driver);
    cassert(OS_SUCC == result);

    result = register_dump(&rtl8139_debug);
    cassert(OS_SUCC == result);
}
init_driver_call(init_ethernet);

