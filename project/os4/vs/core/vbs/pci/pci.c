/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : pci.c
 * version     : 1.0
 * description : (key) not hot-plug
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include <vbs.h>

/***************************************************************
 global variable declare
 ***************************************************************/

#define MAX_PCI_BUS_NUM 0x100
#define MAX_PCI_DEVICE_NUM 32
#define MAX_PCI_FUNC_NUM 8

#define MAX_PCI_INT_NUM 0x4

/* Under PCI, each device has 256 bytes of configuration address space,
   of which the first 64 bytes are standardized as follows: */
#define PCI_VENDOR_ID 0x00 /* 16 bits */
#define PCI_DEVICE_ID 0x02 /* 16 bits */
#define PCI_COMMAND 0x04 /* 16 bits */
#define PCI_COMMAND_IO 0x1 /* Enable response in I/O space */
#define PCI_COMMAND_MEMORY 0x2 /* Enable response in Memory space */
#define PCI_COMMAND_MASTER 0x4 /* Enable bus mastering */
#define PCI_COMMAND_SPECIAL 0x8 /* Enable response to special cycles */
#define PCI_COMMAND_INVALIDATE 0x10 /* Use memory write and invalidate */
#define PCI_COMMAND_VGA_PALETTE 0x20 /* Enable palette snooping */
#define PCI_COMMAND_PARITY 0x40 /* Enable parity checking */
#define PCI_COMMAND_WAIT 0x80 /* Enable address/data stepping */
#define PCI_COMMAND_SERR 0x100 /* Enable SERR */
#define PCI_COMMAND_FAST_BACK 0x200 /* Enable back-to-back writes */

#define PCI_STATUS 0x06 /* 16 bits */
#define PCI_STATUS_CAP_LIST 0x10 /* Support Capability List */
#define PCI_STATUS_66MHZ 0x20 /* Support 66 Mhz PCI 2.1 bus */
#define PCI_STATUS_UDF 0x40 /* Support User Definable Features [obsolete] */
#define PCI_STATUS_FAST_BACK 0x80 /* Accept fast-back to back */
#define PCI_STATUS_PARITY 0x100 /* Detected parity error */
#define PCI_STATUS_DEVSEL_MASK 0x600 /* DEVSEL timing */
#define PCI_STATUS_DEVSEL_FAST 0x000
#define PCI_STATUS_DEVSEL_MEDIUM 0x200
#define PCI_STATUS_DEVSEL_SLOW 0x400
#define PCI_STATUS_SIG_TARGET_ABORT 0x800 /* Set on target abort */
#define PCI_STATUS_REC_TARGET_ABORT 0x1000 /* Master ack of " */
#define PCI_STATUS_REC_MASTER_ABORT 0x2000 /* Set on master abort */
#define PCI_STATUS_SIG_SYSTEM_ERROR 0x4000 /* Set when we drive SERR */
#define PCI_STATUS_DETECTED_PARITY 0x8000 /* Set on parity error */
#define PCI_CLASS_REVISION 0x08 /* High 24 bits are class, low 8 revision */
#define PCI_REVISION_ID 0x08 /* Revision ID */
#define PCI_CLASS_PROG 0x09 /* Reg. Level Programming Interface */
#define PCI_CLASS_DEVICE 0x0a /* Device class */

#define PCI_CACHE_LINE_SIZE 0x0c /* 8 bits */
#define PCI_LATENCY_TIMER 0x0d /* 8 bits */
#define PCI_HEADER_TYPE 0x0e /* 8 bits */
#define PCI_HEADER_TYPE_NORMAL 0
#define PCI_HEADER_TYPE_BRIDGE 1
#define PCI_HEADER_TYPE_CARDBUS 2

#define PCI_BIST 0x0f /* 8 bits */
#define PCI_BIST_CODE_MASK 0x0f /* Return result */
#define PCI_BIST_START 0x40 /* 1 to start BIST, 2 secs or less */
#define PCI_BIST_CAPABLE 0x80 /* 1 if BIST capable */

/* Base addresses specify locations in memory or I/O space.
   Decoded size can be determined by writing a value of
   0xffffffff to the register, and reading it back.  Only
   1 bits are decoded. */
#define PCI_BASE_ADDRESS_0 0x10 /* 32 bits */
#define PCI_BASE_ADDRESS_1 0x14 /* 32 bits [htype 0,1 only] */
#define PCI_BASE_ADDRESS_2 0x18 /* 32 bits [htype 0 only] */
#define PCI_BASE_ADDRESS_3 0x1c /* 32 bits */
#define PCI_BASE_ADDRESS_4 0x20 /* 32 bits */
#define PCI_BASE_ADDRESS_5 0x24 /* 32 bits */
#define PCI_BASE_ADDRESS_SPACE 0x01 /* 0 = memory, 1 = I/O */
#define PCI_BASE_ADDRESS_SPACE_IO 0x01
#define PCI_BASE_ADDRESS_SPACE_MEMORY 0x00
#define PCI_BASE_ADDRESS_MEM_TYPE_MASK 0x06
#define PCI_BASE_ADDRESS_MEM_TYPE_32 0x00 /* 32 bit address */
#define PCI_BASE_ADDRESS_MEM_TYPE_1M 0x02 /* Below 1M [obsolete] */
#define PCI_BASE_ADDRESS_MEM_TYPE_64 0x04 /* 64 bit address */
#define PCI_BASE_ADDRESS_MEM_PREFETCH 0x08 /* prefetchable? */
#define PCI_BASE_ADDRESS_MEM_MASK (~0x0fUL)
#define PCI_BASE_ADDRESS_IO_MASK (~0x03UL)
/* bit 1 is reserved if address_space = 1 */

