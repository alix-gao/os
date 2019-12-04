/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : disk.c
 * version     : 1.0
 * description : (key) abstract
 * author      : gaocheng
 * date        : 2011-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include <disk.h>
#include <vfs.h>

/***************************************************************
 global variable declare
 ***************************************************************/

/* 最大数量不是26个字母 */
#define MAX_DISK_NUM 0x10
LOCALD struct disk_device *disk_list[MAX_DISK_NUM] = { OS_NULL };
/* lock for disk_list */
LOCALD rwlock_t disk_list_lock = { 0 };

/* 当前使用磁盘的设备id */
LOCALD volatile os_u32 current_disk_id _CPU_ALIGNED_ = MAX_DISK_NUM;

/***************************************************************
 function declare
 ***************************************************************/

/***************************************************************
 * description : 获取当前使用磁盘的设备id
 * history     :
 ***************************************************************/
os_u32 curr_disk_device_id(os_void)
{
    os_u32 device_id;
    device_id = current_disk_id;
    rmb();
    return device_id;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret set_disk_device_id(os_u32 id)
{
    if (MAX_DISK_NUM > id) {
        current_disk_id = id;
        wmb();
        return OS_SUCC;
    } else {
        return OS_FAIL;
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
struct disk_device *get_disk_device(os_u32 device_id)
{
    if (MAX_DISK_NUM > device_id) {
        return disk_list[device_id];
    } else {
        return OS_NULL;
    }
}

/***************************************************************
 * description : 查询磁盘的几何参数, 优先从内存中查找, 如果不存在则实际查询
 * history     :
 ***************************************************************/
os_ret query_disk(os_u32 device_id, struct disk_para *para)
{
    struct disk_device *disk;
    os_ret ret;

    cassert((MAX_DISK_NUM > device_id) && (OS_NULL != disk_list[device_id]) && (OS_NULL != para));

    read_lock(&disk_list_lock);
    disk = disk_list[device_id];
    read_unlock(&disk_list_lock);

    if ((0 != disk->para.bytes_per_sec)
     && (0 != disk->para.max_lba)) {
        para->bytes_per_sec = disk->para.bytes_per_sec;
        para->max_lba = disk->para.max_lba;
    } else {
        /* 查询 */
        ret = disk->operation->query(disk->disk, para);
        if (OS_SUCC != ret) {
            return OS_FAIL;
        }
        disk->para.bytes_per_sec = para->bytes_per_sec;
        disk->para.max_lba = para->max_lba;
        barrier();
    }
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API read_disk(os_u32 device_id, os_u32 start_sector, os_u16 num, OUT os_u8 *buffer)
{
    os_void *disk;
os_ret (*read)(os_void *disk, os_u32 lba, os_u16 num, os_u8 *buffer);

    read_lock(&disk_list_lock);
    if ((MAX_DISK_NUM > device_id) && (OS_NULL != disk_list[device_id]) && (OS_NULL != buffer)) {
        disk = disk_list[device_id]->disk;
        read = disk_list[device_id]->operation->read;
        read_unlock(&disk_list_lock);
        return read(disk, start_sector, num, buffer);
    }
    read_unlock(&disk_list_lock);
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API write_disk(os_u32 device_id, os_u32 start_sector, os_u16 num, OUT os_u8 *buffer)
{
    os_void *disk;
os_ret (*write)(os_void *disk, os_u32 lba, os_u16 num, os_u8 *buffer);

    read_lock(&disk_list_lock);
    if ((MAX_DISK_NUM > device_id) && (OS_NULL != disk_list[device_id]) && (OS_NULL != buffer)) {
        disk = disk_list[device_id]->disk;
        write = disk_list[device_id]->operation->write;
        read_unlock(&disk_list_lock);
        return write(disk, start_sector, num, buffer);
    }
    read_unlock(&disk_list_lock);
    return OS_FAIL;
}

/***************************************************************
 * description : 重新检查未识别文件系统的磁盘
 * history     :
 ***************************************************************/
os_void update_disk_fs(enum fs_type type, os_u32 vfs_index)
{
    struct disk_device **d;
    os_u32 i;

    for (i = 0, d = disk_list; i < MAX_DISK_NUM; i++, d++) {
        if (OS_NULL != *d) {
            if (OS_NULL == (*d)->vfs) {
                /* 识别文件系统, 成功失败都要继续检查 */
                if (OS_SUCC == check_disk_spec_fs(i, type)) {
                    write_lock(&disk_list_lock);
                    /* double check */
                    if (OS_NULL == (*d)->vfs) {
                        (*d)->vfs_index = vfs_index;
                    }
                    write_unlock(&disk_list_lock);
                }
            }
        }
    }
}

/***************************************************************
 * description : 显示磁盘列表
 * history     :
 ***************************************************************/
LOCALC os_void dump_disk_info(os_void)
{
    os_u32 i;
    const struct vfs *fs;

    for (i = 0; i < MAX_DISK_NUM; i++) {
        if (OS_NULL != disk_list[i]) {
            print("device %s id: %d, %x ", disk_list[i]->name, i, disk_list[i]->para.bytes_per_sec);

            fs = disk_list[i]->vfs;
            if ((OS_NULL != fs) && (FS_BUTT > fs->type)) {
                print("(vfs type %d)", fs->type);
            }
            print("\n");
        }
    }
    print("cdi:%d\n", current_disk_id);
}
LOCALD os_u8 disk_debug_name[] = { "disk" };
LOCALD struct dump_info disk_debug = {
    disk_debug_name,
    dump_disk_info
};

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API create_file(os_u32 device_id, IN os_u8 *file_name)
{
    const struct vfs *fs;

    read_lock(&disk_list_lock);
    if ((MAX_DISK_NUM > device_id) && (OS_NULL != disk_list[device_id]) && (OS_NULL != file_name)) {
        fs = disk_list[device_id]->vfs;
        read_unlock(&disk_list_lock);
        if ((OS_NULL == fs) || (FS_BUTT <= fs->type) || (OS_NULL == fs->operation)) {
            return OS_FAIL;
        }
        return fs->operation->vfs_create(device_id, OS_NULL, file_name);
    }
    read_unlock(&disk_list_lock);
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API delete_file(os_u32 device_id, IN os_u8 *file_name)
{
    const struct vfs *fs;

    read_lock(&disk_list_lock);
    if ((MAX_DISK_NUM > device_id) && (OS_NULL != disk_list[device_id]) && (OS_NULL != file_name)) {
        fs = disk_list[device_id]->vfs;
        read_unlock(&disk_list_lock);
        if ((OS_NULL == fs) || (FS_BUTT <= fs->type) || (OS_NULL == fs->operation)) {
            return OS_FAIL;
        }
        return fs->operation->vfs_destroy(device_id, OS_NULL, file_name);
    }
    read_unlock(&disk_list_lock);
    return OS_FAIL;
}

/***************************************************************
 * description : 显示某个磁盘的文件列表
 * history     :
 ***************************************************************/
os_void OS_API list_file(os_u32 device_id)
{
    const struct vfs *fs;

    read_lock(&disk_list_lock);
    if ((MAX_DISK_NUM > device_id) && (OS_NULL != disk_list[device_id])) {
        fs = disk_list[device_id]->vfs;
        read_unlock(&disk_list_lock);
        if ((OS_NULL == fs) || (FS_BUTT <= fs->type) || (OS_NULL == fs->operation)) {
            flog("vfs invalid\n");
            return;
        }
        fs->operation->vfs_list(device_id, OS_NULL);
        return;
    }
    read_unlock(&disk_list_lock);
    flog("disk do not exist, can not list\n");
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
HFILE OS_API open_file(os_u32 device_id, IN os_u8 *file_name)
{
    const struct vfs *fs;

    read_lock(&disk_list_lock);
    if ((MAX_DISK_NUM > device_id) && (OS_NULL != disk_list[device_id]) && (OS_NULL != file_name)) {
        fs = disk_list[device_id]->vfs;
        read_unlock(&disk_list_lock);
        if ((OS_NULL == fs) || (FS_BUTT <= fs->type) || (OS_NULL == fs->operation)) {
            return OS_NULL;
        }
        return fs->operation->vfs_open(device_id, OS_NULL, file_name);
    }
    read_unlock(&disk_list_lock);
    return OS_NULL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API close_file(HFILE handle)
{
    const struct vfs *fs;
    struct vfs_handle *vfs_h;
    os_u32 device_id;

    if (OS_NULL != handle) {
        vfs_h = handle;
        device_id = vfs_h->device_id;

        cassert(MAX_DISK_NUM > device_id);

        read_lock(&disk_list_lock);
        cassert(OS_NULL != disk_list[device_id]);
        fs = disk_list[device_id]->vfs;
        read_unlock(&disk_list_lock);

        if ((OS_NULL == fs) || (FS_BUTT <= fs->type) || (OS_NULL == fs->operation)) {
            return OS_FAIL;
        }
        return fs->operation->vfs_close(handle);
    }
    flog("disk do not exist, can not close\n");
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API read_file(HFILE file_handle, os_u8 *buffer, os_u32 len)
{
    const struct vfs *fs;

    if ((OS_NULL != file_handle) && (OS_NULL != buffer) && (0 != len)) {
        read_lock(&disk_list_lock);
        cassert(OS_NULL != disk_list[((struct vfs_handle *) file_handle)->device_id]);
        fs = disk_list[((struct vfs_handle *) file_handle)->device_id]->vfs;
        read_unlock(&disk_list_lock);
        return fs->operation->vfs_read(file_handle, buffer, len);
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API write_file(HFILE file_handle, os_u8 *buffer, os_u32 len)
{
    const struct vfs *fs;

    if ((OS_NULL != file_handle) && (OS_NULL != buffer) && (0 != len)) {
        read_lock(&disk_list_lock);
        cassert(OS_NULL != disk_list[((struct vfs_handle *) file_handle)->device_id]);
        fs = disk_list[((struct vfs_handle *) file_handle)->device_id]->vfs;
        read_unlock(&disk_list_lock);
        return fs->operation->vfs_write(file_handle, buffer, len);
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_u32 OS_API size_of_file(HFILE file_handle)
{
    const struct vfs *fs;

    if (OS_NULL != file_handle) {
        read_lock(&disk_list_lock);
        cassert(OS_NULL != disk_list[((struct vfs_handle *) file_handle)->device_id]);
        fs = disk_list[((struct vfs_handle *) file_handle)->device_id]->vfs;
        read_unlock(&disk_list_lock);
        return fs->operation->vfs_size(file_handle);
    }
    return 0;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API seek_file(HFILE file_handle, os_s32 offset, enum file_seek pos)
{
    const struct vfs *fs;

    if ((OS_NULL != file_handle) && (SEEK_POS_BUT >= pos)) {
        read_lock(&disk_list_lock);
        cassert(OS_NULL != disk_list[((struct vfs_handle *) file_handle)->device_id]);
        fs = disk_list[((struct vfs_handle *) file_handle)->device_id]->vfs;
        read_unlock(&disk_list_lock);
        return fs->operation->vfs_seek(file_handle, offset, pos);
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_bool OS_API end_of_file(HFILE file_handle)
{
    const struct vfs *fs;

    if (OS_NULL != file_handle) {
        read_lock(&disk_list_lock);
        cassert(OS_NULL != disk_list[((struct vfs_handle *) file_handle)->device_id]);
        fs = disk_list[((struct vfs_handle *) file_handle)->device_id]->vfs;
        read_unlock(&disk_list_lock);
        return fs->operation->vfs_eof(file_handle);
    }
    return OS_FALSE;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret OS_API change_working_dir(os_u32 device_id, IN os_u8 *dir_name)
{
    const struct vfs *fs;

    read_lock(&disk_list_lock);
    if ((MAX_DISK_NUM > device_id) && (OS_NULL != disk_list[device_id]) && (OS_NULL != dir_name)) {
        fs = disk_list[device_id]->vfs;
        read_unlock(&disk_list_lock);
        if ((OS_NULL == fs) || (FS_BUTT <= fs->type) || (OS_NULL == fs->operation)) {
            flog("change dir, vfs error\n");
            return OS_FAIL;
        }
        return fs->operation->vfs_change_directory(device_id, dir_name);
    }
    read_unlock(&disk_list_lock);
    flog("change dir, input para error\n");
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
HDIR OS_API open_dir(os_u32 device_id, IN os_u8 *dir_name)
{
    const struct vfs *fs;

    read_lock(&disk_list_lock);
    if ((MAX_DISK_NUM > device_id) && (OS_NULL != disk_list[device_id]) && (OS_NULL != dir_name)) {
        fs = disk_list[device_id]->vfs;
        read_unlock(&disk_list_lock);
        if ((OS_NULL == fs) || (FS_BUTT <= fs->type) || (OS_NULL == fs->operation)) {
            flog("change dir, vfs error\n");
            return OS_NULL;
        }
        return fs->operation->vfs_open_dir(device_id, dir_name);
    }
    read_unlock(&disk_list_lock);
    flog("change dir, input para error\n");
    return OS_NULL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret test_load(os_u8 *file_name)
{
    HFILE fp;
    os_u8 *buffer;
    os_ret result;
    os_u32 device_id;

    device_id = curr_disk_device_id();

    fp = open_file(device_id, file_name);
    if (OS_NULL == fp) {
        flog("open file fail\n");
        return OS_FAIL;
    }

    buffer = kmalloc(0x100);
    if (OS_NULL == buffer) {
        result = OS_FAIL;
        goto phase_1;
    }

    result = read_file(fp, buffer, 0x100);
    if (OS_SUCC != result) {
        result = OS_FAIL;
        flog("read file fail\n");
        goto phase_2;
    }
    flog("file data addr: %x\n", buffer);
  phase_2:
    //kfree(buffer); this is test function, reserved for show
  phase_1:
    close_file(fp);
    return result;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret test_edit(os_u8 *file_name)
{
    HFILE fp;
    os_ret result;
    os_u32 device_id;
    os_u8 buffer[] = { 'h', 'o', 'w', 'l', '\r', '\n' };
    os_u8 end[] = { 'j', 'a', 'c', 'k' };
    os_u32 i;

    device_id = curr_disk_device_id();

    /* 1. test multiple write */
    fp = open_file(device_id, file_name);
    if (OS_NULL == fp) {
        flog("open file fail\n");
        return OS_FAIL;
    }
    for (i = 0; i < 0x1000; i++) {
        result = write_file(fp, buffer, sizeof(buffer));
        if (OS_SUCC != result) {
            result = OS_FAIL;
            flog("read file fail\n");
        }
    }
    close_file(fp);

    /* 2. write from beginning */
    fp = open_file(device_id, file_name);
    if (OS_NULL == fp) {
        flog("open file fail\n");
        return OS_FAIL;
    }
    result = write_file(fp, end, sizeof(end));
    if (OS_SUCC != result) {
        result = OS_FAIL;
        flog("read file fail\n");
    }
    close_file(fp);

    /* 3. write from ending */
    fp = open_file(device_id, file_name);
    if (OS_NULL == fp) {
        flog("open file fail\n");
        return OS_FAIL;
    }
    seek_file(fp, 0, SEEK_POS_END);
    result = write_file(fp, end, sizeof(end));
    if (OS_SUCC != result) {
        result = OS_FAIL;
        flog("read file fail\n");
    }
    close_file(fp);

    return result;
}

os_ret test_write_ex(os_u32 device_id, os_u32 start_sector, os_u16 num)
{
    struct disk_para para = {0};
    os_u8 *buffer;
    os_ret result;

    if (OS_NULL != disk_list[device_id]) {
        para.bytes_per_sec = 512;
        print("start_sector %d, num %d, bytepersec %d.\n", start_sector, num, para.bytes_per_sec);
        buffer = kmalloc(num * para.bytes_per_sec);
        if (OS_NULL == buffer) {
            print("malloc fail\n");
            return OS_FAIL;
        }
        mem_set(buffer, 0xffffffff, num * para.bytes_per_sec);

        result = read_disk(device_id, start_sector, num, buffer);
        if (OS_SUCC != result) {
            print("read fail\n");
            result = OS_FAIL;
            goto end;
        }

        print("test write end\n");
        result = OS_SUCC;
      end:
        kfree(buffer);
        return result;
    }
    return OS_FAIL;
}

os_ret test_write(os_u32 device_id, os_u32 start_sector, os_u16 num)
{
    os_void *disk;
    struct disk_para para = {0};
    os_u8 *buffer;
    os_ret result;

    if (OS_NULL != disk_list[device_id]) {
        /* 获取入参 */
        disk = disk_list[device_id]->disk;
        /* 查询 */
        result = disk_list[device_id]->operation->query(disk, &para);
        if (OS_SUCC != result) {
            print("query fail\n");
            return OS_FAIL;
        }
        print("start_sector %d, num %d, bytepersec %d.\n", start_sector, num, para.bytes_per_sec);
        buffer = kmalloc(num * para.bytes_per_sec);
        if (OS_NULL == buffer) {
            print("malloc fail\n");
            return OS_FAIL;
        }
        mem_set(buffer, 0xffffffff, num * para.bytes_per_sec);
        result = disk_list[device_id]->operation->read(disk, start_sector, num, buffer);
        if (OS_SUCC != result) {
            print("read fail\n");
            result = OS_FAIL;
            goto end;
        }
        print("read %x\n", buffer);
        for (result = 0; result < 0x10; result++) {
            print("%x ", buffer[result]);
        } print("\n");

        buffer[0x8] = 0x29;
        result = disk_list[device_id]->operation->write(disk, start_sector, num, buffer);
        if (OS_SUCC != result) {
            print("write fail\n");
            result = OS_FAIL;
            goto end;
        }
        mem_set(buffer, 0xffffffff, num * para.bytes_per_sec);
        result = disk_list[device_id]->operation->read(disk, start_sector, num, buffer);
        if (OS_SUCC != result) {
            print("read fail\n");
            result = OS_FAIL;
            goto end;
        }
        print("addr %x\n", buffer);
        for (result = 0; result < 0x10; result++) {
            print("%x ", buffer[result]);
        } print("\n");
        result = write_disk(device_id, start_sector, num, buffer);
        if (OS_SUCC != result) {
            print("write fail\n");
            result = OS_FAIL;
            goto end;
        }
        print("test write end\n");
        result = OS_SUCC;
      end:
        kfree(buffer);
        return result;
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret test_read(os_u32 device_id, os_u32 start_sector, os_u16 num)
{
    os_void *disk;
    struct disk_para para = {0};
    os_u8 *buffer;
    os_ret result;

    if (OS_NULL != disk_list[device_id]) {
        /* 获取入参 */
        disk = disk_list[device_id]->disk;
        /* 查询 */
        result = disk_list[device_id]->operation->query(disk, &para);
        if (OS_SUCC != result) {
            print("query fail\n");
            return OS_FAIL;
        }
        print("size %d\n", para.bytes_per_sec);
        buffer = kmalloc(num * para.bytes_per_sec + 0x200);
        if (OS_NULL == buffer) {
            print("malloc fail\n");
            return OS_FAIL;
        }
        mem_set(buffer, 0xffffffff, 0x200 + num * para.bytes_per_sec);
        result = disk_list[device_id]->operation->read(disk, start_sector, num, buffer);
        if (OS_SUCC != result) {
            print("read fail\n");
            result = OS_FAIL;
            goto end;
        }
        print("addr %x\n", buffer);
        for (result = 0; result < 0x10; result++) {
            print("%x ", buffer[result]);
        } print("\n");
        result = OS_SUCC;
      end:
        kfree(buffer);
        return result;
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_disk_read(os_u32 argc, os_u8 *argv[])
{
    os_u32 id;
    os_u32 start_sector;
    os_u16 block_num;

    if (3 != argc) {
        return OS_FAIL;
    }

    /* 入参1是数字 */
    id = str2num(argv[0]);
    start_sector = str2num(argv[1]);
    block_num = str2num(argv[2]);
    test_read(id, start_sector, block_num);
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_disk_write(os_u32 argc, os_u8 *argv[])
{
    os_u32 id;
    os_u32 start_sector;
    os_u16 block_num;

    if (3 != argc) {
        return OS_FAIL;
    }

    /* 入参1是数字 */
    id = str2num(argv[0]);
    start_sector = str2num(argv[1]);
    block_num = str2num(argv[2]);
    test_write(id, start_sector, block_num);
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_load_file(os_u32 argc, os_u8 *argv[])
{
    if (1 == argc) {
        return test_load(argv[0]);
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret cmd_edit_file(os_u32 argc, os_u8 *argv[])
{
    if (1 == argc) {
        return test_edit(argv[0]);
    }
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret register_disk(struct disk_device *dev)
{
    os_u32 i;
    os_ret result;

    cassert(OS_NULL != dev);

    /* 填充磁盘几何参数信息 */
    result = dev->operation->query(dev->disk, &dev->para);
    if (OS_SUCC != result) {
        return OS_FAIL;
    }

    /* 分配一个磁盘描述符 */
    write_lock(&disk_list_lock);
    for (i = 0; i < MAX_DISK_NUM; i++) {
        if (OS_NULL == disk_list[i]) {
            disk_list[i] = dev;
            write_unlock(&disk_list_lock);

            /* 识别文件系统 */
            dev->vfs_index = mount_disk_fs(i);
            if (0 == dev->vfs_index) {
                flog("check disk file system fail\n");
            }
            if (i == curr_disk_device_id()) {
                change_working_dir(i, "\\system");
            }
            return OS_SUCC;
        }
    }
    write_unlock(&disk_list_lock);
    return OS_FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret unregister_disk(struct disk_device *dev)
{
    os_uint i;
    os_ret ret;

    cassert(OS_NULL != dev);

    ret = OS_FAIL;
    write_lock(&disk_list_lock);
    for (i = 0; i < MAX_DISK_NUM; i++) {
        if (dev == disk_list[i]) {
            write_unlock(&disk_list_lock);
            unmount_disk_fs(i, dev->vfs_index);
            write_lock(&disk_list_lock);
            disk_list[i] = OS_NULL; /* bugfix, the order */
            ret = OS_SUCC;
            break;
        }
    }
    write_unlock(&disk_list_lock);
    return ret;
}

/***************************************************************
 * description : 抽象功能模块初始化函数
 * history     :
 ***************************************************************/
LOCALC os_void init_disk(os_void)
{
    os_ret result;

    init_rw_lock(&disk_list_lock);

    register_cmd("rd", cmd_disk_read);
    register_cmd("wt", cmd_disk_write);
    register_cmd("load", cmd_load_file);
    register_cmd("edit", cmd_edit_file);

    result = register_dump(&disk_debug);
    cassert(OS_SUCC == result);
}
init_afunc(init_disk);

