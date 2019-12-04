/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : mass_storage.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __MASS_STORAGE_H__
#define __MASS_STORAGE_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/
#define ms_dbg(fmt, arg...) do { if (0) { print(fmt, ##arg); } else { flog(fmt, ##arg); } } while (0)

/***************************************************************
 enum define
 ***************************************************************/

/***************************************************************
 * description : mass storage direction
 ***************************************************************/
enum ms_data_dir {
    MS_DATA_DIR_IN = 0x80,
    MS_DATA_DIR_OUT = 0x00,
    MS_DATA_DIR_NONE = 0x01
};

/***************************************************************
 struct define
 ***************************************************************/

#define CBW_SIGNATURE 0x43425355

/***************************************************************
 * description : mass storage command block wrapper
 ***************************************************************/
struct ms_cbw_struct {
    os_u32 dCBWSignature __attribute__((packed));
    os_u32 dCBWTag __attribute__((packed));
    os_u32 dCBWDataTransferLength __attribute__((packed));
    os_u8  bmCBWFlags __attribute__((packed));
    os_u8  bCBWLUN __attribute__((packed));
    os_u8  bCBWLength __attribute__((packed));
    os_u8  CBWCB[16] __attribute__((packed));
} _aligned_(1);

#define CSW_SIGNATURE 0x53425355

/***************************************************************
 * description : mass storage command status wrapper
 ***************************************************************/
struct ms_csw_struct {
    os_u32 dCSWSignature __attribute__((packed));
    os_u32 dCSWTag __attribute__((packed));
    os_u32 dCSWDataResidue __attribute__((packed));
    os_u8  bCSWStatus __attribute__((packed));
} _aligned_(1);

/***************************************************************
 * description :
 ***************************************************************/
struct ms_logic_disk {
    struct list_node logic_unit;
    HDEVICE usb;
    struct disk_device *vdisk;
    os_u8 lun;
    HEVENT ms_op;
};

#define MAX_LUN 7

/***************************************************************
 * description : mass storage info (ms设备common级信息)
 ***************************************************************/
struct mass_storage_struct {
    os_u8 pipe_in;
    os_u8 pipe_out;
    struct ms_lun_struct {
        os_u8 device_type; /* 设备类型, 例如usb-cdrom、usb-disk */
        os_u32 last_lba;
        os_u32 block_size;
    } ms_lun[MAX_LUN];

    struct list_node lun_head; /* link to struct ms_logic_disk */
};

/***************************************************************
 * description : ufi command block description
 *               usbmass-ufi10.pdf table 2 - typical command block for most commands
 ***************************************************************/
struct ufi_cbd_struct {
    os_u8 operation_code __attribute__((packed));
    os_u8 lun __attribute__((packed)); /* 逻辑单元号 */
    os_u32 logical_block_address __attribute__((packed));
    os_u8 reserved1 __attribute__((packed));
    os_u16 length __attribute__((packed));
    os_u8 reserved2 __attribute__((packed));
    os_u8 reserved3 __attribute__((packed));
    os_u8 reserved4 __attribute__((packed));
} __attribute__((packed));

/***************************************************************
 * description : ufi command block description
 *               usbmass-ufi10.pdf Table 3 - Typical Command Block for Some Extended Commands
 ***************************************************************/
struct ufi_cbd_ex_struct {
    os_u8 operation_code __attribute__((packed));
    os_u8 lun __attribute__((packed)); /* 逻辑单元号 */
    os_u32 logical_block_address __attribute__((packed));
    os_u32 length __attribute__((packed));
    os_u8 reserved3 __attribute__((packed));
    os_u8 reserved4 __attribute__((packed));
} __attribute__((packed));

/***************************************************************
 extern function
 ***************************************************************/

#pragma pack()

#endif /* end of header */

