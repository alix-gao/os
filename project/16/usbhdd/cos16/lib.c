
#include "typedef.h"
#include "lib.h"

static const BYTE hex_template[] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };

VOID print_char(BYTE c, BYTE color, WORD row, WORD col)
{
    WORD position;

    position = (80 * row + col) * 2;

    do {
        asm push ax
        asm push es
        asm push di

        asm mov ax,0x0b800
        asm mov es,ax
        asm mov di,position
        asm mov ah,color
        asm mov al,c
        asm mov es:[di],ax

        asm pop di
        asm pop es
        asm pop ax
    } while (0);
}

VOID print_string(const BYTE *str, BYTE color, WORD row, WORD col)
{
    while ('\0' != *str) {
        print_char(*str, color, row, col++);
        str++;
    }
}

VOID print_hex(BYTE value, BYTE color, WORD row, WORD col)
{
    print_char(hex_template[value >> 04], color, row, col + 0);
    print_char(hex_template[value & 0xf], color, row, col + 1);
}

VOID print_32bit_hex(DWORD value, BYTE color, WORD row, WORD col)
{
    print_char(hex_template[high_8bit(high_16bit(value)) >> 04], color, row, col++);
    print_char(hex_template[high_8bit(high_16bit(value)) & 0xf], color, row, col++);

    print_char(hex_template[low_8bit(high_16bit(value)) >> 04], color, row, col++);
    print_char(hex_template[low_8bit(high_16bit(value)) & 0xf], color, row, col++);

    print_char(hex_template[high_8bit(low_16bit(value)) >> 04], color, row, col++);
    print_char(hex_template[high_8bit(low_16bit(value)) & 0xf], color, row, col++);

    print_char(hex_template[low_8bit(low_16bit(value)) >> 04], color, row, col++);
    print_char(hex_template[low_8bit(low_16bit(value)) & 0xf], color, row, col++);
}

VOID cpy_mem(BYTE *dest, const BYTE *src, WORD count)
{
    while (count) {
        --count;

        *dest = *src;
        ++dest;
        ++src;
    }
}

VOID set_32bit_value(DWORD *dest, DWORD src)
{
    cpy_mem((BYTE *) dest, (BYTE *) &src, sizeof(DWORD));
}

INT tc_32bit_cmp(DWORD a, DWORD b)
{
    if (high_16bit(a) > high_16bit(b)) {
        return 1;
    } else if (high_16bit(a) < high_16bit(b)) {
        return -1;
    } else {
        if (low_16bit(a) > low_16bit(b)) {
            return 1;
        } else if (low_16bit(a) < low_16bit(b)) {
            return -1;
        } else {
            return 0;
        }
    }
}

INT cmp_str(const BYTE *s1, const BYTE *s2)
{
    WORD i = 0;

    while (s1[i] == s2[i]) {
        if ('\0' == s1[i]) {
            /* equal */
            return 0;
        }
        i++;
    }
    return s1[i] - s2[i];
}

/* turbo c, DWORD + DWORD is error.
   DWORD + WORD is ok. */
VOID tc_32bit_add(DWORD *result, DWORD a, DWORD b)
{
    /* error, because complier use es and no initial with ss */
#if 0
    asm push ax
    asm mov ax,ss
    asm mov es,ax
    asm pop ax
#endif
    /* use_ss(tmp_reg); */

    *result = a + low_16bit(b);

    high_16bit(*result) += high_16bit(b);

    /* pop_ss(tmp_reg); */
}

VOID tc_32bit_sub(DWORD *result, DWORD a, DWORD b)
{
    low_16bit(*result) = low_16bit(a) - low_16bit(b);

    if (low_16bit(a) < low_16bit(b)) {
        --high_16bit(a);
    }

    high_16bit(*result) = high_16bit(a) - high_16bit(b);
}

VOID tc_32bit_mul(DWORD *result, DWORD a, DWORD b)
{
    DWORD temp;

    temp = 0;
    for (; b != 0; --b) {
        tc_32bit_add(&temp, a, temp);
    }
    set_32bit_value(result, temp);
}

VOID tc_word_div(WORD *quotient, WORD *remainder, WORD divisor, WORD dividend)
{
    WORD temp;

    temp = 0;
    while (divisor >= dividend) {
        divisor -= dividend;
        temp++;
    }

    *quotient = temp;
    *remainder = divisor;
}

VOID tc_32bit_div(DWORD *quotient, DWORD *remainder, DWORD divisor, DWORD dividend)
{
    DWORD temp = 0;

    while (-1 != tc_32bit_cmp(divisor, dividend)) { /* divisor >= dividend */
        tc_32bit_sub(&divisor, divisor, dividend); /* divisor -= dividend; */
        temp++;
    }

    set_32bit_value(quotient, temp); /* *quotient = temp; */
    set_32bit_value(remainder, divisor); /* *remainder = divisor; */
}

