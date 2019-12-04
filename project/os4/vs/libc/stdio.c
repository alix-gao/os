/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : stdio.c
 * version     : 1.0
 * description :
 * author      : gaocheng
 * date        : 2012-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <typedef.h>
#include <arch.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>

/***************************************************************
 global variable declare
 ***************************************************************/
LOCALD volatile vprint pvprint _CPU_ALIGNED_ = NULL;

/***************************************************************
 function declare
 ***************************************************************/

/***************************************************************
 * description : show message in text mode
 * history     :
 ***************************************************************/
void show_text(u8 *msg, u8 row)
{
    u16 *vram;

    vram = (u16 *)(0x0b8000 + (row * 80) * 2);
    while ('\0' != *msg) {
        *vram++ = *msg++ | 0x0f00;
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
uint register_print(vprint func)
{
    pvprint = func;
    return SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
uint __cdecl print(const u8 *format, ...)
{
    va_list arg_list;
    lock_t eflag;

    cassert(NULL != format);

    lock_int(eflag);
    va_start(arg_list, format);
    if (pvprint) {
        pvprint(format, arg_list);
    }
    va_end(arg_list);
    unlock_int(eflag);

    return SUCC;
}

static const u8 hex_template[17] = {'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
static u32 parse_hex(u8 *buff, u32 len, u32 integer, u8 align)
{
    u32 si;
    u32 pos;
    u32 count;

    count = 0;
    pos = 0;
    if (len) {
        for ((0 == align)?(si = 28):(si = 4 * (align - 1)); ; si -= 4) {
            pos = integer >> si;
            pos = pos & 0x0f;
            *buff++ = hex_template[pos];
            if (len <= ++count) {
                return len;
            }
            if (0 == si) {
                break;
            }
        }
    }
    return count;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
u32 snprint(u8 *buff, u32 len, const u8 *format, va_list args)
{
    u32 integer;
    u32 count;
#define ALIGN_SIZE 4
    u8 n[ALIGN_SIZE];
    u8 align;

    align = 0;
    count = 0;
    while (*format) {
        switch (*format++) {
        default:
            if (len <= count++) {
                return 0;
            }
            *buff++ = *(format - 1);
            break;
        case '\n': /* for windows */
            if (len <= count++) {
                return 0;
            }
            *buff++ = '\r';
            if (len <= count++) {
                return 0;
            }
            *buff++ = '\n';
            break;
        case '%':
            switch (*format) {
            case 's':
                do {
                    u8 *string;
                    string = (u8 *) va_arg(args, u32);
                    while (*string) {
                        if (len <= count++) {
                            return 0;
                        }
                        *buff++ = *string++;
                    }
                } while (0);
                break;
            case 'c':
                if (len <= count++) {
                    return 0;
                }
                *buff++ = va_arg(args, u32);
                break;
            case 'd':
                do {
                    u32 ui;
                    u32 temp;
                    u32 flag = 1;
                    integer = va_arg(args, u32);
                    for (ui = 1000000000; ui > 1; ui = ui / 10) {
                        temp = integer / ui;
                        if ((0 != temp) || (0 == flag)) {
                            flag = 0;
                            integer -= (temp * ui);
                            if (len <= count++) {
                                return 0;
                            }
                            *buff++ = '0' + temp;
                        }
                    }
                    if (len <= count++) {
                        return 0;
                    }
                    *buff++ = '0' + integer;
                } while (0);
                break;
            case 'x':
                 align = parse_hex(buff, len - count, va_arg(args, u32), 0);
                 count += align;
                 buff += align;
                 break;
            default:
                // e.g., %nx
                // 'align' is clear always
                for (align = 0; (align < (ALIGN_SIZE - 1)) && (*format >= '0') && (*format <= '9'); align++, format++) {
                    n[align] = *format;
                }
                if ((0 == align) || (align >= (ALIGN_SIZE - 1)) || ('x' != *format)) {
                    *buff++ = '%';
                    count++;
                    format -= (align + 1);
                    break;
                }
                n[align] = '\0';
                align = str2num(n);
                align = parse_hex(buff, len - count, va_arg(args, u32), align);
                count += align;
                buff += align;
                break;
            }
            format++;
            break;
        }
    }
    return count;
}

/* !!! overwrite default function that could be called by compiler */
int snprintf(char *buffer, u32 count, const char *format, ...)
{
    va_list args;
    int i;

    va_start(args, format);
    i = snprint(buffer, count, format, args);
    va_end(args);
    return i;
}

int printf(const char *format, ...)
{
    va_list args;

    va_start(args, format);
    print("printf: ");
    if (pvprint) {
        pvprint(format, args);
    }
    va_end(args);
    return 0;
}

