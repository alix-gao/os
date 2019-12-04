/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : math.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2012-04-22
 ***************************************************************/

#ifndef __MATH_H__
#define __MATH_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/
/* 无符号除法 "=a"(quotient),"=d"(remainder) */
#define divl(i,j,quotient,remainder) __asm__ __volatile__("movl $0,%%edx\n\t"\
                                                          "movl %0,%%eax\n\t"\
                                                          "divl %1\n\t"\
                                                          "movl %%eax,%2\n\t"\
                                                          "movl %%edx,%3\n\t"\
                                                          :\
                                                          :"m"(i),"m"(j),"m"(quotient),"m"(remainder)\
                                                          :"eax","edx")

/* 上取整, no assert */
static inline u32 divl_cell(u32 a, u32 b)
{
    u32 quotient, remainder;

    divl(a, b, quotient, remainder);
    if (0 != remainder) {
        quotient++;
    }
    return quotient;
}

/* 下取整 */
static inline u32 divl_floor(u32 a, u32 b)
{
    u32 quotient, remainder;

    divl(a, b, quotient, remainder);
    return quotient;
}

/* 绝对值 */
#define abs(x) (((s32)(x) > 0)?(x):(-(x)))

static inline u32 abs_diff(u32 a, u32 b)
{
    return (a > b) ? (a - b) : (b - a);
}

/* 结构体偏移量 */
#define struct_offset(stru, item) ((pointer) &((stru *) 0)->item)

#define min(a,b) (((a)<(b))?(a):(b))

#define max(a,b) (((a)>(b))?(a):(b))

u32 power_of_2(u32 num);

/* add overflow */
static inline BOOL add_u32_overflow(u32 a, u32 b)
{
    if ((UINT32_MAX - a) < b) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static inline BOOL add_u16_overflow(u16 a, u16 b)
{
    if ((UINT16_MAX - a) < b) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static inline BOOL add_u8_overflow(u8 a, u8 b)
{
    if ((UINT8_MAX - a) < b) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/* multipule overflow */
static inline BOOL mul_u32_overflow(u32 a, u32 b)
{
    if ((0 != u32_msb(a)) && (0 != u32_msb(b))) {
        return TRUE;
    } else {
        return FALSE;
    }
}

#pragma pack()

#endif /* end of header */

