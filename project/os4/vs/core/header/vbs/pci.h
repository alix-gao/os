/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : pci.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __VBS_PCI_H__
#define __VBS_PCI_H__

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
 * description :
 ***************************************************************/
struct pci_device_id {
    os_u16 vendor, device; /* Vendor and device ID or PCI_ANY_ID */
    os_u32 class, class_mask; /* (class,subclass,prog-if) triplet */
};

/***************************************************************
 * description :
 ***************************************************************/
struct pci_driver {
    os_u8 *name;
    const struct pci_device_id *id_table; /* NULL if wants all devices */
    os_ret (*probe)(HDEVICE pci); /* new device inserted */
    os_void (*remove)(HDEVICE pci); /* device removed (NULL if not a hot-plug capable driver) */
    os_ret (*interrupt_1)(HDEVICE pci); /* first interrupt */
    os_ret (*interrupt_2)(HDEVICE pci); /* second interrupt */
};

/***************************************************************
 extern function
 ***************************************************************/
os_ret register_pci_driver(const struct pci_driver *driver);
os_ret enable_pci_int(HDEVICE pci);
os_ret disable_pci_int(HDEVICE pci);
os_u32 in_pci_reg(HDEVICE pci, os_u32 offset);
os_void out_pci_reg(HDEVICE pci, os_u32 offset, os_u32 value);

os_void enable_pci_dma(HDEVICE pci);
os_u32 get_pci_dev_bar(HDEVICE pci, os_uint idx);
os_u32 get_pci_dev_cmd(HDEVICE pci);

os_void set_pci_dedicated(HDEVICE pci, os_void *dedicated);
os_void *get_pci_dedicated(HDEVICE pci);

#pragma pack()

#endif /* end of header */

