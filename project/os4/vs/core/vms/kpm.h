/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : kpm.h
 * version     : 1.0
 * description : kernel page memory
 * author      : gaocheng
 * date        : 2011-04-22
 ***************************************************************/

#ifndef __KPM_H__
#define __KPM_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/
#ifdef _X86_
#define KPM_ALIGN (PAGE_ALIGN)
#else
#error "cpu cache alian need to be reset!!!"
#endif

/***************************************************************
 enum define
 ***************************************************************/

/***************************************************************
 struct define
 ***************************************************************/

/***************************************************************
 extern function
 ***************************************************************/
os_void OS_API *alloc_coherent_mem(os_u32 size, os_u32 align, os_u32 line);
os_void OS_API *free_coherent_mem(os_void **addr, os_u32 line);

#pragma pack()

#endif /* end of header */

