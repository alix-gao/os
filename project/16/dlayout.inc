;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; /* (C)Copyright 2009, gaocheng.
;
;    All Rights Reserved.
;
;    file name  : dlayout.inc
;    version    : 1.0
;    author     : gaocheng
;    date       : 2011-04-22
;    description: disk layout, chs depends on specified device. */
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

BYTE_PER_SECTOR = 512; /* 200H = 512 */

DBR_HEAD = 1; /* mbr cannot use lba. */
DBR_SECTOR = 1;
DBR_CYLINDER = 0;

ASM16_LBA = 126; /* 7eH = 2*3fH = 126 */
ASM16_LEN = 8192; /* 2000H = 8192. len is 16bit limit */

COS16_LBA = 189; /* 0bdH = 3*3fH = 189 */
COS16_LEN = 64512; /* 0fc00H = 64512 */
