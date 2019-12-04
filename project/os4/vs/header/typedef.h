/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : typedef.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __LIB_TYPE_H__
#define __LIB_TYPE_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/
/* according to C99 */
//#define NULL 0

#ifndef NULL
#define NULL ((void *) 0)
#endif

typedef unsigned char u8;
typedef signed char s8;
typedef unsigned short int u16;
typedef signed short int s16;
typedef unsigned long int u32;
typedef signed long int s32;
typedef unsigned long long int u64;
typedef signed long long int s64;
typedef unsigned int uint;
typedef signed int sint;

#ifndef __DEFINE_POINTER__
#define __DEFINE_POINTER__
/* pointer maybe negtive, e.g., mips. so use unsigned */
typedef unsigned long int pointer;
#endif

#ifndef __CONST_LIMIT__
#define __CONST_LIMIT__

#define SINT32_C(c) (c##L)
#define UINT32_C(c) (c##UL)
#define SINT64_C(c) (c##LL)
#define UINT64_C(c) (c##ULL)

#define UINT8_MAX 0xff
#define UINT16_MAX 0xffff
#define UINT32_MAX 0xffffffff
#define UINT64_MAX 0xffffffffffffffff

#define u32_msb(a) ((a) & UINT32_C(1 << 31))

#endif

/* cancel c's overload */
#ifndef __C_OVERLOAD__
#define __C_OVERLOAD__

/* 局部函数定义, local code */
#define LOCALC static

/* 全局函数定义, global code */
#define GLOBALC extern

/* 局部变量定义, local data */
#define LOCALD static

/* 全局变量定义, global data */
#define GLOBALD

/* 函数内全局变量定义, global data in function */
#define GLOBALDIF static

/* 全局变量声明, global reference data */
#define GLOBALREFD extern

/* 全局函数声明, global reference code */
#define GLOBALREFC extern

#endif

/* parameter */
#ifndef __PARA_ATTR__
#define __PARA_ATTR__

#define IN const
#define OUT
#define INOUT

#endif

#ifndef SUCC
#define SUCC 0
#endif

#ifndef FAIL
#define FAIL 1
#endif

/* BOOL */
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef BOOL
typedef uint BOOL;
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

#pragma pack()

#endif

