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
;     // ���̲�����
;     u8 ns; // ÿ�ŵ�������
;     u8 nh; // ����ͷ��
;     u16 nc; // ���ŵ���
;
;     u8 bsp_flag; // δ��¼0, �Ѽ�¼1
;     u8 rsvd;
;     u8 ss; // ��ʼ�ŵ���,��ʼΪ1
;     u8 sh; // ��ʼ��ͷ��,��ʼΪ0
;     u16 sc; // ��ʼ�ŵ���,��ʼΪ0
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