/* Header type 0 (normal devices) */
#define PCI_CARDBUS_CIS 0x28
#define PCI_SUBSYSTEM_VENDOR_ID 0x2c
#define PCI_SUBSYSTEM_ID 0x2e
#define PCI_ROM_ADDRESS 0x30 /* Bits 31..11 are address, 10..1 reserved */
#define PCI_ROM_ADDRESS_ENABLE 0x01
#define PCI_ROM_ADDRESS_MASK (~0x7ffUL)

#define PCI_CAPABILITY_LIST 0x34 /* Offset of first capability list entry */

/* 0x35-0x3b are reserved */
#define PCI_INTERRUPT_LINE 0x3c /* 8 bits */
#define PCI_INTERRUPT_PIN 0x3d /* 8 bits */
#define PCI_MIN_GNT 0x3e /* 8 bits */
#define PCI_MAX_LAT 0x3f /* 8 bits */

/* Header type 1 (PCI-to-PCI bridges) */
#define PCI_PRIMARY_BUS 0x18 /* Primary bus number */
#define PCI_SECONDARY_BUS 0x19 /* Secondary bus number */
#define PCI_SUBORDINATE_BUS 0x1a /* Highest bus number behind the bridge */
#define PCI_SEC_LATENCY_TIMER 0x1b /* Latency timer for secondary interface */
#define PCI_IO_BASE 0x1c /* I/O range behind the bridge */
#define PCI_IO_LIMIT 0x1d
#define PCI_IO_RANGE_TYPE_MASK 0x0f /* I/O bridging type */
#define PCI_IO_RANGE_TYPE_16 0x00
#define PCI_IO_RANGE_TYPE_32 0x01
#define PCI_IO_RANGE_MASK ~0x0f
#define PCI_SEC_STATUS 0x1e /* Secondary status register, only bit 14 used */
#define PCI_MEMORY_BASE 0x20 /* Memory range behind */
#define PCI_MEMORY_LIMIT 0x22
#define PCI_MEMORY_RANGE_TYPE_MASK 0x0f
#define PCI_MEMORY_RANGE_MASK ~0x0f
#define PCI_PREF_MEMORY_BASE 0x24 /* Prefetchable memory range behind */
#define PCI_PREF_MEMORY_LIMIT 0x26
#define PCI_PREF_RANGE_TYPE_MASK 0x0f
#define PCI_PREF_RANGE_TYPE_32 0x00
#define PCI_PREF_RANGE_TYPE_64 0x01
#define PCI_PREF_RANGE_MASK ~0x0f
#define PCI_PREF_BASE_UPPER32 0x28 /* Upper half of prefetchable memory range */
#define PCI_PREF_LIMIT_UPPER32 0x2c
#define PCI_IO_BASE_UPPER16 0x30 /* Upper half of I/O addresses */
#define PCI_IO_LIMIT_UPPER16 0x32
/* 0x34 same as for htype 0 */
/* 0x35-0x3b is reserved */
#define PCI_ROM_ADDRESS1 0x38 /* Same as PCI_ROM_ADDRESS, but for htype 1 */
/* 0x3c-0x3d are same as for htype 0 */
#define PCI_BRIDGE_CONTROL 0x3e
#define PCI_BRIDGE_CTL_PARITY 0x01 /* Enable parity detection on secondary interface */
#define PCI_BRIDGE_CTL_SERR 0x02 /* The same for SERR forwarding */
#define PCI_BRIDGE_CTL_NO_ISA 0x04 /* Disable bridging of ISA ports */
#define PCI_BRIDGE_CTL_VGA 0x08 /* Forward VGA addresses */
#define PCI_BRIDGE_CTL_MASTER_ABORT 0x20 /* Report master aborts */
#define PCI_BRIDGE_CTL_BUS_RESET 0x40 /* Secondary bus reset */
#define PCI_BRIDGE_CTL_FAST_BACK 0x80 /* Fast Back2Back enabled on secondary interface */

