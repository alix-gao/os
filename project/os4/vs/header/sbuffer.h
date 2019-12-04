/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : sbuffer.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2012-04-22
 ***************************************************************/

#ifndef __SBUFFER_H__
#define __SBUFFER_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

typedef void *(*ALLOC_FUNCPTR)(u32 len);
typedef void *(*FREE_FUNCPTR)(void *addr);
typedef void *SBUFFER_HANDLE;

SBUFFER_HANDLE create_sbuffer(u32 sbuffer_item_num, u32 sbuffer_item_len, ALLOC_FUNCPTR alloc, FREE_FUNCPTR free);
uint destroy_sbuffer(IN SBUFFER_HANDLE handle);
BOOL sbuffer_empty(IN SBUFFER_HANDLE handle);
BOOL sbuffer_full(IN SBUFFER_HANDLE handle);
uint push_sbuffer(IN SBUFFER_HANDLE handle, IN void *item);
uint pop_sbuffer(IN SBUFFER_HANDLE handle, OUT void *item);

#pragma pack()

#endif /* end of header */

