/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : image.c
 * version     : 1.0
 * description : abstract
 * author      : gaocheng
 * date        : 2011-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include <disk.h>
#include <image.h>

/***************************************************************
 global variable declare
 ***************************************************************/

/***************************************************************
 * description :
 ***************************************************************/
struct image_register_info {
    struct list_node node;
    const struct image_info *info;
};

LOCALD struct image_register_info image_head = {0};
LOCALD rwlock_t image_info_lock;

/***************************************************************
 function declare
 ***************************************************************/

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void init_image(os_void)
{
    init_rw_lock(&image_info_lock);

    init_list_head(&image_head.node);
    image_head.info = OS_NULL;
}

/***************************************************************
 * description : 注册信息要求是全局的
 * history     :
 ***************************************************************/
os_ret register_image(const struct image_info *image)
{
    struct image_register_info *new;

    cassert(OS_NULL != image);

    new = kmalloc(sizeof(struct image_register_info));
    if (OS_NULL == new) {
        flog("register image fail\n");
        return OS_FAIL;
    }
    new->info = image;
    write_lock(&image_info_lock);
    add_list_head(&image_head.node, &new->node);
    write_unlock(&image_info_lock);
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret unregister_image(const struct image_info *image)
{
    struct image_register_info *img;

    cassert(OS_NULL != image);

    img = list_addr(image, struct image_register_info, info);
    if (OS_NULL == img) {
        return OS_FAIL;
    }
    write_lock(&image_info_lock);
    del_list(&img->node);
    write_unlock(&image_info_lock);
    kfree(img);
    return OS_SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC const struct image_operation *lookup_image_node(os_u8 *ename)
{
    struct list_node *i;
    struct image_register_info *qnode;
    const struct image_operation *result;

    result = OS_NULL;

    read_lock(&image_info_lock);
    loop_list(i, &image_head.node) {
        qnode = list_addr(i, struct image_register_info, node);
        /* compare, 不区分大小写 */
        if (((0 == abs(ename[0] - qnode->info->type[0])) || (('a' - 'A') == abs(ename[0] - qnode->info->type[0])))
         && ((0 == abs(ename[1] - qnode->info->type[1])) || (('a' - 'A') == abs(ename[1] - qnode->info->type[1])))
         && ((0 == abs(ename[2] - qnode->info->type[2])) || (('a' - 'A') == abs(ename[2] - qnode->info->type[2])))) {
            result = qnode->info->operations;
            break;
        }
    }
    read_unlock(&image_info_lock);
    return result;
}

/***************************************************************
 * description : there is an impliced '.' character between the main
 *               part of the name and the extension part of the name.
 * history     :
 ***************************************************************/
LOCALC os_u8 *get_file_ename(IN os_u8 *file_name)
{
    while ('.' != *file_name) {
        file_name++;
    }
    return (os_u8 *) ++file_name;
}

/***************************************************************
 * description : 在窗口中绘制图像
 * history     :
 ***************************************************************/
os_ret OS_API show_image(IN HWINDOW handle, IN os_u8 *file_name, enum show_image_mode_type mode)
{
    HFILE fp;
    const struct image_operation *operation;

    if ((OS_NULL == handle) || (OS_NULL == file_name) || (SI_MODE_BUTT <= mode)) {
        return OS_FAIL;
    }

    /* 获取文件后缀名 */
    operation = lookup_image_node(get_file_ename(file_name));
    if (OS_NULL == operation) {
        flog("not support this type image\n");
        return OS_FAIL;
    }

    fp = open_file(curr_disk_device_id(), file_name);
    if (OS_NULL == fp) {
        flog("open image file fail\n");
        return OS_FAIL;
    }

    operation->show(handle, fp, mode);

    (os_void) close_file(fp);

    return OS_SUCC;
}

