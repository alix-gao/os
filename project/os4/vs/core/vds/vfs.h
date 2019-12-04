/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : vfs.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __VFS_H__
#define __VFS_H__

/***************************************************************
 include header file
 ***************************************************************/
#include <disk.h>

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/
#define MAX_FILE_NAME_LEN 0x100

/***************************************************************
 enum define
 ***************************************************************/

/***************************************************************
 * enum name   : fs_type_enum
 * description :
 ***************************************************************/
enum fs_type {
    FS_FAT16,
    FS_FAT32,
    FS_BUTT
};

/***************************************************************
 struct define
 ***************************************************************/

/***************************************************************
 * description : HFILE
 ***************************************************************/
struct vfs_handle {
    os_u32 device_id;
    os_u8 file_name[MAX_FILE_NAME_LEN]; /* 文件名需要存储 */
    os_u32 total_size; /* 文件的实际大小 */
    klm_handle data; /* 文件数据头节点 */
    os_u32 file_pointer; /* 文件读写指针 */
    os_bool file_wflag; /* 文件写标志 */
};

/***************************************************************
 * description :
 ***************************************************************/
struct vfs {
    os_u8 *name;
    enum fs_type type;
    const struct vfs_operations {
        os_ret (*vfs_mount)(os_u32 device_id);
        os_ret (*vfs_unmount)(os_u32 device_id);

        os_ret (*vfs_change_directory)(os_u32 device_id, IN os_u8 *dir_name);
        os_ret (*vfs_create_directory)(os_u32 device_id, IN HDIR dir, IN os_u8 *dir_name);
        os_ret (*vfs_destroy_directory)(os_u32 device_id, IN HDIR dir, IN os_u8 *dir_name);

        HDIR (*vfs_open_dir)(os_u32 device_id, IN os_u8 *dir_name);
        os_ret (*vfs_close_dir)(os_u32 device_id, IN HDIR dir);

        os_ret (*vfs_list)(os_u32 device_id, IN HDIR dir);
        os_ret (*vfs_create)(os_u32 device_id, IN HDIR dir, IN os_u8 *file_name);
        os_ret (*vfs_destroy)(os_u32 device_id, IN HDIR dir, IN os_u8 *file_name);
        os_ret (*vfs_rename)(os_u32 device_id, IN HDIR dir, IN os_u8 *file_name, IN os_u8 *new_file_name);

        HFILE (*vfs_open)(os_u32 device_id, IN HDIR dir, IN os_u8 *file_name);
        os_ret (*vfs_seek)(HFILE file_handle, os_s32 offset, enum file_seek pos);
        os_ret (*vfs_read)(HFILE handle, os_u8 *buffer, os_u32 len);
        os_ret (*vfs_write)(HFILE handle, os_u8 *buffer, os_u32 len);
        os_bool (*vfs_eof)(HFILE file_handle);
        os_u32 (*vfs_size)(HFILE file_handle);
        os_ret (*vfs_close)(HFILE handle);
    } *operation;
};

/***************************************************************
 extern function
 ***************************************************************/

#pragma pack()

#endif

