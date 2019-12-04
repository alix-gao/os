/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : i82579.c
 * version     : 1.0
 * description : Intel 6 Series Express Chipset MAC, Intel 82579LM Gigabit Network Connection
 * author      : sicui
 * date        : 2013-08-02
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <lib.h>
#include <os.h>
#include <core.h>
#include <vbs.h>

/***************************************************************
 global variable declare
 ***************************************************************/
LOCALD os_u32 enter = 0;
LOCALD os_u32 mbara = 0;

/***************************************************************
 function declare
 ***************************************************************/

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret ethernet_probe(HDEVICE pci)
{
    flog("intel 82579lm!!!\n");
    enter = 1;
    mbara = get_pci_dev_bar(pci, 0);
    return OS_SUCC;
}

/***************************************************************
 * description : pci共享中断的上半部
 * history     :
 ***************************************************************/
LOCALC os_ret ethernet_interrupt_1(HDEVICE pci)
{
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
 ***************************************************************/
LOCALD const struct pci_device_id ethernet_ids = {
    /* vendor: intel */
    PCI_VENDOR_ID_INTEL, 0x1502,
    /* class: ethernet 0x20000 */
    PCI_CLASS_NETWORK_ETHERNET << 8, ~0
};

LOCALD const struct pci_driver ethernet_driver = {
    "Intel 82579LM Gigabit Network Connection",
    &ethernet_ids,
    ethernet_probe,
    OS_NULL,
    ethernet_interrupt_1,
    ethernet_interrupt_2
};

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void dump_ethernet_info(os_void)
{
    print("%d %x\n", enter, mbara);
}

LOCALD os_u8 ethernet_debug_name[] = { "i82579" };
LOCALD struct dump_info ethernet_debug = {
    ethernet_debug_name,
    dump_ethernet_info
};

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void init_ethernet(os_void)
{
    os_ret result;

    result = register_pci_driver(&ethernet_driver);
    cassert(OS_SUCC == result);

    result = register_dump(&ethernet_debug);
    cassert(OS_SUCC == result);
}
init_driver_call(init_ethernet);

