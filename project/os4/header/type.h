/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : type.h
 * version     : 1.0
 * description : ������̬����
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

/* �ֲ���������, local code */
#define LOCALC static

/* ȫ�ֺ�������, global code */
#define GLOBALC extern

/* �ֲ���������, local data */
#define LOCALD static

/* ȫ�ֱ�������, global data */
#define GLOBALD

/* ������ȫ�ֱ�������, global data in function */
#define GLOBALDIF static

/* ȫ�ֱ�������, global reference data */
#define GLOBALREFD extern

/* ȫ�ֺ�������, global reference code */
#define GLOBALREFC extern

#endif

/* parameter */
#ifndef __PARA_ATTR__
#define __PARA_ATTR__

#define IN const
#define OUT
#define INOUT

#endif

/* ����ϵͳAPI���� */
#define OS_API __stdcall

#define OS_CALLBACK __stdcall

#define IRQ_FUNC __cdecl

/* ����ϵͳ��� */
typedef void *HANDLE;

/* �߳̾�� */
typedef void *HTASK;

/* ���ھ�� */
typedef void *HWINDOW;

/* �豸�����ľ�� */
typedef void *HDEVICE;

/* ��ʱ����� */
typedef void *HTIMER;

/* �ź������� */
typedef void *HEVENT;

/* �ļ���� */
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

