/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : usb_hub.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2011-04-22
 ***************************************************************/

#ifndef __USB_HUB_H__
#define __USB_HUB_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/
/* refer to table 11-21. port status field, wPortStatus */
#define PSB_CCS (UINT32_C(0x1) << 0)
#define PSB_PED (UINT32_C(0x1) << 1)
#define PSB_SUSPEND (UINT32_C(0x1) << 2)
#define PSB_OVERCURRENT (UINT32_C(0x1) << 3)
#define PSB_RESET (UINT32_C(0x1) << 4)
/* Port Power: (PORT_POWER) This field reflects a port¡¯s logical, power control state. Because hubs can
   implement different methods of port power switching, this field may or may not represent whether power is
   applied to the port. The device descriptor reports the type of power switching implemented by the hub. */
#define PSB_PP (UINT32_C(0x1) << 8)
#define PSB_LSDA (UINT32_C(0x1) << 9)
#define PSB_HSDA (UINT32_C(0x1) << 10)
#define PSB_PTM (UINT32_C(0x1) << 11)
#define PSB_PIC (UINT32_C(0x1) << 12)

/***************************************************************
 * description : 11.24.2.6 Get Hub Status
 ***************************************************************/
struct Hub_Status_and_Change_Status {
    os_u16 wHubStatus;
    os_u16 wHubChange;
};

/***************************************************************
 * description : 11.24.2.7 Get Port Status
 *               The first word of data contains wPortStatus (refer to Table 11-21). The second word of data contains
 *               wPortChange (refer to Table 11-22 (i think it is a mistake in protocol)).
 ***************************************************************/
struct Port_Status_and_Change_Status {
    os_u16 wPortStatus;
    os_u16 wPortChange;
};

/* refer to 11.24.1 standard requests, 150ms */
#define EHCI_HUB_TIMEOUT 150

/***************************************************************
 enum define
 ***************************************************************/
/***************************************************************
 * description : refer to table 11-17 hub class feature selectors
 ***************************************************************/
enum usb_hub_class_feature_selectors_type {
    USB_C_HUB_LOCAL_POWER = 0,
    USB_C_HUB_OVER_CURRENT = 1,

    USB_PORT_CONNECTION = 0,
    USB_PORT_ENABLE = 1,
    USB_PORT_SUSPEND = 2,
    USB_PORT_OVER_CURRENT = 3,
    USB_PORT_RESET = 4,
    USB_PORT_POWER = 8,
    USB_PORT_LOW_SPEED = 9,
    USB_C_PORT_CONNECTION = 16,
    USB_C_PORT_ENABLE = 17,
    USB_C_PORT_SUSPEND = 18,
    USB_C_PORT_OVER_CURRENT = 19,
    USB_C_PORT_RESET = 20,
    USB_PORT_TEST = 21,
    USB_INDICATOR = 22
};

/***************************************************************
 struct define
 ***************************************************************/
/***************************************************************
 * description :
 ***************************************************************/
struct usb_hub_private {
    struct usb_hub_descriptor hub_descriptor;
};

/***************************************************************
 extern function
 ***************************************************************/

#pragma pack()

#endif /* end of header */

