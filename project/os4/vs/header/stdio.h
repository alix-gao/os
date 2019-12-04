/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : stdio.h
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2012-04-22
 ***************************************************************/

#ifndef __STDIO_H__
#define __STDIO_H__

/***************************************************************
 include header file
 ***************************************************************/

#include <stdarg.h>

#pragma pack(4)

typedef uint (*vprint)(const u8 *format, va_list args);

uint register_print(vprint func);

uint __cdecl print(const u8 *format, ...);

void show_text(u8 *msg, u8 row);

#pragma pack()

#endif /* end of header */

