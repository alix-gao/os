/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : vfs.c
 * version     : 1.0
 * description : (key) abstract
 * author      : gaocheng
 * date        : 2011-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include <vfs.h>

/***************************************************************
 global variable declare
 ***************************************************************/
/* 懒得再定义一层以动态链表方式存储 */
#define MAX_FS_NUM 0x10
LOCALD struct vfs *vfs_list[MAX_FS_NUM] = { OS_NULL }; /* 1 ~ MAX_FS_NUM */

LOCALD rwlock_t vfs_lock;

/***************************************************************
 function declare
 ***************************************************************/

LOCALC os_void init_vfs(os_void)
{
    os_u32 i;

    for (i = 0; i < MAX_FS_NUM; i++) {
        vfs_list[i] = OS_NULL;
    }
    init_rw_lock(&vfs_lock);
}
init_afunc(init_vfs);

/***************************************************************
 * description : 检查磁盘的文件系统
 * history     :
 ***************************************************************/
os_u32 mount_disk_fs(os_u32 device_id)
{
    os_u32 i;

    /* 根据当前支持的文件系统逐个尝试 */
    for (i = 1; i < MAX_FS_NUM; i++) {
        read_lock(&vfs_lock);
        if (OS_NULL != vfs_list[i]) {
            if (vfs_list[i]->operation->vfs_mount) {
                os_ret (*mount)(os_u32 device_id);
                mount = vfs_list[i]->operation->vfs_mount;
                read_unlock(&vfs_lock);
                if (OS_SUCC == mount(device_id)) {
                    return i;
                }
            }
        }
        read_unlock(&vfs_lock);
    }
    return 0;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret unmount_disk_fs(os_u32 device_id, os_u32 vfs_index)
{
    read_lock(&vfs_lock);
    if (OS_NULL != vfs_list[vfs_index]) {
        if (vfs_list[vfs_index]->operation->vfs_unmount) {
            os_ret (*unmount)(os_u32 device_id);
            unmount = vfs_list[vfs_index]->operation->vfs_unmount;
            read_unlock(&vfs_lock);
            return unmount(device_id);
        }
    }
    read_unlock(&vfs_lock);
    return OS_FAIL;
}

/***************************************************************
 * description : 检查磁盘是否是指定的文件系统
 * history     :
 ***************************************************************/
os_ret check_disk_spec_fs(os_u32 device_id, enum fs_type type)
{
    os_u32 i;

    read_lock(&vfs_lock);
    /* 根据当前支持的文件系统逐个尝试 */
    for (i = 1; i < MAX_FS_NUM; i++) {
        if ((OS_NULL != vfs_list[i]) && (type == vfs_list[i]->type)) {
            if (vfs_list[i]->operation->vfs_mount) {
                os_ret (*mount)(os_u32 device_id);
                mount = vfs_list[i]->operation->vfs_mount;
                read_unlock(&vfs_lock);
                return mount(device_id);
            }
        }
    }
    read_unlock(&vfs_lock);
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret register_fs(const struct vfs *fs)
{
    os_u32 i;

    cassert(OS_NULL != fs);

    /* 关联到vfs支持的文件系统中 */
    write_lock(&vfs_lock);
    for (i = 1; i < MAX_FS_NUM; i++) {
        if (OS_NULL == vfs_list[i]) {
            vfs_list[i] = (struct vfs *) fs;
            break;
        }
    }
    write_unlock(&vfs_lock);
    if (MAX_FS_NUM == i) {
        return OS_FAIL;
    }

    /* 扫描是否有磁盘未被识别 */
    update_disk_fs(fs->type, i);
    return OS_SUCC;
}

