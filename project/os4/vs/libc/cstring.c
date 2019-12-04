/***************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name   : cstring.c
 * version     : 1.0
 * description : (key)
 * author      : gaocheng
 * date        : 2009-04-22
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
 * description : 当s1<s2, 返回值<0
 *               当s1=s2, 返回值=0
 *               当s1>s2, 返回值>0
 * history     :
 ***************************************************************/
s32 compare_string(const u8 *s1, const u8 *s2)
{
    uint i = 0;

    cassert((NULL != s1) && (NULL != s2));

    while (s1[i] == s2[i]) {
        if ('\0' == s1[i]) {
            /* equal */
            return 0;
        }
        i++;
    }
    return s1[i]-s2[i];
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
uint lookup_string(const u8 *str, const u8 *sub, u32 *index)
{
    u32 i, j;

    for (i = 0; '\0' != str[i]; i++) {
        for (j = 0; '\0' != sub[j]; j++) {
            if (sub[j] != str[i + j]) {
                break;
            }
        }
        if ('\0' == sub[j]) {
            *index = i;
            return SUCC;
        }
    }
    return FAIL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
u32 string_length(const u8 *str)
{
    u32 len = 0;

    if (NULL != str) {
        while ('\0' != *str) {
            len++;
            str++;
        }
        return len;
    }
    return 0;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
u32 string_max_length(const u8 *str, u32 count)
{
    const u8 *s;

    for (s = str; ('\0' != *s) && (count--); s++) {
        ; /* nothing */
    }
    return (pointer) s - (pointer) str; /* not u32 */
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
uint copy_string(u8 *dst, const u8 *src)
{
    cassert((NULL != dst) && (NULL != src));

    while ('\0' != (*dst++ = *src++)) {
        ;
    }

    return SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
uint copy_n_string(u8 *dst, const u8 *src, u32 cnt)
{
    cassert((NULL != dst) && (NULL != src));

    while (('\0' != (*dst++ = *src++)) && (0 != cnt)) {
        cnt--;
    }

    return SUCC;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
s32 string_to_num(const u8 *str)
{
    u32 i = 0;
    s32 result = 0;
    s32 win = 0;
    s32 f = 1; //默认为正

    cassert(NULL != str);

    /* 正负 */
    if ('+' == str[i]) {
        i++;
    } else if ('-' == str[i]) {
        i++;
        f = -1;
    }

    /* 十六进制 */
    if (('0' == str[i]) && ('x' == str[i+1])) {
        i += 2;
        while ('\0' != str[i]) {
            /* 参数检查 */
            if (('0' <= str[i]) && ('9' >= str[i])) {
                win = str[i] - '0';
            } else if (('a' <= str[i]) && ('f' >= str[i])) {
                win = str[i] - 'a' + 10;
            } else {
                /* 入参错误 */
                return 0;
            }
            result = result*0x10 + win;
            i++;
        }
        return f * result;
    }

    /* 十进制 */
    while ('\0' != str[i]) {
        /* 参数检查 */
        if (('0' <= str[i]) && ('9' >= str[i])) {
            win = str[i] - '0';
        } else {
            /* 参数错误 */
            return 0;
        }

        result = result*10 + win;

        i++;
    }
    return f*result;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
LOCALC u8 *break_string(const u8 *s, const u8 *accept)
{
    const char *p;
    const char *t;
    const char *a;

    for (p = s; *p != '\0'; ++p) {
        for (a = accept, t = p; *a != '\0'; ++t, ++a) {
            if (*t != *a)
                break;
        }
        if (*a == '\0')
            return (u8 *) t;
    }
    return NULL;
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
u8 *split_string(u8 **str, const u8 *sub)
{
    u8 *sbegin;
    u8 *end;
    u32 len;

    cassert((NULL != str) && (NULL != sub));

    sbegin = *str;
    if (sbegin == NULL) {
        return NULL;
    }

    len = string_length(sub);

    end = break_string(sbegin, sub);
    if (end) {
        *(end - len) = '\0';
        *str = end;
    } else {
        *str = NULL;
    }
    return sbegin;
}

/***************************************************************
 * description : kmp
 * history     :
 ***************************************************************/
void kmp_next(u8 *sub, s32 next[])
{
    uint i;
    s32 j;
    u32 len;

    cassert(NULL != sub);

    next[0] = -1;

    len = string_length(sub);
    j = -1;
    for (i = 1; i < len; i++) {
        while ((j > -1) && sub[i] != sub[j + 1]) {
            j = next[j - 1];
        }
        if (sub[j + 1] == sub[i]) {
            j++;
        }
        next[i] = j;
    }
}

/***************************************************************
 * description :
 * history     :
 ***************************************************************/
s32 kmp(u8 *str, u8 *sub, s32 next[])
{
    u32 l1,l2;
    u32 i;
    s32 j;

    cassert((NULL != str) && (NULL != sub) && (NULL != next));

    l1 = string_length(str);
    l2 = string_length(sub);

    j = -1;
    for (i = 0; i < l1; i++) {
        while ((j > -1) && (str[i] != sub[j + 1])) {
            j = next[j];
        }
        if (str[i] == sub[j + 1]) {
            j++;
        }
        if (j == (l2 - 1)) {
            return i - l2 + 1;
        }
    }
    return -1;
}

/* !!! overwrite default function that could be called by compiler */
char *strcpy(char *dest, const char *src)
{
    char *tmp = dest;

    while ((*dest++ = *src++) != '\0')
        ; /* nothing */
    return tmp;
}

char *strncpy(char *dest, char *src, unsigned int n)
{
    char *tmp = dest;

    while (n) {
        if ((*tmp = *src) != 0)
            src++;
        tmp++;
        n--;
    }
    return dest;
}

unsigned int strlen(char *s)
{
    const char *sc;

    for (sc = s; *sc != '\0'; ++sc)
        ; /* nothing */
    return sc - s;
}

unsigned int strnlen(const char *s, unsigned int count)
{
    const char *sc;

    for (sc = s; count-- && *sc != '\0'; ++sc)
        ; /* nothing */
    return sc - s;
}

int strncmp(const char *cs, const char *ct, unsigned int count)
{
    unsigned char c1, c2;

    while (count) {
        c1 = *cs++;
        c2 = *ct++;
        if (c1 != c2)
            return c1 < c2 ? -1 : 1;
        if (!c1)
            break;
        count--;
    }
    return 0;
}

int strcmp(const char *cs, const char *ct)
{
    unsigned char c1, c2;

    while (1) {
        c1 = *cs++;
        c2 = *ct++;
        if (c1 != c2)
            return c1 < c2 ? -1 : 1;
        if (!c1)
            break;
    }
    return 0;
}

char *strcat(char *dest, const char *src)
{
    char *tmp = dest;

    while (*dest)
        dest++;
    while ((*dest++ = *src++) != '\0')
        ;
    return tmp;
}

char *strncat(char *dest, const char *src, unsigned int count)
{
    char *tmp = dest;

    if (count) {
        while (*dest)
            dest++;
        while ((*dest++ = *src++) != 0) {
            if (--count == 0) {
                *dest = '\0';
                break;
            }
        }
    }
    return tmp;
}

