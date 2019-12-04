/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : disk.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __DISK_H__
#define __DISK_H__

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
struct disk_para {
    os_u32 bytes_per_sec;
    os_u32 max_lba;
};

/***************************************************************
 * description :
 ***************************************************************/
struct disk_operations {
    os_ret (*query)(os_void *disk, struct disk_para *para);
    os_ret (*read)(os_void *disk, os_u32 lba, os_u16 num, os_u8 *buffer);
    os_ret (*write)(os_void *disk, os_u32 lba, os_u16 num, os_u8 *buffer);
};

/***************************************************************
 * description :
 ***************************************************************/
struct disk_device {
    os_u8 *name;

    os_void *disk; /* operation的入参 */
    struct disk_operations *operation;

    struct disk_para para; /* 几何参数 */

    /* 文件系统信息 */
    const os_void *vfs; // struct vfs *vfs;
    os_void *vfs_cache;
    os_u32 vfs_index;
};

/***************************************************************
 extern function
 ***************************************************************/
struct disk_device *get_disk_device(os_u32 device_id);

os_ret register_disk(struct disk_device *dev);
os_ret unregister_disk(struct disk_device *dev);
os_ret query_disk(os_u32 device_id, struct disk_para *para);
os_ret OS_API read_disk(os_u32 device_id, os_u32 start_sector, os_u16 num, os_u8 *buffer);
os_ret OS_API write_disk(os_u32 device_id, os_u32 start_sector, os_u16 num, os_u8 *buffer);

os_u32 curr_disk_device_id(os_void);
os_ret set_disk_device_id(os_u32 id);

#pragma pack()

#endif /* end of header */

