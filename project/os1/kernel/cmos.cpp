
#include<io.h>
#include <cmos.h>

int get_cmos_second()
{
    int sec;
    cmos_read(0,sec);
    bcd_to_bin(sec);
    return sec;
}

