/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : memory.c
 * version     : 1.0
 * description : (key)
 * author      : gaocheng
 * date        : 2012-04-22
 ***************************************************************/

/***************************************************************
 include header file
 ***************************************************************/
#include <typedef.h>
#include <arch.h>
#include <assert.h>

/***************************************************************
 global variable declare
 ***************************************************************/

/***************************************************************
 function declare
 ***************************************************************/
/***************************************************************
 * description :
 * history     :
 ***************************************************************/
uint copy_mem(u8 *dest, const u8 *src, u32 count)
{
    cassert((NULL != dest) && (NULL != src));
    /* overflow: src + count, dest + count */
    cassert(((UINT32_MAX - (pointer) src) >= count) && ((UINT32_MAX - (pointer) dest) >= count));
    cassert((dest >= src + count) || (src >= dest + count));

    while (count-- > 0) {
        *dest++ = *src++;
    }
    return SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
uint move_mem(u8 *dest, const u8 *src, u32 count)
{
    if (count != 0) {
        if (dest <= src) {
            do {
            *dest++ = *src++;
            } while (--count != 0);
        } else {
            src += count;
            dest += count;
            do {
                *--dest = *--src;
            } while (--count != 0);
        }
        return SUCC;
    }
    return FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
uint set_mem(u8 *s, u8 ch, u32 n)
{
    u32 i;

    if (NULL != s) {
        for (i = 0; i < n; i++) {
            *s = ch;
            s++;
        }
        return SUCC;
    }
    return FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
uint cmp_mem(u8 *m1, u8 *m2, u32 n)
{
    u32 i = 0;

    cassert((NULL != m1) && (NULL != m2));

    while (m1[i] == m2[i]) {
        i++;
        if (i == n) {
            return TRUE;
        }
    }
    return FALSE;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
void write_be_u32(u8 *mem, u32 val)
{
    mem[0] = ((u8 *) &val)[3];
    mem[1] = ((u8 *) &val)[2];
    mem[2] = ((u8 *) &val)[1];
    mem[3] = ((u8 *) &val)[0];

}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
void write_be_u16(u8 *mem, u16 val)
{
    mem[0] = ((u8 *) &val)[1];
    mem[1] = ((u8 *) &val)[0];
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
u32 read_be_u32(const u8 *mem)
{
    u32 val;

    ((u8 *) &val)[0] = mem[3];
    ((u8 *) &val)[1] = mem[2];
    ((u8 *) &val)[2] = mem[1];
    ((u8 *) &val)[3] = mem[0];

    return val;
}

/* !!! overwrite default function that could be called by compiler */
void *memcpy(void *dest, const void *src, u32 count)
{
    while (count-- > 0) {
        *(char *) dest++ = *(char *) src++;
    }
    return dest;
}

void *memmove(void *dest, const void *src, u32 count)
{
    char *d, *s;

    d = dest;
    s = (char *) src;
    if (count != 0) {
        if (d <= s) {
            do {
            *d++ = *s++;
            } while (--count != 0);
        } else {
            s += count;
            d += count;
            do {
                *--d = *--s;
            } while (--count != 0);
        }
    }
    return dest;
}

void* memset(void *dest, int c, u32 count)
{
    int i;

    if (NULL != dest) {
        for (i = 0; i < count; i++) {
            *(char *) dest = c;
            dest++;
        }
    }
    return dest;
}

int memcmp(const void *s1, const void *s2, unsigned int n)
{
    const unsigned char *p1 = s1, *p2 = s2;

    if (0 == n) {
        return 0;
    }

    while (*p1 == *p2) {
        p1++;
        p2++;
        n--;
        if (n == 0) {
            return 0;
        }
    }

    return *p1 - *p2;
}