static VOID exchange_dword(DWORD *a, DWORD *b)
{
    high_16bit(*a) = high_16bit(*a) ^ high_16bit(*b);
    high_16bit(*b) = high_16bit(*a) ^ high_16bit(*b);
    high_16bit(*a) = high_16bit(*a) ^ high_16bit(*b);

    low_16bit(*a) = low_16bit(*a) ^ low_16bit(*b);
    low_16bit(*b) = low_16bit(*a) ^ low_16bit(*b);
    low_16bit(*a) = low_16bit(*a) ^ low_16bit(*b);
}

BYTE add_is_overflow(DWORD a, DWORD b)
{
    DWORD c;
    INT ret;

    tc_32bit_sub(&c, DWORD_MAX_C, a);
    ret = tc_32bit_cmp(c, b);
    if (-1 == ret) {
        return TRUE;
    } else {
        return FALSE;
    }
}

BYTE mul_is_overflow(DWORD a, DWORD b)
{
    DWORD quotient;
    DWORD remainder;
    INT ret;

    ret = tc_32bit_cmp(a, b);
    if (-1 == ret) {
        /* exchange */
        exchange_dword(&a, &b);
    }

    tc_32bit_div(&quotient, &remainder, DWORD_MAX_C, a);
    ret = tc_32bit_cmp(quotient, b);
    if (-1 == ret) {
        return TRUE;
    } else if (1 == ret){
        return FALSE;
    } if (0 == ret) {
        if (0 != remainder) {
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        /* abort */
        return FALSE;
    }
}

VOID test_libc(VOID)
{
    DWORD a, b, c, d;
    WORD row = 15;
    WORD col = 0;

    /* add */
    a = 1;
    b = 2;
    tc_32bit_add(&c, a, b);
    print_32bit_hex(c, 0xf, row, col); col += 9;
/* result: 3 */

    a = 0xfffffffc;
    b = 8;
    tc_32bit_add(&c, a, b);
    print_32bit_hex(c, 0xf, row, col); col += 9;
/* result: 4 */

    /* sub */
    a = 1;
    b = 2;
    tc_32bit_sub(&c, a, b);
    print_32bit_hex(c, 0xf, row, col); col += 9;
/* result: ffffffff */

    a = 4;
    b = 2;
    tc_32bit_sub(&c, a, b);
    print_32bit_hex(c, 0xf, row, col); col += 9;
/* result: 2 */

    a = 0x10001;
    b = 0x20002;
    tc_32bit_sub(&c, a, b);
    print_32bit_hex(c, 0xf, row, col); col += 9;
/* result: fffeffff */

    a = 0x20002;
    b = 0x10002;
    tc_32bit_sub(&c, a, b);
    print_32bit_hex(c, 0xf, row, col); col += 9;
/* result: 10000 */

    a = 0x10002;
    b = 0x10002;
    tc_32bit_sub(&c, a, b);
    print_32bit_hex(c, 0xf, row, col); col += 9;
/* result: 0 */

    /* mul */
    a = 0x102;
    b = 0x101;
    tc_32bit_mul(&c, a, b);
    print_32bit_hex(c, 0xf, row, col); col += 9;
/* result: 10302 */

    a = 0x10002;
    b = 0x10001;
    tc_32bit_mul(&c, a, b);
    print_32bit_hex(c, 0xf, row, col); col += 9;
/* result: 30002 */

    a = 0x80000002;
    b = 0x80001;
    tc_32bit_mul(&c, a, b);
    print_32bit_hex(c, 0xf, row, col); col += 9;
/* result: 80100002 */

    /* div */
    a = 0x80000002;
    b = 0x80001;
    tc_32bit_div(&c, &d, a, b);
    print_32bit_hex(c, 0xf, row, col); col += 9;
/* result: fff */

    a = 0x123;
    b = 0x80001;
    tc_32bit_div(&c, &d, a, b);
    print_32bit_hex(c, 0xf, row, col); col += 9;
/* result: 0 */

    /* overflow */
    do {
        BYTE of;

        of = add_is_overflow(0x8fff0010, 0x80000001);
        print_hex(of, 0xf, row, col); col += 3;
/* result: 1 */

        of = add_is_overflow(0x0010, 0x80000001);
        print_hex(of, 0xf, row, col); col += 3;
/* result: 0 */

        of = mul_is_overflow(0x80000002, 0x80001);
        print_hex(of, 0xf, row, col); col += 3;
/* result: 1 */

        of = mul_is_overflow(0x12, 0x80001);
        print_hex(of, 0xf, row, col); col += 3;
/* result: 0 */

        of = mul_is_overflow(0xff, 0x1010101);
        print_hex(of, 0xf, row, col); col += 3;
/* result: 0 */
    } while (0);

    do {
        BYTE *tes;
        init_32p(tes, 0x123456);
        *tes = 0xcd;
    } while (0);

    /* abort */
    abort();
}

