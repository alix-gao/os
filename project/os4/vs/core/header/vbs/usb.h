/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : usb.h
 * version     : 1.0
 * description : usb interface
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __USB_INTERFACE_H__
#define __USB_INTERFACE_H__

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
/* USB types */
#define USB_TYPE_STANDARD (UINT32_C(0x00) << 5)
#define USB_TYPE_CLASS    (UINT32_C(0x01) << 5)
#define USB_TYPE_VENDOR   (UINT32_C(0x02) << 5)
#define USB_TYPE_RESERVED (UINT32_C(0x03) << 5)

/* USB recipients
   usb_20.pdf table 9-4 */
#define USB_RECIP_MASK      0x1f
#define USB_RECIP_DEVICE    0x00
#define USB_RECIP_INTERFACE 0x01
#define USB_RECIP_ENDPOINT  0x02
#define USB_RECIP_OTHER     0x03

/* USB directions
   usb_20.pdf table 9-4 */
#define USB_DIR_OUT 0x0
#define USB_DIR_IN 0x80

/* Standard requests
   usb_20.pdf table 9-4 */
#define USB_REQ_GET_STATUS        0x00
#define USB_REQ_CLEAR_FEATURE     0x01
#define USB_REQ_SET_FEATURE       0x03
#define USB_REQ_SET_ADDRESS       0x05
#define USB_REQ_GET_DESCRIPTOR    0x06
#define USB_REQ_SET_DESCRIPTOR    0x07
#define USB_REQ_GET_CONFIGURATION 0x08
#define USB_REQ_SET_CONFIGURATION 0x09
#define USB_REQ_GET_INTERFACE     0x0A
#define USB_REQ_SET_INTERFACE     0x0B
#define USB_REQ_SYNCH_FRAME       0x0C

/* Table 9-6. Standard Feature Selectors */
#define DEVICE_REMOTE_WAKEUP 1 /* recipient: device */
#define ENDPOINT_HALT 0 /* recipient: endpoint */
#define TEST_MODE 2 /* recipient: device */

/* hub class request codes, table 11-16 */
#define USB_HUB_GET_STATUS 0
#define USB_HUB_CLEAR_FEATURE 1
#define USB_HUB_RESERVED 2
#define USB_HUB_SET_FEATURE 3
#define USB_HUB_GET_DESCRIPTOR 6
#define USB_HUB_SET_DESCRIPTOR 7
#define USB_HUB_CLEAR_TT_BUFFER 8
#define USB_HUB_RESET_TT 9
#define USB_HUB_GET_TT_STATUS 10
#define USB_HUB_STOP_TT 11

/* refer to HID1-11.pdf, 7.1 Standard Requests */
#define HID_DESCRIPTOR_TYPE_HID 0x21
#define HID_DESCRIPTOR_TYPE_REPORT 0x22
#define HID_DESCRIPTOR_TYPE_PHYSICAL 0x23

/***************************************************************
 * description : usb_20.pdf table 9-2
 ***************************************************************/
struct usb_setup_data {
    os_u8 bmRequestType;
    os_u8 bRequest;
    os_u16 wValue;
    os_u16 wIndex;
    os_u16 wLength;
} __attribute__((packed));

/***************************************************************
 * description : Device descriptor
 ***************************************************************/
struct usb_device_descriptor {
    os_u8 bLength;
    os_u8 bDescriptorType;
    os_u16 bcdUSB;
    os_u8 bDeviceClass;
    os_u8 bDeviceSubClass;
    os_u8 bDeviceProtocol;
    os_u8 bMaxPacketSize0;
    os_u16 idVendor;
    os_u16 idProduct;
    os_u16 bcdDevice;
    os_u8 iManufacturer;
    os_u8 iProduct;
    os_u8 iSerialNumber;
    os_u8 bNumConfigurations;
} __attribute__((packed));

/***************************************************************
 * description : configuration descriptor information
 *               sizeof(struct usb_config_descriptor) = 9
 ***************************************************************/
struct usb_config_descriptor {
    os_u8  bLength;
    os_u8  bDescriptorType;
    os_u16 wTotalLength;
    os_u8  bNumInterfaces;
    os_u8  bConfigurationValue;
    os_u8  iConfiguration;
    os_u8  bmAttributes;
    os_u8  MaxPower;
} __attribute__((packed));

/***************************************************************
 * description : string descriptor
 *               sizeof(struct usb_interface_descriptor) = 9
 ***************************************************************/
struct usb_interface_descriptor {
    os_u8 bLength;
    os_u8 bDescriptorType;
    os_u8 bInterfaceNumber;
    os_u8 bAlternateSetting;
    os_u8 bNumEndpoints;
    os_u8 bInterfaceClass;
    os_u8 bInterfaceSubClass;
    os_u8 bInterfaceProtocol;
    os_u8 iInterface;
} __attribute__((packed));

/***************************************************************
 * description :
 ***************************************************************/
struct usb_device_qualifier {
    os_u8 bLength __attribute__((packed));
    os_u8 bDescriptorType __attribute__((packed));
    os_u16 bcdUSB __attribute__((packed));
    os_u8 bDeviceClass __attribute__((packed));
    os_u8 bDeviceSubClass __attribute__((packed));
    os_u8 bDeviceProtocol __attribute__((packed));
    os_u8 bMaxPacketSize0 __attribute__((packed));
    os_u8 bNumConfigurations __attribute__((packed));
    os_u8 bResvered __attribute__((packed));
} __attribute__((packed));