/* Header type 2 (CardBus bridges) */
#define PCI_CB_CAPABILITY_LIST 0x14
/* 0x15 reserved */
#define PCI_CB_SEC_STATUS 0x16 /* Secondary status */
#define PCI_CB_PRIMARY_BUS 0x18 /* PCI bus number */
#define PCI_CB_CARD_BUS 0x19 /* CardBus bus number */
#define PCI_CB_SUBORDINATE_BUS 0x1a /* Subordinate bus number */
#define PCI_CB_LATENCY_TIMER 0x1b /* CardBus latency timer */
#define PCI_CB_MEMORY_BASE_0 0x1c
#define PCI_CB_MEMORY_LIMIT_0 0x20
#define PCI_CB_MEMORY_BASE_1 0x24
#define PCI_CB_MEMORY_LIMIT_1 0x28
#define PCI_CB_IO_BASE_0 0x2c
#define PCI_CB_IO_BASE_0_HI 0x2e
#define PCI_CB_IO_LIMIT_0 0x30
#define PCI_CB_IO_LIMIT_0_HI 0x32
#define PCI_CB_IO_BASE_1 0x34
#define PCI_CB_IO_BASE_1_HI 0x36
#define PCI_CB_IO_LIMIT_1 0x38
#define PCI_CB_IO_LIMIT_1_HI 0x3a
#define PCI_CB_IO_RANGE_MASK ~0x03
/* 0x3c-0x3d are same as for htype 0 */
#define PCI_CB_BRIDGE_CONTROL 0x3e
#define PCI_CB_BRIDGE_CTL_PARITY 0x01 /* Similar to standard bridge control register */
#define PCI_CB_BRIDGE_CTL_SERR 0x02
#define PCI_CB_BRIDGE_CTL_ISA 0x04
#define PCI_CB_BRIDGE_CTL_VGA 0x08
#define PCI_CB_BRIDGE_CTL_MASTER_ABORT 0x20
#define PCI_CB_BRIDGE_CTL_CB_RESET 0x40 /* CardBus reset */
#define PCI_CB_BRIDGE_CTL_16BIT_INT 0x80 /* Enable interrupt for 16-bit cards */
#define PCI_CB_BRIDGE_CTL_PREFETCH_MEM0 0x100 /* Prefetch enable for both memory regions */
#define PCI_CB_BRIDGE_CTL_PREFETCH_MEM1 0x200
#define PCI_CB_BRIDGE_CTL_POST_WRITES 0x400
#define PCI_CB_SUBSYSTEM_VENDOR_ID 0x40
#define PCI_CB_SUBSYSTEM_ID 0x42
#define PCI_CB_LEGACY_MODE_BASE 0x44 /* 16-bit PC Card legacy mode base address (ExCa) */
/* 0x48-0x7f reserved */

/* Capability lists */
#define PCI_CAP_LIST_ID 0 /* Capability ID */
#define PCI_CAP_ID_PM 0x01 /* Power Management */
#define PCI_CAP_ID_AGP 0x02 /* Accelerated Graphics Port */
#define PCI_CAP_ID_VPD 0x03 /* Vital Product Data */
#define PCI_CAP_ID_SLOTID 0x04 /* Slot Identification */
#define PCI_CAP_ID_MSI 0x05 /* Message Signalled Interrupts */
#define PCI_CAP_ID_CHSWP 0x06 /* CompactPCI HotSwap */
#define PCI_CAP_LIST_NEXT 1 /* Next capability in the list */
#define PCI_CAP_FLAGS 2 /* Capability defined flags (16 bits) */
#define PCI_CAP_SIZEOF 4

/* Power Management Registers */
#define PCI_PM_CAP_VER_MASK 0x0007 /* Version */
#define PCI_PM_CAP_PME_CLOCK 0x0008 /* PME clock required */
#define PCI_PM_CAP_AUX_POWER 0x0010 /* Auxilliary power support */
#define PCI_PM_CAP_DSI 0x0020 /* Device specific initialization */
#define PCI_PM_CAP_D1 0x0200 /* D1 power state support */
#define PCI_PM_CAP_D2 0x0400 /* D2 power state support */
#define PCI_PM_CAP_PME 0x0800 /* PME pin supported */
#define PCI_PM_CTRL 4 /* PM control and status register */
#define PCI_PM_CTRL_STATE_MASK 0x0003 /* Current power state (D0 to D3) */
#define PCI_PM_CTRL_PME_ENABLE 0x0100 /* PME pin enable */
#define PCI_PM_CTRL_DATA_SEL_MASK 0x1e00 /* Data select (??) */
#define PCI_PM_CTRL_DATA_SCALE_MASK 0x6000 /* Data scale (??) */
#define PCI_PM_CTRL_PME_STATUS 0x8000 /* PME pin status */
#define PCI_PM_PPB_EXTENSIONS 6 /* PPB support extensions (??) */
#define PCI_PM_PPB_B2_B3 0x40 /* Stop clock when in D3hot (??) */
#define PCI_PM_BPCC_ENABLE 0x80 /* Bus power/clock control enable (??) */
#define PCI_PM_DATA_REGISTER 7 /* (??) */
#define PCI_PM_SIZEOF 8

/* AGP registers */
#define PCI_AGP_VERSION 2 /* BCD version number */
#define PCI_AGP_RFU 3 /* Rest of capability flags */
#define PCI_AGP_STATUS 4 /* Status register */
#define PCI_AGP_STATUS_RQ_MASK 0xff000000 /* Maximum number of requests - 1 */
#define PCI_AGP_STATUS_SBA 0x0200 /* Sideband addressing supported */
#define PCI_AGP_STATUS_64BIT 0x0020 /* 64-bit addressing supported */
#define PCI_AGP_STATUS_FW 0x0010 /* FW transfers supported */
#define PCI_AGP_STATUS_RATE4 0x0004 /* 4x transfer rate supported */
#define PCI_AGP_STATUS_RATE2 0x0002 /* 2x transfer rate supported */
#define PCI_AGP_STATUS_RATE1 0x0001 /* 1x transfer rate supported */
#define PCI_AGP_COMMAND 8 /* Control register */
#define PCI_AGP_COMMAND_RQ_MASK 0xff000000 /* Master: Maximum number of requests */
#define PCI_AGP_COMMAND_SBA 0x0200 /* Sideband addressing enabled */
#define PCI_AGP_COMMAND_AGP 0x0100 /* Allow processing of AGP transactions */
#define PCI_AGP_COMMAND_64BIT 0x0020 /* Allow processing of 64-bit addresses */
#define PCI_AGP_COMMAND_FW 0x0010 /* Force FW transfers */
#define PCI_AGP_COMMAND_RATE4 0x0004 /* Use 4x rate */
#define PCI_AGP_COMMAND_RATE2 0x0002 /* Use 4x rate */
#define PCI_AGP_COMMAND_RATE1 0x0001 /* Use 4x rate */
#define PCI_AGP_SIZEOF 12

