/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : uhci.c
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
#include "uhci.h"

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
LOCALC os_ret uhci_interrupt_1(HDEVICE pci)
{
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret uhci_interrupt_2(HDEVICE pci)
{
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret uhci_probe(HDEVICE pci)
{
    flog("uhci probe\n");
    return OS_FAIL;
}

/* 通用uhci设备信息 */
LOCALD const struct pci_device_id pci_uhci_ids = {
    /* no matter who makes it */
    PCI_ANY_ID, PCI_ANY_ID,
    /* handle any USB OHCI controller */
    PCI_CLASS_SERIAL_USB_UHCI, ~0
};

/* 通用uhci设备驱动表 */
LOCALD const struct pci_driver pci_uhci_driver = {
    "uhci-usb",
    &pci_uhci_ids,
    uhci_probe,
    OS_NULL,
    uhci_interrupt_1,
    uhci_interrupt_2
};

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void init_uhci_driver(os_void)
{
    os_ret result;

    result = register_pci_driver(&pci_uhci_driver);
    cassert(OS_SUCC == result);
}
bus_init_func(BUS_P1, init_uhci_driver);