#define endpoint_num(bEndpointAddress) ((bEndpointAddress) & 0xf)
#define endpoint_is_in(bEndpointAddress) (!!((bEndpointAddress) & 0x80))
#define endpoint_is_out(bEndpointAddress) (!((bEndpointAddress) & 0x80))
#define endpoint_mps(wMaxPacketSize) ((wMaxPacketSize) & 0x7ff)

/***************************************************************
 * description : usb_20.pdf table 9-13 standard endpoint descriptor
 ***************************************************************/
enum usb_transfer_type {
    USB_CTRL_TRANSFER = 0,
    USB_ISO_TRANSFER = 1,
    USB_BULK_TRANSFER = 2,
    USB_INTERRUPT_TRANSFER = 3,
    USB_INVALID_TRANSFER
};
#define get_usb_transfer_type(bmAttributes) ((bmAttributes) & 0x3)

/***************************************************************
 * description : endpoint descriptor
 *               sizeof(struct usb_endpoint_descriptor) = 7
 ***************************************************************/
struct usb_endpoint_descriptor {
    os_u8 bLength;
    os_u8 bDescriptorType;
    os_u8 bEndpointAddress;
    os_u8 bmAttributes;
    os_u16 wMaxPacketSize;
    os_u8 bInterval;
} __attribute__((packed));

/***************************************************************
 * description : string descriptor
 ***************************************************************/
struct usb_string_descriptor {
    os_u8  bLength;
    os_u8  bDescriptorType;
    os_u16 wData[1];
} __attribute__((packed));

/***************************************************************
 * description :
 ***************************************************************/
struct hid_class_descriptor {
    os_u8 bDescriptorType;
    os_u16 wDescriptorLength;
} __attribute__((packed));

/***************************************************************
 * description : hid descriptor
 *               sizeof(struct usb_endpoint_descriptor) = 9, 12...
 ***************************************************************/
struct hid_descriptor {
    os_u8 bLength;
    os_u8 bDescriptorType;
    os_u16 bcdHID;
    os_u8 bCountryCode;
    os_u8 bNumDescriptors;
    struct hid_class_descriptor desc[1];
} __attribute__((packed));

/* Device and/or Interface Class codes */
#define USB_CLASS_PER_INTERFACE 0 /* for DeviceClass */
#define USB_CLASS_AUDIO         1
#define USB_CLASS_COMM          2
#define USB_CLASS_HID           3
#define USB_CLASS_PHYSICAL      5
#define USB_CLASS_PRINTER       7
#define USB_CLASS_MASS_STORAGE  8
#define USB_CLASS_HUB           9
#define USB_CLASS_DATA          10
#define USB_CLASS_APP_SPEC      0xfe
#define USB_CLASS_VENDOR_SPEC   0xff

#define USB_ANY_ID 0xff

/***************************************************************
 * description :
 ***************************************************************/
struct usb_device_id {
    os_u16 idVendor;
    os_u16 idProduct;

    os_u8 bInterfaceClass;
    os_u8 bInterfaceSubClass;
    os_u8 bInterfaceProtocol;
};

/***************************************************************
 * description :
 ***************************************************************/
struct usb_driver {
    os_u8 *name;
    const struct usb_device_id *id;
    /* usb event handler */
    os_ret (*add_device)(HDEVICE usb); /* new device inserted */
    os_void (*del_device)(HDEVICE usb); /* device removed (NULL if not a hot-plug capable driver) */
};

/***************************************************************
 extern function
 ***************************************************************/
os_ret recv_usb_control_data(HDEVICE usb, struct usb_setup_data *cmd, os_void *data, os_uint len);
os_ret send_usb_control_data(HDEVICE usb, struct usb_setup_data *cmd, os_void *data, os_uint len);
os_ret send_usb_bulk_data(HDEVICE usb, os_u8 endpoint, os_void *data, os_uint len);
os_ret recv_usb_bulk_data(HDEVICE usb, os_u8 endpoint, os_void *data, os_uint len);
os_ret add_usb_int_pipe(HDEVICE usb, os_u8 endpoint, os_u8 *data, os_uint len, os_u8 bInterval, os_void (*recv)(os_u8 *data));
os_ret del_usb_int_pipe(HDEVICE usb, os_u8 endpoint);

os_void set_usb_dedicated(HDEVICE usb, os_void *dedicated);
os_void *get_usb_dedicated(HDEVICE usb);

/***************************************************************
 * description :
 ***************************************************************/
enum usb_speed_type {
    USB_LOW_SPEED,
    USB_FULL_SPEED,
    USB_HIGH_SPEED,
    USB_SUPER_SPEED,
    USB_INVALID_SPEED
};

enum usb_speed_type usb_device_speed(HDEVICE usb);

struct usb_device_descriptor *usb_device_desc(HDEVICE usb);
struct usb_config_descriptor *usb_config_desc(HDEVICE usb);

/***************************************************************
 * description :
 ***************************************************************/
struct usb_itf_info {
    struct usb_interface_descriptor *interface;
    struct usb_endpoint_descriptor **endpoint; /* point to array */
    struct hid_descriptor *hid;
};
struct usb_itf_info *usb_interface_info(HDEVICE usb);

#pragma pack()

#endif /* end of header */

