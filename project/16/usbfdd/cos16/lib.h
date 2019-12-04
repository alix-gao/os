/****************************************************************
 * copyright (c) 2009, gaocheng
 * all rights reserved.
 *
 * file name  : lib.h
 * version    : 1.0
 * description:
 * author     : gaocheng
 * date       : 2009-04-22
 ****************************************************************/

#ifndef __LIB_H__
#define __LIB_H__

/****************************************************************
 include head file
 ****************************************************************/

/****************************************************************
 macro define
 ****************************************************************/
#if 0
/* 32Î»µØÖ· */
#define p32addr(addr) ((addr) * 0x1000)

/* ÉèÖÃÖ¸Õë¶Î¼Ä´æÆ÷, set pointer segment */
#define push_ps(reg_ps, reg_es) \
    do { \
        WORD reg_xs = (reg_ps) >> 4; \
        asm push ax; \
        asm push es; \
        asm mov ax,reg_xs; \
        asm mov es,ax; \
        asm pop reg_es; \
        asm pop ax; \
    } while (0)

#define use_ss(reg_es) \
    do { \
        asm push ax; \
        asm push es; \
        asm mov ax,ss; \
        asm mov es,ax; \
        asm pop reg_es; \
        asm pop ax; \
    } while (0)

#define pop_ss(reg_es) \
    do { \
        asm push reg_es; \
        asm pop es; \
    } while (0)
#endif

#define init_32p(p, addr) \
    do { \
        *(WORD *) &(p) = (addr); /* offset */ \
        *((WORD *) &(p) + 1) = ((addr) & 0xffff0000) >> 4; /* segment */ \
    } while (0)

#define abort() \
    do { \
        asm sti; \
        while (1) { \
            asm hlt; \
        } \
    } while (0)

#define high_16bit(dword_v) (*((WORD *) &(dword_v) + 1))
#define low_16bit(dword_v) (*(WORD *) &(dword_v))

#define high_8bit(word_v) (*((BYTE *) &(word_v) + 1))
#define low_8bit(word_v) (*(BYTE *) &(word_v))

/****************************************************************
 enum define
 ****************************************************************/

/****************************************************************
 struct define
 ****************************************************************/

/****************************************************************
 extern function
 ****************************************************************/
VOID cpy_mem(BYTE *dest, const BYTE *src, WORD count);

INT cmp_str(const BYTE *s1, const BYTE *s2);
INT tc_32bit_cmp(DWORD a, DWORD b);
VOID set_32bit_value(DWORD *dest, DWORD src);

BYTE add_is_overflow(DWORD a, DWORD b);
BYTE mul_is_overflow(DWORD a, DWORD b);

VOID print_char(BYTE c, BYTE color, WORD row, WORD col);
VOID print_string(const BYTE *str, BYTE color, WORD row, WORD col);
VOID print_hex(BYTE value, BYTE color, WORD row, WORD col);
VOID print_32bit_hex(DWORD value, BYTE color, WORD row, WORD col);

VOID tc_32bit_add(DWORD *result, DWORD a, DWORD b);
VOID tc_32bit_sub(DWORD *result, DWORD a, DWORD b);
VOID tc_32bit_mul(DWORD *result, DWORD a, DWORD b);
VOID tc_word_div(WORD *quotient, WORD *remainder, WORD divisor, WORD dividend);
VOID tc_32bit_div(DWORD *quotient, DWORD *remainder, DWORD divisor, DWORD dividend);

#endif