/* Slot Identification */
#define PCI_SID_ESR 2 /* Expansion Slot Register */
#define PCI_SID_ESR_NSLOTS 0x1f /* Number of expansion slots available */
#define PCI_SID_ESR_FIC 0x20 /* First In Chassis Flag */
#define PCI_SID_CHASSIS_NR 3 /* Chassis Number */

/* Message Signalled Interrupts registers */
#define PCI_MSI_FLAGS 2 /* Various flags */
#define PCI_MSI_FLAGS_64BIT 0x80 /* 64-bit addresses allowed */
#define PCI_MSI_FLAGS_QSIZE 0x70 /* Message queue size configured */
#define PCI_MSI_FLAGS_QMASK 0x0e /* Maximum queue size available */
#define PCI_MSI_FLAGS_ENABLE 0x01 /* MSI feature enabled */
#define PCI_MSI_RFU 3 /* Rest of capability flags */
#define PCI_MSI_ADDRESS_LO 4 /* Lower 32 bits */
#define PCI_MSI_ADDRESS_HI 8 /* Upper 32 bits (if PCI_MSI_FLAGS_64BIT set) */
#define PCI_MSI_DATA_32 8 /* 16 bits of data for 32-bit devices */
#define PCI_MSI_DATA_64 12 /* 16 bits of data for 64-bit devices */

/***************************************************************
 * description : 设备类型配置空间
 ***************************************************************/
struct pci_config_space_device {
    os_u32 base_addr_reg[6]; /* 10h */
    os_u32 reserve1[2]; /* 28h */
    os_u32 extend_rom; /* 30h */
    os_u32 reserve2[2]; /* 34h */
    os_u8 int_line, int_pin, min_gnt, max_lat; /* 3ch */
};

/***************************************************************
 * description : 桥类型配置空间
 ***************************************************************/
struct pci_config_space_bridge {
    os_u32 base_addr_reg[2]; /* 10h */
    os_u8 primary_bus_num, secondary_bus_num, subordinate_bus_num, secondary_latency_timer; /* 18h */
    os_u8 io_base, io_limit; /* 1ch */
    os_u16 secondary_status; /* 1ch */
    os_u16 memory_base, memory_limit; /* 20h */
    os_u16 prefetchable_memory_base, prefetchable_memory_limit; /* 24h */
    os_u32 prefetchable_base_upper_32bits; /* 28h */
    os_u32 prefetchable_limit_upper_32bits; /* 2ch */
    os_u16 io_base_upper_16bits, io_limit_upper_16bits; /* 30h */
    os_u32 reserved_capabilities_pointer; /* 34h */
    os_u32 expansion_rom_base_addr; /* 38h */
    os_u8 int_line, int_pin; /* 3ch */
    os_u16 bridge_control; /* 3ch */
};

/***************************************************************
 * description :
 ***************************************************************/
struct pci_config_space {
    os_u16 vendor_id, device_id; /* 00h */
    os_u16 command, status; /* 04h */
    os_u32 classcode_revisionid; /* 08h */
    os_u8 cacheline_size, latency_timer, head_type, bist; /* 0ch */
    union {
        struct pci_config_space_device device;
        struct pci_config_space_bridge bridge;
    } u;
};

#define PCI_CHECK 0xbcaebcae

/***************************************************************
 * description :
 ***************************************************************/
struct pci_dev {
    struct list_node list;
    os_u32 bus, device, func;
    struct pci_config_space config_info; /* pci设备配置空间信息 */
    os_u32 check;
    os_void *dedicated;
    const struct pci_driver *driver;
};

/***************************************************************
 * description :
 ***************************************************************/
struct pci_bridge {
    struct list_node list;
    struct pci_config_space config_info; /* pci桥设备配置空间 */
};

/***************************************************************
 * description :
 ***************************************************************/
struct pci_bus {
    struct list_node device_head; //struct pci_dev *device;
    struct list_node bridge_head; //struct pci_bridge *bridge;
};

/* 总线索引 */
LOCALD struct pci_bus pci_bus_info[MAX_PCI_BUS_NUM] = { 0 };
/* there is no lock for pci, because we do not support hot-plug */

/***************************************************************
 function declare
 ***************************************************************/
#define PCI_CONF1_ADDRESS(bus, dev, fn, reg) (0x80000000 | ((bus) << 16) | ((dev) << 11) | ((fn) << 8) | ((reg) & ~3))

/***************************************************************
 * description : 使用配制机制1
 * history     :
 ***************************************************************/
LOCALC inline os_u8 pci_bus_in_config_byte(os_u32 bus, os_u32 device, os_u32 func, os_u32 offset)
{
    os_u8 value;

    outl_p(PCI_ADDR_REG, PCI_CONF1_ADDRESS(bus, device, func, offset));
    inb_p(PCI_DATA_REG + (offset & 3), value);

    return value;
}

/***************************************************************
 * description : 使用配制机制1
 * history     :
 ***************************************************************/
LOCALC inline os_u16 pci_bus_in_config_word(os_u32 bus, os_u32 device, os_u32 func, os_u32 offset)
{
    os_u16 value;

    outl_p(PCI_ADDR_REG, PCI_CONF1_ADDRESS(bus, device, func, offset));
    inw_p(PCI_DATA_REG + (offset & 2), value);

    return value;
}

