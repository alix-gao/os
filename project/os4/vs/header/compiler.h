/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : compiler.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2013-04-22
 ***************************************************************/

#ifndef __COMPILER_H__
#define __COMPILER_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/

#define _aligned_(x) __attribute__((aligned(x)))

/* gcc asm string */
#define _str(x) #x
#define asm_str(x) _str(x)

/* This intrinsic does not generate code, it creates a barrier across which
   the compiler will not schedule data access instructions. */
#define barrier() __asm__ __volatile__("": : :"memory")

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

