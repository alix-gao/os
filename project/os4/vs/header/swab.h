/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : swab.h
 * version     : 1.0
 * description : �����ֽ�
 * author      : gaocheng
 * date        : 2013-04-22
 ***************************************************************/

#ifndef __SWAB_H__
#define __SWAB_H__

#define ___constant_swab16(x) ((u16)( \
    (((u16)(x) & (u16)0x00ffU) << 8) | \
    (((u16)(x) & (u16)0xff00U) >> 8)))

#define ___constant_swab32(x) ((u32)( \
    (((u32)(x) & (u32)0x000000ffUL) << 24) | \
    (((u32)(x) & (u32)0x0000ff00UL) <<  8) | \
    (((u32)(x) & (u32)0x00ff0000UL) >>  8) | \
    (((u32)(x) & (u32)0xff000000UL) >> 24)))

#define ___constant_swab64(x) ((u64)( \
    (((u64)(x) & (u64)0x00000000000000ffULL) << 56) | \
    (((u64)(x) & (u64)0x000000000000ff00ULL) << 40) | \
    (((u64)(x) & (u64)0x0000000000ff0000ULL) << 24) | \
    (((u64)(x) & (u64)0x00000000ff000000ULL) <<  8) | \
    (((u64)(x) & (u64)0x000000ff00000000ULL) >>  8) | \
    (((u64)(x) & (u64)0x0000ff0000000000ULL) >> 24) | \
    (((u64)(x) & (u64)0x00ff000000000000ULL) >> 40) | \
    (((u64)(x) & (u64)0xff00000000000000ULL) >> 56)))

#endif

