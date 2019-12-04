/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : type.h
 * version     : 1.0
 * description : 基本型态定义
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __TYPE_H__
#define __TYPE_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/
/* according to C99 */
//#define NULL 0

#define OS_NULL ((void *) 0)

typedef void os_void;

typedef unsigned char os_u8;
typedef signed char os_s8;
typedef unsigned short int os_u16;
typedef signed short int os_s16;
typedef unsigned long int os_u32;
typedef signed long int os_s32;
typedef unsigned long long int os_u64;
typedef signed long long int os_s64;
typedef unsigned int os_uint;
typedef signed int os_sint;

#ifndef __DEFINE_POINTER__
#define __DEFINE_POINTER__
/* pointer maybe negtive, e.g., mips. so use unsigned. */
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

/* 操作系统API类型 */
#define OS_API __stdcall

#define OS_CALLBACK __stdcall

#define IRQ_FUNC __cdecl

/* 操作系统句柄 */
typedef void *HANDLE;

/* 线程句柄 */
typedef void *HTASK;

/* 窗口句柄 */
typedef void *HWINDOW;

/* 设备上下文句柄 */
typedef void *HDEVICE;

/* 定时器句柄 */
typedef void *HTIMER;

/* 信号量类型 */
typedef void *HEVENT;

/* 文件句柄 */
typedef void *HFILE;
typedef void *HDIR;

/* return or result */
enum {
    OS_SUCC,
    OS_FAIL,
    OS_TIMEOUT
};
typedef os_uint os_ret;

/* BOOL */
enum {
    OS_FALSE = 0,
    OS_TRUE = 1
};
typedef os_uint os_bool;

#define BIT(n) (1UL << (n))

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

