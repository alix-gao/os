/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : mass_storage.c
 * version     : 1.0
 * description : (key) usually, 1 config descriptor, 1 interface descriptor, 2 endpoint descriptor
 * author      : gaocheng
 * date        : 2011-01-01
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include <vds.h>
#include <vbs.h>
#include <disk.h>
#include "scsi.h"
#include "mass_storage.h"

/***************************************************************
 global variable declare
 ***************************************************************/

/***************************************************************
 function declare
 ***************************************************************/

/***************************************************************
 * description : usbmassbulk_10.pdf 3.1 Bulk-Only Mass Storage Reset (class-specific request)
 *               This request is used to reset the mass storage device and its associated interface.
 *               This class-specific request shall ready the device for the next CBW from the host.
 * history     :
 ***************************************************************/
LOCALC os_ret usb_ms_reset(HDEVICE usb)
{
    struct usb_setup_data setup = { 0 };
    struct usb_itf_info *intf_info;

    intf_info = usb_interface_info(usb);
    if (OS_NULL == intf_info->interface) {
        return OS_FAIL;
    }

    /* bmRequestType: Class, Interface, host to device */
    setup.bmRequestType = USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_OUT;
    setup.bRequest = 0xff;
    setup.wValue = 0;
    setup.wIndex = intf_info->interface->bInterfaceNumber;
    setup.wLength = 0;

    if (OS_SUCC != send_usb_control_data(usb, &setup, OS_NULL, 0)) {
        return OS_FAIL;
    }

    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret clear_feature_endpoint(HDEVICE usb, os_u8 endpoint)
{
    struct usb_setup_data setup = {0};

    setup.bmRequestType = USB_TYPE_STANDARD | USB_RECIP_ENDPOINT | USB_DIR_OUT;
    setup.bRequest = USB_REQ_CLEAR_FEATURE;
    setup.wValue = ENDPOINT_HALT;
    setup.wIndex = endpoint;
    setup.wLength = 0;

    if (OS_SUCC != send_usb_control_data(usb, &setup, 0, 0)) {
        return OS_FAIL;
    }
    return OS_SUCC;
}

/***************************************************************
 * description : usbmassbulk_10.pdf 5.3.4 reset recovery
 * history     :
 ***************************************************************/
LOCALC os_ret ms_reset_recovery(HDEVICE usb)
{
    struct mass_storage_struct *dedicated;

    /* For Reset Recovery the host shall issue in the following order: :
       (a) a Bulk-Only Mass Storage Reset
       (b) a Clear Feature HALT to the Bulk-In endpoint
       (c) a Clear Feature HALT to the Bulk-Out endpoint */

    if (OS_SUCC != usb_ms_reset(usb)) {
        ms_dbg("ms reset fail\n");
        goto error;
    }

    dedicated = get_usb_dedicated(usb);
    cassert(OS_NULL != dedicated);

    delay_ms(20);

    if (OS_SUCC != clear_feature_endpoint(usb, dedicated->pipe_out)) {
        ms_dbg("ms clear pipe out fail\n");
        goto error;
    }
    if (OS_SUCC != clear_feature_endpoint(usb, dedicated->pipe_in)) {
        ms_dbg("ms clear pipe in fail\n");
        goto error;
    }

    return OS_SUCC;
  error:
    return OS_FAIL;
}

/***************************************************************
 * description : usbmassbulk_10.pdf 3.2
 * history     :
 ***************************************************************/
LOCALC os_u8 usb_ms_get_max_lun(HDEVICE usb)
{
    struct usb_setup_data setup = { 0 };
    os_u8 buffer[4] = { 0 };
    struct usb_itf_info *intf_info;

    intf_info = usb_interface_info(usb);
    if (OS_NULL == intf_info->interface) {
        return 0;
    }

    /* bmRequestType: Class, Interface, device to host */
    setup.bmRequestType = USB_TYPE_CLASS | USB_RECIP_INTERFACE | USB_DIR_IN;
    setup.bRequest = 0xfe;
    setup.wValue = 0;
    setup.wIndex = intf_info->interface->bInterfaceNumber;
    setup.wLength = 1;

    if (OS_SUCC != recv_usb_control_data(usb, &setup, buffer, 1)) {
        return 0;
    }

    return buffer[0];
}

/* some thumb driver use 100 */
#define CSW_RETRY_TIME 0x4

/***************************************************************
 * description : usbmassbulk_10.pdf
 * history     :
 ***************************************************************/
LOCALC os_ret usb_mass_bulk(HDEVICE usb, os_u8 lun, enum ms_data_dir dir, os_u8 *data, os_u32 data_len, os_u8 *cbcw, os_u8 cbcw_len)
{
GLOBALDIF os_u32 dCBWTag_cnt = 0;
    struct ms_cbw_struct cbw;
    struct ms_csw_struct csw;
    struct mass_storage_struct *dedicated;
    os_u8 csw_time;
    os_ret result;

    dedicated = get_usb_dedicated(usb);

    cbw.dCBWSignature = CBW_SIGNATURE;
    cbw.dCBWTag = dCBWTag_cnt++;
    cbw.dCBWDataTransferLength = data_len;
    cbw.bmCBWFlags = dir; /* direction */
    cbw.bCBWLUN = lun;
    cbw.bCBWLength = cbcw_len;
    mem_cpy(cbw.CBWCB, cbcw, cbcw_len);

    /* 5.3.1 Command Transport */
    result = send_usb_bulk_data(usb, dedicated->pipe_out, &cbw, sizeof(struct ms_cbw_struct));
    if (OS_SUCC == result) {
        /* 5.3.2 Data Transport */
        if (0 != data_len) {
            switch (dir) {
            case MS_DATA_DIR_IN:
                result = recv_usb_bulk_data(usb, dedicated->pipe_in, data, data_len);
                break;
            case MS_DATA_DIR_OUT:
                result = send_usb_bulk_data(usb, dedicated->pipe_out, data, data_len);
                break;
            default:
                cassert(OS_FALSE);
                break;
            }
            if (OS_SUCC != result) {
                ms_dbg("recv bulk data fail\n");
            }
        }
        /* usbmassbulk_10.pdf 3.4 Command Queuing
           The host shall not transfer a CBW to the device until the host has received the CSW for any outstanding CBW.
           If the host issues two consecutive CBWs without an intervening CSW or reset,
           the device response to the second CBW is indeterminate. */
        /* 5.3.3 Status Transport */
        for (csw_time = 0; csw_time < CSW_RETRY_TIME; csw_time++) {
            result = recv_usb_bulk_data(usb, dedicated->pipe_in, &csw, sizeof(struct ms_csw_struct));
            if (OS_SUCC == result) {
                if (CSW_SIGNATURE == csw.dCSWSignature) {
                    if (cbw.dCBWTag != csw.dCSWTag) {
                        ms_dbg("(tag %d %d)", cbw.dCBWTag, csw.dCSWTag);
                        goto ms_reset;
                    } else if ((0 == csw.bCSWStatus) && (0 == csw.dCSWDataResidue)) {
                        return OS_SUCC;
                    } else {
                        ms_dbg("(csw %x %d %d)", csw.dCSWSignature, csw.bCSWStatus, csw.dCSWDataResidue);
                        return OS_FAIL;
                    }
                } else {
                    ms_dbg("csw sig error\n");
                    goto ms_reset;
                }
            } else {
                ms_dbg("in or out error\n");
                switch (dir) {
                case MS_DATA_DIR_IN:
                    clear_feature_endpoint(usb, dedicated->pipe_in);
                    break;
                case MS_DATA_DIR_OUT:
                    clear_feature_endpoint(usb, dedicated->pipe_out);
                    break;
                default:
                    cassert(OS_FALSE);
                    break;
                }
            }
        }
    }
  ms_reset:
    /* If the host detects a STALL of the Bulk-Out endpoint during command transport,
       the host shall respond with a Reset Recovery (see 5.3.4 - Reset Recovery). */
    ms_reset_recovery(usb);
    /* The device shall NAK the status stage of the device request until the Bulk-Only Mass Storage Reset is complete. */
    delay_task(0x10, __LINE__);
    return OS_FAIL;
}

/***************************************************************
 * description : android设备可以关闭mass storage, 此时查询csw状态位为1
 * history     :
 ***************************************************************/
LOCALC os_ret ms_test_ready(HDEVICE usb, os_u8 lun)
{
    os_ret result;
#define SCSI_READY_CMD_LEN 2
    os_u8 cbcw[SCSI_READY_CMD_LEN];

    /* operation code */
    cbcw[0] = TEST_UNIT_READY;
    /* lun */
    cbcw[1] = lun << 5;

    result = usb_mass_bulk(usb, lun, MS_DATA_DIR_OUT, OS_NULL, 0, cbcw, SCSI_READY_CMD_LEN);
    return result;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret ms_get_sense_info(HDEVICE usb, os_u8 lun)
{
#define SCSI_SENSE_CMD_LEN 0xc
    os_u8 cbcw[SCSI_SENSE_CMD_LEN] = { 0 };
#define SENSE_DATA_LEN 18
    os_u8 buffer[SENSE_DATA_LEN] = { 0 };
    os_ret result;

    /* operation code */
    cbcw[0] = REQUEST_SENSE;
    /* lun */
    cbcw[1] = lun << 5;
    /* allocation length */
    cbcw[4] = SENSE_DATA_LEN;

    result = usb_mass_bulk(usb, lun, MS_DATA_DIR_IN, buffer, SENSE_DATA_LEN, cbcw, SCSI_SENSE_CMD_LEN);

    /* print request sense standard data */
    do {
        os_u32 i;
        ms_dbg("sense data:\n");
        for (i = 0; i < SENSE_DATA_LEN; i++) {
            ms_dbg("%x ", buffer[i]);
        } ms_dbg("\n");
    } while (0);

    return result;
}

/***************************************************************
 * description : 记录设备类型
 * history     :
 ***************************************************************/
LOCALC os_ret ms_inquiry_command(HDEVICE usb, os_u8 lun)
{
#define SCSI_INQUIRY_CMD_LEN 6
    os_u8 cbcw[SCSI_INQUIRY_CMD_LEN] = { 0 };
#define INQUIRY_DATA_LEN 50
    os_u8 buffer[INQUIRY_DATA_LEN];
    os_ret result;

    /* operation code */
    cbcw[0] = INQUIRY;
    /* lun */
    cbcw[1] = lun << 5;
    /* allocation length */
    cbcw[4] = INQUIRY_DATA_LEN;
    mem_set(buffer, 0, INQUIRY_DATA_LEN * sizeof(os_u8));

    result = usb_mass_bulk(usb, lun, MS_DATA_DIR_IN, buffer, SCSI_INQUIRY_CMD_LEN, cbcw, SCSI_INQUIRY_CMD_LEN);
    if (OS_SUCC == result) {
        struct mass_storage_struct *dedicated;
        dedicated = get_usb_dedicated(usb);
        dedicated->ms_lun[lun].device_type = buffer[0]; /* inquiry data */
        return OS_SUCC;
    }
    return OS_FAIL;
}

/***************************************************************
 * description : 记录block length和Last Logical Block Address
 * history     :
 ***************************************************************/
LOCALC os_ret ms_read_capacity(HDEVICE usb, os_u8 lun)
{
#define SCSI_CAPACITY_CMD_LEN 0xa
    os_u8 cbcw[SCSI_CAPACITY_CMD_LEN] = { 0 };
#define CAPACITY_DATA_LEN 8
    os_u8 buffer[CAPACITY_DATA_LEN] = { 0 };
    os_ret result;

    /* operation code */
    cbcw[0] = READ_CAPACITY;
    /* lun */
    cbcw[1] = lun << 5;

    result = usb_mass_bulk(usb, lun, MS_DATA_DIR_IN, buffer, CAPACITY_DATA_LEN, cbcw, CAPACITY_DATA_LEN);
    if (OS_SUCC == result) {
        struct mass_storage_struct *dedicated;

        dedicated = get_usb_dedicated(usb);
        /* The Last Logical Block Address field holds the last valid LBA for use with media access commands. */
        dedicated->ms_lun[lun].last_lba = read_be_u32(&buffer[0]);
        /* The Block Length In Bytes field specifies the length in bytes of each logical block for the given capacity descriptor. */
        dedicated->ms_lun[lun].block_size = read_be_u32(&buffer[4]);
        return OS_SUCC;
    }
}

/***************************************************************
 * description : block_num必须是16位(与cbd一致)且不能为0
 * history     :
 ***************************************************************/
LOCALC os_ret ms_write(HDEVICE usb, os_u8 lun, os_u32 lba, os_u16 block_num, os_u8 *buffer)
{
    struct mass_storage_struct *dedicated;
    struct ufi_cbd_struct cbd = { 0 };
    os_u32 block_size;
    os_ret result;

    dedicated = get_usb_dedicated(usb);

    if ((lba + block_num) < dedicated->ms_lun[lun].last_lba) {
        /* caculate the byte count */
        block_size = dedicated->ms_lun[lun].block_size;
        cassert(!mul_u32_overflow(block_num, block_size));
        block_size = block_num * block_size;

        cbd.operation_code = WRITE_10;
        cbd.lun = lun << 5;
        write_be_u32((os_u8 *) &cbd.logical_block_address, lba);
        write_be_u16((os_u8 *) &cbd.length, block_num);

        result = usb_mass_bulk(usb, lun, MS_DATA_DIR_OUT, buffer, block_size, (os_u8 *) &cbd, sizeof(struct ufi_cbd_struct));
        return result;
    }
    ms_dbg("ms write, lba error\n");
    return OS_FAIL;
}

/***************************************************************
 * description : block_num必须是16位(与cbd一致)且不能为0
 *               函数入参必须为blocknum, 以便使size和blocknum一致
 * history     :
 ***************************************************************/
LOCALC os_ret ms_read(HDEVICE usb, os_u8 lun, os_u32 lba, os_u16 block_num, os_u8 *buffer)
{
    struct mass_storage_struct *dedicated;
    struct ufi_cbd_struct cbd = { 0 };
    os_u32 block_size;
    os_ret result;

    dedicated = get_usb_dedicated(usb);

    if ((lba + block_num) < dedicated->ms_lun[lun].last_lba) {
        /* caculate the byte count */
        block_size = dedicated->ms_lun[lun].block_size;
        cassert(!mul_u32_overflow(block_num, block_size));
        block_size = block_num * block_size;

        cbd.operation_code = READ_10;
        cbd.lun = lun << 5;
        write_be_u32((os_u8 *) &cbd.logical_block_address, lba);
        write_be_u16((os_u8 *) &cbd.length, block_num);

        result = usb_mass_bulk(usb, lun, MS_DATA_DIR_IN, buffer, block_size, (os_u8 *) &cbd, sizeof(struct ufi_cbd_struct));
        return result;
    }
    ms_dbg("ms read, lba error\n");
    return OS_FAIL;
}

/* max transfer length, type a value in the range of 65535 (64 KB) to 2097120 (2 MB). */
#if 1
#define MAX_TRANSFER_SIZE 0x10000
#else
/* apacer thumb driver could read larger blocks */
#define MAX_TRANSFER_SIZE 0x1fffe0
#endif

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret usb_mass_storage_read(os_void *disk, os_u32 lba, os_u16 block_num, os_u8 *buffer)
{
    struct ms_logic_disk *info;
    struct mass_storage_struct *dedicated;
    os_u32 block_size;
    os_u16 max_transfer_block;
    os_u16 tmp_block;
    os_ret result;

    if ((OS_NULL != disk) && (OS_NULL != buffer) && (0 != block_num)) {
        info = disk;
        dedicated = get_usb_dedicated(info->usb);
        cassert(OS_NULL != dedicated);
        block_size = dedicated->ms_lun[info->lun].block_size;
        max_transfer_block = MAX_TRANSFER_SIZE / block_size;
        cassert(UINT16_MAX >= (MAX_TRANSFER_SIZE / block_size));
        enter_critical_section(info->ms_op);
        tmp_block = 0;
        while (tmp_block < block_num) {
            cassert(!mul_u32_overflow(block_size, tmp_block));
            cassert(!add_u32_overflow(lba, tmp_block));
            if (max_transfer_block <= (block_num - tmp_block)) {
                result = ms_read(info->usb, info->lun, lba + tmp_block, max_transfer_block, buffer + block_size * tmp_block);
                tmp_block += max_transfer_block;
            } else {
                result = ms_read(info->usb, info->lun, lba + tmp_block, block_num - tmp_block, buffer + block_size * tmp_block);
                tmp_block = block_num;
            }
            if (SUCC != result) {
                break;
            }
        }
        leave_critical_section(info->ms_op);
        return result;
    } /* input para error */
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret usb_mass_storage_write(os_void *disk, os_u32 lba, os_u16 block_num, os_u8 *buffer)
{
    struct ms_logic_disk *info;
    struct mass_storage_struct *dedicated;
    os_u32 block_size;
    os_u16 max_transfer_block;
    os_u16 tmp_block;
    os_ret result;

    if ((OS_NULL != disk) && (OS_NULL != buffer) && (0 != block_num)) {
        info = disk;
        dedicated = get_usb_dedicated(info->usb);
        cassert(OS_NULL != dedicated);
        block_size = dedicated->ms_lun[info->lun].block_size;
        max_transfer_block = MAX_TRANSFER_SIZE / block_size;
        cassert(UINT16_MAX >= (MAX_TRANSFER_SIZE / block_size));
        enter_critical_section(info->ms_op);
        tmp_block = 0;
        while (tmp_block < block_num) {
            cassert(!mul_u32_overflow(block_size, tmp_block));
            cassert(!add_u32_overflow(lba, tmp_block));
            if (max_transfer_block <= (block_num - tmp_block)) {
                result = ms_write(info->usb, info->lun, lba + tmp_block, max_transfer_block, buffer + block_size * tmp_block);
                tmp_block += max_transfer_block;
            } else {
                result = ms_write(info->usb, info->lun, lba + tmp_block, block_num - tmp_block, buffer + block_size * tmp_block);
                tmp_block = block_num;
            }
            if (SUCC != result) {
                break;
            }
        }
        leave_critical_section(info->ms_op);
        return result;
    } /* input para error */
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret usb_mass_storage_query(os_void *disk, struct disk_para *para)
{
    struct ms_logic_disk *info;
    struct mass_storage_struct *dedicated;

    if ((OS_NULL != disk) && (OS_NULL != para)) {
        info = disk;
        dedicated = get_usb_dedicated(info->usb);
        para->max_lba = dedicated->ms_lun[info->lun].last_lba;
        para->bytes_per_sec = dedicated->ms_lun[info->lun].block_size;
        return OS_SUCC;
    }
    return OS_FAIL;
}

LOCALD struct disk_operations ms_operations = {
    usb_mass_storage_query,
    usb_mass_storage_read,
    usb_mass_storage_write
};

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret usb_mass_storage_probe(HDEVICE usb)
{
    struct disk_device *device;
    struct mass_storage_struct *dedicated;
    struct usb_itf_info *intf_info;
    os_uint i;
    os_u8 pipe_in = 0;
    os_u8 pipe_out = 0;
    os_u8 lun;
    struct ms_logic_disk *logic_disk;
    os_ret result;

    ms_dbg("mass storage\n");

    cassert(OS_NULL != usb);

    /* for cleanup, dedicated = device = para = OS_NULL; */
    dedicated = OS_NULL;
    device = OS_NULL;
    logic_disk = OS_NULL;

    /* 从端点描述符中找到ms需要的两个端点信息 */

    intf_info = usb_interface_info(usb);
    for (i = 0; i < intf_info->interface->bNumEndpoints; i++) {
        if (USB_BULK_TRANSFER == get_usb_transfer_type(intf_info->endpoint[i]->bmAttributes)) {
            if (endpoint_is_in(intf_info->endpoint[i]->bEndpointAddress)) {
                pipe_in = endpoint_num(intf_info->endpoint[i]->bEndpointAddress);
            } else if (endpoint_is_out(intf_info->endpoint[i]->bEndpointAddress)) {
                pipe_out = endpoint_num(intf_info->endpoint[i]->bEndpointAddress);
            } else {
                cassert(OS_FALSE);
            }
        }
    }

    if ((0 == pipe_in) || (0 == pipe_out)) {
        ms_dbg("there is no enough endpoint for ms.\n");
        return OS_FAIL;
    }

    dedicated = kmalloc(sizeof(struct mass_storage_struct));
    if (OS_NULL == dedicated) {
        ms_dbg("alloc ms dedicated info fail\n");
        goto cleanup;
    }
    mem_set(dedicated, 0, sizeof(struct mass_storage_struct));
    init_list_head(&dedicated->lun_head);

    set_usb_dedicated(usb, dedicated);
    dedicated->pipe_in = pipe_in;
    dedicated->pipe_out = pipe_out;

    /* 重启设备, 当android设备关闭mass storage时, 重启一次再重启时会重启失败. sumsung设备第一次重启就失败 */
    result = ms_reset_recovery(usb);
    if (OS_SUCC != result) {
        ms_dbg("ms reset fail\n");
        /* goto cleanup; // sumsung android phone could NAK this command and reset can only try once */
    }
    delay_task(10, __LINE__);

    /* get lun */
    lun = usb_ms_get_max_lun(usb);

    /* Logical Unit Numbers on the device shall be numbered contiguously starting from LUN 0 to a maximum LUN of 15 (Fh). */
    for (i = 0; i <= lun; i++) {
#define MS_READY_TIME 0x10
        os_u32 time = MS_READY_TIME;
        while (time--) {
            /* 设备就绪 */
            result = ms_test_ready(usb, i); /* ready? */
            if (OS_SUCC == result) {
                ms_dbg("lun %d ready\n", i);
                break;
            }
            /* sense information */
            if (OS_SUCC != ms_get_sense_info(usb, i)) {
                ms_dbg("lun %d get sense fail\n", i);
            }
        }
        if (OS_SUCC != result) {
            ms_dbg("lun %d is not ready\n", i);
            break;
        }
        /* 查询设备类型 */
        result = ms_inquiry_command(usb, i);
        if (OS_SUCC != result) {
            ms_dbg("lun %d inquiry fail\n", i);
            result = ms_inquiry_command(usb, i);
            if (OS_SUCC != result) {
                ms_dbg("lun %d inquiry fail\n", i);
                continue;
            }
        }

        /* 获取设备能力 */
        result = ms_read_capacity(usb, i);
        if (OS_SUCC != result) {
            ms_dbg("lun %d capacity fail\n", i);
            continue;
        }

        ms_dbg("last lba: %x, size %x\n", dedicated->ms_lun[i].last_lba, dedicated->ms_lun[i].block_size);

        logic_disk = kmalloc(sizeof(struct ms_logic_disk));
        if (OS_NULL == logic_disk) {
            ms_dbg("alloc para fail\n");
            goto cleanup;
        }
        add_list_head(&dedicated->lun_head, &logic_disk->logic_unit);
        logic_disk->lun = i; /* 记录逻辑单元号 */
        logic_disk->usb = usb;
        result = create_critical_section(&logic_disk->ms_op, __LINE__);
        if (OS_SUCC != result) {
            ms_dbg("init ms_op fail\n");
            goto cleanup;
        }

        /* 注册逻辑设备 */
        device = kmalloc(sizeof(struct disk_device));
        if (OS_NULL == device) {
            ms_dbg("alloc v-device fail\n");
            goto cleanup;
        }
        mem_set(device, 0, sizeof(struct disk_device));
        logic_disk->vdisk = device;
        switch (dedicated->ms_lun[i].device_type) {
        case TYPE_DISK:
            device->name = "usb-disk";
            break;
        case TYPE_TAPE:
            device->name = "usb-tape";
            break;
        case TYPE_PROCESSOR:
            device->name = "usb-processor";
            break;
        case TYPE_WORM:
            device->name = "usb-worm";
            break;
        case TYPE_ROM:
            device->name = "usb-rom";
            break;
        case TYPE_SCANNER:
            device->name = "usb-scanner";
            break;
        case TYPE_MOD:
            device->name = "usb-mod";
            break;
        case TYPE_MEDIUM_CHANGER:
            device->name = "usb-medium-changer";
            break;
        case TYPE_COMM:
            device->name = "usb-comm";
            break;
        case TYPE_ENCLOSURE:
            device->name = "usb-enclosure";
            break;
        case TYPE_NO_LUN:
        default:
            break;
        }
        device->operation = &ms_operations;
        device->disk = logic_disk;
        device->vfs = OS_NULL;
        device->vfs_cache = OS_NULL;
        device->para.bytes_per_sec = dedicated->ms_lun[i].block_size;
        device->para.max_lba = dedicated->ms_lun[i].last_lba;
        device->vfs_index = 0;
        result = register_disk(device);
        if (OS_SUCC != result) {
            ms_dbg("register disk fail\n");
        }
    }
    return OS_SUCC;
  cleanup:
    if (OS_NULL != device) { unregister_disk(device); kfree(device); }
    if (OS_NULL != logic_disk) { kfree(logic_disk); }
    if (OS_NULL != dedicated) { kfree(dedicated); }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void usb_mass_storage_remove(HDEVICE usb)
{
    struct mass_storage_struct *dedicated;
    struct list_node *i, *_save;
    struct ms_logic_disk *device;

    cassert(OS_NULL != usb);

    ms_dbg("remove mass storage\n");

    dedicated = get_usb_dedicated(usb);

    loop_del_list(i, _save, &dedicated->lun_head) {
        device = list_addr(i, struct ms_logic_disk, logic_unit);

        ms_dbg("remove lun %s %d\n", device->vdisk->name, device->vdisk->vfs_index);
        unregister_disk(device->vdisk);
        kfree(device->vdisk);
        destroy_critical_section(device->ms_op);
        kfree(device);
    }

    kfree(dedicated);
}

LOCALD const struct usb_device_id usb_mass_storage_id = {
    USB_ANY_ID, USB_ANY_ID, /* any vender, any product id. */
    USB_CLASS_MASS_STORAGE,
    USB_ANY_ID,
    USB_ANY_ID
};

/* 通用usb设备驱动表 */
LOCALD const struct usb_driver usb_mass_storage_driver = {
    "usb-mass-storage",
    &usb_mass_storage_id,
    usb_mass_storage_probe,
    usb_mass_storage_remove
};

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_void init_usb_mass_storage_driver(os_void)
{
    os_ret result;

    ms_dbg("install mass storage driver\n");

    /* 注册的时序无法保证, 因此要求注册了即可使用设备 */
    result = register_usb_driver(&usb_mass_storage_driver);
    cassert(OS_SUCC == result);
}
device_init_func(init_usb_mass_storage_driver);

