/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : rtc.h
 * version     : 1.0
 * description : real time clock
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

#ifndef __RTC_H__
#define __RTC_H__

/***************************************************************
 include header file
 ***************************************************************/

#pragma pack(4)

/***************************************************************
 macro define
 ***************************************************************/
/* 实时时钟中断向量号 */
#define RTC_INT_VECTOR 8

/* rtc operation */
#define RTC_REG_A 0xa
#define RTC_REG_B 0xb
#define RTC_REG_C 0xc
#define RTC_REG_D 0xd

#define rtc_port(x) (0x70+(x))

#define rtc_read_b(addr,val) ({outb_p(rtc_port(0),(addr));\
                               __asm__("nop");\
                               inb_p(rtc_port(1),(val));})

#define rtc_write_b(addr,val) ({outb_p(rtc_port(0),(addr));\
                                __asm__("nop");\
                                outb_p(rtc_port(1),(val));})

#define TIMER_MAX_NUM 0x20

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

