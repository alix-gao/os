
#include <kernel_api.h>
#include <cmos.h>
#include <os_type.h>
#include <graph_disp.h>
#include <8259a.h>

extern void RTC_int();

void open_8259a_for_rtc()
{
     os_uint8 reg = 0;

     /* master */
     inb_p(0x21, reg);
     reg = reg & 0xfb;
     outb_p(0x21, reg);
     print_int_x_no_pos(reg);

     /* slave */
     inb_p(0xa1, reg);
     reg = reg & 0xfe;
     outb_p(0xa1, reg);
     print_int_x_no_pos(reg);
}

void RTC_init()
{
     /* init chip reg */
     os_uint8 reg = 0;

     __asm__("cli");
     /* reg b. */
     //we set bit 7-0 = 0.
     rtc_read_b(reg_b, reg);
     print_int_x_no_pos(reg);
     /* result is 0x2 */

     /* close all int */
     reg = 0xc2;
     rtc_write_b(reg_b, reg);
     print_int_x_no_pos(reg);

     /* we can reset time, day, month, year. here. */

     /* reg a, we not set. */
     //divider-control bits = 010.
     //Rate Selection bits = 0110.
     rtc_read_b(reg_a, reg);
     reg = 0xa6;
     /* uip is read only. */
     rtc_write_b(reg_a, reg);

     //print_int_x_no_pos((int)reg);
     /* result is 0xa6 */

     /* reg c. clear int flag. */
     rtc_read_b(reg_c, reg);
     print_int_x_no_pos(reg);

     /* reg d. clear valid ram and time, start work. */
     rtc_read_b(reg_d, reg);
     print_int_x_no_pos(reg);

     /* install int response. */
     os_int_install(RTC_int, 0x28);

     /* open 8259a 2 */
     open_8259a_for_rtc();

     outb(0xa0,0x20);
     outb(0x20,0x20);

     /* open int */
     reg = 0x42;
     rtc_write_b(reg_b, reg);

     __asm__("sti");
}

void f_RTC_int()
{
     os_uint8 reg = 0;

     /* here print will bring vmware stack overflow in vmware. */
     //print_int_x_no_pos(0xffff);

     rtc_read_b(reg_c, reg);

     send_slave_8259a_eoi();
     //outb(0xa0,0x20);
     //outb(0x20,0x20);
}

