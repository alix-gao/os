/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : kdm.h
 * version     : 1.0
 * description : kernel dynamic memory
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __KDM_H__
#define __KDM_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/
#define KDMB_COUNT 0x10

/***************************************************************
 enum define
 ***************************************************************/
/***************************************************************
 * enum name   : kdm_status_enum
 * description :
 ***************************************************************/
enum kdm_status {
    KDM_STATUS_IDLE,
    KDM_STATUS_BUSY,
    KDM_STATUS_BUTT
};

/***************************************************************
 struct define
 ***************************************************************/
/***************************************************************
 * description : 内核动态内存配置信息
 ***************************************************************/
struct kdm_cfg_item {
    os_u32 size;
    os_u32 count;
};

/***************************************************************
 * description :
 ***************************************************************/
struct kdmcb {
    os_u8 *addr;
    os_u32 align;
    /* 回收的位置 */
    os_u32 kdmcb_info_index;
    enum kdm_status status;
    /* 分配代码行号 */
    const os_u8 *file;
    os_u32 line;
    /* 此处使用单链表, 不使用循环双向链表 */
    struct kdmcb *next;
};

/***************************************************************
 * description :
 ***************************************************************/
struct kdmcb_info {
    os_u32 size;
    os_u32 idle_num;
    struct kdmcb *idle_block;
};

/***************************************************************
 * description : 内核动态内存块头
 ***************************************************************/
struct kdm_head {
    /* 回指内存控制块 */
    struct kdmcb *kdmcb;
    /* 校验位 */
#define KDM_CRC UINT32_C(0xdddddddd)
    os_u32 check;
};

/***************************************************************
 extern function
 ***************************************************************/

#pragma pack()

#endif /* end of header */

