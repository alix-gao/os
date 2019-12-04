/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : version.c
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2009-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <lib.h>
#include <initial.h>

/***************************************************************
 global variable declare
 ***************************************************************/
LOCALD u8 os_version[] BDATA = { "4.0" };

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description :
 * history     :
 ***************************************************************/
void STARTUP version(void)
{
    flog("version:%s, data:%s, time:%s\n", os_version, __DATE__, __TIME__);
}

int __cdecl callee(int a, int b)
{
    int c;
    c = a + b;
    return c;
}

void caller(void)
{
    int r;
    r = callee(1, 2);
    r = callee(3, 4);
}

