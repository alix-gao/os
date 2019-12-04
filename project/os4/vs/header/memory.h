/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : memory.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2012-04-22
 ***************************************************************/

#ifndef __MEMORY_H__
#define __MEMORY_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

#define exchange(a,b) \
    do { \
        (a)=(a)^(b); \
        (b)=(a)^(b); \
        (a)=(a)^(b); \
    } while (0)

void write_be_u32(u8 *mem, u32 val);
void write_be_u16(u8 *mem, u16 val);
u32 read_be_u32(const u8 *mem);

uint copy_mem(u8 *dest, const u8 *src, u32 count);
uint move_mem(u8 *dest, const u8 *src, u32 count);
uint set_mem(u8 *s, u8 ch, u32 n);
#define mem_cpy(dest,src,count) copy_mem((u8 *)(dest),(u8 *)(src),(u32)(count))
#define mem_mov(dest,src,count) move_mem((u8 *)(dest),(u8 *)(src),(u32)(count))
#define mem_set(s,ch,n) set_mem((u8 *)(s),(u8)(ch),(u32)(n))
#define mem_cmp(m1,m2,n) cmp_mem((u8 *)(m1),(u8 *)(m2),(u32)(n))

#define array_size(a) (sizeof(a) / sizeof(a[0]))

#pragma pack()

#endif /* end of header */