/***************************************************************
 * description : 使用配制机制1
 * history     :
 ***************************************************************/
LOCALC inline os_u32 pci_bus_in_config_dword(os_u32 bus, os_u32 device, os_u32 func, os_u32 offset)
{
    os_u32 value;

    outl_p(PCI_ADDR_REG, PCI_CONF1_ADDRESS(bus, device, func, offset));
    inl_p(PCI_DATA_REG, value);

    return value;
}

/***************************************************************
 * description : 使用配制机制1
 * history     :
 ***************************************************************/
LOCALC inline os_u8 in_pci_config_byte(struct pci_dev *pci, os_u32 offset)
{
    return pci_bus_in_config_dword(pci->bus, pci->device, pci->func, offset);
}

/***************************************************************
 * description : 使用配制机制1
 * history     :
 ***************************************************************/
LOCALC inline os_u16 in_pci_config_word(struct pci_dev *pci, os_u32 offset)
{
    return pci_bus_in_config_dword(pci->bus, pci->device, pci->func, offset);
}

/***************************************************************
 * description : 使用配制机制1
 * history     :
 ***************************************************************/
LOCALC inline os_u32 in_pci_config_dword(struct pci_dev *pci, os_u32 offset)
{
    return pci_bus_in_config_dword(pci->bus, pci->device, pci->func, offset);
}

/***************************************************************
 * description : 使用配制机制1
 * history     :
 ***************************************************************/
LOCALC inline os_u8 pci_bus_out_config_byte(os_u32 bus, os_u32 device, os_u32 func, os_u32 offset, os_u8 value)
{
    outl_p(PCI_ADDR_REG, PCI_CONF1_ADDRESS(bus, device, func, offset));
    outb_p(PCI_DATA_REG, value);

    return value;
}

/***************************************************************
 * description : 使用配制机制1
 * history     :
 ***************************************************************/
LOCALC inline os_u16 pci_bus_out_config_word(os_u32 bus, os_u32 device, os_u32 func, os_u32 offset, os_u16 value)
{
    outl_p(PCI_ADDR_REG, PCI_CONF1_ADDRESS(bus, device, func, offset));
    outw_p(PCI_DATA_REG, value);

    return value;
}

/***************************************************************
 * description : 使用配制机制1
 * history     :
 ***************************************************************/
LOCALC inline os_void pci_bus_out_config_dword(os_u32 bus, os_u32 device, os_u32 func, os_u32 offset, os_u32 value)
{
    outl_p(PCI_ADDR_REG, PCI_CONF1_ADDRESS(bus, device, func, offset));
    outl_p(PCI_DATA_REG, value);
}

/***************************************************************
 * description : 使用配制机制1
 * history     :
 ***************************************************************/
LOCALC inline os_u8 out_pci_config_byte(struct pci_dev *pci, os_u32 offset, os_u8 value)
{
    return pci_bus_out_config_byte(pci->bus, pci->device, pci->func, offset, value);
}

/***************************************************************
 * description : 使用配制机制1
 * history     :
 ***************************************************************/
LOCALC inline os_u16 out_pci_config_word(struct pci_dev *pci, os_u32 offset, os_u16 value)
{
    return pci_bus_out_config_word(pci->bus, pci->device, pci->func, offset, value);
}

/***************************************************************
 * description : 使用配制机制1
 * history     :
 ***************************************************************/
