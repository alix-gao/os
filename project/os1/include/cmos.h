
#ifndef _cmos_h
#define _cmos_h

#include <io.h>

#define cmos_read(addr,value) ({outb_p(0x70,addr);inb_p(0x71,value);})
//定义宏。将BCD码转换成二进制数值。BCD码利用半个字节（4比特）表示一个10进制数，
//因此一个字节表示2个10进制数。
#define bcd_to_bin(val) ((val)=((val)&15)+((val)>>4)*10)

/* rtc operation */

#define seconds_place       0x0
#define seconds_alarm_place 0x1
#define minutes_place       0x2
#define minutes_alarm_place 0x3
#define hours_place         0x4
#define hours_alarm_place   0x5
#define day_of_week_place   0x6
#define date_of_month_place 0x7
#define month_place         0x8
#define year_place          0x9
#define reg_a               0xa
#define reg_b               0xb
#define reg_c               0xc
#define reg_d               0xd

#define rtc_port(x) (0x70+(x))

#define rtc_read_b(addr, val) ({outb_p(rtc_port(0), (addr));\
                                __asm__("nop");\
                                inb_p(rtc_port(1), (val));})

#define rtc_write_b(addr, val) ({outb_p(rtc_port(0), (addr));\
                                 __asm__("nop");\
                                 outb_p(rtc_port(1), (val));})

#endif

