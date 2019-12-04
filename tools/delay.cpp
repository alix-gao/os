/****************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name  : io_delay.cpp
 * version    : 1.0
 * description:
 * author     : gaocheng
 * date       : 2009-04-22
 ****************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

int main(int argc,char *argv[])
{
    Sleep(100);
    if (1 == argc) {
        system("pause");
    }
}

