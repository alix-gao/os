/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : harddisk.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2011-01-01
 ***************************************************************/

#ifndef __HARDDISK_H__
#define __HARDDISK_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/
#define SYS_HARDDISK_INT_VECTOR 0xe

#define HD_PORT_DATA        0x1f0
#define HD_PORT_ERROR       0x1f1
#define HD_PORT_SECT_COUNT  0x1f2
#define HD_PORT_SECT_NUM    0x1f3
#define HD_PORT_CYL_LOW     0x1f4
#define HD_PORT_CYL_HIGH    0x1f5
#define HD_PORT_DRV_HEAD    0x1f6
#define HD_PORT_STATUS      0x1f7
#define HD_PORT_COMMAND     0x1f7
#define HD_READ_COMMAND     0x20
#define HD_WRITE_COMMAND    0x30

/***************************************************************
 enum define
 ***************************************************************/

/***************************************************************
 struct define
 ***************************************************************/

/***************************************************************
 extern function
 ***************************************************************/

#pragma pack()

#endif /* end of header */

