;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; (C)Copyright 2009, gaocheng.
;
; All Rights Reserved.
;
; file name  : grf.inc
; version    : 1.0
; author     : gaocheng
; date       : 2011-04-22
; description: global ram file
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; struct global_ram_file {
;     // 磁盘参数表
;     u8 ns; // 每磁道扇区数
;     u8 nh; // 最大磁头数
;     u16 nc; // 最大磁道号
;
;     u8 bsp_flag; // 未记录0, 已记录1
;     u8 rsvd;
;     u8 ss; // 起始磁道号,起始为1
;     u8 sh; // 起始磁头数,起始为0
;     u16 sc; // 起始磁道号,起始为0
; };

; global ram file seg
GRF_SEG = 9000H

; boot disk para table
GRF_BDPT_S = 00H
GRF_BDPT_H = 01H
GRF_BDPT_C = 02H

; boot start position
GRF_BSP_FLAG = 04H
GRF_BSP_SS = 06H
GRF_BSP_SH = 07H
GRF_BSP_SC = 08H
