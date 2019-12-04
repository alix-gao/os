/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : usb.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __USB_H__
#define __USB_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/

/***************************************************************
 * enum name   : usb_direction
 * description :
 ***************************************************************/
enum usb_log_level {
    USB_INFO,
    USB_WARNING,
    USB_ERROR
};
#define usb_dbg(level, fmt, arg...) do { if (USB_INFO <= (level)) { flog(fmt"\n", ##arg); } } while (0)
//#define usb_dbg(level, fmt, arg...) do { if (USB_WARNING <= (level)) { print(fmt"\n", ##arg); } } while (0)

/* 默认端点描述符长度 */
#define DEFAULT_MAX_PACKET_SIZE 8

/* 单个主控制器下的usb设备数量, 包含了默认地址0 */
#define USB_DEVICE_COUNT 128

/* 一个usb设备的端点数量, refer to usb_20.pdf 9.6.6 endpoint */
#define EP_NUM 16

#define DEFAULT_INTERFACE_SETTING 0

/* HID requests */
#define USB_REQ_GET_REPORT      0x01
#define USB_REQ_GET_IDLE        0x02
#define USB_REQ_GET_PROTOCOL    0x03
#define USB_REQ_SET_REPORT      0x09
#define USB_REQ_SET_IDLE        0x0A
#define USB_REQ_SET_PROTOCOL    0x0B

/* Descriptor types usb_20.pdf table 9-5 */
#define USB_DT_DEVICE 0x01
#define USB_DT_CONFIG 0x02
#define USB_DT_STRING 0x03
#define USB_DT_INTERFACE 0x04
#define USB_DT_ENDPOINT 0x05
#define USB_DT_QUALIFIER 0x06
#define USB_DT_OTHERSPEEDCONFIGURATION 0x07
#define USB_DT_INTERFACEPOWER 0x08

#define USB_DT_HID          (USB_TYPE_CLASS | 0x01)
#define USB_DT_REPORT       (USB_TYPE_CLASS | 0x02)
#define USB_DT_PHYSICAL     (USB_TYPE_CLASS | 0x03)
#define USB_DT_HUB          (USB_TYPE_CLASS | 0x09)

/***************************************************************
 enum define
 ***************************************************************/

/***************************************************************
 struct define
 ***************************************************************/

/***************************************************************
 * description : hub descriptor, 11.23.2.1 Hub Descriptor
 ***************************************************************/
struct usb_hub_descriptor {
    os_u8 bDescLength;
    os_u8 bDescriptorType;
    os_u8 bNbrPorts;
    os_u16 wHubCharacteristics;
    os_u8 bPwrOn2PwrGood;
    os_u8 DeviceRemovable[256/8];
    os_u8 PortPwrCtrlMask[256/8];
} __attribute__((packed));

/***************************************************************
 * description :
 ***************************************************************/
struct usb_config_info {
    struct usb_config_descriptor config;
    os_u8 total_intf_count; /* include different alternate setting */
    struct usb_itf_info *intf_info; /* point to array */
};

/***************************************************************
 * description : usb HDEVICE
 ***************************************************************/
struct usb_hdevice {
#define HUSB_CHECK 0x5b05ba5b
    os_u32 check;
    struct usb_device *dev;
    struct usb_itf_info *itf; /* current device interface */
    os_bool load_driver;
    /* for special */
    os_u32 timeout_proportion;
    os_void *dedicated;
};

/***************************************************************
 * description :
 ***************************************************************/
struct usb_device {
    /* uhci, ohci, ehci */
    os_void *host_controller;
    const struct usb_host_controller_operations {
        os_ret (*usb_receive_control_transfer)(struct usb_device *usb, os_u32 addr, os_u8 ed_num, struct usb_setup_data *cmd, os_void *buffer, os_uint len);
        os_ret (*usb_send_control_transfer)(struct usb_device *usb, os_u32 addr, os_u8 ed_num, struct usb_setup_data *cmd, os_void *buffer, os_uint len);
        os_ret (*usb_receive_bulk_transfer)(struct usb_device *usb, os_u8 pipe, os_u8 *buffer, os_uint len);
        os_ret (*usb_send_bulk_transfer)(struct usb_device *usb, os_u8 pipe, os_u8 *buffer, os_uint len);
        os_ret (*usb_receive_interrupt_transfer)(struct usb_device *usb, os_u8 pipe, os_u8 *buffer, os_uint len, os_u8 bInterval, os_void (*recv)(os_u8 *data));
        os_ret (*usb_cancel_interrupt_transer)(struct usb_device *usb, os_u8 pipe);

        /* usb address */
        os_u32 (*alloc_usb_device_addr)(struct usb_device *usb);
        os_ret (*free_usb_device_addr)(struct usb_device *usb);
        /* usb endpoints */
        os_ret (*create_usb_endpoint)(struct usb_device *usb);
        os_ret (*destroy_usb_endpoint)(struct usb_device *usb);
        /* control endpoint */
        os_ret (*reset_usb_default_endpoint)(struct usb_device *usb);
    } *hc_operation;

    os_u8 usb_addr; /* device address */
    enum usb_speed_type speed; /* speed type */

    os_u32 hub_addr;
    os_u32 hub_port_num;

    struct usb_device_descriptor descriptor; /* 设备描述符唯一 */

    os_u8 **config_buffer; /* 每一个配置描述符下的配置信息, 包括接口描述符、端点描述符等等 */
    struct usb_config_info *config_info; /* point to array */
    os_u8 config_index; /* current config array index */

    /* HDEVICE */
    struct usb_hdevice *multiple_function;
};

/***************************************************************
 extern function
 ***************************************************************/
struct usb_device *alloc_usb_device(const os_void *controller, const enum usb_speed_type speed, const struct usb_host_controller_operations *operation);
os_void *free_usb_device(struct usb_device *device);

os_ret register_usb_driver(const struct usb_driver *driver);
os_ret enum_usb_device(struct usb_device *usb);
os_ret unenum_usb_device(struct usb_device *usb);

enum usb_direction {
    USB_IN = 0,
    USB_OUT = 1,
    USB_DIR_CNT
};

os_u16 usb_endpoint_mps(struct usb_device *usb, os_u8 ep_num, enum usb_direction dir);
struct usb_device *find_usb_device(os_void *hc, os_u32 hub_addr, os_u32 port);

os_u8 alloc_usb_addr(os_u8 addr_bitmap[USB_DEVICE_COUNT/8]);
os_void free_usb_addr(os_u8 addr_bitmap[USB_DEVICE_COUNT/8], os_u8 addr);

#define delay_frame() delay_ms(2)

#pragma pack()

#endif /* end of header */

