/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : cmd.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __CMD_H__
#define __CMD_H__

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
 extern function
 ***************************************************************/
os_void start_trace(os_void);
os_void trace_str_info(const os_u8 *str);
os_void trace_hex_info(const os_u32 hex);
os_void stop_trace(os_void);
os_bool trace_is_ok(void);

#pragma pack()

#endif /* end of header */
