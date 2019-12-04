
#include <driver.h>
#include <cmos.h>
#include <os_type.h>
#include <graph_disp.h>

void drive_test()
{
     /* test int */
     os_uint8 reg = 0;
     //__asm__ __volatile__("int $0x28");

     rtc_read_b(reg_a, reg);
     print_int_x_no_pos(reg);

     rtc_read_b(reg_b, reg);
     print_int_x_no_pos(reg);

     rtc_read_b(reg_c, reg);
     print_int_x_no_pos(reg);
     rtc_read_b(reg_c, reg);
     print_int_x_no_pos(reg);

     rtc_read_b(reg_d, reg);
     print_int_x_no_pos(reg);
}

void driver_init()
{
     RTC_init();
     init_pci();
     drive_test();
}

