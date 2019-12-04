/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : string.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2012-04-22
 ***************************************************************/

#ifndef __STRING_H__
#define __STRING_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

s32 compare_string(const u8 *s1, const u8 *s2);
u32 string_length(const u8 *str);
u32 string_max_length(const u8 *str, u32 count);
s32 string_to_num(const u8 *str);
u32 copy_string(u8 *src, const u8 *dst);
u8 *split_string(u8 **str, const u8 *sub);

void kmp_next(u8 *sub, s32 next[]);
s32 kmp(u8 *str, u8 *sub, s32 next[]);

#define str_cpy(dest,src) copy_string(dest,src)
#define str_ncpy(dest,src, n) copy_n_string(dest,src, n)
#define str_cmp(s1,s2) compare_string((u8 *)(s1),(u8 *)(s2))
#define str_len(str) string_length(str)
#define str_m_len(str, cnt) string_max_length(str, cnt)
#define str2num(str) string_to_num(str)
#define str_brk(str, c) split_string(str, c)

#pragma pack()

#endif /* end of header */

