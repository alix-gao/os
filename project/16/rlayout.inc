;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; (C)Copyright 2009, gaocheng.
;
; All Rights Reserved.
;
; file name  : rlayout.inc
; version    : 1.0
; author     : gaocheng
; date       : 2011-04-22
; description: ram layout
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; macro

SYSTEM_STACK_SEG = 1000H
SYSTEM_STACK_OFFSET = 0fffcH ; 1000H-4

MBR_SEG = 0000H
MBR_OFFSET = 7c00H

DBR_SEG = 0000H
DBR_OFFSET = 7c00H

ASM16_SEG = 0000H
ASM16_OFFSET = 2000H

COS16_SEG = 0000H
COS16_OFFSET = 4000H
