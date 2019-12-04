/* CRC-CCITT (0xFFFF) */

#include "typedef.h"
#include "lib.h"

BYTE read_hmb(DWORD addr);
WORD crc16(DWORD address, DWORD length)
{
    BYTE x;
    WORD crc = 0xFFFF;

    while (-1 != tc_32bit_cmp(length, 1)) {
        tc_32bit_sub(&length, length, 1);
        x = crc >> 8 ^ read_hmb(address);
        tc_32bit_add(&address, address, 1);
        x ^= x >> 4;
        crc = (crc << 8) ^ ((WORD)(x << 12)) ^ ((WORD)(x << 5)) ^ ((WORD) x);
    }
    return crc;
}