LOCALC inline os_void out_pci_config_dword(struct pci_dev *pci, os_u32 offset, os_u32 value)
{
    pci_bus_out_config_dword(pci->bus, pci->device, pci->func, offset, value);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void read_pci_config_space(os_u32 bus, os_u32 device, os_u32 func, os_u32 *config_space)
{
    os_u32 i;

    for (i = 0; i < sizeof(struct pci_config_space); i += 4) {
        *config_space = pci_bus_in_config_dword(bus, device, func, i);
        config_space++;
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void scan_pci_device(os_u32 bus, os_u32 device, os_u32 func)
{
    os_u32 t;
    struct pci_dev *tmp_dev;
    struct pci_bridge *tmp_bridge;

    /* check vender id and device id */
    t = pci_bus_in_config_dword(bus, device, func, struct_offset(struct pci_config_space, vendor_id));
    if ((0xffffffff != t) && (0x00000000 != t) && (0x0000ffff != t) && (0xffff0000 != t)) {
        switch (0x7f & pci_bus_in_config_byte(bus, device, func, struct_offset(struct pci_config_space, head_type))) {
        case PCI_HEADER_TYPE_NORMAL:
            tmp_dev = kmalloc(sizeof(struct pci_dev));
            if (OS_NULL == tmp_dev) {
                return;
            }
            tmp_dev->check = PCI_CHECK;
            tmp_dev->bus = bus;
            tmp_dev->device = device;
            tmp_dev->func = func;
            /* 读取整个pci空间 */
            read_pci_config_space(bus, device, func, (os_u32 *) &tmp_dev->config_info);
            add_list_head(&pci_bus_info[bus].device_head, &tmp_dev->list);
            break;

        case PCI_HEADER_TYPE_BRIDGE:
            tmp_bridge = kmalloc(sizeof(struct pci_bridge));
            if (OS_NULL == tmp_bridge) {
                return;
            }
            /* 读取整个pci空间 */
            read_pci_config_space(bus, device, func, (os_u32 *) &tmp_bridge->config_info);
            add_list_head(&pci_bus_info[tmp_bridge->config_info.u.bridge.primary_bus_num].bridge_head, &tmp_bridge->list);
            break;

        case PCI_HEADER_TYPE_CARDBUS:
        default:
            break;
        }
        /* interrupt pin 值为1表示其使用INT A,2则为INT B,3则为INT C,4则为INT D */
    }
}

/***************************************************************
 * description : 扫描pci总线、设备、功能
 *               总线号最大255, 每个总线上最多256个设备, 即256个逻辑设备
 *               逻辑设备 = 设备+功能
 *               每个设备上最多8个功能
 * history     :
 ***************************************************************/
LOCALC os_void scan_pci_bus(os_void)
{
    os_u32 bus, device, func;
    os_u8 head_type;

    /* 总线号最大255 */
    for (bus = 0; bus < MAX_PCI_BUS_NUM; bus++) {
        for (device = 0; device < MAX_PCI_DEVICE_NUM; device++) {
            scan_pci_device(bus, device, 0);
            head_type = pci_bus_in_config_byte(bus, device, 0, struct_offset(struct pci_config_space, head_type));
            if (0 == (head_type & 0x80)) {
                /* single function device */
                continue;
            }
            for (func = 1; func < MAX_PCI_FUNC_NUM; func++) {
                scan_pci_device(bus, device, func);
            }
        }
    }
}

/***************************************************************
 * description : 查找设备, 多个时产生链表
 * history     :
 ***************************************************************/
LOCALC struct pci_dev *lookup_pci_device(os_u16 dev_id, os_u16 ven_id, os_u32 cls_id)
{
    os_u32 i;
    struct list_node *node, *_save;
    struct pci_dev *device;

    /* 总线号最大255 */
    for (i = 0; i < MAX_PCI_BUS_NUM; i++) {
        loop_del_list(node, _save, &pci_bus_info[i].device_head) {
            device = list_addr(node, struct pci_dev, list);
            if (((PCI_ANY_ID != dev_id) && (device->config_info.device_id != dev_id))
             || ((PCI_ANY_ID != ven_id) && (device->config_info.vendor_id != ven_id))) {
                continue;
            }

            if ((device->config_info.classcode_revisionid >> 8) == cls_id) {
                del_list(&device->list);
                return device;
            }
        }
    }
    return OS_NULL;
}

/***************************************************************
 * description : 启动pci设备可以竞争总线, dma
 * history     :
 ***************************************************************/
os_void enable_pci_dma(HDEVICE pci)
{
    struct pci_dev *dev;
    os_u16 cmd;

    cassert(OS_NULL != pci);
    dev = pci;
    cassert(PCI_CHECK == dev->check);

    cmd = in_pci_config_byte(dev, struct_offset(struct pci_config_space, command));
    if (!(cmd & PCI_COMMAND_MASTER)) {
        cmd |= PCI_COMMAND_MASTER;
        out_pci_config_word(dev, struct_offset(struct pci_config_space, command), cmd);
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_u32 get_pci_dev_bar(HDEVICE pci, os_uint idx)
{
    struct pci_dev *device;

    cassert(OS_NULL != pci);
    device = pci;
    cassert(PCI_CHECK == device->check);

    /* 从6个资源地址中找到io地址 */
    if (idx < array_size(device->config_info.u.device.base_addr_reg)) {
        return device->config_info.u.device.base_addr_reg[idx];
    }
    return 0;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_u32 get_pci_dev_cmd(HDEVICE pci)
{
    struct pci_dev *dev;

    cassert(OS_NULL != pci);
    dev = pci;
    cassert(PCI_CHECK == dev->check);

    return dev->config_info.command;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_u32 check_pci_config_type(os_void)
{
    os_u32 save, tmp;
    lock_t eflag;

    lock_int(eflag);

    /* Check if configuration type 1 works. */
    outb(0xCFB, 0x01);
    inl(0xCF8, save);
    outl(0xCF8, 0x80000000);
    inl(0xCF8, tmp);
    if (0x80000000 == tmp) {
        outl(0xCF8, save);
        unlock_int(eflag);
        return 1;
    }
    outl(0xCF8, save);

    /* Check if configuration type 2 works. */
    outb(0xCFB, 0x00);
    outb(0xCF8, 0x00);
    outb(0xCFA, 0x00);
    //if (inb(0xCF8) == 0x00 && inb(0xCFA) == 0x00) {
    inl(0xCF8, tmp);
    if (0 == (tmp & 0x00ff00ff)) {
        unlock_int(eflag);
        return 2;
    }

    unlock_int(eflag);
    return 0;
}

/***************************************************************
 * description :
 ***************************************************************/
struct pci_int_link {
    struct pci_dev *device;
    struct pci_int_link *next;
};

/***************************************************************
 * description :
 ***************************************************************/
struct pci_interrupt {
    os_u32 irq; /* 共享中断号 */
    struct pci_int_link *link; /* pci中断链 */

};
LOCALD struct pci_interrupt pci_int[MAX_PCI_INT_NUM] = { 0 };
/* there is no lock for pci, because we do not support hot-plug */

/***************************************************************
 * description : IRQ_FUNC, 中断上半部
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC handle_pci_int_1(os_u32 irq)
{
    os_u32 i;
    struct pci_interrupt *t;
    struct pci_int_link *node;

    for (i = 0, t = pci_int; i < MAX_PCI_INT_NUM; i++, t++) {
        /* polling interrupt */
        if (irq == t->irq) {
            node = t->link;
            /* 设备是否有中断 */
            while (node) {
                if (node->device->driver->interrupt_1) {
                    if (OS_SUCC == node->device->driver->interrupt_1(node->device)) {
                        return;
                    }
                }
                node = node->next;
            }
        }
    }
}

/***************************************************************
 * description : IRQ_FUNC, 中断下半部
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC handle_pci_int_2(os_u32 irq)
{
    os_u32 i;
    struct pci_interrupt *t;
    struct pci_int_link *node;

    for (i = 0, t = pci_int; i < MAX_PCI_INT_NUM; i++, t++) {
        /* polling interrupt */
        if (irq == t->irq) {
            node = t->link;
            /* 设备是否有中断 */
            while (node) {
                if (node->device->driver->interrupt_2) {
                    if (OS_SUCC == node->device->driver->interrupt_2(node->device)) {
                        return;
                    }
                }
                node = node->next;
            }
        }
    }
}

/***************************************************************
 * description : pci共享中断
 * history     :
 ***************************************************************/
os_ret enable_pci_int(HDEVICE pci)
{
    struct pci_dev *device;
    os_u32 record;
    struct pci_int_link *node;
    os_ret result;
    os_u32 i;
    os_u8 int_line;
    lock_t eflag;

    /* 入参检查 */
    cassert(OS_NULL != pci);
    device = pci;
    cassert(PCI_CHECK == device->check);

    int_line = device->config_info.u.device.int_line;
    if (15 < int_line) {
        print("pci device does not support interrupt\n");
        return OS_FAIL;
    }

    lock_int(eflag);

    /* 查找分配pci中断表 */
    record = MAX_PCI_INT_NUM;
    for (i = 0; i < MAX_PCI_INT_NUM; i++) {
        if (int_line == pci_int[i].irq) {
            break;
        } else if (-1 == pci_int[i].irq) {
            record = i;
        } else {
        }
    }

    if (i >= MAX_PCI_INT_NUM) {
        if (record == MAX_PCI_INT_NUM) {
            /* 根据record的初始值, 分配失败 */
            goto fail;
        } else {
            /* 分配 */
            i = record;
        }
    }

    pci_int[i].irq = int_line;
    if (OS_NULL == pci_int[i].link) {
        /* 安装pci设备中断, 不在上半部中完成会引发cpu复位 */
        result = install_int(int_line, handle_pci_int_1, handle_pci_int_2);
        if (OS_SUCC != result) {
            goto fail;
        }
    }

    node = kmalloc(sizeof(struct pci_int_link));
    if (OS_NULL == node) {
        print("alloc pci int link fail\n");
        goto fail;
    }

    node->device = device;
    node->next = pci_int[i].link;
    pci_int[i].link = node;

    unlock_int(eflag);
    return OS_SUCC;
  fail:
    unlock_int(eflag);
    flog("add pci device interrupt fail.\n");
    return OS_FAIL;
}

/***************************************************************
 * description : pci共享中断
 * history     :
 ***************************************************************/
os_ret disable_pci_int(HDEVICE pci)
{
    struct pci_dev *device;
    os_u32 record;
    struct pci_int_link *last;
    struct pci_int_link *curr;
    os_u8 int_line;
    os_u32 i;
    lock_t eflag;

    /* 入参检查 */
    cassert(OS_NULL != pci);
    device = pci;
    cassert(PCI_CHECK == device->check);

    int_line = device->config_info.u.device.int_line;
    if (15 < int_line) {
        print("pci device does not support interrupt\n");
        return OS_FAIL;
    }

    lock_int(eflag);

    /* 查找分配pci中断表 */
    record = MAX_PCI_INT_NUM;
    for (i = 0; i < MAX_PCI_INT_NUM; i++) {
        if (int_line == pci_int[i].irq) {
            break;
        }
    }
    if (MAX_PCI_INT_NUM == i) {
        goto fail;
    }

    /* 在相应的中断号上查找 */
    curr = pci_int[i].link;
    while (curr) {
        if (device == curr->device) {
            /* 记录删除节点 */
            break;
        }
        curr = curr->next;
    }

    last = pci_int[i].link;
    while (last) {
        if (curr == last->next) {
            /* 查找删除节点的上一节点 */
            break;
        }
        last = last->next;
    }

    if (OS_NULL == last) {
        if (OS_NULL == curr) {
            /* curr节点没找到 */
        } else {
            /* curr是第一个节点 */
            pci_int[i].link = curr->next;
            kfree(curr);
        }
    } else {
        /* 两个节点不重合, 删除 */
        last->next = curr->next;
        kfree(curr);
    }

    unlock_int(eflag);
    return OS_SUCC;
  fail:
    /* 中断未注册 */
    unlock_int(eflag);
    flog("del pci int fail.\n");
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret register_pci_driver(const struct pci_driver *driver)
{
    struct list_node head;
    struct list_node *node;
    struct pci_dev *device;

    if ((OS_NULL == driver) || (OS_NULL == driver->id_table)) {
        return OS_FAIL;
    }

    init_list_head(&head);

    /* 设备匹配 */
    while (OS_NULL != (device = lookup_pci_device(driver->id_table->device, driver->id_table->vendor, driver->id_table->class & driver->id_table->class_mask))) {
        add_list_head(&head, &device->list);
    }

    loop_list(node, &head) {
        device = list_addr(node, struct pci_dev, list);

        /* 注册中断等信息 */
        device->driver = driver;

        /* 探测 */
        if (driver->probe) {
            driver->probe(device);
        }
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 ***************************************************************/
struct irq_info {
    os_u8 bus, devfn; /* Bus, device and function */
    struct {
        os_u8 link; /* IRQ line ID, chipset dependent, 0=not routed */
        os_u16 bitmap; /* Available IRQs */
    } irq[4];
    os_u8 slot; /* Slot number, 0=onboard */
    os_u8 rfu;
};

/***************************************************************
 * description :
 ***************************************************************/
struct pci_irq_route_table {
    os_u32 signature;                  /* PIRQ_SIGNATURE should be here */
    os_u16 version;                    /* PIRQ_VERSION */
    os_u16 size;                       /* Table size in bytes */
    os_u8 rtr_bus, rtr_devfn;          /* Where the interrupt router lies */
    os_u16 exclusive_irqs;             /* IRQs devoted exclusively to PCI usage */
    os_u16 rtr_vendor, rtr_device;     /* Vendor and device ID of interrupt router */
    os_u32 miniport_data;              /* Crap */
    os_u8 rfu[11];
    os_u8 checksum;                    /* Modulo 256 checksum must give zero */
    struct irq_info slots[0];
};
LOCALD struct pci_irq_route_table *pci_irq = OS_NULL;

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC struct pci_irq_route_table *pci_lookup_irq_route_table(os_void)
{
    struct pci_irq_route_table *route_table;
    os_u8 *addr;
    os_u16 size;
    os_u16 i;
    os_u8 sum;

#define PCI_IRQ_SIGNATURE (('$' << 0) + ('P' << 8) + ('I' << 16) + ('R' << 24))
#define PCI_BIOS_ADDR_START 0xf0000
#define PCI_BIOS_ADDR_END 0x100000

    for (addr = (os_u8 *) PCI_BIOS_ADDR_START; addr < (os_u8 *) PCI_BIOS_ADDR_END; addr += sizeof(os_u32)) {
        if (PCI_IRQ_SIGNATURE != *(os_u32 *) addr) {
            continue;
        }

        /* 签名找到, 校验 */
        route_table = (struct pci_irq_route_table *) addr;
        sum = 0;
        size = route_table->size;
        for (i = 0; i < size; i++) {
            sum += addr[i];
        }

        if (0 != sum) {
            flog("pci bios check sum fail.\n");
            return OS_NULL;
        }
        return route_table;
    }
    flog("pci bios irq route table not found.\n");
    return OS_NULL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void dump_pci(os_void)
{
    os_u32 i;
    struct list_node *node;
    struct pci_dev *device;
    struct pci_bridge *bridge;

    for (i = 0; i < MAX_PCI_BUS_NUM; i++) {
        loop_list(node, &pci_bus_info[i].device_head) {
            device = list_addr(node, struct pci_dev, list);
            print("d:%d %d %d %x %x %d %d\n", i, device->device, device->func, *(os_u32 *) &device->config_info, device->config_info.classcode_revisionid, device->config_info.head_type, device->config_info.u.device.int_line);
        }

        loop_list(node, &pci_bus_info[i].bridge_head) {
            bridge = list_addr(node, struct pci_bridge, list);
            print("b:%d %x %x %d %d\n", i, *(os_u32 *) &bridge->config_info, bridge->config_info.classcode_revisionid, bridge->config_info.head_type, bridge->config_info.u.device.int_line);
        }
    }
}

LOCALD os_u8 pci_debug_name[] = { "pci" };
LOCALD struct dump_info pci_debug = {
    pci_debug_name,
    dump_pci
};

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void init_pci(os_void)
{
    os_u32 i;

    if (1 != check_pci_config_type()) {
        /* 只支持1型操作, 2型操作需要占用16M空间 */
        return;
    }

    /* init pci int */
    for (i = 0; i < MAX_PCI_INT_NUM; i++) {
        pci_int[i].irq = -1;
        pci_int[i].link = OS_NULL;
    }

    /* 初始化总线信息 */
    for (i = 0; i < MAX_PCI_BUS_NUM; i++) {
        init_list_head(&pci_bus_info[i].device_head);
        init_list_head(&pci_bus_info[i].bridge_head);
    }

    /* 扫描总线 */
    scan_pci_bus();

    pci_irq = pci_lookup_irq_route_table();
    if (OS_NULL == pci_irq) {
        return;
    }

    if (OS_SUCC != register_dump(&pci_debug)) {
        flog("pci register dump fail\n");
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void set_pci_dedicated(HDEVICE pci, os_void *dedicated)
{
    struct pci_dev *dev;

    cassert(OS_NULL != pci);
    dev = pci;
    cassert(PCI_CHECK == dev->check);
    dev->dedicated = dedicated;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void *get_pci_dedicated(HDEVICE pci)
{
    struct pci_dev *dev;

    cassert(OS_NULL != pci);
    dev = pci;
    cassert(PCI_CHECK == dev->check);
    return dev->dedicated;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_u32 in_pci_reg(HDEVICE pci, os_u32 offset)
{
    struct pci_dev *dev;

    cassert(OS_NULL != pci);
    dev = pci;
    cassert(PCI_CHECK == dev->check);
    return pci_bus_in_config_dword(dev->bus, dev->device, dev->func, offset);
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void out_pci_reg(HDEVICE pci, os_u32 offset, os_u32 value)
{
    struct pci_dev *dev;

    cassert(OS_NULL != pci);
    dev = pci;
    cassert(PCI_CHECK == dev->check);
    pci_bus_out_config_dword(dev->bus, dev->device, dev->func, offset, value);
}

