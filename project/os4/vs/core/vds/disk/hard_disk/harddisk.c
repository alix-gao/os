/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : harddisk.c
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2011-01-01
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <core.h>
#include "harddisk.h"

/***************************************************************
 global variable declare
 ***************************************************************/
/* 硬盘操作信号量 */
//LOCALD HEVENT hd_disk = OS_NULL;

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_ret read_harddisk(os_u8 head, os_u16 cylinder, os_u8 sector, os_u32 sector_num, os_u8 *addr)
{
    os_u32 i;
    os_u8 tmp;

#define READ_HARDDISK_TIMES 9

    if (OS_NULL != addr) {
#if 1
        for (i = 0; i < READ_HARDDISK_TIMES; i++) {
            inb_p(HD_PORT_STATUS, tmp);
            if (0x40 != (tmp & 0xc0)) {
                delay_task(1, __LINE__);
            }
        }
        if (READ_HARDDISK_TIMES == i) {
            flog("wait harddisk timeout.\n");
            return OS_FAIL;
        }
#endif

        outb_p(HD_PORT_DRV_HEAD, 0xa0 | head);
        outb_p(HD_PORT_SECT_COUNT, sector_num);
        outb_p(HD_PORT_SECT_NUM, sector);
        outb_p(HD_PORT_CYL_LOW, (os_u8) cylinder);
        outb_p(HD_PORT_CYL_HIGH, (os_u8)(cylinder >> 8));
        outb_p(HD_PORT_COMMAND, HD_READ_COMMAND);

        for (i = 0; i < READ_HARDDISK_TIMES; i++) {
            inb_p(HD_PORT_STATUS, tmp);
            if (!(tmp & 0x8)) {
                delay_task(1, __LINE__);
            }
        }
        if (READ_HARDDISK_TIMES == i) {
            flog("read harddisk timeout.\n");
            return OS_FAIL;
        }

        insl(HD_PORT_DATA, addr, sector_num<<7);

        return OS_SUCC;
    }

    return OS_FAIL;
}

#if 0
/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC os_ret write_harddisk(os_u32 cylinder, os_u32 head, os_u32 sector, os_u32 sector_num, os_u32 *addr)
{
    os_u8 tmp;

    if (OS_NULL != addr) {
        do {
            inb(HD_PORT_STATUS, tmp);
        } while (0x40 != (tmp & 0xc0));

        outb(HD_PORT_SECT_COUNT, sector_num);
        outb(HD_PORT_SECT_NUM, sector);
        outb(HD_PORT_CYL_LOW, cylinder);
        outb(HD_PORT_CYL_HIGH, cylinder >> 8);
        outb(HD_PORT_DRV_HEAD, 0xa0 | head);
        outb(HD_PORT_COMMAND, HD_WRITE_COMMAND);

        do {
            inb(HD_PORT_STATUS, tmp);
        } while (!(tmp & 0x8));

        outsl(addr, sector_num<<7, HD_PORT_DATA);

        return OS_SUCC;
    }

    return OS_FAIL;
}
#endif

/***************************************************************
 * description : 硬盘中断响应, IRQ_FUNCPTR
 * history     :
 ***************************************************************/
LOCALC os_void IRQ_FUNC handle_harddisk_int(os_u32 irq)
{
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
os_void init_harddisk_int(os_void)
{
    /* 初始化硬盘中断 */
    install_int(SYS_HARDDISK_INT_VECTOR, handle_harddisk_int, OS_NULL);
}

