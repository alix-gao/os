/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : assert.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2011-04-22
 ***************************************************************/

#ifndef __ASSERT_H__
#define __ASSERT_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

#ifndef __ARCH_H__
    #error "include arch.h before"
#endif

/***************************************************************
 macro define
 ***************************************************************/

#undef cassert
#ifdef ____DEBUG_VERSION____
uint __cdecl print(const u8 *format, ...);
void dump_stack(uint __cdecl (*show)(const u8 *format, ...));

#define cassert(condition) \
    do { \
        if (condition) { \
        } else { \
            print("assert!!! file:%s, function:%s, line:%d\n", __FILE__, __FUNCTION__, __LINE__); \
            dump_stack(print); \
            sti(); \
            while (1) hlt(); \
        } \
    } while (0)

#define cassert_word(condition, fmt, arg...) \
    do { \
        if (condition) { \
        } else { \
            print("assert!!! file:%s, function:%s, line:%d\n", __FILE__, __FUNCTION__, __LINE__); \
            print(fmt, ##arg); \
            dump_stack(print); \
            sti(); \
            while (1) hlt(); \
        } \
    } while (0)
#else
#define cassert(condition)
#define cassert_word(condition, fmt, arg...)
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

#endif /* end of header */

